
#pragma once
#ifdef USE_MOTORS
#include <Wire.h>
#include "shared.h"

class MotorManager
{
    // Adafruit_PWMServoDriver pwm;

public:
    MotorManager()

    {
        // pwm = Adafruit_PWMServoDriver();
        initMotors();
    }
    void initMotors();
    void initDriver();
    bool handleMotorCommand(String command);
    bool handleParameterMessage(parameter_message parameter);
    void updateMotors();
    void setMotor(int motorVal);
};
#endif