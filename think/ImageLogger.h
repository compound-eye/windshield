#ifndef IMAGELOGGER_H
#define IMAGELOGGER_H

#include "queue.h"
#include <opencv2/core.hpp>


class ImageLogger {
public:
    ImageLogger(const std::string& path): logDir(path) {}

    Queue<cv::Mat,3> imagesToLog;

    void Start();
    void Stop();
    void BackgroundLoop();

private:
    std::string logDir;
    pthread_t thread;
};

#endif // IMAGELOGGER_H
