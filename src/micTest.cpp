#ifdef MIC_TEST
#include <Arduino.h>
#include <driver/i2s.h>


// #define I2S_DIN 26 // I2S Data
// #define I2S_SCK 5  // I2S BCK
// #define I2S_WS 18  // I2S LRCK

// I2S configuration
#define I2S_WS 18
#define I2S_SD 26
#define I2S_SCK 5
#define I2S_SAMPLE_BIT_COUNT 16
#define SOUND_SAMPLE_RATE 8000
#define SOUND_CHANNEL_COUNT 1
#define I2S_PORT I2S_NUM_0

const int I2S_DMA_BUF_COUNT = 8;
const int I2S_DMA_BUF_LEN = 1024;
// sound sample (16 bits) amplification
const int AmplifyFactor = 5;  // <= 1 if no amplification of the sound sample

#if I2S_SAMPLE_BIT_COUNT == 32
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

void i2s_install()
{
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SoundSampleRate,
        .bits_per_sample = i2s_bits_per_sample_t(16),
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = BufferLen,
        .use_apll = false};
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
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

void setup()
{
    Serial.begin(115200);
    Serial.println("Setup I2S ...");

    i2s_install();
    i2s_setpin();
    i2s_start(I2S_PORT);

    Serial.println("I2S setup complete");
}

void loop()
{

    size_t bytesRead = 0;
    esp_err_t result = i2s_read(I2S_PORT, &Buffer, BufferNumBytes, &bytesRead, portMAX_DELAY);

    if (result == ESP_OK)
    {
   
            int16_t samplesRead = bytesRead / 2; // 16 bit per sample
            if (AmplifyFactor > 1)
            {
                // amplify the sound sample, by simply multiple it by some "amplify factor"
                for (int i = 0; i < samplesRead; ++i)
                {
                    int32_t val = Buffer[i];
                    val = AmplifyFactor * val;
                    if (val > 32700)
                    {
                        val = 32700;
                    }
                    else if (val < -32700)
                    {
                        val = -32700;
                    }
                    Buffer[i] = val;
                }
            }
            // print the sound sample
            for (int i = 0; i < samplesRead; ++i)
            {
                Serial.println(Buffer[i]);
            }
       
    }

    delay(100); // Small delay to not flood the serial output
}

#endif // MIC_TEST