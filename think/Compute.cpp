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
    const int minLineLength = cap->imageHeight / 4;
    const double maxGap = 25;
    const double rho = 2., theta = 0.02;

    const float midLeft  = 0.5 * cap->imageWidth;
    const float midRight = 0.4 * cap->imageWidth;

    cv::Mat2f line(1, 2); cv::Vec2f* pLine = line[0];
    cv::Mat bw(cap->imageHeight, cap->imageWidth, CV_8UC1);
    static const int toGray[3*2] = {0,0, 0,1, 0,2};

    if (log && ! parms.perspective.empty()) {
        parms.Write(log->directory + "/data.yml");
    }

    static double K[3*3] = {
        2*122.2460543979631, 0, 2*79.82599847454172,
        0, 2*122.7093282327254, 2*62.79411441679434,
        0, 0, 1,
    };
    static double D[5] = {-0.4561153976709965, 0.2832957850680789, 0.0008219042752253125, 0.0006463450928625212, 0};
    cv::Mat1d camera(3, 3, K), distortion(1, 5, D);

    cv::Mat map1, map2;
    cv::initUndistortRectifyMap(camera, distortion, cv::Mat(), camera,
                                cv::Size(cap->imageWidth, cap->imageHeight), CV_16SC2,
                                map1, map2);

    for (int i = 0; ! cap->imagesCaptured.quitting; ++i) {
        OutputData out;
        out.imageBefore = cap->imagesCaptured.Dequeue();
        if (! out.imageBefore.empty()) {

            // Log every 5th images.
            if (log && i % 5 == 0 && log->imagesToLog.size < log->imagesToLog.Capacity()) {
                log->imagesToLog.Enqueue(out.imageBefore);
            }

#if 1
            const double CannyThreshold1 = 120.;
            const double CannyThreshold2 = 150.;

            // Convert to grayscale.
            cv::cvtColor(out.imageBefore, bw, cv::COLOR_BGR2GRAY);
#elif 1
            const double CannyThreshold1 = 10.;
            const double CannyThreshold2 = 23.;

            // Convert to Lab color space and use the a channel for red-green differentiation.
            cv::cvtColor(out.imageBefore, out.imageAfter, cv::COLOR_BGR2Lab);
            static const int pick1[2] = {1,0};
            cv::mixChannels(&out.imageAfter, 1, &bw, 1, pick1, 1);
            if (outputPic) {
                cv::mixChannels(&bw, 1, &out.imageAfter, 1, toGray, 3);
            }
#else
            const double CannyThreshold1 = 10.;
            const double CannyThreshold2 = 15.;

            // Convert to Lab color space and use the b channel for blue-yellow differentiation.
            cv::cvtColor(out.imageBefore, out.imageAfter, cv::COLOR_BGR2Lab);
            static const int pick1[2] = {2,0};
            cv::mixChannels(&out.imageAfter, 1, &bw, 1, pick1, 1);
            if (outputPic) {
                cv::mixChannels(&bw, 1, &out.imageAfter, 1, toGray, 3);
            }
#endif
            cv::remap(bw, bw, map1, map2, cv::INTER_LINEAR);
            cv::rotate(bw, bw, cv::ROTATE_180);
            // Smooth it out.
            cv::blur(bw, bw, cv::Size(5,5));
            // Detect edges.
            cv::Canny(bw, bw, CannyThreshold1, CannyThreshold2);
            // Find line segments.
            cv::HoughLinesP(bw.rowRange(0, 0.6*cap->imageHeight),
                            out.lines, rho, theta, HoughThreshold, minLineLength, maxGap);

            std::vector<LineInfo> lines;
            lines.reserve(out.lines.size());
            for (Lines::iterator pl = out.lines.begin(); pl != out.lines.end(); ++pl) {
                cv::Vec4i& l = *pl;
                pLine[0] = cv::Vec2f(l[0], l[1]);
                pLine[1] = cv::Vec2f(l[2], l[3]);

                // See the line from bird's-eye view.
                if (! parms.perspective.empty()) {
                    cv::perspectiveTransform(line, line, parms.perspective);
                }

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
#elif 1
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

Compute::Compute(VideoSource* c, ImageLogger* lg, const cv::Mat& perspective, bool outPic)
    : cap(c)
    , log(lg)
    , outputPic(outPic)
    , countFramesWithoutLines(0)
{
    parms.perspective = perspective;
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
