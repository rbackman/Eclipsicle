
#ifdef USE_AUDIO
#include "audio.h"
#include <driver/i2s_std.h>
#include <math.h>

// #define USE_SPEAKER 1
#define USE_MIC 1

#define I2S_PORT I2S_NUM_0

bool sdCardMounted = false;

// handles for the new I2S driver
i2s_chan_handle_t mic_chan; // RX channel for microphone
i2s_chan_handle_t spk_chan; // TX channel for speaker
bool speakerConnected = false;

#define I2S_SAMPLE_BIT_COUNT 16
#define SOUND_SAMPLE_RATE 16000
#define SOUND_CHANNEL_COUNT 1

const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;
// sound sample (16 bits) amplification
const int AmplifyFactor = 5; // <= 1 if no amplification of the sound sample

#if I2S_SAMPLE_BIT_COUNT == 16
const int StreamBufferNumBytes = 512;
const int StreamBufferLen = StreamBufferNumBytes / 4;
int32_t StreamBuffer[StreamBufferLen];
#else
#if SOUND_SAMPLE_RATE == 16000
// for 16 bits ... 16000 sample per second (32000 bytes per second; since 16 bits per sample) ==> 512 bytes = 16 ms per read
const int StreamBufferNumBytes = 512;
#else
// for 16 bits ... 8000 sample per second (16000 bytes per second; since 16 bits per sample) ==> 256 bytes = 16 ms per read
const int StreamBufferNumBytes = 256;
#endif
const int StreamBufferLen = StreamBufferNumBytes / 2;
int16_t StreamBuffer[StreamBufferLen];
#endif

const int MaxAmplifyFactor = 20;
const int DefAmplifyFactor = 10;
const int SoundSampleRate = 8000; // will be 16-bit per sample
const int SoundNumChannels = 1;
const int BufferNumBytes = 256;
const int BufferLen = BufferNumBytes / 2;
int16_t Buffer[BufferLen];
bool micConnected = false;
#define DEBUG_AMPLITUDE_THRESHOLD 500
#define DEBUG_AMPLITUDE_MAX 10000

// void i2s_install()
// {
//     const i2s_config_t i2s_config = {
//         .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
//         .sample_rate = SOUND_SAMPLE_RATE,
//         .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
//         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//         .communication_format = I2S_COMM_FORMAT_STAND_I2S,
//         .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//         .dma_buf_count = I2S_DMA_BUF_COUNT,
//         .dma_buf_len = I2S_DMA_BUF_LEN,
//         .use_apll = false,
//         .tx_desc_auto_clear = true,
//         .fixed_mclk = 0};
//     esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
//     if (err != ESP_OK)
//     {
//         Serial.printf("Failed to install I2S driver: %d\n", err);
//     }
// }

// void i2s_setpin()
// {
//     const i2s_pin_config_t pin_config = {
//         .bck_io_num = I2S_SCK,
//         .ws_io_num = I2S_WS,
//         .data_out_num = -1,
//         .data_in_num = I2S_SD};
//     i2s_set_pin(I2S_PORT, &pin_config);
// }
void i2s_mic_setup()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);

    if (i2s_new_channel(&chan_cfg, &spk_chan, &mic_chan) != ESP_OK)
    {
        Serial.println("Failed to allocate I2S channels");
        micConnected = false;
        return;
    }

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SOUND_SAMPLE_RATE),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = I2S_DATA_BIT_WIDTH_16BIT,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false,

        },
        .gpio_cfg = {.mclk = GPIO_NUM_NC, .bclk = (gpio_num_t)AUDIO_BCLK_PIN, .ws = (gpio_num_t)AUDIO_WS_PIN, .dout = GPIO_NUM_NC, .din = (gpio_num_t)AUDIO_DIN_PIN, .invert_flags = {
                                                                                                                                                                         .mclk_inv = false,
                                                                                                                                                                         .bclk_inv = false,
                                                                                                                                                                         .ws_inv = false,
                                                                                                                                                                     }},
    };

    if (i2s_channel_init_std_mode(mic_chan, &std_cfg) == ESP_OK &&
        i2s_channel_enable(mic_chan) == ESP_OK)
    {
        micConnected = true;
        Serial.println("Microphone I2S setup complete");
    }
    else
    {
        micConnected = false;
        Serial.println("Failed to init microphone I2S");
    }
}

