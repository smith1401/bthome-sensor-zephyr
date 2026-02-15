#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#include "bthome.h"
#include "sensors.h"

int main()
{
    bthome_data_array_t bthome_data;
    bthome_init(&bthome_data);

    sensors_init_battery();

    printk("Starting BTHome sensor '%s'\n", CONFIG_BT_DEVICE_NAME);

    /* Start advertising */
    bthome_start_advertise(&bthome_data, BTHOME_NO_ENCRYPT_TRIGGER_BASE);

    for (;;)
    {
        struct batt_t batt = {0};
        struct sensor_value temp_ds18b20 = {0};

        uint32_t start = k_uptime_get_32();

        if (sensors_read_temperature_ds18b20(&temp_ds18b20) == 0)
        {
            bthome_insert_float(&bthome_data, ID_TEMPERATURE_PRECISE, sensor_value_to_float(&temp_ds18b20));
        }

        sensors_read_battery(&batt);
        bthome_insert_int(&bthome_data, ID_BATTERY, (int)(batt.percentage * 100));

        printk("[%08d] batt = %3d%% temp_ds18b20 = %2d.%02dC\n",
               k_uptime_get_32(),
               (int)(batt.percentage * 100),
               temp_ds18b20.val1, temp_ds18b20.val2 / 10000);

        bthome_update_advertise(&bthome_data);

        uint32_t stop = k_uptime_get_32();
        k_msleep(CONFIG_BTHOME_ADVERTISING_INTERVAL_MS - (stop - start));
    }

    bthome_free(&bthome_data);

    return 0;
}