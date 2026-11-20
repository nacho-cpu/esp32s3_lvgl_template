#include "bsp_temp.h"

#include "driver/temperature_sensor.h"
#include "esp_check.h"
#include "esp_log.h"

static const char *TAG = "bsp_temp";

static temperature_sensor_handle_t s_tsens = NULL;

esp_err_t bsp_temp_init(void)
{
    if (s_tsens != NULL) {
        return ESP_OK;
    }

    temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
    temperature_sensor_handle_t handle = NULL;
    ESP_RETURN_ON_ERROR(temperature_sensor_install(&cfg, &handle), TAG, "install");

    esp_err_t err = temperature_sensor_enable(handle);
    if (err != ESP_OK) {
        temperature_sensor_uninstall(handle);
        ESP_LOGE(TAG, "enable: %s", esp_err_to_name(err));
        return err;
    }

    s_tsens = handle;
    ESP_LOGI(TAG, "Temperature sensor enabled");
    return ESP_OK;
}

esp_err_t bsp_temp_get_celsius(float *celsius)
{
    ESP_RETURN_ON_FALSE(celsius != NULL, ESP_ERR_INVALID_ARG, TAG, "null output");
    if (s_tsens == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return temperature_sensor_get_celsius(s_tsens, celsius);
}
