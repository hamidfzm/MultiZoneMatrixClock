#ifndef TIMEZONE_H
#define TIMEZONE_H

#include "config.h"
#include <Arduino.h>

struct TimezoneInfo {
  const char* name;
  long standardOffset;  // Offset in seconds (without DST)
  bool usesDST;         // Whether this timezone uses daylight saving
};

class TimezoneManager {
public:
  TimezoneManager();
  
  void init();
  long getCurrentOffset(int tzIndex);
  const char* getTimezoneName(int index);
  bool isDST(int tzIndex);
  int getEnabledCount();
  int nextEnabledTZ(int start);
  
  bool tzEnabled[TZ_COUNT];
  
private:
  bool isDSTActive(); // Check if DST is currently active (US rules)
  TimezoneInfo timezones[TZ_COUNT];
};

#endif
