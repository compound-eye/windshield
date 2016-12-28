#include "Capture.h"
#include "Compute.h"
#include "ImageLogger.h"
#include "Timer.h"

#include "Motor.h"
#include "Radio.h"
#include "Navio/Util.h"

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>


static volatile bool quit = false;

static void Quit(int) {
    quit = true;
}

static const int refreshRate = 30;

int main(int /*argc*/, char** /*argv*/) {
    if (! check_apm()) {
        system("sudo modprobe bcm2835-v4l2");

        Capture cap(0);
        ImageLogger log;
        Compute compute(&cap, &log);
        cap.Start();
        log.Start();
        compute.Start();

        Motor motor;
        Radio rc;
        Timer timer;

        signal(SIGHUP, Quit);
        signal(SIGINT, Quit);

        timer.Start();
        while (! quit) {
            OutputData data;
            compute.SwapOutputData(data);

            float leftMotor, rightMotor;
            if (data.direction == GoBack) {
                leftMotor  = 0.;
                rightMotor = 0.;
            } else {
                float throttle = rc.ReadThrottle();
                if (data.direction == GoStraight) {
                    leftMotor  = throttle;
                    rightMotor = throttle;
                } else {
                    float steer = (M_PI_2 - atan2(data.hiY - data.loY, data.hiX - data.loX)) / 10.;
                    leftMotor  = steer > 0. ? (1. - steer) * throttle : throttle;
                    rightMotor = steer < 0. ? (1. + steer) * throttle : throttle;
                }
            }
            motor.SetLeftMotor (leftMotor);
            motor.SetRightMotor(rightMotor);

            struct timespec sleep = {0, 1000 * timer.NextSleep(refreshRate, INT_MAX)};
            nanosleep(&sleep, NULL);
        }

        motor.SetLeftMotor (0.);
        motor.SetRightMotor(0.);

        cap.Stop();
        compute.Stop();
        log.Stop();
    }
    return 0;
}
