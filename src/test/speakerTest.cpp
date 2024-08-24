#ifdef SPEAKER_TEST

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <AudioGeneratorWAV.h>
#include <AudioFileSourceSD.h>

#include <AudioOutputI2S.h>
#include <driver/i2s.h>

// Master Pin Layout
// 3v3                    Vin
// Gnd                    Gnd

// 15: SD_CS_PIN          13: BUTTON_LEFT
// 2:  SD_SCK             12: BUTTON_RIGHT
// 4:                     14: BUTTON_DOWN
// 16: RX BUTTON_TRIG     27: BUTTON_UP
// Tx:                    26: I2S_SD (Data In for INMP441,Data Out for MAX98357)
// 5:  I2S_SCK            25:
// 18: I2S_WS             33: LED
// 19: SD_MISO            32: SLIDER5
// 21: OLED_SDA            35: SLIDER3
// rx0:                   34: SLIDER4
// tx0:                   39: SLIDER2
// 22: OLED_SCL           36: SLIDER1
// 23: SD_MOSI            37:

// // SD Card pins
// #define SD_CS 15
// #define SD_SCK 2
// #define SD_MISO 19
// #define SD_MOSI 23

// // I2S pins
// #define I2S_BCLK 5
// #define I2S_LRC 18
// #define I2S_DOUT 26

// SD Card pins
#define SD_CS 33
#define SD_SCK 14
#define SD_MISO 12
#define SD_MOSI 13

// I2S pins
#define I2S_BCLK 26
#define I2S_LRC 25
#define I2S_DOUT 27
#define I2S_MIC_SD 35
#define BUTTON_TRIGGER 0 // GPIO0 button on esp32

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;

bool isRecording = false;
bool sdloaded = false;
void writeWAVHeader(File &file, uint32_t sampleRate, uint16_t bitDepth, uint16_t channels)
{
  // WAV header format
  uint8_t wavHeader[44] = {
      'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
      16, 0, 0, 0, 1, 0, (uint8_t)channels, 0, (uint8_t)(sampleRate & 0xff), (uint8_t)((sampleRate >> 8) & 0xff), (uint8_t)((sampleRate >> 16) & 0xff), (uint8_t)((sampleRate >> 24) & 0xff),
      (uint8_t)((sampleRate * channels * bitDepth / 8) & 0xff), (uint8_t)(((sampleRate * channels * bitDepth / 8) >> 8) & 0xff), (uint8_t)(((sampleRate * channels * bitDepth / 8) >> 16) & 0xff), (uint8_t)(((sampleRate * channels * bitDepth / 8) >> 24) & 0xff),
      (uint8_t)((channels * bitDepth / 8) & 0xff), (uint8_t)(((channels * bitDepth / 8) >> 8) & 0xff), (uint8_t)bitDepth, 0, 'd', 'a', 't', 'a', 0, 0, 0, 0};

  file.write(wavHeader, sizeof(wavHeader));
}

void updateWAVHeader(File &file)
{
  uint32_t fileSize = file.size();
  file.seek(4);
  file.write((uint8_t *)&fileSize, 4);
  uint32_t dataChunkSize = fileSize - 44;
  file.seek(40);
  file.write((uint8_t *)&dataChunkSize, 4);
}

void i2s_mic_setup()
{
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};

  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num = I2S_LRC,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_MIC_SD};

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// I2S configuration for playback
void i2s_playback_setup()
{
  const i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};

  const i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num = I2S_LRC,
      .data_out_num = I2S_DOUT,
      .data_in_num = I2S_PIN_NO_CHANGE};

  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);
}

// I2S Configuration
// void i2s_setup() {
//   const i2s_config_t i2s_config = {
//       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
//       .sample_rate = 44100,
//       .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
//       .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//       .communication_format = I2S_COMM_FORMAT_STAND_I2S,
//       .intr_alloc_flags = 0,
//       .dma_buf_count = 8,
//       .dma_buf_len = 64,
//       .use_apll = false,
//       .tx_desc_auto_clear = true,
//       .fixed_mclk = 0};

//   const i2s_pin_config_t pin_config = {
//       .bck_io_num = I2S_BCLK,
//       .ws_io_num = I2S_LRC,
//       .data_out_num = I2S_DOUT,
//       .data_in_num = I2S_MIC_SD };

//   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//   i2s_set_pin(I2S_NUM_0, &pin_config);
// }
void playAudio()
{
  file = new AudioFileSourceSD("/recording.wav");
  wav = new AudioGeneratorWAV();
  if (wav->begin(file, out))
  {
    Serial.println("Playback started");
  }
  else
  {
    Serial.println("Failed to start playback");
    delete file;
    delete wav;
    file = nullptr;
    wav = nullptr;
  }
}

void stopRecording()
{
  isRecording = false;
  Serial.println("Recording stopped.");
  playAudio();
}

