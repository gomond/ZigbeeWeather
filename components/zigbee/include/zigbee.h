#ifndef ZIGBEE_H
#define ZIGBEE_H

#include "esp_err.h"
#include "driver/uart.h"

// Initialize UART transport to external Zigbee radio
esp_err_t zigbee_init(uart_port_t uart_num, int tx_pin, int rx_pin, int baud);

// Set 64-bit destination address for outgoing XBee API frames
void zigbee_set_dest64(const uint8_t dest64[8]);

// Send a sensor reading as a small JSON payload over the Zigbee transport
// Uses XBee API Transmit Request (0x10) to send to `dest64` address.
esp_err_t zigbee_send_sensor(float temperature, float humidity);

// Send raw XBee API Transmit Request with provided payload
esp_err_t zigbee_send_xbee_transmit(const uint8_t dest64[8], const uint8_t *data, size_t len);

#endif // ZIGBEE_H
