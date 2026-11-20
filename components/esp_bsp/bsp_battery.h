#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__
#include <stdio.h>

#define BSP_BATTERY_ADC_CHANNEL   ADC_CHANNEL_4
#define BSP_BATTERY_ADC_ATTEN     ADC_ATTEN_DB_12
#define BSP_BATTERY_ADC_UNIT      ADC_UNIT_1

#ifdef __cplusplus
extern "C" {
#endif

void bsp_battery_init(void);
void bsp_battery_get_voltage(float *voltage, uint16_t *adc_value);

/**
 * @brief Estimate the battery state of charge.
 *
 * Averages 8 consecutive voltage readings and maps them through a
 * piecewise-linear single-cell Li-ion discharge curve (4.20 V = 100 %,
 * 3.30 V = 0 %).
 *
 * @param[out] out_voltage Optional (may be NULL). Averaged battery voltage in
 *                         volts, or -1.0f when no valid reading is available.
 * @return 0..100 on success, or -1 if the ADC is not calibrated / no reading.
 */
int bsp_battery_get_percent(float *out_voltage);

#ifdef __cplusplus
}
#endif


#endif