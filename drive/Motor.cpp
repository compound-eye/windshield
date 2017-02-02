#include "Motor.h"
#include <pigpio.h>
#include <cmath>
#include <iostream>

static const int pinRightPWM = 14;
static const int pinRight1   = 10;
static const int pinRight2   = 25;
static const int pinLeftPWM  = 24;
static const int pinLeft1    = 17;
static const int pinLeft2    =  4;

Motor::Motor() {
  gpioInitialise();

  gpioSetPWMfrequency(pinRightPWM, 500); gpioSetPWMrange(pinRightPWM, 10000);
  gpioSetPWMfrequency(pinLeftPWM,  500); gpioSetPWMrange(pinLeftPWM,  10000);

  gpioSetMode(pinRight1, PI_OUTPUT);
  gpioSetMode(pinRight2, PI_OUTPUT);
  gpioSetMode(pinLeft1,  PI_OUTPUT);
  gpioSetMode(pinLeft2,  PI_OUTPUT);
}

Motor::~Motor() {
  gpioTerminate();
}

void Motor::SetLeftMotor(float v) {
  gpioPWM(pinLeftPWM, 10000. * fabs(v));
  gpioWrite(pinLeft1, v <  0.);
  gpioWrite(pinLeft2, v >= 0.);
}

void Motor::SetRightMotor(float v) {
  gpioPWM(pinRightPWM, 10000. * fabs(v));
  gpioWrite(pinRight1, v <  0.);
  gpioWrite(pinRight2, v >= 0.);
}
