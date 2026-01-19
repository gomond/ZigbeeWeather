#include "zigbee.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "zigbee";
static uart_port_t s_uart_num = UART_NUM_1;
static uint8_t s_dest64[8] = {0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF}; // default broadcast-ish

esp_err_t zigbee_init(uart_port_t uart_num, int tx_pin, int rx_pin, int baud)
{
    s_uart_num = uart_num;
    uart_config_t uart_config = {
        .baud_rate = baud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    esp_err_t ret = uart_param_config(uart_num, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed: %d", ret);
        return ret;
    }
    ret = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed: %d", ret);
        return ret;
    }
    ret = uart_driver_install(uart_num, 1024, 0, 0, NULL, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed: %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "Zigbee UART initialized on UART%d TX=%d RX=%d @%d", uart_num, tx_pin, rx_pin, baud);
    return ESP_OK;
}

esp_err_t zigbee_send_sensor(float temperature, float humidity)
{
    char buf[128];
    int len = snprintf(buf, sizeof(buf), "{\"t\":%.2f,\"h\":%.2f}\n", temperature, humidity);
    if (len <= 0) return ESP_FAIL;
    return zigbee_send_xbee_transmit(s_dest64, (const uint8_t*)buf, (size_t)len);
}

void zigbee_set_dest64(const uint8_t dest64[8])
{
    if (dest64) memcpy(s_dest64, dest64, 8);
}

static uint8_t xbee_checksum(const uint8_t *frame_data, size_t frame_len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < frame_len; ++i) sum += frame_data[i];
    return (uint8_t)(0xFF - sum);
}

esp_err_t zigbee_send_xbee_transmit(const uint8_t dest64[8], const uint8_t *data, size_t len)
{
    if (!data || len == 0) return ESP_ERR_INVALID_ARG;
    uint8_t frame_type = 0x10; // Zigbee Transmit Request
    uint8_t frame_id = 0x01; // nonzero to get status
    uint8_t dest16[2] = {0xFF, 0xFE}; // unknown 16-bit address
    uint8_t broadcast_radius = 0x00;
    uint8_t options = 0x00;

    uint8_t destbuf[8];
    if (dest64) memcpy(destbuf, dest64, 8);
    else memcpy(destbuf, s_dest64, 8);

    size_t frame_data_len = 1 + 1 + 8 + 2 + 1 + 1 + len; // type + id + dest64 + dest16 + radius + options + payload
    uint8_t *frame_data = malloc(frame_data_len);
    if (!frame_data) return ESP_ERR_NO_MEM;
    size_t idx = 0;
    frame_data[idx++] = frame_type;
    frame_data[idx++] = frame_id;
    for (int i = 0; i < 8; ++i) frame_data[idx++] = destbuf[i];
    frame_data[idx++] = dest16[0];
    frame_data[idx++] = dest16[1];
    frame_data[idx++] = broadcast_radius;
    frame_data[idx++] = options;
    memcpy(&frame_data[idx], data, len);
    idx += len;

    uint8_t checksum = xbee_checksum(frame_data, frame_data_len);

    // Build API frame: 0x7E + length(2) + frame_data + checksum
    size_t packet_len = 1 + 2 + frame_data_len + 1;
    uint8_t *packet = malloc(packet_len);
    if (!packet) { free(frame_data); return ESP_ERR_NO_MEM; }
    size_t p = 0;
    packet[p++] = 0x7E;
    packet[p++] = (uint8_t)((frame_data_len >> 8) & 0xFF);
    packet[p++] = (uint8_t)(frame_data_len & 0xFF);
    memcpy(&packet[p], frame_data, frame_data_len); p += frame_data_len;
    packet[p++] = checksum;

    int written = uart_write_bytes(s_uart_num, (const char*)packet, (size_t)p);
    free(frame_data);
    free(packet);
    if (written < 0) return ESP_FAIL;
    ESP_LOGI(TAG, "XBee TX: sent %d bytes", written);
    return ESP_OK;
}
