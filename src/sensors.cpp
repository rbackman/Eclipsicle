

#include "sensors.h"
#include "shared.h"
#include "FunctionalInterrupt.h"

SensorManager *SensorManager::instance = nullptr;

// #define USE_BUTTON_INTERRUPTS

#ifdef USE_BUTTON_INTERRUPTS
int button1Pin = -1;
int button2Pin = -1;
int button3Pin = -1;
int button4Pin = -1;

volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;
volatile bool button4Pressed = false;

void handleButton1Interrupt()
{
    button1Pressed = true;
}
static void handleButton2Interrupt()
{

    button2Pressed = true;
}
static void handleButton3Interrupt()
{

    button3Pressed = true;
}
static void handleButton4Interrupt()
{
    button4Pressed = true;
}

#endif

SensorManager::SensorManager(SensorGrid sensors) : sensorGrid(sensors)
{
    instance = this;
    for (int i = 0; i < sensors.size(); i++)
    {
#ifdef USE_BUTTON_INTERRUPTS
        if (sensors[i].type == BUTTON)
        {
            // Serial.printf("Creating Button %s %d\n", sensors[i].name, sensors[i].pin);
            pinMode(sensors[i].pin, INPUT_PULLUP);
            if (sensors[i].name == "Button1")
            {
                button1Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton1Interrupt, FALLING);
            }
            if (sensors[i].name == "Button2")
            {
                button2Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton2Interrupt, FALLING);
            }
            if (sensors[i].name == "Button3")
            {
                button3Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton3Interrupt, FALLING);
            }
            if (sensors[i].name == "Button4")
            {
                button4Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton4Interrupt, FALLING);
            }
        }
        else
            pinMode(sensors[i].pin, INPUT);
#else
        if (sensors[i].type == BUTTON)
        {
            pinMode(sensors[i].pin, INPUT_PULLUP);
        }
        else
        {
            pinMode(sensors[i].pin, INPUT);
        }
#endif
    }
}
bool SensorManager::buttonPressed(SensorID id)
{

    for (int i = 0; i < sensorGrid.size(); i++)
    {

        if (sensorGrid[i].sensorID == id)
        {
            if (sensorGrid[i].value == 1 && sensorGrid[i].changed)
            {
                sensorGrid[i].changed = false;
                sensorGrid[i].value = 0;
                return true;
            }
        }
    }
    return false;
}

bool SensorManager::messageAvailable()
{

    if (sensorGrid[currentSensor].changed)
    {
        return true;
    }
    return false;
}
sensor_message SensorManager::getNextMessage()
{
    if (sensorGrid[currentSensor].changed)
    {
        sensorGrid[currentSensor].changed = false;
        sensor_message message;
        message.sensorType = sensorGrid[currentSensor].type;
        message.type = MESSAGE_TYPE_SENSOR;
        message.value = sensorGrid[currentSensor].value;
        message.sensorId = sensorGrid[currentSensor].sensorID;
        return message;
    }
    // should never get here
}

void SensorManager::updateSensors()
{
    // only updates one sensor at a time to avoid saturating the ADC

    currentSensor++;
    if (currentSensor >= sensorGrid.size())
    {
        currentSensor = 0;
    }
    SensorState *sensor = &sensorGrid[currentSensor];

    long currentTime = millis();
    // && currentTime - grid[i].lastDebounceTime > DEBOUNCE_DELAY

    if (currentTime - sensor->lastDebounceTime > DEBOUNCE_DELAY)
    {
        sensor->lastDebounceTime = currentTime;
    }
    else
    {
        // too soon to check again
        return;
    }
    if (sensor->type == BUTTON)
    {
#ifdef USE_BUTTON_INTERRUPTS
        if (sensor->pin == button1Pin && button1Pressed)
        {
            if (sensor->value == 1)
            {
                sensor->value = 0;
            }
            else
            {
                sensor->value = 1;
            }

            button1Pressed = false;
        }
        if (sensor->pin == button2Pin && button2Pressed)
        {
            if (sensor->value == 1)
            {
                sensor->value = 0;
            }
            else
            {
                sensor->value = 1;
            }
            button2Pressed = false;
        }
        if (sensor->pin == button3Pin && button3Pressed)
        {
            if (sensor->value == 1)
            {
                sensor->value = 0;
            }
            else
            {
                sensor->value = 1;
            }
            button3Pressed = false;
        }
        if (sensor->pin == button4Pin && button4Pressed)
        {
            if (sensor->value == 1)
            {
                sensor->value = 0;
            }
            else
            {
                sensor->value = 1;
            }
            button4Pressed = false;
        }

#else
        Serial1.println("Button update" + String(getSensorName(sensor->sensorID)));
        int value = digitalRead(sensor->pin) == LOW ? 1 : 0;

        if (value != sensor->value && !sensor->changed) // only change on button press
        {

            sensor->value = value;
            sensor->changed = true;

            if (!sensor->initialized)
            {
                sensor->initialized = true;

                return;
            }
        }
#endif
        return;
    }
    else

    {
        // linearize logrithmic sensor value
        int value = map(pow(analogRead(sensor->pin), 1.5), 262048, 0, 0, 255);
        // value = constrain(value, 0, 255);
        // if(currentSensor==1){
        //     Serial.println("Sensor: " + String(sensor->name) + " :" + String(value) + " analog: " + String(analog));
        // }

        value = 255 - value;

        if (!sensor->initialized)
        {
            sensor->initialized = true;
            for (int i = 0; i < 5; i++)
            {
                sensor->lastValues[i] = value;
                sensor->value = value;
                sensor->changed = true;
            }
            
            return;
        }

        // insert value into sensor array and shift others to the left
        for (int i = 0; i < 4; i++)
        {
            sensor->lastValues[i] = sensor->lastValues[i + 1];
        }
        sensor->lastValues[4] = value;

        int average = 0;
        for (int i = 0; i < 5; i++)
        {
            average += sensor->lastValues[i];
        }
        average = average / 5;

        if (abs(average - sensor->value) > sensor->tolerance)
        {
            sensor->value = average;
            sensor->changed = true;
            if (isVerbose() || printSensor == currentSensor || printSensor == -2)
            {
                // Serial.println("print Sensor changed: " + String(sensor->name) + " Average: " + String(average) + " :" + String(output));
                // Serial.printf("Sensor Changed: %s Average: %d\n", sensor->name, average);
            }
        }
    }
}

int SensorManager::getSensorValue(SensorID id, int min, int max)
{
    for (int i = 0; i < sensorGrid.size(); i++)
    {
        SensorState *sensor = &sensorGrid[i];
        if (sensor->sensorID == id)
        {
            int value = map(sensor->value, 0, 255, min, max);

            return value;
        }
    }
    return 0;
}

bool SensorManager::handleSensorCommand(String command)
{
    if (command.startsWith("print"))
    {
        String sensorToPrint = command.substring(6);
        sensorToPrint.trim();
        Serial.println("Printing sensor value for " + sensorToPrint);

        for (int i = 0; i < sensorGrid.size(); i++)
        {
            SensorState *sensor = &sensorGrid[i];
            String sensorName = getSensorName(sensor->sensorID);
            sensorName.trim();
            Serial.println("Checking sensor: " + sensorName);
            if (sensorName.equalsIgnoreCase(sensorToPrint))
            {
                printSensor = i;
                Serial.println("Printing sensor: " + sensorName);
                return true;
            }
            else
            {
                Serial.println("not equal " + sensorName + " " + sensorToPrint);
            }
        }
    }
    return false;
}