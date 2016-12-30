#include "Compute.h"
#include "ImageLogger.h"
#include "VideoSource.h"
#include <iostream>
#include <opencv2/imgproc.hpp>


static int redChannel   = 2;
static int greenChannel = 1;
static int blueChannel  = 0;
static int grayChannels = -1;

enum SegMethod {Hough, LSD, Contours};


template<typename T>
static inline T square(T x) {
    return x * x;
}

template<typename T>
static inline T length2(const cv::Vec<T,4>& v) {
    return square(v(0) - v(2)) + square(v(1) - v(3));
}

static bool longer(const cv::Vec4i& v, const cv::Vec4i& w) {
    return length2(v) > length2(w);
}

void OutputData::UseLineForDirection(const cv::Vec4i& line) {
    if (line(1) < line(3)) {
        loX = line(0);
        loY = line(1);
        hiX = line(2);
        hiY = line(3);
    } else {
        hiX = line(0);
        hiY = line(1);
        loX = line(2);
        loY = line(3);
    }

    const int mid = image.cols / 2;
    direction = loY < image.rows/3 && (mid < loX && loX < hiX || mid > loX && loX > hiX)
              ? Turn
              : GoStraight;
}

typedef std::vector<cv::Point> Polygon;
typedef std::vector<Polygon> ContoursT;

void Compute::BackgroundLoop() {
    const SegMethod seg = Contours;

    const int    HoughThreshold = cap->imageWidth * cap->imageHeight / 6500;
    const double HoughMinLineLength = cap->imageHeight / 4.;
    const double HoughMaxGap = cap->imageHeight / 50.;
    const double HoughRho = 2.;

#if 1
    const double CannyThreshold1 =  50.;
    const double CannyThreshold2 = 100.;
    const double HoughTheta = 0.05;
    const int colorChannels = redChannel;
#else
    const double CannyThreshold1 = 100.;
    const double CannyThreshold2 = 200.;
    const double HoughTheta = 0.02;
    const int colorChannels = grayChannels;
#endif

    cv::Ptr<cv::LineSegmentDetector> lsd;
    if (seg == LSD) {
        lsd = cv::createLineSegmentDetector();
    }

    cv::Mat gray8(cap->imageHeight, cap->imageWidth, CV_8UC1), bw;
    cv::Vec4f line;
    static const int grayTo3Ch[3*2] = {0,0, 0,1, 0,2};

    for (int i = 0; ! cap->imagesCaptured.quitting; ++i) {
        cv::Mat inp(cap->imagesCaptured.Dequeue());
        if (! inp.empty()) {

            if (log && i % 10 == 0 && log->imagesToLog.size < log->imagesToLog.Capacity()) {
                log->imagesToLog.Enqueue(inp.clone());
            }

            OutputData out(cap->imageWidth, cap->imageHeight);

            if (seg == Contours) {
                cv::cvtColor(inp, inp, cv::COLOR_BGR2Lab);
                cv::inRange(inp, cv::Scalar(60, 100, 70), cv::Scalar(220, 130, 120), gray8);

                ContoursT contours;
                cv::findContours(gray8, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                for (ContoursT::iterator p = contours.begin(); p != contours.end(); ++p) {
                    Polygon& poly = *p;
                    cv::Rect r = cv::boundingRect(poly);
                    if (r.y < cap->imageHeight/3) {
                        //cv::approxPolyDP(poly, poly, 5., true);
                        cv::fitLine(poly, line, cv::DIST_L2, 0., HoughRho, HoughTheta);
                        cv::Point p1(-9999*line(0) + line(2), -9999*line(1) + line(3)),
                                  p2( 9999*line(0) + line(2),  9999*line(1) + line(3));
                        cv::clipLine(r, p1, p2);
                        out.lines.push_back(cv::Vec4i(p1.x, p1.y, p2.x, p2.y));
                    }
                }
#if 1
                cv::mixChannels(&gray8, 1, &out.image, 1, grayTo3Ch, 3);
#else
                static const int to3Ch[3*2] = {0,0, 0,1, 0,2};
                cv::mixChannels(&inp, 1, &out.image, 1, to3Ch, 3);
                cv::drawContours(out.image, contours, -1, cv::Scalar(0,255,0));
#endif
            }
            else {
                if (colorChannels == grayChannels) {
                    cv::cvtColor(inp, gray8, cv::COLOR_BGR2GRAY);
                } else {
                    const int colorFromTo[1*2] = {colorChannels,0};
                    cv::mixChannels(&inp, 1, &gray8, 1, colorFromTo, 1);
                }

                if (seg == LSD) {
                    lsd->detect(gray8, out.lines);
                } else if (seg == Hough) {
                    cv::Canny(gray8, bw, CannyThreshold1, CannyThreshold2);
                    cv::HoughLinesP(bw, out.lines, HoughRho, HoughTheta, HoughThreshold, HoughMinLineLength, HoughMaxGap);
                }

                cv::mixChannels(&gray8, 1, &out.image, 1, grayTo3Ch, 3);
            }

            std::sort(out.lines.begin(), out.lines.end(), longer);
            out.direction = GoBack;
            for (Lines::iterator l = out.lines.begin();
                 out.direction != Turn && l != out.lines.end() && length2(*l) > square(cap->imageHeight/3);
                 ++l) {
                out.UseLineForDirection(*l);
            }

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

Compute::Compute(VideoSource* c, ImageLogger* lg): cap(c), log(lg) {
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
