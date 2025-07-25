#ifdef AUDIO_TEST
#include <Arduino.h>
#include "../lib/serial.h"
#include "../lib/audio.h"

SerialManager *serialManager = nullptr;
AudioManager *audioManager = nullptr;

void setup()
{
    serialManager = new SerialManager();

    audioManager = new AudioManager();
}
void loop()
{

    delay(100); // Small delay to not flood the serial output
}

#endif