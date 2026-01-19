#ifndef SHT20_H
#define SHT20_H

#include "esp_err.h"
#include "driver/i2c.h"

/**
 * Read temperature and humidity from SHT20 (SHT2x family) sensor.
 * @param i2c_num I2C port (e.g., I2C_NUM_0)
 * @param addr I2C address (0x40)
 * @param temperature out: degrees Celsius
 * @param humidity out: relative humidity percent
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t sht20_read_temp_humi(i2c_port_t i2c_num, uint8_t addr, float *temperature, float *humidity);

#endif // SHT20_H
