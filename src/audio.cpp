#ifdef MASTER_BOARD
#include "audio.h"


#define AUDIO_OUT_PIN 25 // DAC output pin

SPIClass spi(VSPI); // Create a new SPI instance

#define SD_CS_PIN 15 // SD Card Chip Select
#define SD_MOSI 23   // SD Card MOSI
#define SD_MISO 19   // SD Card MISO
#define SD_SCK 2     // SD Card SCK

bool sdCardMounted = false;

// I2S configuration
#define I2S_WS 18
#define I2S_SD 26
#define I2S_SCK 5
#define I2S_SAMPLE_BIT_COUNT 16
#define SOUND_SAMPLE_RATE 16000
#define SOUND_CHANNEL_COUNT 1
#define I2S_PORT I2S_NUM_0

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

#define DEBUG_AMPLITUDE_THRESHOLD 500
#define DEBUG_AMPLITUDE_MAX 10000

void i2s_install()
{
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SOUND_SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BIT_COUNT),
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.printf("Failed to install I2S driver: %d\n", err);
    }
}

void i2s_setpin()
{
    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD};
    i2s_set_pin(I2S_PORT, &pin_config);
}

AudioManager::AudioManager()
{

    init();
}
void AudioManager::debugAudio()
{
    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, 0);

    if (result == ESP_OK && bytesRead > 0)
    {
        int16_t samplesRead = bytesRead / 2;
        int amplitude = getAmplitude(Buffer, samplesRead);
        printAmplitude(amplitude);
    }
    else
    {
        Serial.println("Failed to read audio data");
    }
}
void AudioManager::playTone(int freq, int duration, int volume)
{

    ledcSetup(0, 5000, 8);
    ledcAttachPin(AUDIO_OUT_PIN, 0);
    ledcWriteTone(0, freq);
    // Set the volume
    ledcWrite(0, volume);
    delay(duration);
    ledcWriteTone(0, 0);
}

void AudioManager::init()
{
    Serial.println("Initializing AudioManager...");

    spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS_PIN);

    if (!SD.begin(SD_CS_PIN, spi))
    {
        Serial.println("Card Mount Failed");
    }
    else
    {
        sdCardMounted = true;
        Serial.println("SD Card mounted successfully");
    }

    i2s_install();
    i2s_setpin();
    i2s_start(I2S_PORT);

    Serial.println("AudioManager initialized.");
    // delay(500);
    // checkMic();
}

int AudioManager::getDecibel()
{
    int32_t buffer32[64] = {0};
    size_t bytes_read;
    i2s_read(I2S_NUM_0, &buffer32, sizeof(buffer32), &bytes_read, portMAX_DELAY);
    return buffer32[0];
}

int frameCount = 0;

int AudioManager::getAmplitude(int16_t *buffer, int bufferSize)
{
    int32_t sum = 0;
    for (int i = 0; i < bufferSize; i++)
    {
        sum += abs(buffer[i]);
    }
    return sum / bufferSize;
}

void AudioManager::printAmplitude(int amplitude)
{
    int normalizedAmplitude = map(amplitude, 0, DEBUG_AMPLITUDE_MAX, 0, 40);
    Serial.print("|");
    for (int i = 0; i < 40; i++)
    {
        if (i < normalizedAmplitude)
        {
            Serial.print("#");
        }
        else
        {
            Serial.print(" ");
        }
    }
    Serial.print("| ");
    Serial.println(amplitude);
}

