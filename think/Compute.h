#ifndef COMPUTE_H
#define COMPUTE_H

#include <opencv2/core.hpp>

class ImageLogger;
class VideoSource;


typedef std::vector<cv::Vec4i> Lines;

enum Direction {GoStraight, Turn, GoBack};

struct OutputData {
    cv::Mat imageBefore, imageAfter;
    Lines lines;
    Direction direction;
    int loX, loY, hiX, hiY;

    void UseLineForDirection(const cv::Vec4i& line);
};

class Compute {
public:
    Compute(VideoSource* c, ImageLogger* lg);
    ~Compute();

    void Start();
    void Stop();
    void BackgroundLoop();

    void SwapOutputData(OutputData& data);

private:
    VideoSource* cap;
    ImageLogger* log;
    OutputData outputData;
    pthread_mutex_t dataMutex;
    pthread_t thread;
};

#endif // COMPUTE_H
