#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include "bthome.h"
#include "sensors.h"

#define IMP_PER_KWH 1000UL

static bthome_data_array_t bthome_data;
static float energy = 0.0;
static float power = 0.0;
static int64_t last_pulse = 0;

void energy_cb()
{
    int64_t pulse_delta = k_uptime_get() - last_pulse;

    energy += 0.001f;
    power = (float)((3600000000ULL) / IMP_PER_KWH) / (float)pulse_delta;

    printk("[%08d] LED pulse -> energy=%d Wh, power=%d W\n",
           k_uptime_get_32(),
           (int)(energy * 1000),
           (int)power);

    struct batt_t batt;
    sensors_read_battery(&batt);

    bthome_insert_int(&bthome_data, ID_BATTERY, (int)(batt.percentage * 100));
    bthome_insert_float(&bthome_data, ID_ENERGY, energy);

    if (last_pulse > 0)
        bthome_insert_float(&bthome_data, ID_POWER, power);

    bthome_update_advertise(&bthome_data);

    last_pulse = k_uptime_get();
}

int main()
{
    bthome_init(&bthome_data);

    sensors_init_battery();
    sensors_init_energy(energy_cb);

    // printk("Starting BTHome sensor\n");
    printk("Starting BTHome sensor '%s'\n", CONFIG_BT_DEVICE_NAME);

    /* Start advertising */
    bthome_start_advertise(&bthome_data, BTHOME_NO_ENCRYPT_TRIGGER_BASE);

    return 0;
}