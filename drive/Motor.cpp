#include "Motor.h"
#include <cmath>
#include <iostream>
extern "C" {
  #include "RPIO/c_gpio/c_gpio.h"
  #include "RPIO/c_pwm/pwm.h"
}

static const int pinRightPWM = 14;
static const int pinRight1   = 10;
static const int pinRight2   = 25;
static const int pinLeftPWM  = 24;
static const int pinLeft1    = 17;
static const int pinLeft2    =  4;

static const int DMA = 0;

Motor::Motor() {
  gpio_setup();

  set_loglevel(LOG_LEVEL_ERRORS);
  pwm_setup(PULSE_WIDTH_INCREMENT_GRANULARITY_US_DEFAULT, DELAY_VIA_PWM);
  init_channel(DMA, SUBCYCLE_TIME_US_DEFAULT);

  setup_gpio(pinRight1, OUTPUT, 0);
  setup_gpio(pinRight2, OUTPUT, 0);
  setup_gpio(pinLeft1,  OUTPUT, 0);
  setup_gpio(pinLeft2,  OUTPUT, 0);
}

Motor::~Motor() {
  shutdown();
  cleanup();
}

static int PulseWidth(float v) {
  return fabs(v) * get_channel_subcycle_time_us(DMA) / get_pulse_incr_us();
}

void Motor::SetLeftMotor(float v) {
  add_channel_pulse(DMA, pinLeftPWM, 0, PulseWidth(0.6*v));
  output_gpio(pinLeft1, v <  0.);
  output_gpio(pinLeft2, v >= 0.);
}

void Motor::SetRightMotor(float v) {
  add_channel_pulse(DMA, pinRightPWM, 0, PulseWidth(0.6*v));
  output_gpio(pinRight1, v <  0.);
  output_gpio(pinRight2, v >= 0.);
}
