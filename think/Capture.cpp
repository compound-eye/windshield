#include "Capture.h"
#include "Timer.h"
#include <iostream>
#include <time.h>


static const int cameraWidth = 320, cameraHeight = 240;
static const float cameraFPS  = -1.;
static const int videoFileFPS = -1;


Capture::Capture(int device)
    : cap(device)
    , fps(-1)
    , canDropFrame(false)
{
#if 0
    imageWidth  = cameraWidth;
    imageHeight = cameraHeight;
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  cameraWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, cameraHeight);
#else
    imageWidth  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    imageHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cerr << "camera: " << imageWidth << " x " << imageHeight << std::endl;
#endif

    if (cameraFPS > 0) {
        cap.set(cv::CAP_PROP_FPS, cameraFPS);
    }
}

Capture::Capture(const char* filename)
    : cap(filename)
    , fps(videoFileFPS)
    , canDropFrame(false)
{
    imageWidth  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    imageHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

void Capture::BackgroundLoop() {
    cv::Mat image;
    int frameCount = 0;
    Timer timer;

    timer.Start();
    while (! imagesCaptured.quitting) {

        if (! cap.read(image)) {break;}

        if (canDropFrame && imagesCaptured.size >= imagesCaptured.Capacity()) {
            std::cerr << "Capture dropped frame." << std::endl;
        } else {
            imagesCaptured.Enqueue(image);

            ++frameCount;
            struct timespec sleep = {0, fps > 0 ? 1000 * timer.NextSleep(fps, frameCount) : 0};
            if (sleep.tv_nsec > 0) {
                nanosleep(&sleep, NULL);
            }
        }
    }
    timer.PrintTimeStats(frameCount);
}

static void* CaptureThread(void* cap) {
    ((Capture*)cap)->BackgroundLoop();
    return 0;
}

void Capture::Start() {
    pthread_create(&thread, NULL, CaptureThread, this);
}

void Capture::Stop() {
    imagesCaptured.Quit();
    pthread_join(thread, NULL);
}
