#ifndef IMAGELOGGER_H
#define IMAGELOGGER_H

#include "queue.h"
#include <opencv2/core.hpp>


class ImageLogger {
public:
    Queue<cv::Mat,3> imagesToLog;

    void Start();
    void Stop();
    void BackgroundLoop();

private:
    pthread_t thread;
};

#endif // IMAGELOGGER_H
