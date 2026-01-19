// Zigbee application component public API
#ifndef ZIGBEE_APP_H
#define ZIGBEE_APP_H

#include "esp_err.h"

// Initialize the Zigbee application. Uses UART fallback transport when
// an internal Zigbee stack is not present.
esp_err_t zigbee_app_init(void);

// Report a temperature (degrees C) and relative humidity (percent).
esp_err_t zigbee_app_report(float temperature_c, float humidity_percent);

#endif // ZIGBEE_APP_H
#ifndef ZIGBEE_APP_H
#define ZIGBEE_APP_H

#include "esp_err.h"

// Initialize the Espressif Zigbee stack and register a Temperature/Humidity endpoint.
// Returns ESP_OK on success, ESP_ERR_NOT_SUPPORTED if Zigbee stack not available.
esp_err_t zigbee_app_init(void);

// Report temperature (C) and humidity (%) to the Zigbee network (ZCL report).
// Returns ESP_OK on success, ESP_ERR_NOT_SUPPORTED if Zigbee stack not available.
esp_err_t zigbee_app_report(float temperature, float humidity);

#endif // ZIGBEE_APP_H
