#ifndef MOTOR_H
#define MOTOR_H

#include "Navio/PWM.h"


class Motor {
public:
    Motor();

    void SetLeftMotor (float v);
    void SetRightMotor(float v);

private:
    PWM pwm;
};

#endif // MOTOR_H
