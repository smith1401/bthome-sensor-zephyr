#include "bthome.h"

#include <nrfx_clock.h>

K_MUTEX_DEFINE(mtx);

static uint8_t get_data_len(uint8_t id) {
  switch (id) {
  case ID_PACKET:
  case ID_BATTERY:
  case ID_COUNT:
  case ID_HUMIDITY:
  case ID_MOISTURE:
  case ID_UV:
  case STATE_BATTERY_LOW:
  case STATE_BATTERY_CHARGING:
  case STATE_CO:
  case STATE_COLD:
  case STATE_CONNECTIVITY:
  case STATE_DOOR:
  case STATE_GARAGE_DOOR:
  case STATE_GAS_DETECTED:
  case STATE_GENERIC_BOOLEAN:
  case STATE_HEAT:
  case STATE_LIGHT:
  case STATE_LOCK:
  case STATE_MOISTURE:
  case STATE_MOTION:
  case STATE_MOVING:
  case STATE_OCCUPANCY:
  case STATE_OPENING:
  case STATE_PLUG:
  case STATE_POWER_ON:
  case STATE_PRESENCE:
  case STATE_PROBLEM:
  case STATE_RUNNING:
  case STATE_SAFETY:
  case STATE_SMOKE:
  case STATE_SOUND:
  case STATE_TAMPER:
  case STATE_VIBRATION:
  case STATE_WINDOW:
  case EVENT_BUTTON:
    return 1;
    break;
  case ID_DURATION:
  case ID_ENERGY:
  case ID_GAS:
  case ID_ILLUMINANCE:
  case ID_POWER:
  case ID_PRESSURE:
    return 3;
    break;
  case ID_COUNT4:
  case ID_ENERGY4:
  case ID_GAS4:
  case ID_VOLUME:
  case ID_WATER:
    return 4;
    break;
  default:
    return 2;
  }
}

static uint16_t get_data_factor(uint8_t id) {
  switch (id) {
  case ID_DISTANCEM:
  case ID_ROTATION:
  case ID_TEMPERATURE:
  case ID_VOLTAGE1:
  case ID_VOLUME1:
  case ID_UV:
    return 10;
    break;
  case ID_DEWPOINT:
  case ID_HUMIDITY_PRECISE:
  case ID_ILLUMINANCE:
  case ID_MASS:
  case ID_MASSLB:
  case ID_MOISTURE_PRECISE:
  case ID_POWER:
  case ID_PRESSURE:
  case ID_SPD:
  case ID_TEMPERATURE_PRECISE:
    return 100;
    break;
  case ID_CURRENT:
  case ID_DURATION:
  case ID_ENERGY:
  case ID_ENERGY4:
  case ID_GAS:
  case ID_GAS4:
  case ID_VOLTAGE:
  case ID_VOLUME:
  case ID_VOLUMEFR:
  case ID_WATER:
    return 1000;
    break;
  default:
    return 1;
  }
}

static void bthome_resize(bthome_data_array_t *arr, size_t new_capacity) {
  // size_t new_capacity = arr->capacity + 2;
  bthome_data_t *new_entries = (bthome_data_t *)realloc(
      arr->entries, new_capacity * sizeof(bthome_data_t));
  if (new_entries == NULL) {
    printk("Failed to resize bthome entries\r\n");
  }

  arr->entries = new_entries;
  arr->capacity = new_capacity;
}

static int compare_by_id(const void *a, const void *b) {
  bthome_data_t *entryA = (bthome_data_t *)a;
  bthome_data_t *entryB = (bthome_data_t *)b;

  return (entryA->id - entryB->id); // Sort in ascending order
}

static void arr_sort_by_id(bthome_data_array_t *arr) {
  qsort(arr->entries, arr->size, sizeof(bthome_data_t), compare_by_id);
}

