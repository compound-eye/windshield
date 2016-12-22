#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>


class Timer {
public:
    void Start();
    int NextSleep(int freq, int frameCount);

    void PrintTimeStats(int frameCount);

private:
    struct timeval startTime;
};

#endif // TIMER_H
