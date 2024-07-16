#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <driver/i2s.h>

// #define MIC_WS 18      // I2S LRCLK
// #define MIC_SD_DATA 26 // I2S DOUT
// #define MIC_SCK 5      // I2S BCLK

// #define AUDIO_OUT_PIN 25 // DAC output pin

// #define SD_CS_PIN 15 // SD Card Chip Select
// #define SD_MOSI 23   // SD Card MOSI
// #define SD_MISO 19   // SD Card MISO
// #define SD_SCK 2     // SD Card SCK

class AudioManager
{
public:
    AudioManager();
    void init();
    void update();
    void play();
    void stop();
    void record();
    void setVolume(int volume);
    void saveFile(const char *filename);
    void loadFile(const char *filename);
    void playTone(int freq, int duration, int volume);
    int getDecibel();
    bool isRecordingAudio();
    bool isPlayingAudio();
    void debugAudio();

private:
    void checkMic();
    void writeWAVHeader(File file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels);
    void updateWAVHeader(File file);

    int getAmplitude(int16_t *buffer, int bufferSize);

    void printAmplitude(int amplitude);

    File audioFile;
    bool isRecording = false;
    bool isPlaying = false;
    int volume = 128; // Default volume (0-255)
};
#endif // AUDIO_H