static uint8_t *bthome_to_service_data(bthome_data_array_t *arr, size_t *len) {
  static uint8_t service_data[32];

  // Sort by ID as BTHome protocol requires this
  arr_sort_by_id(arr);

  // This is for 16 bit UUID and encryption flags
  *len += 3;

  // Loop over all data entries and add one byte for type and n bytes for data
  for (int i = 0; i < arr->size; i++) {
    *len += 1;
    *len += arr->entries[i].size;
  }

  uint8_t *p = service_data;

  p[0] = BTHOME_UUID1;
  p[1] = BTHOME_UUID2;
  p[2] = BTHOME_NO_ENCRYPT_TRIGGER_BASE;
  p += 3;

  for (int i = 0; i < arr->size; i++) {
    *p++ = arr->entries[i].id;
    memcpy(p, &arr->entries[i].value, arr->entries[i].size);
    p += arr->entries[i].size;
  }

  return service_data;
}

void bthome_init(bthome_data_array_t *arr) {
  arr->size = 0;
  arr->capacity = INITIAL_CAPACITY;
  arr->entries = (bthome_data_t *)malloc(arr->capacity * sizeof(bthome_data_t));
  if (arr->entries == NULL) {
    printk("Failed to initialize array\n");
  }
}

void bthome_insert_float(bthome_data_array_t *arr, uint8_t id, float value) {
  k_mutex_lock(&mtx, K_FOREVER);

  uint8_t size = get_data_len(id);
  uint16_t factor = get_data_factor(id);

  // Multiply with factor and cast to int
  int temp = value * factor;

  // Check for existing id and update if found
  for (size_t i = 0; i < arr->size; ++i) {
    if (arr->entries[i].id == id) {
      arr->entries[i].value = temp;
      k_mutex_unlock(&mtx);
      return;
    }
  }

  // Check if we need to resize
  if (arr->size == arr->capacity) {
    bthome_resize(arr, arr->capacity + 2);
  }

  // Add new entry
  arr->entries[arr->size].id = id;
  arr->entries[arr->size].size = size;
  arr->entries[arr->size].value = temp;
  ++arr->size;

  k_mutex_unlock(&mtx);
}

void bthome_insert_int(bthome_data_array_t *arr, uint8_t id, int value) {
  k_mutex_lock(&mtx, K_FOREVER);

  uint8_t size = get_data_len(id);
  uint16_t factor = get_data_factor(id);

  // Multiply with factor and cast to int
  int temp = value * factor;

  // Check for existing id and update if found
  for (size_t i = 0; i < arr->size; ++i) {
    if (arr->entries[i].id == id) {
      arr->entries[i].value = temp;
      k_mutex_unlock(&mtx);
      return;
    }
  }

  // Check if we need to resize
  if (arr->size == arr->capacity) {
    bthome_resize(arr, arr->capacity + 2);
  }

  // Add new entry
  arr->entries[arr->size].id = id;
  arr->entries[arr->size].size = size;
  arr->entries[arr->size].value = temp;
  ++arr->size;

  k_mutex_unlock(&mtx);
}

void bthome_insert_button_event(bthome_data_array_t *arr, uint8_t event) {
  k_mutex_lock(&mtx, K_FOREVER);

  // Check for existing id and update if found
  for (size_t i = 0; i < arr->size; ++i) {
    if (arr->entries[i].id == EVENT_BUTTON) {
      arr->entries[i].value = event;
      k_mutex_unlock(&mtx);
      return;
    }
  }

  // Check if we need to resize
  if (arr->size == arr->capacity) {
    bthome_resize(arr, arr->capacity + 2);
  }

  // Add new entry
  arr->entries[arr->size].id = EVENT_BUTTON;
  arr->entries[arr->size].size = 1;
  arr->entries[arr->size].value = event;
  ++arr->size;

  k_mutex_unlock(&mtx);
}

void bthome_insert_dimmer_event(bthome_data_array_t *arr, uint8_t direction,
                                uint8_t steps) {
  k_mutex_lock(&mtx, K_FOREVER);

  // Check for existing id and update if found
  for (size_t i = 0; i < arr->size; ++i) {
    if (arr->entries[i].id == EVENT_DIMMER) {
      arr->entries[i].value = (direction << 8) | steps;
      k_mutex_unlock(&mtx);
      return;
    }
  }

  // Check if we need to resize
  if (arr->size == arr->capacity) {
    bthome_resize(arr, arr->capacity + 2);
  }

  // Add new entry
  arr->entries[arr->size].id = EVENT_DIMMER;
  arr->entries[arr->size].size = 2;
  arr->entries[arr->size].value = (direction << 8) | steps;
  ++arr->size;

  k_mutex_unlock(&mtx);
}

