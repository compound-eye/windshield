#ifndef CAPTURE_H
#define CAPTURE_H

#include "VideoSource.h"
#include <opencv2/videoio.hpp>


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
    const int fps;
    bool canDropFrame;
};

#endif // CAPTURE_H
