#ifndef __BSP_TEMP_H__
#define __BSP_TEMP_H__

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Install and enable the ESP32-S3 internal temperature sensor.
 *
 * Measurement range is configured for -10..80 degC.
 *
 * @return ESP_OK on success, or the error from the temperature_sensor driver.
 */
esp_err_t bsp_temp_init(void);

/**
 * @brief Read the chip temperature.
 *
 * @param[out] celsius Temperature in degrees Celsius.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if @p celsius is NULL,
 *         ESP_ERR_INVALID_STATE if bsp_temp_init() was not called (or failed).
 */
esp_err_t bsp_temp_get_celsius(float *celsius);

#ifdef __cplusplus
}
#endif

#endif
