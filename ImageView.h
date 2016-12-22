#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <GL/glew.h>


class OutputData;

class ImageView {
public:
    ImageView(int imageWidth, int imageHeight);
    ~ImageView();

    void Draw(const OutputData& data, int viewWidth, int viewHeight) const;

private:
    GLuint tex;
};

#endif // IMAGEVIEW_H
