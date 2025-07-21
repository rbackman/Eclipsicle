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
#include "pins.h"

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
    void writeWAVHeader(File &file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels);
    void updateWAVHeader(File &file);
    void checkMic();

    AudioGeneratorWAV *wav;
    AudioOutputI2S *out;
    AudioFileSourceSD *file;

    File audioFile;

    bool isRecording;
    bool isPlaying;
};

#endif