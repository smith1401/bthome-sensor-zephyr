#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

/* BATTERY */
typedef struct {
  // High (h) and low (p) voltage (v) and % (p) points.
  float vh, vl, ph, pl;
} batt_disch_linear_section_t;

typedef struct {
  int16_t raw;
  int32_t millivolts;
  float voltage;
} adc_read_t;

struct batt_t {
  adc_read_t adc_read;
  float percentage;
};

int sensors_init_battery();
void sensors_read_battery(struct batt_t *out);
/* BATTERY */

/* ENERGY */
typedef void (*energy_pulse_callback_t)(void);

int sensors_init_energy(energy_pulse_callback_t cb);
/* ENERGY */

/* TEMPERATURE INTERNAL */
int sensors_read_temperature_internal(struct sensor_value *value);
/* TEMPERATURE INTERNAL */

/* TEMPERATURE DS18B20 */
int sensors_read_temperature_ds18b20(struct sensor_value *value);
/* TEMPERATURE DS18B20 */

/* TEMPERATURE/HUMIDITY AHT10 */
int sensors_read_temperature_humidity_aht10(struct sensor_value *temperature,
                                            struct sensor_value *humidity);
/* TEMPERATURE/HUMIDITY AHT10 */

/* TEMPERATURE/PRESSURE BMP280 */
int sensors_read_temperature_pressure_bmp280(struct sensor_value *temperature,
                                            struct sensor_value *pressure);
/* TEMPERATURE/PRESSURE BMP280 */

/* LIGHT BH1750 */
int sensors_read_light_bh1750(struct sensor_value *light);
/* LIGHT BH1750 */

#endif // __SENSORS_H__