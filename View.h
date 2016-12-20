#ifndef VIEW_H
#define VIEW_H

#include "queue.h"
#include <opencv2/core.hpp>


class View {
public:
    Queue<cv::Mat,11> imagesToDraw;

    virtual ~View() {}
    virtual void Draw(const cv::Mat& m, int viewWidth, int viewHeight) const = 0;
};

#endif // VIEW_H