// I2S configuration for playback
void i2s_playback_setup()
{
    // spk_chan was allocated alongside mic_chan in i2s_mic_setup()
    if (spk_chan == NULL)
    {
        Serial.println("Speaker channel handle not initialized");
        return;
    }
    //  .mclk = GPIO_NUM_NC,
    //             .bclk = (gpio_num_t)AUDIO_BCLK_PIN,
    //             .ws = (gpio_num_t)AUDIO_LRC_PIN,
    //             .dout = GPIO_NUM_NC,
    //             .din = (gpio_num_t)AUDIO_DIN_PIN,
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = (gpio_num_t)AUDIO_BCLK_PIN,
            .ws = (gpio_num_t)AUDIO_WS_PIN,
            .dout = GPIO_NUM_NC,
            .din = (gpio_num_t)AUDIO_DIN_PIN,
        },
    };

    if (i2s_channel_init_std_mode(spk_chan, &std_cfg) == ESP_OK &&
        i2s_channel_enable(spk_chan) == ESP_OK)
    {
        speakerConnected = true;
        Serial.println("Speaker I2S setup complete");
    }
    else
    {
        speakerConnected = false;
        Serial.println("Failed to init speaker I2S");
    }
}

AudioManager::AudioManager()
{

    init();
}
// void AudioManager::debugAudio()
// {
//     size_t bytesRead = 0;
//     esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, 0);

//     if (result == ESP_OK && bytesRead > 0)
//     {
//         int16_t samplesRead = bytesRead / 2;
//         int amplitude = getAmplitude(Buffer, samplesRead);
//         printAmplitude(amplitude);
//     }
//     else
//     {
//         Serial.println("Failed to read audio data");
//     }
// }

void AudioManager::playTone(int freq, int duration, float volume)
{
    const int samples = SAMPLE_RATE * duration / 1000;
    const float volumeScale = volume * 32767.0f; // Scale volume to 16-bit range

    int16_t *buffer = new int16_t[samples];

    // Generate sine wave
    for (int i = 0; i < samples; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        buffer[i] = (int16_t)(sin(2 * M_PI * freq * t) * volumeScale);
    }

    size_t bytesWritten = 0;
    // use the playback I2S channel when writing samples. The microphone uses
    // a separate RX channel so writing to it would fail with "TX mode is not
    // enabled".  The speaker output is handled via `spk_chan`.
    esp_err_t result = i2s_channel_write(spk_chan, buffer,
                                         samples * sizeof(int16_t), &bytesWritten,
                                         portMAX_DELAY);

    if (result != ESP_OK)
    {
        Serial.println("Failed to play tone");
    }

    delete[] buffer;

    // Delay to allow the tone to finish playing
    delay(duration);

    // Play a short period of silence to stop the tone
    int16_t silence[100] = {0};
    // flush the output on the speaker I2S port
    i2s_channel_write(spk_chan, silence, sizeof(silence), &bytesWritten,
                      portMAX_DELAY);
}

void AudioManager::init()
{
    Serial.println("Initializing AudioManager...");

#ifdef USE_MIC
    // init i2s for mic
    i2s_mic_setup();

    checkMic();

#endif

#ifdef USE_SPEAKER
    // init i2s for speaker
    i2s_playback_setup();
#endif
    Serial.println("AudioManager initialized.");
}
int AudioManager::getDecibel()
{
    Serial.println("Getting decibel level...");
    if (!micConnected)
    {
        Serial.println("Mic not connected.");
        return 0;
    }

    int32_t buffer32[64] = {0};
    size_t bytes_read = 0;
    if (i2s_channel_read(mic_chan, buffer32, sizeof(buffer32), &bytes_read,
                         portMAX_DELAY) != ESP_OK ||
        bytes_read == 0)
    {
        Serial.println("Failed to read audio data");
        return 0;
    }

    int samples = bytes_read / sizeof(int32_t);
    double sum_sq = 0;
    for (int i = 0; i < samples; ++i)
    {
        float sample = buffer32[i] / 32768.0f;
        sum_sq += sample * sample;
    }
    float rms = sqrtf(sum_sq / samples);
    int db = (int)(20.0f * log10f(rms + 1e-12f));

    Serial.println("Decibels: " + String(db));
    return db;
}

