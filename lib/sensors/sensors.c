#include "sensors.h"

/* BATTERY */
static const struct adc_dt_spec adc_batt_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

// Shared buffer and adc_sequence.
static int16_t buf;
static struct adc_sequence sequence = {
    .buffer = &buf,
    .buffer_size = sizeof(buf),
};

static int read_adc_spec(const struct adc_dt_spec *spec, adc_read_t *out) {
  adc_sequence_init_dt(spec, &sequence);

  adc_read(spec->dev, &sequence);

  int32_t val_mv = buf;
  adc_raw_to_millivolts_dt(spec, &val_mv);
  val_mv = MAX(0, val_mv);

  out->raw = buf;
  out->millivolts = val_mv;
  out->voltage = val_mv / 1000.0f;
  return 0;
}

int sensors_init_battery() {
  int err;

  err = adc_channel_setup_dt(&adc_batt_spec);

  if (err < 0) {
    printk("Failed to setup ADC for battery (%d)\n", err);
  }

  return 0;
}

void sensors_read_battery(struct batt_t *out) {
  read_adc_spec(&adc_batt_spec, &out->adc_read);

  // Must be sorted by .vh.
  static const batt_disch_linear_section_t sections[] = {
      {.vh = 3.00f, .vl = 2.90f, .ph = 1.00f, .pl = 0.42f},
      {.vh = 2.90f, .vl = 2.74f, .ph = 0.42f, .pl = 0.18f},
      {.vh = 2.74f, .vl = 2.44f, .ph = 0.18f, .pl = 0.06f},
      {.vh = 2.44f, .vl = 2.01f, .ph = 0.06f, .pl = 0.00f},
  };

  const float v = out->adc_read.voltage;

  if (v > sections[0].vh) {
    out->percentage = 1.0f;
    return;
  }
  for (int i = 0; i < ARRAY_SIZE(sections); i++) {
    const batt_disch_linear_section_t *s = &sections[i];
    if (v > s->vl) {
      out->percentage =
          s->pl + (v - s->vl) * ((s->ph - s->pl) / (s->vh - s->vl));
      return;
    }
  }
  out->percentage = 0.0f;
  return;
}
/* BATTERY */

/* ENERGY */

static struct k_work energy_pulse_work;
static struct gpio_callback energy_isr_cb;
static energy_pulse_callback_t energy_user_cb = NULL;

static struct gpio_dt_spec energy_pd =
    GPIO_DT_SPEC_GET(DT_NODELABEL(energy_pd), gpios);

static void energy_pulse_isr(const struct device *dev, struct gpio_callback *cb,
                             uint32_t pins) {
  k_work_submit(&energy_pulse_work);
}

static void energy_pulse_cb(struct k_work *work) {
  int button_state = gpio_pin_get_dt(&energy_pd);

  if (button_state > 0) {
    energy_user_cb();
  }
}

int sensors_init_energy(energy_pulse_callback_t cb) {
  device_is_ready(energy_pd.port);
  gpio_pin_configure_dt(&energy_pd, GPIO_INPUT | energy_pd.dt_flags);

  k_work_init(&energy_pulse_work, energy_pulse_cb);
  // EDGE interrupts seem to consume more power than LEVEL ones.
  // For GPIO_INT_EDGE_BOTH: 16 uA idle.
  // For GPIO_INT_LEVEL_ACTIVE: 3 uA idle.
  // Related issue:
  // https://github.com/zephyrproject-rtos/zephyr/issues/28499
  // Apparently sense-edge-mask brings the power consumption down to
  // 3 uA for EDGE interrupts too.
  gpio_pin_interrupt_configure_dt(&energy_pd, GPIO_INT_EDGE_FALLING);
  gpio_init_callback(&energy_isr_cb, energy_pulse_isr, BIT(energy_pd.pin));
  gpio_add_callback(energy_pd.port, &energy_isr_cb);

  energy_user_cb = cb;

  return 0;
}
/* ENERGY */

/* TEMPERATURE INTERNAL */
static const struct device *const temp_sensor_internal =
    DEVICE_DT_GET_ANY(nordic_nrf_temp);

