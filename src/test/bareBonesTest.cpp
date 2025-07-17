#ifdef BARE_BONES_TEST

#include <Arduino.h>
#include <ArduinoJson.h>

void setup()
{
    Serial.begin(921600);
    delay(1000); // Wait for serial monitor to open
    Serial.println("Bare Bones Test Setup Complete");
}
void loop()
{
    Serial.println("Bare Bones Test Loop Running");
    delay(1000); // Delay to avoid flooding the serial output
}

#endif