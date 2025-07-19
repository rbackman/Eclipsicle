
#ifdef USE_SENSORS
#include <Arduino.h>
#include "sensors.h"
#include "shared.h"
// #define USE_BUTTON_INTERRUPTS

#ifdef USE_BUTTON_INTERRUPTS
#include "FunctionalInterrupt.h"
#endif
SensorManager *SensorManager::instance = nullptr;

#ifdef USE_ACCELEROMETER
// setup accelerometer
if (!mpu.begin())
{
    Serial.println("Failed to find MPU6050 chip");
}

pinMode(buttonPin, INPUT_PULLUP);
Serial.println("MPU6050 Found!");

// setupt motion detection
mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
mpu.setMotionDetectionThreshold(1);
mpu.setMotionDetectionDuration(20);
mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
mpu.setInterruptPinPolarity(true);
mpu.setMotionInterrupt(true);
#endif

#ifdef USE_BUTTON_INTERRUPTS
int button1Pin = -1;
int button2Pin = -1;
int button3Pin = -1;
int button4Pin = -1;
SPISettings settings(1000000, MSBFIRST, SPI_MODE0);
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

SensorManager::SensorManager(SensorGrid sensors, SPIClass *spi) : sensorGrid(sensors), spiBus(spi)
{
    instance = this;
    for (int i = 0; i < sensors.size(); i++)
    {
        if (sensors[i].csPin != -1 && spiBus != nullptr)
        {

            pinMode(sensors[i].csPin, OUTPUT);
            digitalWrite(sensors[i].csPin, HIGH); // Deselect the sensor
        }
        if (sensors[i].type == DIAL || sensors[i].type == SLIDER)
        {
            sensors[i].tolerance = 5; // Set tolerance for DIAL and SLIDER sensors
        }
#ifdef USE_BUTTON_INTERRUPTS
        if (sensors[i].type == BUTTON)
        {
            pinMode(sensors[i].pin, INPUT);
            if (sensors[i].sensorID == BUTTON_UP)
            {
                button1Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton1Interrupt, FALLING);
            }
            if (sensors[i].sensorID == BUTTON_DOWN)
            {
                button2Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton2Interrupt, FALLING);
            }
            if (sensors[i].sensorID == BUTTON_LEFT)
            {
                button3Pin = sensors[i].pin;
                attachInterrupt(digitalPinToInterrupt(sensors[i].pin), handleButton3Interrupt, FALLING);
            }
            if (sensors[i].sensorID == BUTTON_RIGHT)
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
    return sensor_message{MESSAGE_TYPE_TEXT, DIAL, SLIDER1, 0}; // return an empty message if no message is available

    // should never get here
}
int SensorManager::readADC(SensorState *sensor)
{
    if (spiBus == nullptr)
    {
        Serial.println("SPI bus not initialized");
        return 0;
    }

    if (sensor->csPin == -1)
    {
        Serial.println("No chip select pin for this sensor");
    }

    spiBus->beginTransaction(settings);
    digitalWrite(sensor->csPin, LOW);

    byte command = 0b10000000 | (sensor->pin << 4); // Start bit + single-ended + channel
    spiBus->transfer(0x01);                         // Start bit
    byte highByte = spiBus->transfer(command);
    byte lowByte = spiBus->transfer(0x00);

    digitalWrite(sensor->csPin, HIGH);
    spiBus->endTransaction();

    return ((highByte & 0x03) << 8) | lowByte; // 10-bit result
}
void SensorManager::updateSensors()
{
    // only update one sensor each cycle to avoid saturating the ADC
    currentSensor++;
    if (currentSensor >= sensorGrid.size())
    {
        currentSensor = 0;
    }
    SensorState *sensor = &sensorGrid[currentSensor];

    long currentTime = millis();
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
    else if (sensor->csPin > 0)
    {
        int value = readADC(sensor);
        if (sensor->tolerance > 0)
        {

            if (abs(value - sensor->value) <= sensor->tolerance)
            {
                return; // Value is within tolerance, no change
            }
        }
        if (value != -1 && value != sensor->value && !sensor->changed) // only change on value change
        {
            if (isVerbose() || printSensor == currentSensor || printSensor == -2)
            {
                Serial.printf("Sensor %s value changed: %d\n", getSensorName(sensor->sensorID), value);
            }
            sensor->value = value;
            sensor->changed = true;
        }
    }
    else
    {
        printf("Reading analog pin %d (%s)\n", sensor->pin, getSensorName(sensor->sensorID));
        int anlg = analogRead(sensor->pin);
        // linearize logrithmic sensor value
        int value = map(pow(anlg, 1.5), 262048, 0, 0, 255);
        // value = constrain(value, 0, 255);

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
                // Optional verbose logging for sensor changes
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

#endif