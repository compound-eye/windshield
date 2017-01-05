#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <GL/glew.h>


struct Mouse {
    int x0, y0, x1, y1;
    bool down;
};

class OutputData;

class ImageView {
public:
    ImageView(int imageWidth, int imageHeight);
    ~ImageView();

    void Draw(const OutputData& data, int viewWidth, int viewHeight, const Mouse& mouse) const;

private:
    GLuint tex;
};

#endif // IMAGEVIEW_H
