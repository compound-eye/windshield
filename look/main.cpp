#include "Capture.h"
#include "Compute.h"
#include "ImageView.h"
#include "Timer.h"

#include <dirent.h>
#include <GL/glew.h>
#include <GL/freeglut.h>


static VideoSource* cap = NULL;
static Compute* compute = NULL;

static int viewWidth, viewHeight;
static ImageView* view = NULL;

static const int refreshRate = 30;
static Timer timer;

static GLsync rendering = 0;

static void Display() {
    if (! rendering) {
        OutputData data;
        compute->SwapOutputData(data);
        if (! data.imageAfter.empty()) {
            view->Draw(data, viewWidth, viewHeight);

            glDeleteSync(rendering);
            rendering = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            glutSwapBuffers();
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
    sprintf(fname, "%s/%s/%%03d.png", roverImagesDir, dirs[countDirs-1]->d_name);

    for (int i = 0; i < countDirs; ++i) {
        free(dirs[i]);
    }
    free(dirs);

    return fname;
}

static void Init(int& argc, char**argv) {
    cap = new Capture(RoverImageFileName(), cv::CAP_IMAGES);
    //cap = new Capture("/home/haoyang/rover-images/2016-12-30-165058/%03d.png", cv::CAP_IMAGES);
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
    compute = new Compute(cap, NULL);

    cap->Start();
    compute->Start();

    glutTimerFunc(0, Refresh, 0);
    glutDisplayFunc(Display);
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
