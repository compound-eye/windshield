#ifndef COMPUTE_H
#define COMPUTE_H

#include <pthread.h>

class VideoSource;
class View;


class Compute {
public:
    Compute(VideoSource* c, View* v): cap(c), view(v) {}

    void Start();
    void Stop();

    void BackgroundLoop();

private:
    VideoSource* cap;
    View* view;
    pthread_t thread;
};

#endif // COMPUTE_H
