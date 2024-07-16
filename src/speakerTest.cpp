#ifdef SPEAKER_TEST
#include <Arduino.h>
#include "SD.h"
#include "SPI.h"
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// Pin definitions
#define SD_CS_PIN    15
#define SD_SCK       2
#define SD_MISO      19
#define SD_MOSI      23
#define I2S_BCLK     5
#define I2S_LRC      18
#define I2S_DOUT     25
#define BUTTON_TRIGGER 16

// Audio objects
AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

// Function prototypes
void playAudio();
void stopAudio();

void setup() {
  Serial.begin(115200);
  
  // Initialize SD card
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Set up button
  pinMode(BUTTON_TRIGGER, INPUT_PULLUP);

  // Set up I2S audio output
  out = new AudioOutputI2S(0,1);
  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  out->SetGain(1);  // Adjust this value to change volume (0.0 - 1.0)

  Serial.println("Setup complete. Press the trigger button to play audio.");
}

void loop() {
  if (digitalRead(BUTTON_TRIGGER) == LOW) {
    delay(50);  // Simple debounce
    if (digitalRead(BUTTON_TRIGGER) == LOW && !wav) {
      playAudio();
    }
  }

  if (wav && wav->isRunning()) {
    if (!wav->loop()) {
      stopAudio();
    }
  }
}

void playAudio() {
  stopAudio();  // Stop any currently playing audio

  file = new AudioFileSourceSD("/recording.wav");
  id3 = new AudioFileSourceID3(file);
  wav = new AudioGeneratorWAV();

  if (wav->begin(id3, out)) {
    Serial.println("Audio playback started");
  } else {
    Serial.println("Failed to start audio playback");
    stopAudio();
  }
}

void stopAudio() {
  if (wav) {
    wav->stop();
    delete wav;
    wav = nullptr;
  }
  if (id3) {
    delete id3;
    id3 = nullptr;
  }
  if (file) {
    file->close();
    delete file;
    file = nullptr;
  }
  Serial.println("Audio playback stopped");
}


#endif // MIC_TEST