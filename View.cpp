#include "View.h"


View::View() {
    pthread_mutex_init(&imageMutex, NULL);
}

View::~View() {
    pthread_mutex_destroy(&imageMutex);
}

void View::SwapImageToDraw(cv::Mat& image) {
    cv::Mat x = image;

    pthread_mutex_lock(&imageMutex);
    image = imageToDraw;
    imageToDraw = x;
    pthread_mutex_unlock(&imageMutex);
}
