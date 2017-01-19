#include "Capture.h"
#include "Compute.h"
#include "ImageView.h"
#include "Timer.h"

#include <dirent.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <opencv2/imgproc.hpp>


static VideoSource* cap = NULL;
static Compute* compute = NULL;
static OutputData latestData;

static int viewWidth, viewHeight;
static ImageView* view = NULL;
static Mouse mouse;
static bool mouseMoved = false;

static const int refreshRate = 30;
static Timer timer;

static GLsync rendering = 0;

static void DrawData(const OutputData& data) {
    view->Draw(data, viewWidth, viewHeight, mouse);

    glDeleteSync(rendering);
    rendering = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glutSwapBuffers();

    mouseMoved = false;
}

static void Display() {
    if (! rendering) {
        OutputData newData;
        compute->SwapOutputData(newData);
        if (! newData.imageAfter.empty()) {
            DrawData(newData);
            latestData = newData;
        }
        else if (mouseMoved && ! cap->playing && ! latestData.imageAfter.empty()) {
            DrawData(latestData);
        }
    }
}

static void Refresh(int what) {
    if (rendering) {
        switch (glClientWaitSync(rendering, 0, 1000000000/refreshRate)) {
        case GL_ALREADY_SIGNALED:
        case GL_CONDITION_SATISFIED:
            glDeleteSync(rendering);
            rendering = 0;
            break;
        }
    }

    if (rendering) {
        glutTimerFunc(0, Refresh, what);
    } else {
        Display();
        glutTimerFunc(timer.NextSleep(refreshRate,INT_MAX)/1000, Refresh, what);
    }
}

static void LookAtPoint(const cv::Mat& image, int x, int y) {
    cv::Mat p = cv::Mat(image, cv::Rect(x, y, 1, 1)).clone();
    std::cerr << "R:" << (int)p.data[2] << " G:" << (int)p.data[1] << " B:" << (int)p.data[0] << std::endl;
    cv::cvtColor(p, p, cv::COLOR_BGR2Lab);
    std::cerr << "L:" << (int)p.data[0] << " a:" << (int)p.data[1] << " b:" << (int)p.data[2] << std::endl;
    std::cerr << std::endl;
}

static void LookAtRect(const cv::Mat& image, int x0, int y0, int x1, int y1) {
    cv::Mat inp(image, cv::Rect(std::min(x0,x1), std::min(y0,y1), std::abs(x0-x1), std::abs(y0-y1)));
    if (! inp.empty()) {
        cv::Mat m, lab[3];
        cv::cvtColor(inp, m, cv::COLOR_BGR2Lab);
        cv::split(m, lab);
        double otsu_L = cv::threshold(lab[0], m, 128, 255, cv::THRESH_OTSU);
        double otsu_a = cv::threshold(lab[1], m, 128, 255, cv::THRESH_OTSU);
        double otsu_b = cv::threshold(lab[2], m, 128, 255, cv::THRESH_OTSU);
        double  tri_L = cv::threshold(lab[0], m, 128, 255, cv::THRESH_TRIANGLE);
        double  tri_a = cv::threshold(lab[1], m, 128, 255, cv::THRESH_TRIANGLE);
        double  tri_b = cv::threshold(lab[2], m, 128, 255, cv::THRESH_TRIANGLE);
        std::cerr << "Otsu threshold L:" << otsu_L << " a:" << otsu_a << " b:" << otsu_b << std::endl;
        std::cerr << "Triangle threshold L:" << tri_L << " a:" << tri_a << " b:" << tri_b << std::endl;
        std::cerr << std::endl;
    }
}

static void MouseDrag(int x, int y) {
    mouse.x1 = x;
    mouse.y1 = y;
    mouseMoved = true;
}

static void MouseClick(int /*button*/, int state, int x, int y) {
    switch (state) {
    case GLUT_DOWN:
        mouse.down = true;
        mouse.x0 = mouse.x1 = x;
        mouse.y0 = mouse.y1 = y;

        if (! latestData.imageBefore.empty()) {
            LookAtPoint(latestData.imageBefore, x, y);
        }
        break;

    case GLUT_UP:
        mouse.down = false;

        if (! latestData.imageBefore.empty() && x != mouse.x0 && y != mouse.y0) {
            LookAtRect(latestData.imageBefore, mouse.x0, mouse.y0, x, y);
        }
        break;
    }

    mouseMoved = true;
}

static void Keyboard(unsigned char key, int /*x*/, int /*y*/) {
    if (key == ' ') {
        cap->commands.Enqueue(cap->playing ? VideoSource::Pause : VideoSource::Play);
    }
}

static void SpecialKey(int key, int /*x*/, int /*y*/) {
    switch (key) {
    case GLUT_KEY_UP:
        cap->commands.Enqueue(VideoSource::Rewind);
        break;
    case GLUT_KEY_LEFT:
        cap->commands.Enqueue(VideoSource::PrevFrame);
        break;
    case GLUT_KEY_RIGHT:
        cap->commands.Enqueue(VideoSource::NextFrame);
        break;
    }
}

static void Cleanup() {
    compute->Stop();
    cap->Stop();

    delete view; view = NULL;
    delete compute; compute = NULL;
    delete cap; cap = NULL;
    glDeleteSync(rendering); rendering = 0;
}

static char* RoverImageFileName() {
    char roverImagesDir[150];
    sprintf(roverImagesDir, "%s/rover-images", getenv("HOME"));

    struct dirent** dirs = NULL;
    const int countDirs = scandir(roverImagesDir, &dirs, NULL, alphasort);
    static char fname[200];
    sprintf(fname, "%s/%s/%%04d.png", roverImagesDir, dirs[countDirs-1]->d_name);

    for (int i = 0; i < countDirs; ++i) {
        free(dirs[i]);
    }
    free(dirs);

    return fname;
}

static void Init(int& argc, char**argv) {
    cap = new Capture(RoverImageFileName(), cv::CAP_IMAGES);
    //cap = new Capture("/home/haoyang/rover-images/2017-01-13-234547/%04d.png", cv::CAP_IMAGES);
    //cap = new Capture("/home/haoyang/rover-images/2016-12-30-165058/078.png", cv::CAP_FFMPEG);
    //cap = new Capture(0);

    viewWidth  = cap->imageWidth;
    viewHeight = cap->imageHeight;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(viewWidth, viewHeight);
    glutCreateWindow("Windshield");
    glewInit();

    view = new ImageView(cap->imageWidth, cap->imageHeight);
    compute = new Compute(cap, NULL, true);

    cap->Start();
    compute->Start();

    glutTimerFunc(0, Refresh, 0);
    glutDisplayFunc(Display);
    glutMouseFunc(MouseClick);
    glutMotionFunc(MouseDrag);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(SpecialKey);
    glutCloseFunc(Cleanup);

    timer.Start();
}

int main(int argc, char**argv) {
    Init(argc, argv);
    glutMainLoop();
    return 0;
}
