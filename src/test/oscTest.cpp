#ifdef OSC_TEST
#include <Arduino.h>

#include <driver/i2s_std.h>

#define I2S_SAMPLE_RATE 16000
#define I2S_SAMPLE_BITS I2S_DATA_BIT_WIDTH_16BIT
#define I2S_DMA_BUF_LEN 1024
#define I2S_DMA_BUF_COUNT 4

#define MIC_BCLK 14
#define MIC_WS 15
#define MIC_SD 17

i2s_chan_handle_t rx_chan;

void setup()
{
    Serial.begin(921600);
    delay(1000);
    Serial.println("Starting barebones I2S mic test...");

    // Configure I2S channel
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_chan_handle_t dummy_tx_chan;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &dummy_tx_chan, &rx_chan));

    // Configure I2S standard mode
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = {
            .data_bit_width = I2S_SAMPLE_BITS,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .ws_width = I2S_SAMPLE_BITS,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = false,
            .big_endian = false,
            .bit_order_lsb = false,

        },
        .gpio_cfg = {
            .mclk = GPIO_NUM_0, // Or try GPIO_NUM_1
            .bclk = (gpio_num_t)MIC_BCLK,
            .ws = (gpio_num_t)MIC_WS,
            .dout = GPIO_NUM_NC,
            .din = (gpio_num_t)MIC_SD,
        },
    };
    // std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT; // Already set in initializer

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
}

void loop()
{
    int32_t buffer[256]; // 256 samples (4 bytes/sample)
    size_t bytes_read = 0;

    esp_err_t res = i2s_channel_read(rx_chan, buffer, sizeof(buffer), &bytes_read, 100);

    if (res == ESP_OK && bytes_read > 0)
    {
        Serial.print("Read bytes: ");
        Serial.print(bytes_read);
        Serial.print(" | Sample[0]: ");
        Serial.println(buffer[0]);
    }
    else
    {
        Serial.println("No data or read error");
    }

    delay(100);
}

#endif // MIC_TEST