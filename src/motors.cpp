

int MOTOR_PIN = 18;
#include <esp_now.h>
#include <WiFi.h>
#include "shared.h"
#include "motors.h"

void MotorManager::initMotors()
{
    // setup servo pwm
    pinMode(MOTOR_PIN, OUTPUT);
    ledcSetup(0, 50, 16);        // channel 0, 50 Hz, 16-bit width
    ledcAttachPin(MOTOR_PIN, 0); // GPIO 12 assigned to channel 0
    ledcWrite(0, 0);             // duty cycle 0%
}

bool MotorManager::handleMotorCommand(String cmd)
{
    if (cmd.startsWith("setMotor"))
    {
        int motorVal = cmd.substring(9).toInt();
        if (motorVal >= 0 && motorVal <= 180)
        {

            setMotor(motorVal);
        }
        return true;
    }
    return false;
}

bool MotorManager::respondToParameterMessage(parameter_message parameter)
{
    if (parameter.paramID == PARAM_MOTOR_SPEED)
    {
        setMotor(parameter.value);
        return true;
    }
    return false;
}
void MotorManager::setMotor(int val)
{
    int dutyCycle = map(val, 0, 180, 3277, 6554);
    if (isVerbose())
        Serial.println("motorVal set to " + String(dutyCycle));
    ledcWrite(0, dutyCycle); // set the duty cycle
}
void MotorManager::updateMotors()
{
    //
}
void MotorManager::initDriver()
{
}