int sensors_read_temperature_internal(struct sensor_value *value) {
  int err;

  /* Fetch temperature data from the sensor */
  err = sensor_sample_fetch(temp_sensor_internal);
  if (err < 0) {
    printk("Failed to fetch temperature sample (%d)\n", err);
    goto ret;
  }

  /* Get the temperature value */
  err = sensor_channel_get(temp_sensor_internal, SENSOR_CHAN_DIE_TEMP, value);
  if (err < 0) {
    printk("Failed to get temperature channel value (%d)\n", err);
    goto ret;
  }

ret:
  return err;
}
/* TEMPERATURE INTERNAL */

#if CONFIG_W1
/* TEMPERATURE DS18B20 */
static const struct device *const temp_sensor_ds18b20 =
    DEVICE_DT_GET_ANY(maxim_ds18b20);
    // DEVICE_DT_GET_ANY(DT_ALIAS(ds18));

int sensors_read_temperature_ds18b20(struct sensor_value *value) {
  int err = 0;

  /* Fetch temperature data from the sensor */
  err = sensor_sample_fetch(temp_sensor_ds18b20);
  if (err < 0) {
    printk("Failed to fetch temperature sample (%d)\n", err);
    goto ret;
  }

  /* Get the temperature value */
  err =
      sensor_channel_get(temp_sensor_ds18b20, SENSOR_CHAN_AMBIENT_TEMP, value);
  if (err < 0) {
    printk("Failed to get temperature channel value (%d)\n", err);
    goto ret;
  }

ret:
  return err;
}
/* TEMPERATURE DS18B20 */
#endif

#if CONFIG_I2C
/* TEMPERATURE AND HUMIDITY AHT10 */
static const struct device *const sensor_aht10 =
    DEVICE_DT_GET_ANY(aosong_dht20);

int sensors_read_temperature_humidity_aht10(struct sensor_value *temperature,
                                            struct sensor_value *humidity) {
  int err = 0;

  /* Fetch temperature data from the sensor */
  err = sensor_sample_fetch(sensor_aht10);
  if (err < 0) {
    printk("Failed to fetch temperature/humidity sample (%d)\n", err);
    goto ret;
  }

  /* Get the temperature value */
  err = sensor_channel_get(sensor_aht10, SENSOR_CHAN_AMBIENT_TEMP, temperature);
  if (err < 0) {
    printk("Failed to get temperature channel value (%d)\n", err);
    goto ret;
  }

  /* Get the humidity value */
  err = sensor_channel_get(sensor_aht10, SENSOR_CHAN_HUMIDITY, humidity);
  if (err < 0) {
    printk("Failed to get humdity channel value (%d)\n", err);
    goto ret;
  }

ret:
  return err;
}

/* TEMPERATURE AND PRESSURE BMP280 */
static const struct device *const sensor_bmp280 =
    DEVICE_DT_GET_ANY(bosch_bme280);

int sensors_read_temperature_pressure_bmp280(struct sensor_value *temperature,
                                            struct sensor_value *pressure) {
  int err = 0;

  /* Fetch temperature data from the sensor */
  err = sensor_sample_fetch(sensor_bmp280);
  if (err < 0) {
    printk("Failed to fetch temperature/pressure sample (%d)\n", err);
    goto ret;
  }

  /* Get the temperature value */
  err = sensor_channel_get(sensor_bmp280, SENSOR_CHAN_AMBIENT_TEMP, temperature);
  if (err < 0) {
    printk("Failed to get temperature channel value (%d)\n", err);
    goto ret;
  }

  /* Get the pressure value */
  err = sensor_channel_get(sensor_bmp280, SENSOR_CHAN_PRESS, pressure);
  if (err < 0) {
    printk("Failed to get pressure channel value (%d)\n", err);
    goto ret;
  }

ret:
  return err;
}

/* LIGHT BH1750 */
static const struct device *const sensor_bh1750 =
    DEVICE_DT_GET_ANY(rohm_bh1750);

int sensors_read_light_bh1750(struct sensor_value *light) {
  int err = 0;

  /* Fetch light data from the sensor */
  err = sensor_sample_fetch(sensor_bh1750);
  if (err < 0) {
    printk("Failed to fetch light sample (%d)\n", err);
    goto ret;
  }

  /* Get the light value */
  err = sensor_channel_get(sensor_bh1750, SENSOR_CHAN_LIGHT, light);
  if (err < 0) {
    printk("Failed to get light channel value (%d)\n", err);
    goto ret;
  }

  ret:
    return err;
}
#endif