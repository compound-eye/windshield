#include "ImageView.h"
#include "Compute.h"


ImageView::ImageView(int imageWidth, int imageHeight) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}

ImageView::~ImageView() {
    glDeleteTextures(1, &tex);
}

void ImageView::Draw(const OutputData& data, int viewWidth, int viewHeight) const {
    float w = data.image.cols, h = data.image.rows;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glViewport(0, 0, viewWidth, viewHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., w, h, 0., -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, data.image.data);

    glBegin(GL_QUAD_STRIP);
    glTexCoord2f(0., 1.);  glVertex2f(0, h);
    glTexCoord2f(0., 0.);  glVertex2f(0, 0);
    glTexCoord2f(1., 1.);  glVertex2f(w, h);
    glTexCoord2f(1., 0.);  glVertex2f(w, 0);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);

    glColor3f(1., 1., 0.);
    glBegin(GL_LINES);
    for (Lines::const_iterator l = data.lines.begin(); l != data.lines.end(); ++l) {
        const cv::Vec4i& line = *l;
        glVertex2i(line[0], line[1]);
        glVertex2i(line[2], line[3]);
    }
    glEnd();

    if (data.direction == Turn) {
        glColor3f(1., 0., 0.);
        glBegin(GL_LINES);
            glVertex2i(data.loX, data.loY);
            glVertex2i(data.hiX, data.hiY);
        glEnd();
    }
}
