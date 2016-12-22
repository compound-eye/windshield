#ifndef VIEW_H
#define VIEW_H

#include <opencv2/core.hpp>


class View {
public:
    typedef std::vector<cv::Vec4i> Lines;
    struct Data {
        cv::Mat image;
        Lines lines;
        Data() {}
        Data(int w, int h): image(h, w, CV_8UC3) {}
    };
    void SwapDataToDraw(Data& data);

    virtual ~View();
    virtual void Draw(const Data& data, int viewWidth, int viewHeight) const = 0;

protected:
    View();

private:
    Data dataToDraw;
    pthread_mutex_t dataMutex;
};

#endif // VIEW_H
