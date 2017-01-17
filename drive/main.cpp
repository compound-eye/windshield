#include "Capture.h"
#include "Compute.h"
#include "ImageLogger.h"
#include "Timer.h"

#include "Motor.h"
#include "Radio.h"
#include "Navio/Util.h"

#include <iostream>
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
        float steer = 0;

        signal(SIGHUP, Quit);
        signal(SIGINT, Quit);

        timer.Start();
        while (! quit) {
            OutputData data;
            compute.SwapOutputData(data);

            float throttle = rc.ReadThrottle();
            float leftMotor, rightMotor;
            if (data.direction == GoBack) {
                throttle = std::min(0.75F, throttle);
                if (steer > 0.) {
                    leftMotor  = -throttle;
                    rightMotor =  throttle;
                } else {
                    rightMotor = -throttle;
                    leftMotor  =  throttle;
                }
            } else {
                if (data.direction == GoStraight) {
                    leftMotor  = throttle;
                    rightMotor = throttle;
                } else {
                    steer = rc.ReadSteer();
                    //steer = (20. - 5.*throttle) * (M_PI_2 - atan2(data.hiY - data.loY, data.hiX - data.loX));
                    //std::cerr << "steer = " << steer << std::endl;
                    leftMotor  = steer > 0. ? (1. - std::min( 1.F,steer)) * throttle : throttle;
                    rightMotor = steer < 0. ? (1. + std::max(-1.F,steer)) * throttle : throttle;
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
