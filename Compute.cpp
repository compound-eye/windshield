#include "Compute.h"
#include "VideoSource.h"
#include "View.h"
#include <iostream>
#include <opencv2/imgproc.hpp>


#define countof(v) (sizeof v / sizeof v[0])

static int redChannel   = 2;
static int greenChannel = 1;
static int blueChannel  = 0;

void Compute::BackgroundLoop() {
    const int    HoughThreshold = cap->imageWidth * cap->imageHeight / 6500;
    const double HoughMinLineLength = cap->imageHeight / 4.;
    const double HoughMaxGap = cap->imageHeight / 50.;

    cv::Mat gray(cap->imageHeight, cap->imageWidth, CV_8UC1);

    while (! cap->imagesCaptured.quitting) {
        cv::Mat inp(cap->imagesCaptured.Dequeue());
        if (! inp.empty()) {
#if 1
            cv::cvtColor(inp, gray, cv::COLOR_BGR2GRAY);
#else
            static const int colorFromTo[] = {redChannel,0};
            cv::mixChannels(&inp, 1, &gray, 1, colorFromTo, countof(colorFromTo)/2);
#endif
            cv::Canny(gray, gray, 100., 200.);

            View::Data out(cap->imageWidth, cap->imageHeight);
            cv::HoughLinesP(gray, out.lines, 2., 0.02, HoughThreshold, HoughMinLineLength, HoughMaxGap);

            static const int grayTo3Ch[] = {0,0, 0,1, 0,2};
            cv::mixChannels(&gray, 1, &out.image, 1, grayTo3Ch, countof(grayTo3Ch)/2);
            view->SwapDataToDraw(out);
        }
    }
}

static void* ComputeThread(void* compute) {
    ((Compute*)compute)->BackgroundLoop();
    return 0;
}

void Compute::Start() {
    pthread_create(&thread, NULL, ComputeThread, this);
}

void Compute::Stop() {
    cap->imagesCaptured.Quit();
    pthread_join(thread, NULL);
}
