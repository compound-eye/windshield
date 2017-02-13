#include "Radio.h"
#include <algorithm>


static const int chThrottle = 0,  minThrottle = 1120,  maxThrottle = 1880;
static const int chRoll     = 1,  rightRoll   = 1110,  leftRoll    = 1890;
static const int chPitch    = 2,  upPitch     = 1110,  downPitch   = 1890;
static const int chYaw      = 3,  rightYaw    = 1110,  leftYaw     = 1890;

static const int inpCenter = 1500;
static const int inpCenterR = 5;

Radio::Radio() {
    rc.init();
}

float Radio::ReadThrottle() {
    return std::min(std::max(0.F, float(rc.read(chThrottle) - minThrottle) / (maxThrottle - minThrottle)), 1.F);
}

float Radio::ReadSteer() {
    int steer = rc.read(chRoll);
    if (steer > inpCenter + inpCenterR) {
        return std::min(1.F, float(steer - (inpCenter + inpCenterR)) / (leftRoll - (inpCenter + inpCenterR)));
    } else if (steer < inpCenter - inpCenterR) {
        return std::max(-1.F, float(steer - (inpCenter - inpCenterR)) / (inpCenter - inpCenterR - rightRoll));
    } else {
        return 0.;
    }
}
