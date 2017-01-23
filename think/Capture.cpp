#include "Capture.h"
#include "Timer.h"
#include <iostream>
#include <time.h>


static const int cameraWidth = 320, cameraHeight = 240;
static const float cameraFPS  = -1.;
static const int videoFileFPS = 15;


Capture::Capture(int device)
    : cap(device)
    , fps(-1)
    , canDropFrame(false)
{
    printTimeStats = true;
    playing = true;

#if 1
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

Capture::Capture(const cv::String& filename, int api)
    : cap(filename, api)
    , fps(videoFileFPS)
    , canDropFrame(false)
{
    printTimeStats = false;
    playing = true;
    imageWidth  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    imageHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
}

void Capture::BackgroundLoop() {
    cv::Mat image;
    int frameCount = 0;
    Timer timer;

    timer.Start();
    while (! commands.quitting) {

        bool playThisFrame = true;
        switch (NextCommand(frameCount, timer)) {
        case Rewind:
            cap.set(cv::CAP_PROP_POS_FRAMES, 0.);
            break;
        case NextFrame:
            break;
        case PrevFrame: {
            double prev = cap.get(cv::CAP_PROP_POS_FRAMES) - 2.;
            if (prev >= 0.) {
                cap.set(cv::CAP_PROP_POS_FRAMES, prev);
            }
          } break;
        default:
            playThisFrame = playing;
            break;
        }

        if (playThisFrame) {
            if (cap.read(image)) {
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
            } else if (playing) {
                playing = false;
                PrintTimeStats(frameCount, timer);
            }
        }
    }
    if (playing) {
        PrintTimeStats(frameCount, timer);
    }
}

static void* CaptureThread(void* cap) {
    ((Capture*)cap)->BackgroundLoop();
    return 0;
}

void Capture::Start() {
    pthread_create(&thread, NULL, CaptureThread, this);
}

void Capture::Stop() {
    commands.Quit();
    pthread_join(thread, NULL);
}
