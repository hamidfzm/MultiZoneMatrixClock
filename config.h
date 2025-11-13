#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Hardware configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D8

// EEPROM configuration
#define EEPROM_SIZE 64
#define EEPROM_MAGIC 0x42

// Display timing
const unsigned long WAIT_DURATION = 2000;
const unsigned long STATIC_TIME_DURATION_DEFAULT = 10000; // Default 10 seconds in ms
const unsigned long BLINK_INTERVAL = 500; // Colon blink interval in ms
const unsigned long NTP_UPDATE_INTERVAL = 1000; // Update NTP client every second

// Timezone count
const int TZ_COUNT = 6; // UTC, CST, EST, PST, IRST, and one extra slot

// Settings structure
struct Settings {
  uint8_t magic;
  uint8_t enabled[TZ_COUNT];
  uint8_t intensity;
  uint8_t use12Hour;
  uint8_t debugEnabled;
  uint16_t timeDisplayDuration; // Duration in seconds (default 10)
};

#endif
