#ifndef COMPUTE_H
#define COMPUTE_H

#include "RoverParms.h"

class ImageLogger;
class VideoSource;


typedef std::vector<cv::Vec4i> Lines;

enum Direction {GoStraight, Turn, GoBack};

struct OutputData {
    cv::Mat imageBefore, imageAfter;
    Lines lines;
    float angle;
    int countFramesWithoutLines;
};

class Compute {
public:
    Compute(VideoSource* cap, ImageLogger* log, const cv::Mat& perspective, bool outPic);
    ~Compute();

    void Start();
    void Stop();
    void BackgroundLoop();

    void SwapOutputData(OutputData& data);

private:
    VideoSource* cap;
    ImageLogger* log;
    RoverParms parms;
    bool outputPic;
    int countFramesWithoutLines;
    OutputData outputData;
    pthread_mutex_t dataMutex;
    pthread_t thread;
};

#endif // COMPUTE_H
