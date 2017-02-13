#include "Motor.h"


static const int chTiltCamera = 0;
static const int chLeftMotor  = 1;
static const int chRightMotor = 2;
static const int chPanCamera  = 3;

static const float outMin     = 1.;
static const float outNeutral = 1.5;
static const float outMax     = 2.;

static void InitPWMChannel(PWM& pwm, int ch) {
    pwm.init(ch);
    pwm.enable(ch);
    pwm.set_period(ch, 50);
}

Motor::Motor() {
    InitPWMChannel(pwm, chLeftMotor);
    InitPWMChannel(pwm, chRightMotor);
    InitPWMChannel(pwm, chTiltCamera);
    InitPWMChannel(pwm, chPanCamera);

    pwm.set_duty_cycle(chLeftMotor,  outNeutral);
    pwm.set_duty_cycle(chRightMotor, outNeutral);
    pwm.set_duty_cycle(chPanCamera,  outNeutral);
    pwm.set_duty_cycle(chTiltCamera, 1.);
    pwm.set_duty_cycle(chPanCamera, 1.47);
}

void Motor::SetLeftMotor(float v) {
    pwm.set_duty_cycle(chLeftMotor, v * (outMax - outNeutral) + outNeutral);
}

void Motor::SetRightMotor(float v) {
    pwm.set_duty_cycle(chRightMotor, v * (outMax - outNeutral) + outNeutral);
}