void startRecording()
{
  isRecording = true;
  File recording = SD.open("/recording.wav", FILE_WRITE);
  if (!recording)
  {
    Serial.println("Failed to open file for recording");
    isRecording = false;
    return;
  }
  Serial.println("Recording started.");

  // Write WAV header
  writeWAVHeader(recording, 16000, 16, 1);
  int16_t *buffer = new int16_t[5000];
  size_t bytesRead = 0;
  while (isRecording)
  {
    i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
    recording.write((uint8_t *)buffer, bytesRead);

    if (digitalRead(BUTTON_TRIGGER) == LOW)
    {

      // Update WAV header
      updateWAVHeader(recording);
      recording.close();
      stopRecording();
    }
  }
}

void generate_tone(float frequency, int duration, float volume)
{
  const int sampleRate = 44100;
  const int samples = sampleRate * duration / 1000;
  const float amplitude = volume * 32767.0f;

  int16_t *buffer = new int16_t[samples];

  for (int i = 0; i < samples; i++)
  {
    float t = (float)i / sampleRate;
    buffer[i] = (int16_t)(sin(2 * M_PI * frequency * t) * amplitude);
  }

  size_t bytesWritten = 0;
  i2s_write(I2S_NUM_0, buffer, samples * sizeof(int16_t), &bytesWritten, portMAX_DELAY);

  delete[] buffer;

  int16_t silence[100] = {0};
  i2s_write(I2S_NUM_0, silence, sizeof(silence), &bytesWritten, portMAX_DELAY);
}

void printWavHeader(File &file)
{
  if (file)
  {
    // WAV file header structure
    struct WavHeader
    {
      char riff[4];              // "RIFF"
      uint32_t overall_size;     // File size - 8
      char wave[4];              // "WAVE"
      char fmt_chunk_marker[4];  // "fmt "
      uint32_t length_of_fmt;    // Length of format data
      uint16_t format_type;      // Format type (1=PCM)
      uint16_t channels;         // Number of channels
      uint32_t sample_rate;      // Sample rate
      uint32_t byterate;         // Byte rate
      uint16_t block_align;      // Block align
      uint16_t bits_per_sample;  // Bits per sample
      char data_chunk_header[4]; // "data"
      uint32_t data_size;        // Size of data section
    } header;

    file.read((uint8_t *)&header, sizeof(WavHeader));

    Serial.print("RIFF header: ");
    Serial.write(header.riff, 4);
    Serial.println();
    Serial.print("Overall size: ");
    Serial.println(header.overall_size);
    Serial.print("WAVE header: ");
    Serial.write(header.wave, 4);
    Serial.println();
    Serial.print("FMT header: ");
    Serial.write(header.fmt_chunk_marker, 4);
    Serial.println();
    Serial.print("Length of FMT: ");
    Serial.println(header.length_of_fmt);
    Serial.print("Format type: ");
    Serial.println(header.format_type);
    Serial.print("Number of channels: ");
    Serial.println(header.channels);
    Serial.print("Sample rate: ");
    Serial.println(header.sample_rate);
    Serial.print("Byte rate: ");
    Serial.println(header.byterate);
    Serial.print("Block align: ");
    Serial.println(header.block_align);
    Serial.print("Bits per sample: ");
    Serial.println(header.bits_per_sample);
    Serial.print("Data header: ");
    Serial.write(header.data_chunk_header, 4);
    Serial.println();
    Serial.print("Data size: ");
    Serial.println(header.data_size);
  }
}

void printDirectory(File dir, int numTabs)
{
  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      break; // No more files
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void setup()
{
  Serial.begin(115200);

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS))
  {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // List all files on the SD card
  File root = SD.open("/");
  printDirectory(root, 0);

  pinMode(BUTTON_TRIGGER, INPUT_PULLUP);
  sdloaded = true;

  i2s_playback_setup();
  out = new AudioOutputI2S(0, 1);
  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  out->SetGain(0.2); // Adjust volume (0.0 - 1.0)

  // Initialize I2S microphone
  // i2s_mic_setup();
  // Initialize I2S audio output
  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  out->SetGain(0.2); // Adjust volume (0.0 - 1.0)

  File soundfile = SD.open("/recording.wav");
  printWavHeader(soundfile);

  // Initialize audio file source
  file = new AudioFileSourceSD("/recording.wav"); // Make sure the file exists on the SD card

  // Initialize audio generator
  wav = new AudioGeneratorWAV();

  if (wav->begin(file, out))
  {
    Serial.println("Playback started.");
  }
  else
  {
    Serial.println("Failed to start playback.");
  }
}

void loop()
{
  if (!sdloaded)
  {
    return;
  }
  if (digitalRead(BUTTON_TRIGGER) == LOW)
  {
    delay(50); // Simple debounce
    if (digitalRead(BUTTON_TRIGGER) == LOW)
    {
      if (isRecording)
      {
        stopRecording();
      }
      else
      {
        startRecording();
      }
    }
  }

  if (wav && wav->isRunning())
  {
    if (!wav->loop())
    {
      wav->stop();
      Serial.println("Playback stopped.");
    }
  }
}

#endif