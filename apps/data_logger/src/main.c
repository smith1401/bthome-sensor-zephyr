#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

#include "sensors.h"


int main() {

  sensors_init_battery();

  printk("Starting data logger\n");

  for (;;) {
    struct batt_t batt = {0};
    struct sensor_value temperature_bmp = {0};
    struct sensor_value temperature_aht = {0};
    struct sensor_value humidity = {0};
    struct sensor_value pressure = {0};
    struct sensor_value light = {0};

    uint32_t start = k_uptime_get_32();
    
    /* Read sensor data */
    sensors_read_battery(&batt);
    sensors_read_temperature_humidity_aht10(&temperature_aht, &humidity);
    sensors_read_temperature_pressure_bmp280(&temperature_bmp, &pressure);
    sensors_read_light_bh1750(&light);

    printk(
        "[%08d]\n"
        "\tbatt = %3d%%\n"
        "\ttemperature_bmp = %2d.%02dC\n"
        "\ttemperature_aht = %2d.%02dC\n"
        "\thumidity = %2d.%02d%%\n"
        "\tpressure =  %d.%d kPa\n"
        "\tlight =  %d.%d lux\n",
        k_uptime_get_32(), 
        (int)(batt.percentage * 100), 
        temperature_bmp.val1, temperature_bmp.val2 / 10000,
        temperature_aht.val1, temperature_aht.val2 / 10000,
        humidity.val1, humidity.val2 / 10000,
        pressure.val1, pressure.val2,
        light.val1, light.val2);

    uint32_t stop = k_uptime_get_32();
    k_msleep(CONFIG_APP_LOGGING_INTERVAL_MS - (stop - start));
  }

  return 0;
}