#include "Compute.h"
#include "VideoSource.h"
#include <iostream>
#include <opencv2/imgproc.hpp>

#define countof(v) (sizeof v / sizeof v[0])


static int redChannel   = 2;
static int greenChannel = 1;
static int blueChannel  = 0;
static int grayChannels = -1;
static int blueHue      = -2;

void Compute::BackgroundLoop() {
    const int    HoughThreshold = cap->imageWidth * cap->imageHeight / 6500;
    const double HoughMinLineLength = cap->imageHeight / 4.;
    const double HoughMaxGap = cap->imageHeight / 50.;
    const double HoughRho = 2.;

#if 0
    const double CannyThreshold1 =  50.;
    const double CannyThreshold2 = 100.;
    const double HoughTheta = 0.05;
    const int colorChannels = blueHue;
#else
    const double CannyThreshold1 = 100.;
    const double CannyThreshold2 = 200.;
    const double HoughTheta = 0.02;
    const int colorChannels = grayChannels;
#endif

    cv::Mat gray(cap->imageHeight, cap->imageWidth, CV_8UC1), whiteLines;

    while (! cap->imagesCaptured.quitting) {
        cv::Mat inp(cap->imagesCaptured.Dequeue());
        if (! inp.empty()) {

            if (colorChannels == grayChannels) {
                cv::cvtColor(inp, gray, cv::COLOR_BGR2GRAY);
            } else if (colorChannels == blueHue) {
                cv::cvtColor(inp, inp, cv::COLOR_BGR2HSV_FULL);
                cv::inRange(inp, cv::Scalar(124, 100, 80), cv::Scalar(174, 255, 255), gray);
                cv::GaussianBlur(gray, gray, cv::Size(5.,5.), 0.);
            } else {
                const int colorFromTo[] = {colorChannels,0};
                cv::mixChannels(&inp, 1, &gray, 1, colorFromTo, countof(colorFromTo)/2);
            }

            cv::Canny(gray, whiteLines, CannyThreshold1, CannyThreshold2);

            OutputData out(cap->imageWidth, cap->imageHeight);
            cv::HoughLinesP(whiteLines, out.lines, HoughRho, HoughTheta, HoughThreshold, HoughMinLineLength, HoughMaxGap);

            static const int grayTo3Ch[] = {0,0, 0,1, 0,2};
            cv::mixChannels(&whiteLines, 1, &out.image, 1, grayTo3Ch, countof(grayTo3Ch)/2);
            SwapOutputData(out);
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

Compute::Compute(VideoSource* c): cap(c) {
    pthread_mutex_init(&dataMutex, NULL);
}

Compute::~Compute() {
    pthread_mutex_destroy(&dataMutex);
}

void Compute::SwapOutputData(OutputData& data) {
    OutputData x = data;

    pthread_mutex_lock(&dataMutex);
    data = outputData;
    outputData = x;
    pthread_mutex_unlock(&dataMutex);
}
