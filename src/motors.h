
#pragma once
#include <Wire.h>
// #include <Adafruit_PWMServoDriver.h>

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
    bool respondToParameterMessage(parameter_message parameter);
    void updateMotors();
    void setMotor(int motorVal);
};