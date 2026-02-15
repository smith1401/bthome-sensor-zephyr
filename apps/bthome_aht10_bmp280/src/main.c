#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#include "bthome.h"
#include "sensors.h"

static bthome_data_array_t bthome_data;

int main() {
  bthome_init(&bthome_data);

  sensors_init_battery();

  printk("Starting BTHome sensor '%s'\n", CONFIG_BT_DEVICE_NAME);

  /* Start advertising */
  bthome_start_advertise(&bthome_data, BTHOME_NO_ENCRYPT_TRIGGER_BASE);

  for (;;) {
    struct batt_t batt = {0};
    struct sensor_value temperature = {0};
    struct sensor_value humidity = {0};
    struct sensor_value pressure = {0};

    uint32_t start = k_uptime_get_32();

    sensors_read_battery(&batt);
    bthome_insert_int(&bthome_data, ID_BATTERY, (int)(batt.percentage * 100));
    // bthome_insert_float(&bthome_data, ID_VOLTAGE, batt.adc_read.voltage);

    if (sensors_read_temperature_humidity_aht10(&temperature, &humidity) == 0) {

      bthome_insert_float(&bthome_data, ID_TEMPERATURE_PRECISE,
                          sensor_value_to_float(&temperature));
      bthome_insert_float(&bthome_data, ID_HUMIDITY_PRECISE,
                          sensor_value_to_float(&humidity));
    }

    if (sensors_read_temperature_pressure_bmp280(&temperature, &pressure) == 0) {

      // bthome_insert_float(&bthome_data, ID_TEMPERATURE_PRECISE,
      //                     sensor_value_to_float(&temperature));
      bthome_insert_float(&bthome_data, ID_PRESSURE,
                          sensor_value_to_float(&pressure) * 10.0F);
    }

    // bthome_insert_float(&bthome_data, ID_PRESSURE,
    //                       sensor_value_to_float(&pressure));

    printk(
        "[%08d] batt = %3d%% temperature = %2d.%02dC humidity = %2d.%02d%% pressure =  %2d.%02d\n",
        k_uptime_get_32(), (int)(batt.percentage * 100), temperature.val1,
        temperature.val2 / 10000, humidity.val1, humidity.val2 / 10000,
        pressure.val1, pressure.val2 / 10000);

    bthome_update_advertise(&bthome_data);

    uint32_t stop = k_uptime_get_32();
    k_msleep(CONFIG_BTHOME_ADVERTISING_INTERVAL_MS - (stop - start));
  }

  bthome_free(&bthome_data);

  return 0;
}