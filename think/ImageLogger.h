#ifndef IMAGELOGGER_H
#define IMAGELOGGER_H

#include "queue.h"
#include <opencv2/core.hpp>


class ImageLogger {
public:
    ImageLogger(const std::string& parentPath);

    Queue<cv::Mat,3> imagesToLog;

    void Start();
    void Stop();
    void BackgroundLoop();

    std::string directory;

private:
    pthread_t thread;
};

#endif // IMAGELOGGER_H
