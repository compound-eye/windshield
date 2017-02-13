#ifndef MOTOR_H
#define MOTOR_H


class Motor {
public:
    Motor();
    ~Motor();

    void SetLeftMotor (float v);
    void SetRightMotor(float v);
};

#endif // MOTOR_H
