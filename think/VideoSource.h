#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include "queue.h"
#include <opencv2/core.hpp>


class VideoSource {
public:
    int imageWidth, imageHeight;

    Queue<cv::Mat,1> imagesCaptured;

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual ~VideoSource() {}
};

#endif // VIDEOSOURCE_H
