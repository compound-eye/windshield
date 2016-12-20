#include "Timer.h"
#include <iostream>


void Timer::Start() {
    gettimeofday(&startTime, 0);
}

int Timer::NextSleep(int freq, int frameCount) {
    int period = 1000000 / freq;
    struct timeval now;
    if (gettimeofday(&now, 0) == 0) {
        int elapsed = 1000000 * (now.tv_sec - startTime.tv_sec) + now.tv_usec - startTime.tv_usec;
        int cur = elapsed / period;
        int sleep = cur < frameCount ? (cur + 1) * period - elapsed : 0;
        return sleep;
    } else {
        return period;
    }
}

void Timer::PrintTimeStats(int frameCount) {
    struct timeval now;
    if (gettimeofday(&now, 0) == 0) {
        double elapsed = now.tv_sec - startTime.tv_sec + (now.tv_usec - startTime.tv_usec)/1e6;
        std::cerr << frameCount << " frames in " << elapsed << " seconds; fps = " << frameCount / elapsed << std::endl;
    }
}
