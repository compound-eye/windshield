#include "VideoSource.h"
#include "Timer.h"


VideoSource::~VideoSource() {}

VideoSource::Command VideoSource::NextCommand(int& frameCount, Timer& timer) {
    if (playing && commands.size <= 0) {
        return Noop;
    } else {
        Command cmd = commands.Dequeue();
        switch (cmd) {

        case Play:
        case Rewind:
            playing = true;
            frameCount = 0;
            timer.Start();
            break;

        case Pause:
        case NextFrame:
        case PrevFrame:
            if (playing) {
                playing = false;
                PrintTimeStats(frameCount, timer);
            }
            break;
        }

        return cmd;
    }
}

void VideoSource::PrintTimeStats(int frameCount, Timer& timer) {
    if (printTimeStats) {
        timer.PrintTimeStats(frameCount);
    }
}
