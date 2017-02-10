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

static const int refreshRate = 40;

int main(int /*argc*/, char** /*argv*/) {
    if (! check_apm()) {
        system("sudo modprobe bcm2835-v4l2");

        // The program runs in four threads. This way, the computer always has something to
        // do while waiting for the next captured frame.
        // . The Capture thread captures images from the camera.
        // . The ImageLogger thread writes every 5th images on disk for later replay.
        // . The Compute thread looks for lines in the image, and computes the steering angle.
        // . Finally, here the main thread checks the latest steering angle periodically,
        //   and adjusts the motor speeds accordingly.

        Capture cap(0);
        ImageLogger log;
        Compute compute(&cap, &log, false);
        cap.Start();
        log.Start();
        compute.Start();

        Motor motor;
        //Radio rc;
        Timer timer;

        signal(SIGHUP, Quit);
        signal(SIGINT, Quit);

        timer.Start();
        while (! quit) {
            OutputData data;
            compute.SwapOutputData(data);

            float throttle   = 0.7; //rc.ReadThrottle();
            float leftMotor  = throttle;
            float rightMotor = throttle;
            float steer      = 0.5 * data.angle; //+ 2.*rc.ReadSteer();
            if (steer < 0.) {
                rightMotor *= 1. + std::max(-1.F, steer);
            } else {
                leftMotor  *= 1. - std::min( 1.F, steer);
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
