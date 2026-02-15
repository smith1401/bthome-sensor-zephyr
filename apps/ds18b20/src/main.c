/*
 * Copyright (c) 2022 Caspar Friedrich <c.s.w.friedrich@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/w1.h>
#include <zephyr/drivers/sensor.h>
#include "sensors.h"

void w1_search_callback(struct w1_rom rom, void *user_data)
{
	printk("Device found; family: 0x%02x, serial: 0x%016llx\n", rom.family,
		   w1_rom_to_uint64(&rom));
}

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(w1));

	if (!device_is_ready(dev))
	{
		printk("Device not ready\n");
		return 0;
	}

	int num_devices = w1_search_rom(dev, w1_search_callback, NULL);

	printk("Number of devices found on bus: %d\n", num_devices);

	for (;;)
	{
		struct sensor_value temperature = {0};
		sensors_read_temperature_ds18b20(&temperature);
		printk("[%08d] Temperature=%d.%d\n", k_uptime_get_32(), temperature.val1, temperature.val2);
	}
	return 0;
}