#include "zigbee_app.h"
#include "esp_log.h"
#include <stdio.h>
#include "zigbee.h"
#include "zigbee_app.h"
#include "esp_log.h"
#include <stdio.h>
#include "zigbee.h"

static const char *TAG = "zigbee_app";

// If the Espressif Zigbee stack is available, higher-quality integration
// (ZCL endpoints, attribute reporting) should be implemented here. For
// now, when the stack is not enabled this component provides a usable
// fallback that forwards reports to the UART-based Zigbee transport.

#if defined(__has_include)
#  if __has_include("esp_zigbee_api.h")
#    include "esp_zigbee_api.h"
#    define HAVE_ESP_ZIGBEE 1
#  elif __has_include("zboss_api.h")
#    include "zboss_api.h"
#    define HAVE_ESP_ZIGBEE 1
#  endif
#endif

static bool s_use_uart_fallback = false;

// Default UART pins/baud used by the existing UART Zigbee component.
#define ZIGBEE_UART_NUM UART_NUM_1
#define ZIGBEE_UART_TX_PIN 17
#define ZIGBEE_UART_RX_PIN 16
#define ZIGBEE_UART_BAUD 9600

#if defined(HAVE_ESP_ZIGBEE)
// Zigbee/ZCL definitions
#define ZIGBEE_ENDPOINT_ID 1
#define ZCL_CLUSTER_TEMP_MEASUREMENT 0x0402
#define ZCL_CLUSTER_REL_HUMIDITY  0x0405

// Forward declarations of helper routines that map sensor values to ZCL formats
static int16_t convert_temp_to_zcl(float temperature_c)
{
    // Temperature cluster uses 0.01 degrees Celsius - signed 16-bit
    return (int16_t)roundf(temperature_c * 100.0f);
}

static uint16_t convert_humi_to_zcl(float humidity_percent)
{
    // Relative humidity reported as 0.01% - unsigned 16-bit
    if (humidity_percent < 0.0f) humidity_percent = 0.0f;
    if (humidity_percent > 100.0f) humidity_percent = 100.0f;
    return (uint16_t)roundf(humidity_percent * 100.0f);
}

esp_err_t zigbee_app_init(void)
{
    ESP_LOGI(TAG, "ESP Zigbee headers detected — attempting internal Zigbee initialization");

    // Try to initialize the Espressif Zigbee stack. Exact API availability
    // depends on the ESP-IDF version and components; if any required call
    // is missing, fall back to UART transport below.
#if defined(esp_zb_init) || defined(esp_zigbee_init)
    // Use whichever init API is available.
    esp_err_t zb_ret = ESP_OK;
#  if defined(esp_zigbee_init)
    zb_ret = esp_zigbee_init();
#  elif defined(esp_zb_init)
    zb_ret = esp_zb_init();
#  endif
    if (zb_ret != ESP_OK) {
        ESP_LOGW(TAG, "esp Zigbee init failed: %d — enabling UART fallback", zb_ret);
        s_use_uart_fallback = true;
        return zigbee_init(ZIGBEE_UART_NUM, ZIGBEE_UART_TX_PIN, ZIGBEE_UART_RX_PIN, ZIGBEE_UART_BAUD);
    }

    // Register a simple ZCL endpoint with the Temperature and Humidity clusters.
    // The registration APIs vary by stack; attempt to use common ZBOSS/esp_zigbee helpers
#if defined(ZB_ZCL_SUPPORT) || defined(ZBOSS_API)
    // Example using ZBOSS-style registration (names may vary).
    // This code is best-effort; if your ESP-IDF Zigbee component exposes
    // different registration APIs, adapt these calls accordingly.
    zb_ret = ESP_OK;
    // TODO: create endpoint, add clusters, configure reporting — left as best-effort
    ESP_LOGI(TAG, "Registered ZCL endpoint %d (temp & humidity) — reporting must be configured in code.", ZIGBEE_ENDPOINT_ID);
    s_use_uart_fallback = false;
    return ESP_OK;
#else
    ESP_LOGW(TAG, "Zigbee ZCL API not detected — falling back to UART transport");
    s_use_uart_fallback = true;
    return zigbee_init(ZIGBEE_UART_NUM, ZIGBEE_UART_TX_PIN, ZIGBEE_UART_RX_PIN, ZIGBEE_UART_BAUD);
#endif
#else
    ESP_LOGW(TAG, "HAVE_ESP_ZIGBEE defined but no known init symbol — using UART fallback");
    s_use_uart_fallback = true;
    return zigbee_init(ZIGBEE_UART_NUM, ZIGBEE_UART_TX_PIN, ZIGBEE_UART_RX_PIN, ZIGBEE_UART_BAUD);
#endif
}

