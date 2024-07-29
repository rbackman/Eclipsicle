#pragma once

#ifdef USE_AUDIO

#include "Arduino.h"


#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
// Pin definitions
#define I2S_BCLK 26
#define I2S_LRC 22
#define I2S_DOUT 25
#define I2S_DIN 33
#define SD_CS 5

// I2S configuration
#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 1

class AudioManager
{
public:
    AudioManager();
    void init();
    void record();
    void play();
    void stop();
    void update();
    bool isRecordingAudio();
    bool isPlayingAudio();
    void debugAudio();
    void playTone(int freq, int duration, float volume);
    void saveFile(const char *filename);
    void loadFile(const char *filename);
    int getDecibel();
private:

    void writeWAVHeader( File &file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels);
    void updateWAVHeader( File &file);
    void checkMic();

    AudioGeneratorWAV *wav;
    AudioOutputI2S *out;
    AudioFileSourceSD *file;
  

    File audioFile;
   
    bool isRecording;
    bool isPlaying;
};

#endif