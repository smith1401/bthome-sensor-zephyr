#ifndef __BTHOME_H__
#define __BTHOME_H__

#include <stdlib.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/kernel.h>

#define BLE_ADVERT_MAX_LEN 31
#define MEASUREMENT_MAX_LEN                                                    \
  23 // 23=31(BLE_ADVERT_MAX_LEN)-3(FLAG)-1(SERVICE_DATA)-2(UUID)-1(ENCRYPT)-1(serviceData
     // length bit)
#define BIND_KEY_LEN 16
#define NONCE_LEN 13
#define MIC_LEN 4

#define BTHOME_GAP_ADV_SLOW_INT_MIN 0x3e80 /* 10 s      */
#define BTHOME_GAP_ADV_SLOW_INT_MAX 0x4000 /* 10.24 s    */

#define ADV_PARAM                                                              \
  BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, BT_GAP_ADV_SLOW_INT_MIN,         \
                  BT_GAP_ADV_SLOW_INT_MAX, NULL)

#define ADV_PARAM_BTHOME(min, max)                                             \
  BT_LE_ADV_PARAM(BT_LE_ADV_OPT_USE_IDENTITY, min, max, NULL)

#define FLAG 0x020106
#define FLAG1 0x02
#define FLAG2 0x01
#define FLAG3 0x06
#define BTHOME_UUID 0xFCD2
#define BTHOME_UUID1 0xD2
#define BTHOME_UUID2 0xFC
#define BTHOME_SERVICE_DATA 0x16

#define BTHOME_NO_ENCRYPT 0x40
#define BTHOME_NO_ENCRYPT_TRIGGER_BASE 0x44
#define BTHOME_ENCRYPT 0x41
#define BTHOME_ENCRYPT_TRIGGER_BASE 0x45

#define SHORT_NAME 0x08
#define COMPLETE_NAME 0x09

#define ID_PACKET 0x00
#define ID_BATTERY 0x01
#define ID_CO2 0x12
#define ID_COUNT 0x09
#define ID_COUNT2 0x3D
#define ID_COUNT4 0x3E
#define ID_CURRENT 0x43
#define ID_DEWPOINT 0x08
#define ID_DISTANCE 0x40
#define ID_DISTANCEM 0x41
#define ID_DURATION 0x42
#define ID_ENERGY 0x0A
#define ID_ENERGY4 0x4D
#define ID_GAS 0x4B
#define ID_GAS4 0x4C
#define ID_HUMIDITY 0x2E
#define ID_HUMIDITY_PRECISE 0x03
#define ID_ILLUMINANCE 0x05
#define ID_MASS 0x06
#define ID_MASSLB 0x07
#define ID_MOISTURE 0x2F
#define ID_MOISTURE_PRECISE 0x14
#define ID_PM25 0x0D
#define ID_PM10 0x0E
#define ID_POWER 0x0B
#define ID_PRESSURE 0x04
#define ID_ROTATION 0x3F
#define ID_SPD 0x44
#define ID_TEMPERATURE_PRECISE 0x02
#define ID_TEMPERATURE 0x45
#define ID_TVOC 0x13
#define ID_VOLTAGE 0x0C
#define ID_VOLTAGE1 0x4A
#define ID_VOLUME 0x4E
#define ID_VOLUME1 0x47
#define ID_VOLUME2 0x48
#define ID_VOLUMEFR 0x49
#define ID_UV 0x46
#define ID_WATER 0x4F

#define STATE_BATTERY_LOW 0x15
#define STATE_BATTERY_CHARGING 0x16
#define STATE_CO 0x17
#define STATE_COLD 0x18
#define STATE_CONNECTIVITY 0x19
#define STATE_DOOR 0x1A
#define STATE_GARAGE_DOOR 0x1B
#define STATE_GAS_DETECTED 0x1C
#define STATE_GENERIC_BOOLEAN 0x0F
#define STATE_HEAT 0x1D
#define STATE_LIGHT 0x1E
#define STATE_LOCK 0x1F
#define STATE_MOISTURE 0x20
#define STATE_MOTION 0x21
#define STATE_MOVING 0x22
#define STATE_OCCUPANCY 0x23
#define STATE_OPENING 0x11
#define STATE_PLUG 0x24
#define STATE_POWER_ON 0x10
#define STATE_PRESENCE 0x25
#define STATE_PROBLEM 0x26
#define STATE_RUNNING 0x27
#define STATE_SAFETY 0x28
#define STATE_SMOKE 0x29
#define STATE_SOUND 0x2A
#define STATE_TAMPER 0x2B
#define STATE_VIBRATION 0x2C
#define STATE_WINDOW 0x2D

#define STATE_ON 0x01
#define STATE_OFF 0x00

#define EVENT_BUTTON 0x3A
#define EVENT_BUTTON_NONE 0x00
#define EVENT_BUTTON_PRESS 0x01
#define EVENT_BUTTON_DOUBLE_PRESS 0x02
#define EVENT_BUTTON_TRIPLE_PRESS 0x03
#define EVENT_BUTTON_LONG_PRESS 0x04
#define EVENT_BUTTON_LONG_DOUBLE_PRESS 0x05
#define EVENT_BUTTTON_LONG_TRIPLE_PRESS 0x06

#define EVENT_DIMMER 0x3C
#define EVENT_DIMMER_NONE 0x00
#define EVENT_DIMMER_LEFT 0x01
#define EVENT_DIMMER_RIGHT 0x02

#define INITIAL_CAPACITY 2

typedef struct __packed {
  uint8_t id;
  uint8_t size;
  int value;
} bthome_data_t;

typedef struct {
  bthome_data_t *entries;
  int size;
  int capacity;
} bthome_data_array_t;

void bthome_init(bthome_data_array_t *arr);
void bthome_insert_float(bthome_data_array_t *arr, uint8_t id, float value);
void bthome_insert_int(bthome_data_array_t *arr, uint8_t id, int value);
void bthome_insert_button_event(bthome_data_array_t *arr, uint8_t event);
void bthome_insert_dimmer_event(bthome_data_array_t *arr, uint8_t direction,
                                uint8_t steps);
void bthome_remove(bthome_data_array_t *arr, uint8_t id);
void bthome_free(bthome_data_array_t *arr);
void bthome_debug_print(bthome_data_array_t *arr);
void bthome_start_advertise(bthome_data_array_t *arr, uint8_t bthome_mode);
void bthome_update_advertise(bthome_data_array_t *arr);

#endif // __BTHOME_H__