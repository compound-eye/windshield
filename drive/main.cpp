#include "Timer.h"
#include "Navio/PWM.h"
#include "Navio/RCInput.h"
#include "Navio/RGBled.h"
#include "Navio/Util.h"
#include <algorithm>
#include <limits.h>
#include <stdlib.h>
#include <time.h>


static const int inpThrottle = 0,  minThrottle = 1120,  maxThrottle = 1880;
static const int inpRoll     = 1,  rightRoll   = 1110,  leftRoll    = 1890;
static const int inpPitch    = 2,  upPitch     = 1110,  downPitch   = 1890;
static const int inpYaw      = 3,  rightYaw    = 1110,  leftYaw     = 1890;

static const int inpCenter  = 1500;
static const int inpCenterR = 5;

static const int outTiltCamera = 0;
static const int outLeftMotor  = 1;
static const int outRightMotor = 2;
static const int outPanCamera  = 3;

static const float outMin     = 1.;
static const float outNeutral = 1.5;
static const float outMax     = 2.;


static const int refreshRate = 30;


static void InitPWMChannel(PWM& pwm, int ch) {
    pwm.init(ch);
    pwm.enable(ch);
    pwm.set_period(ch, 50);
}

int main(int /*argc*/, char** /*argv*/) {
    if (! check_apm()) {
        system("sudo modprobe bcm2835-v4l2");

        PWM pwm;
        RCInput rc;
        Timer timer;

        InitPWMChannel(pwm, outLeftMotor);
        InitPWMChannel(pwm, outRightMotor);
        InitPWMChannel(pwm, outTiltCamera);
        InitPWMChannel(pwm, outPanCamera);
        rc.init();

        pwm.set_duty_cycle(outPanCamera,  outNeutral);
        pwm.set_duty_cycle(outTiltCamera, outNeutral);

        timer.Start();
        for (;;) {

            int steer = rc.read(inpRoll);
            float rightSteerScale = std::min(std::max(-1., 2. * (steer - rightRoll) / (inpCenter - inpCenterR - rightRoll) - 1.), 1.);
            float leftSteerScale  = std::min(std::max(-1., 2. * (steer - leftRoll)  / (inpCenter + inpCenterR - leftRoll ) - 1.), 1.);
            float throttleScale = float(rc.read(inpThrottle) - minThrottle) / (maxThrottle - minThrottle);

            pwm.set_duty_cycle(outLeftMotor,   leftSteerScale * throttleScale * (outMax - outNeutral) + outNeutral);
            pwm.set_duty_cycle(outRightMotor, rightSteerScale * throttleScale * (outMax - outNeutral) + outNeutral);

            struct timespec sleep = {0, 1000 * timer.NextSleep(refreshRate, INT_MAX)};
            if (sleep.tv_nsec > 0) {
                nanosleep(&sleep, NULL);
            }
        }
    }
    return 0;
}