void bthome_remove(bthome_data_array_t *arr, uint8_t id) {
  k_mutex_lock(&mtx, K_FOREVER);

  // Check for existing id and break if found
  size_t i;
  for (i = 0; i < arr->size; ++i) {
    if (arr->entries[i].id == id)
      break;
  }

  // Check if we did not find the id
  if (i == arr->size) {
    k_mutex_unlock(&mtx);
    return;
  }

  // If we found the id, move all entries by one space afterwards
  while (i < arr->size) {
    arr->entries[i] = arr->entries[i + 1];
    i++;
  }

  // bthome_resize(arr, arr->capacity - 1);

  // Subtract one from size
  --arr->size;

  k_mutex_unlock(&mtx);
}

void bthome_free(bthome_data_array_t *arr) {
  k_mutex_lock(&mtx, K_FOREVER);

  free(arr->entries);

  k_mutex_unlock(&mtx);
}

void bthome_debug_print(bthome_data_array_t *arr) {
  k_mutex_lock(&mtx, K_FOREVER);

  printk("---- BTHOME DATA ----\r\n");
  for (int i = 0; i < arr->size; i++) {
    printk("Id: 0x%02X, Value: %d\n", arr->entries[i].id,
           arr->entries[i].value);
  }
  printk("---------------------\r\n");

  k_mutex_unlock(&mtx);
}

void bthome_start_advertise(bthome_data_array_t *arr, uint8_t bthome_mode) {
  k_mutex_lock(&mtx, K_FOREVER);

  int err;

  uint8_t service_data[] = {BT_UUID_16_ENCODE(BTHOME_UUID), bthome_mode};

  struct bt_data ad[] = {
      BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
      BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
              sizeof(CONFIG_BT_DEVICE_NAME) - 1),
      BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data))};

#ifdef CONFIG_BOARD_NRF51_IBKS105
  // Write UICR section
  *(uint32_t *)0x10001008 = 0xFFFFFF00;

  // Set the external high frequency clock source to 32 MHz
  NRF_CLOCK->XTALFREQ = 0xFFFFFF00;

  // Start the external high frequency crystal
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  // Wait for the external oscillator to start up
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
  }
#endif

  /* Initialize the Bluetooth Subsystem */
  err = bt_enable(NULL);
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    k_mutex_unlock(&mtx);
    return;
  }

  /* Start advertising */
  int min = (int)(((float)CONFIG_BTHOME_ADVERTISING_INTERVAL_MS) / 0.625f);
  int max =
      (int)(((float)CONFIG_BTHOME_ADVERTISING_INTERVAL_MS + 200) / 0.625f);

  err =
      bt_le_adv_start(ADV_PARAM_BTHOME(min, max), ad, ARRAY_SIZE(ad), NULL, 0);
  if (err) {
    printk("Advertising failed to start (err %d)\n", err);
    k_mutex_unlock(&mtx);
    return;
  }

  k_mutex_unlock(&mtx);
}

void bthome_update_advertise(bthome_data_array_t *arr) {
  k_mutex_lock(&mtx, K_FOREVER);

  int err;
  size_t len = 0;

  uint8_t *service_data = bthome_to_service_data(arr, &len);

  for (size_t i = 0; i < len; i++) {
    if (i % 16 == 0) {
      printk("\n");
    }
    printk("%02X ", service_data[i]);
  }
  printk("\n");

  bthome_debug_print(arr);

  struct bt_data ad[] = {
      BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
      BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
              sizeof(CONFIG_BT_DEVICE_NAME) - 1),
      BT_DATA(BT_DATA_SVC_DATA16, service_data, len)};

  err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);

  k_mutex_unlock(&mtx);
}
