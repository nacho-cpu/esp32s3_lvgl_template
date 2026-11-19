#include "bsp_battery.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#include "esp_log.h"

static char *TAG = "bsp_battery";


static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_chan0_handle = NULL;
static bool do_calibration1_chan0;


static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


void bsp_battery_init(void)
{

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = BSP_BATTERY_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = BSP_BATTERY_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BSP_BATTERY_ADC_CHANNEL, &config));

    //-------------ADC1 Calibration Init---------------//

    do_calibration1_chan0 = example_adc_calibration_init(BSP_BATTERY_ADC_UNIT, BSP_BATTERY_ADC_CHANNEL, BSP_BATTERY_ADC_ATTEN, &adc1_cali_chan0_handle);
}


void bsp_battery_get_voltage(float *voltage, uint16_t *adc_value)
{
    int adc_raw;
    int voltage_int;
    
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BSP_BATTERY_ADC_CHANNEL, &adc_raw));
    
    // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, BSP_BATTERY_ADC_CHANNEL, adc_raw);
    if (do_calibration1_chan0)
    {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw, &voltage_int));
        *voltage = (voltage_int / 1000.0f) * 3.0; 

        if (adc_value)
        {
            *adc_value = adc_raw;
        }   
    }
}


#define BATTERY_SAMPLES 8

typedef struct {
    float voltage;
    float percent;
} battery_curve_point_t;

/* Single-cell Li-ion discharge curve, ordered from full to empty */
static const battery_curve_point_t s_li_ion_curve[] = {
    { 4.20f, 100.0f },
    { 4.10f,  90.0f },
    { 4.00f,  79.0f },
    { 3.90f,  65.0f },
    { 3.80f,  50.0f },
    { 3.70f,  32.0f },
    { 3.60f,  18.0f },
    { 3.50f,   8.0f },
    { 3.30f,   0.0f },
};

static int battery_voltage_to_percent(float v)
{
    const size_t n = sizeof(s_li_ion_curve) / sizeof(s_li_ion_curve[0]);

    if (v >= s_li_ion_curve[0].voltage) {
        return 100;
    }
    if (v <= s_li_ion_curve[n - 1].voltage) {
        return 0;
    }
    for (size_t i = 1; i < n; i++) {
        const battery_curve_point_t *hi = &s_li_ion_curve[i - 1];
        const battery_curve_point_t *lo = &s_li_ion_curve[i];
        if (v >= lo->voltage) {
            float t = (v - lo->voltage) / (hi->voltage - lo->voltage);
            float pct = lo->percent + t * (hi->percent - lo->percent);
            return (int)(pct + 0.5f);
        }
    }
    return 0;
}

int bsp_battery_get_percent(float *out_voltage)
{
    float sum = 0.0f;

    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        /* bsp_battery_get_voltage() leaves *voltage untouched if the ADC is
         * not calibrated, so start from an invalid value to detect it */
        float v = -1.0f;
        bsp_battery_get_voltage(&v, NULL);
        if (v < 0.0f) {
            if (out_voltage) {
                *out_voltage = -1.0f;
            }
            return -1;
        }
        sum += v;
    }

    float avg = sum / BATTERY_SAMPLES;
    if (out_voltage) {
        *out_voltage = avg;
    }
    return battery_voltage_to_percent(avg);
}