int frameCount = 0;

// int AudioManager::getAmplitude(int16_t *buffer, int bufferSize)
// {
//     int32_t sum = 0;
//     for (int i = 0; i < bufferSize; i++)
//     {
//         sum += abs(buffer[i]);
//     }
//     return sum / bufferSize;
// }

// void AudioManager::printAmplitude(int amplitude)
// {
//     int normalizedAmplitude = map(amplitude, 0, DEBUG_AMPLITUDE_MAX, 0, 40);
//     Serial.print("|");
//     for (int i = 0; i < 40; i++)
//     {
//         if (i < normalizedAmplitude)
//         {
//             Serial.print("#");
//         }
//         else
//         {
//             Serial.print(" ");
//         }
//     }
//     Serial.print("| ");
//     Serial.println(amplitude);
// }

void AudioManager::update()
{
#ifdef USE_MIC
    if (isRecording)
    {
        size_t bytesRead = 0;
        uint8_t buffer[512];
        esp_err_t result =
            i2s_channel_read(mic_chan, buffer, sizeof(buffer), &bytesRead,
                             portMAX_DELAY);

        if (result == ESP_OK && bytesRead > 0)
        {
            audioFile.write(buffer, bytesRead);
        }
    }
#endif
#if USE_SPEAKER
    if (isPlaying && wav->isRunning())
    {
        if (!wav->loop())
        {
            stop();
        }
    }
#endif
}

void AudioManager::record()
{
#ifdef USE_MIC
    if (isRecording)
        return;

    audioFile = SD.open("/recording.wav", FILE_WRITE);
    if (!audioFile)
    {
        Serial.println("Failed to open file for recording");
        return;
    }

    writeWAVHeader(audioFile, SAMPLE_RATE, BITS_PER_SAMPLE, CHANNELS);

    isRecording = true;
    Serial.println("Recording started.");
#endif
}

void AudioManager::stop()
{
#ifdef USE_MIC
    if (isRecording)
    {
        updateWAVHeader(audioFile);
        audioFile.close();
        isRecording = false;
        Serial.println("Recording stopped.");
    }
#endif

#ifdef USE_SPEAKER
    if (isPlaying)
    {
        wav->stop();
        isPlaying = false;
        Serial.println("Playback stopped.");
    }
#endif
}

void AudioManager::checkMic()
{
    if (!micConnected)
    {
        Serial.println("Mic not connected.");
        return;
    }
    Serial.println("Checking microphone...");
    unsigned long startTime = millis();
    int nonZeroCount = 0;
    int totalSamples = 0;

    size_t bytes_read;
    while (millis() - startTime < 5000) // Run for 5 seconds
    {
        i2s_channel_read(mic_chan, &Buffer, sizeof(Buffer), &bytes_read,
                         300);
        if (bytes_read > 0)
        {
            for (int i = 0; i < bytes_read / 4; i++)
            { // Divide by 4 because each sample is 4 bytes
                // int32_t sample = Buffer[i];
                // int32_t sample24bit = sample >> 8; // Shift right by 8 to get 24-bit value
                // Serial.printf("Raw: %d, 24-bit: %d\n", sample, sample24bit);
                // if (sample != 0)
                //     nonZeroCount++;
                // totalSamples++;
                Serial.printf("%d,", Buffer[i]);
            }
        }
        else
        {
            Serial.printf("No data read from mic\n");
        }
    }
    Serial.printf("Mic check complete. Non-zero samples: %d/%d\n", nonZeroCount, totalSamples);
}

void AudioManager::play()
{
#ifdef USE_SPEAKER
    if (isPlaying)
        return;

    file = new AudioFileSourceSD("/recording.wav");
    wav = new AudioGeneratorWAV();

    if (wav->begin(file, out))
    {
        isPlaying = true;
        Serial.println("Playback started.");
    }
    else
    {
        Serial.println("Failed to start playback.");
    }
#endif
}