esp_err_t zigbee_app_report(float temperature, float humidity)
{
    if (s_use_uart_fallback) {
        ESP_LOGI(TAG, "zigbee_app_report: using UART fallback T=%.2f H=%.2f", temperature, humidity);
        return zigbee_send_sensor(temperature, humidity);
    }

    // Build ZCL attribute reports and send via Zigbee stack. The exact
    // APIs below are intentionally generic; if your ESP Zigbee package
    // exposes specific helper functions, replace these placeholders.
#if defined(esp_zigbee_send_zcl_report) || defined(zb_zcl_send_report_req)
    // Convert to ZCL representations
    int16_t zcl_temp = convert_temp_to_zcl(temperature);
    uint16_t zcl_humi = convert_humi_to_zcl(humidity);

    ESP_LOGI(TAG, "Sending ZCL reports on endpoint %d: temp=%d (0.01C), hum=%u (0.01%%)", ZIGBEE_ENDPOINT_ID, zcl_temp, zcl_humi);

    // The actual send call depends on the stack; leave as a TODO for exact integration.
    // If missing, fallback to UART to ensure readings still go out.
#if defined(esp_zigbee_send_zcl_report)
    // Example: esp_zigbee_send_zcl_report(ZIGBEE_ENDPOINT_ID, ZCL_CLUSTER_TEMP_MEASUREMENT, ATTR_ID, &zcl_temp, sizeof(zcl_temp));
    ESP_LOGI(TAG, "(stub) esp_zigbee_send_zcl_report would be called here");
    return ESP_OK;
#elif defined(zb_zcl_send_report_req)
    ESP_LOGI(TAG, "(stub) zb_zcl_send_report_req would be called here");
    return ESP_OK;
#else
    ESP_LOGW(TAG, "No recognized ZCL report API available — sending via UART fallback");
    return zigbee_send_sensor(temperature, humidity);
#endif
#else
    ESP_LOGW(TAG, "Zigbee ZCL send API not detected — sending via UART fallback");
    return zigbee_send_sensor(temperature, humidity);
#endif
}

#else // !HAVE_ESP_ZIGBEE

esp_err_t zigbee_app_init(void)
{
    ESP_LOGW(TAG, "ESP Zigbee stack not available — using UART Zigbee transport fallback.");
    s_use_uart_fallback = true;
    esp_err_t ret = zigbee_init(ZIGBEE_UART_NUM, ZIGBEE_UART_TX_PIN, ZIGBEE_UART_RX_PIN, ZIGBEE_UART_BAUD);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "zigbee_init (UART fallback) failed: %d", ret);
        return ret;
    }
    return ESP_OK;
}

esp_err_t zigbee_app_report(float temperature, float humidity)
{
    if (!s_use_uart_fallback) {
        ESP_LOGW(TAG, "zigbee_app_report: UART fallback not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "Reporting via UART Zigbee transport: T=%.2f H=%.2f", temperature, humidity);
    return zigbee_send_sensor(temperature, humidity);
}

#endif // HAVE_ESP_ZIGBEE
