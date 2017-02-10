#ifndef CAPTURE_H
#define CAPTURE_H

#include "VideoSource.h"
#include <opencv2/videoio.hpp>


class ImageLogger;

class Capture : public VideoSource {
public:
    Capture(int device);
    Capture(const cv::String& filename, int api);

    void Start();
    void Stop();

    void BackgroundLoop();

private:
    pthread_t thread;
    cv::VideoCapture cap;
    cv::Ptr<ImageLogger> log;
    const int fps;
    bool canDropFrame;

    void StartLog();
};

#endif // CAPTURE_H