// void AudioManager::setVolume(int volume)
// {
//     this->volume = volume;
//     Serial.print("Volume set to ");
//     Serial.println(volume);
// }

void AudioManager::saveFile(const char *filename)
{
#ifdef USE_SD
    if (!sdCardMounted)
    {
        Serial.println("SD Card not mounted.");
        return;
    }
    if (isRecording)
    {
        Serial.println("Cannot save file while recording.");
        return;
    }
    File destFile = SD.open(filename, FILE_WRITE);
    if (!destFile)
    {
        Serial.println("Failed to open destination file for saving.");
        return;
    }

    File sourceFile = SD.open("/recording.wav");
    if (!sourceFile)
    {
        Serial.println("Failed to open source file for saving.");
        destFile.close();
        return;
    }

    while (sourceFile.available())
    {
        destFile.write(sourceFile.read());
    }

    sourceFile.close();
    destFile.close();
    Serial.println("File saved.");
#endif
}

void AudioManager::loadFile(const char *filename)
{
#ifdef USE_SD
    if (isRecording)
    {
        Serial.println("Cannot load file while recording.");
        return;
    }

    audioFile = SD.open(filename);
    if (!audioFile)
    {
        Serial.println("Failed to open file for loading.");
        return;
    }

    Serial.println("File loaded.");
#endif
}

bool AudioManager::isRecordingAudio()
{
    return isRecording;
}

bool AudioManager::isPlayingAudio()
{
    return isPlaying;
}
void AudioManager::writeWAVHeader(fs::File &file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels)
{
#ifdef USE_SD
    uint32_t fileSize = 0;      // Placeholder for file size
    uint32_t subchunk2Size = 0; // Placeholder for subchunk2 size

    // Chunk ID "RIFF"

    file.write((const uint8_t *)"RIFF", 4);

    // Chunk size (fileSize - 8), will be filled in later
    file.write((const uint8_t *)&fileSize, 4);

    // Format "WAVE"
    file.write((const uint8_t *)"WAVE", 4);

    // Subchunk1 ID "fmt "
    file.write((const uint8_t *)"fmt ", 4);

    // Subchunk1 size (16 for PCM)
    uint32_t subchunk1Size = 16;
    file.write((const uint8_t *)&subchunk1Size, 4);

    // Audio format (1 for PCM)
    uint16_t audioFormat = 1;
    file.write((const uint8_t *)&audioFormat, 2);

    // Number of channels
    file.write((const uint8_t *)&channels, 2);

    // Sample rate
    file.write((const uint8_t *)&sampleRate, 4);

    // Byte rate (SampleRate * NumChannels * BitsPerSample/8)
    uint32_t byteRate = sampleRate * channels * bitDepth / 8;
    file.write((const uint8_t *)&byteRate, 4);

    // Block align (NumChannels * BitsPerSample/8)
    uint16_t blockAlign = channels * bitDepth / 8;
    file.write((const uint8_t *)&blockAlign, 2);

    // Bits per sample
    file.write((const uint8_t *)&bitDepth, 2);

    // Subchunk2 ID "data"
    file.write((const uint8_t *)"data", 4);

    // Subchunk2 size (NumSamples * NumChannels * BitsPerSample/8), will be filled in later
    file.write((const uint8_t *)&subchunk2Size, 4);
#endif
}

void AudioManager::updateWAVHeader(File &file)
{
#ifdef USE_SD
    // Get the final file size
    uint32_t fileSize = file.size();
    // Chunk size (fileSize - 8)
    uint32_t chunkSize = fileSize - 8;
    // Subchunk2 size (fileSize - 44)
    uint32_t subchunk2Size = fileSize - 44;

    // Update the chunk size in the header
    file.seek(4);
    file.write((const uint8_t *)&chunkSize, 4);

    // Update the subchunk2 size in the header
    file.seek(40);
    file.write((const uint8_t *)&subchunk2Size, 4);
#endif
}
#endif