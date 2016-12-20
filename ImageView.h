#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "View.h"
#include <GL/glew.h>


class ImageView: public View {
public:
    ImageView(int imageWidth, int imageHeight);
    ~ImageView();

    void Draw(const cv::Mat& m, int viewWidth, int viewHeight) const;

private:
    GLuint tex;
};

#endif // IMAGEVIEW_H
