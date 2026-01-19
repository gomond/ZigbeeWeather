#include "sht20.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "sht20";

#define SHT20_ADDR_DEFAULT 0x40
#define SHT20_CMD_TEMP 0xF3
#define SHT20_CMD_HUMI 0xF5

static esp_err_t sht20_read_raw(i2c_port_t i2c_num, uint8_t addr, uint8_t cmd, uint16_t *out_raw)
{
    uint8_t data[3] = {0};
    esp_err_t ret = i2c_master_write_to_device(i2c_num, addr, &cmd, 1, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "write cmd 0x%02x failed: %d", cmd, ret);
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    ret = i2c_master_read_from_device(i2c_num, addr, data, 3, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "read data failed: %d", ret);
        return ret;
    }
    uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
    *out_raw = raw;
    return ESP_OK;
}

esp_err_t sht20_read_temp_humi(i2c_port_t i2c_num, uint8_t addr, float *temperature, float *humidity)
{
    if (addr == 0) addr = SHT20_ADDR_DEFAULT;
    uint16_t raw_t = 0;
    uint16_t raw_h = 0;
    esp_err_t ret = sht20_read_raw(i2c_num, addr, SHT20_CMD_TEMP, &raw_t);
    if (ret != ESP_OK) return ret;
    ret = sht20_read_raw(i2c_num, addr, SHT20_CMD_HUMI, &raw_h);
    if (ret != ESP_OK) return ret;

    *temperature = -46.85f + 175.72f * ((float)raw_t / 65536.0f);
    *humidity = -6.0f + 125.0f * ((float)raw_h / 65536.0f);
    if (*humidity < 0) *humidity = 0;
    if (*humidity > 100) *humidity = 100;
    return ESP_OK;
}
