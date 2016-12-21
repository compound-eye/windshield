#ifndef VIEW_H
#define VIEW_H

#include <opencv2/core.hpp>


class View {
public:
    virtual ~View();
    virtual void Draw(const cv::Mat& image, int viewWidth, int viewHeight) const = 0;

    void SwapImageToDraw(cv::Mat& image);

protected:
    View();

private:
    cv::Mat imageToDraw;
    pthread_mutex_t imageMutex;
};

#endif // VIEW_H
