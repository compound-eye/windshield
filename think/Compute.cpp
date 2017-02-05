#include "Compute.h"
#include "ImageLogger.h"
#include "VideoSource.h"
#include <iostream>
#include <opencv2/imgproc.hpp>


template<typename T>
static inline T square(T x) {
    return x * x;
}

struct LineInfo {
    float angle;
    float length;
};

static bool byAngle(const LineInfo& v, const LineInfo& w) {
    return v.angle < w.angle;
}

void Compute::BackgroundLoop() {
    const int HoughThreshold = cap->imageWidth * cap->imageHeight / 6500;
    const int minLineLength = cap->imageHeight / 3;
    const double maxGap = 25;
    const double rho = 2., theta = 0.02;

    // the matrix produced by birdeye program
    static double Hdata[3*3] = {
         0.6675041881879467,    -0.6301258503975012,  49.80068082287637,
        -0.006612602777747778,   0.7948226618810976,  -7.541413704343782,
        -8.961567957315749e-05, -0.003982602759291037, 1,
    };
    const cv::Mat_<double> H(3, 3, Hdata);

    const float midLeft  = 0.5 * cap->imageWidth;
    const float midRight = 0.4 * cap->imageWidth;

    cv::Mat2f line(1, 2); cv::Vec2f* pLine = line[0];
    cv::Mat bw(cap->imageHeight, cap->imageWidth, CV_8UC1);
    static const int toGray[3*2] = {0,0, 0,1, 0,2};

    for (int i = 0; ! cap->imagesCaptured.quitting; ++i) {
        OutputData out;
        out.imageBefore = cap->imagesCaptured.Dequeue();
        if (! out.imageBefore.empty()) {

            // Log every 5th images.
            if (log && i % 5 == 0 && log->imagesToLog.size < log->imagesToLog.Capacity()) {
                log->imagesToLog.Enqueue(out.imageBefore);
            }

#if 0
            const double CannyThreshold1 = 100.;
            const double CannyThreshold2 = 130.;

            // Convert to grayscale.
            cv::cvtColor(out.imageBefore, bw, cv::COLOR_BGR2GRAY);
#else
            const double CannyThreshold1 = 10.;
            const double CannyThreshold2 = 40.;

            // Convert to Lab color space and use the a channel for red-green differentiation.
            cv::cvtColor(out.imageBefore, out.imageAfter, cv::COLOR_BGR2Lab);
            static const int pick1[2] = {1,0};
            cv::mixChannels(&out.imageAfter, 1, &bw, 1, pick1, 1);
            if (outputPic) {
                cv::mixChannels(&bw, 1, &out.imageAfter, 1, toGray, 3);
            }
#endif
            // Smooth it out.
            cv::blur(bw, bw, cv::Size(5,5));
            // Detect edges.
            cv::Canny(bw, bw, CannyThreshold1, CannyThreshold2);
            // Find line segments.
            cv::HoughLinesP(bw, out.lines, rho, theta, HoughThreshold, minLineLength, maxGap);

            std::vector<LineInfo> lines;
            lines.reserve(out.lines.size());
            for (Lines::iterator pl = out.lines.begin(); pl != out.lines.end(); ++pl) {
                cv::Vec4i& l = *pl;
                pLine[0] = cv::Vec2d(l[0], l[1]);
                pLine[1] = cv::Vec2d(l[2], l[3]);

                // See the line from bird's-eye view.
                cv::perspectiveTransform(line, line, H);

                // Make sure pLine[0].y <= pLine[1].y
                // Smaller y => the point is closer to the bottom of the image.
                if (pLine[0][1] > pLine[1][1]) {
                    cv::Vec2f v = pLine[0];
                    pLine[0] = pLine[1];
                    pLine[1] = v;
                }

                LineInfo info;
                info.length = sqrt(square(pLine[0][0] - pLine[1][0]) + square(pLine[0][1] - pLine[1][1]));
                // Estimate the steering angle needed to avoid this line.
                // If the line occupies only one side of the image and points outward,
                // set the steering angle to 0, to ignore the sideline and go staight ahead.
                info.angle = midLeft  <= pLine[0][0] && pLine[0][0] <= pLine[1][0]
                          || midRight >= pLine[0][0] && pLine[0][0] >= pLine[1][0]
                          ? 0. : M_PI_2 - atan2(pLine[1][1], pLine[1][0] - pLine[0][0]);
                lines.push_back(info);
            }
            if (lines.empty()) {
                ++countFramesWithoutLines;
                out.angle = 0.;
            }
            else {
                countFramesWithoutLines = 0;

                // Sort the lines by angle, then group them in clusters.
                std::sort(lines.begin(), lines.end(), byAngle);

                std::vector<LineInfo> clusters;
                std::vector<LineInfo>::const_iterator l = lines.begin();
                LineInfo clusterInfo;
                clusterInfo.angle  = (*l).length * (*l).angle;
                clusterInfo.length = (*l).length;
                while (++l != lines.end()) {
                    if ((*l).angle - clusterInfo.angle/clusterInfo.length > 0.25) {
                        clusters.push_back(clusterInfo);
                        clusterInfo.angle  = 0.;
                        clusterInfo.length = 0.;
                    }
                    clusterInfo.angle  += (*l).length * (*l).angle;
                    clusterInfo.length += (*l).length;
                }

                // Pick the steering angle from the cluster with most lines (by length).
                for (std::vector<LineInfo>::const_iterator l = clusters.begin(); l != clusters.end(); ++l) {
                    if (clusterInfo.length < (*l).length) {
                        clusterInfo = *l;
                    }
                }
                out.angle = clusterInfo.angle/clusterInfo.length;
            }

            if (outputPic) {
                // Uncomment one of the sections of the code to look at the images at various stages.
#if 0
                // Look at the image as captured.
                out.imageAfter = out.imageBefore;
#elif 0
                // Look at the image from bird's-eye view.
                cv::warpPerspective(out.imageBefore, out.imageAfter, H, out.imageBefore.size());
#elif 0
                // Look at the edges in the image.
                out.imageAfter.create(bw.rows, bw.cols, CV_8UC3);
                cv::mixChannels(&bw, 1, &out.imageAfter, 1, toGray, 3);
#endif
            }

            out.countFramesWithoutLines = countFramesWithoutLines;
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

Compute::Compute(VideoSource* c, ImageLogger* lg, bool outPic)
    : cap(c)
    , log(lg)
    , outputPic(outPic)
    , countFramesWithoutLines(0)
{
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
