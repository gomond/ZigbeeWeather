#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sht20.h"
#include "zigbee.h"
#include "zigbee_app.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
// Zigbee-specific defaults and error helpers are provided by the
// esp-zigbee components when available; those headers are included
// conditionally in the Zigbee component code. Remove direct includes
// here to avoid missing-header build failures on setups without them.

static const char *TAG = "zigbee_weather";

// Default I2C pins (change if your board uses different pins)
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define I2C_PORT I2C_NUM_0
#define SHT20_ADDR 0x40

// Default UART pins for external Zigbee radio (change if needed)
#define ZIGBEE_UART UART_NUM_1
#define ZIGBEE_TX_PIN 17
#define ZIGBEE_RX_PIN 16
#define ZIGBEE_BAUD 9600

void app_main()
{
    esp_err_t ret;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = 100000},
    };
    ret = i2c_param_config(I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %d", ret);
        return;
    }
    ret = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %d", ret);
        return;
    }

    ESP_LOGI(TAG, "Starting SHT20 reads on I2C%d SDA=%d SCL=%d", I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize Zigbee stack app (preferred). If not available, fall back to UART module.
    bool have_zb = false;
    ret = zigbee_app_init();
    if (ret == ESP_OK) {
        have_zb = true;
    } else {
        ESP_LOGI(TAG, "zigbee_app_init returned %d, using UART fallback", ret);
        ret = zigbee_init(ZIGBEE_UART, ZIGBEE_TX_PIN, ZIGBEE_RX_PIN, ZIGBEE_BAUD);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Zigbee UART init failed: %d", ret);
        }
    }

    while (1) {
        float temperature = 0.0f, humidity = 0.0f;
        ret = sht20_read_temp_humi(I2C_PORT, SHT20_ADDR, &temperature, &humidity);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.2f C, Humidity: %.2f %%", temperature, humidity);
            if (have_zb) {
                esp_err_t zret = zigbee_app_report(temperature, humidity);
                if (zret != ESP_OK) {
                    ESP_LOGW(TAG, "zigbee_app_report failed: %d", zret);
                }
            } else {
                esp_err_t zret = zigbee_send_sensor(temperature, humidity);
                if (zret != ESP_OK) {
                    ESP_LOGW(TAG, "Zigbee UART send failed: %d", zret);
                }
            }
        } else {
            ESP_LOGW(TAG, "Failed to read SHT20: %d", ret);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