void AudioManager::update()
{
    frameCount++;
    if (isRecording)
    {
        size_t bytesRead = 0;
        esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, 0);

        if (result == ESP_OK && bytesRead > 0)
        {
            int16_t samplesRead = bytesRead / 2;
            for (int i = 0; i < samplesRead; ++i)
            {
                int32_t val = Buffer[i] * AmplifyFactor;
                val = constrain(val, -32768, 32767);
                Buffer[i] = val;
            }
            audioFile.write((const uint8_t *)Buffer, bytesRead);
            if (frameCount % 20 == 0)
            {
                Serial.print("-");
            }
        }
    }
    else if (isPlaying)
    {
       
        int16_t sample;
        if (audioFile.available() && audioFile.read((uint8_t *)&sample, 2) == 2) {
            int32_t amplifiedSample = sample * volume / 10;
            amplifiedSample = constrain(amplifiedSample, -32768, 32767);
            int dacValue = map(amplifiedSample, -32768, 32767, 0, 255); // Convert to 8-bit unsigned
            dacWrite(AUDIO_OUT_PIN, dacValue);
            // delayMicroseconds(1000000 / SOUND_SAMPLE_RATE); // Ensure this matches your sample rate
            // if (frameCount % 100 == 0)
            //     printAmplitude(abs(amplifiedSample));
        } else {
            Serial.println("Playback complete.");
            stop();
        }
    }
}

void AudioManager::record()
{
    if (!sdCardMounted)
    {
        Serial.println("SD Card not mounted.");
        return;
    }

    audioFile = SD.open("/recording.wav", FILE_WRITE);
    if (!audioFile)
    {
        Serial.println("Failed to open file for recording");
        return;
    }

    writeWAVHeader(audioFile, SOUND_SAMPLE_RATE, I2S_SAMPLE_BIT_COUNT, SOUND_CHANNEL_COUNT);

    isRecording = true;
    Serial.println("Recording started.");
}

void AudioManager::stop()
{
    if (isRecording)
    {
        updateWAVHeader(audioFile);
        audioFile.close();
        isRecording = false;
        Serial.println("Recording stopped.");

        // Save the recording to a file
        String filename = "/recording" + String(millis()) + ".wav";
        Serial.printf("Saving recording to %s\n", filename.c_str());
        saveFile(filename.c_str());
    }

    if (isPlaying)
    {
        audioFile.close();
        isPlaying = false;
        Serial.println("Playback stopped.");
    }
}

void AudioManager::checkMic()
{
    Serial.println("Checking microphone...");
    unsigned long startTime = millis();
    int nonZeroCount = 0;
    int totalSamples = 0;

    size_t bytes_read;
    while (millis() - startTime < 5000) // Run for 5 seconds
    {
        i2s_read(I2S_NUM_0, &Buffer, sizeof(Buffer), &bytes_read, portMAX_DELAY);
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
                Serial.println(Buffer[i]);
            }
        }
    }
    Serial.printf("Mic check complete. Non-zero samples: %d/%d\n", nonZeroCount, totalSamples);
}

void AudioManager::play()
{
    if (!sdCardMounted)
    {
        Serial.println("SD Card not mounted.");
        return;
    }
    audioFile = SD.open("/recording.wav");
    if (!audioFile)
    {
        Serial.println("Failed to open file for playback");
        return;
    }

    // Skip WAV header
    audioFile.seek(44);

    isPlaying = true;
    Serial.println("Playback started.");
}

void AudioManager::setVolume(int volume)
{
    this->volume = volume;
    Serial.print("Volume set to ");
    Serial.println(volume);
}

void AudioManager::saveFile(const char *filename)
{
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
}

void AudioManager::loadFile(const char *filename)
{
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

    File destFile = SD.open("/recording.wav", FILE_WRITE);
    if (!destFile)
    {
        Serial.println("Failed to open destination file for loading.");
        audioFile.close();
        return;
    }

    while (audioFile.available())
    {
        destFile.write(audioFile.read());
    }

    audioFile.close();
    destFile.close();
    Serial.println("File loaded.");
}

bool AudioManager::isRecordingAudio()
{
    return isRecording;
}

bool AudioManager::isPlayingAudio()
{
    return isPlaying;
}

void AudioManager::writeWAVHeader(File file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels)
{
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
}

void AudioManager::updateWAVHeader(File file)
{
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
}

#endif