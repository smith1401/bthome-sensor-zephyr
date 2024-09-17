#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/drivers/sensor.h>

/* Get a handle to the temperature sensor device */
const struct device *temp_sensor = DEVICE_DT_GET_ANY(nordic_nrf_temp);

#define SERVICE_DATA_LEN 9
#define SERVICE_UUID 0xfcd2 /* BTHome service UUID */
#define IDX_TEMPL 4			/* Index of lo byte of temp in service data*/
#define IDX_TEMPH 5			/* Index of hi byte of temp in service data*/

#define ADV_PARAM                                                        \
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, BT_GAP_ADV_SLOW_INT_MIN, \
					BT_GAP_ADV_SLOW_INT_MAX, NULL)

static uint8_t service_data[SERVICE_DATA_LEN] = {
	BT_UUID_16_ENCODE(SERVICE_UUID),
	0x40,
	0x02, /* Temperature */
	0xc4, /* Low byte */
	0x00, /* High byte */
	0x03, /* Humidity */
	0xbf, /* 50.55%  low byte*/
	0x13, /* 50.55%  high byte*/
};

static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

static int read_temperature_internal(const struct device *temp_dev)
{
	struct sensor_value temp_val;
	int err;

	/* Fetch temperature data from the sensor */
	err = sensor_sample_fetch(temp_sensor);
	if (err)
	{
		printk("Failed to fetch temperature sample (%d)\n", err);
	}

	/* Get the temperature value */
	err = sensor_channel_get(temp_sensor, SENSOR_CHAN_DIE_TEMP, &temp_val);
	if (err)
	{
		printk("Failed to get temperature channel value (%d)\n", err);
	}

	/* Print the temperature in Celsius */
	printk("Temperature: %.2f°C\n", sensor_value_to_double(&temp_val));

	return temp_val.val1 * 100 + temp_val.val2 / 10000;
}

static void bt_ready(int err)
{
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(ADV_PARAM, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err)
	{
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
}

int main(void)
{
	int err;
	int temp = 0;

	printk("Starting BTHome sensor template\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	for (;;)
	{
		/* Simulate temperature from 0C to 25C */
		temp = read_temperature_internal(temp_sensor);
		service_data[IDX_TEMPH] = (temp) >> 8;
		service_data[IDX_TEMPL] = (temp) & 0xff;

		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

		if (err)
		{
			printk("Failed to update advertising data (err %d)\n", err);
		}

		k_sleep(K_MSEC(BT_GAP_ADV_SLOW_INT_MIN));
	}
	return 0;
}
