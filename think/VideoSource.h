#ifndef VIDEOSOURCE_H
#define VIDEOSOURCE_H

#include "queue.h"
#include <opencv2/core.hpp>


class Timer;

class VideoSource {
public:
    enum Command {Noop, Play, Pause, Rewind, NextFrame, PrevFrame, Snapshot};

    int imageWidth, imageHeight;
    Queue<cv::Mat,1> imagesCaptured;
    Queue<Command,2> commands;
    bool printTimeStats;
    bool playing;

    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual ~VideoSource();

    Command NextCommand(int& frameCount, Timer& timer);
    void PrintTimeStats(int  frameCount, Timer& timer);
};

#endif // VIDEOSOURCE_H
