#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72XX.h>
#include "config.h"

enum DisplayState {
  SHOW_TZ_SCROLL,
  SHOW_TZ_WAIT,
  SHOW_TIME_LTR,
  SHOW_TIME_RTL,
  SHOW_TIME_STATIC
};

class DisplayManager {
public:
  DisplayManager();
  void begin(uint8_t intensity);
  void setIntensity(uint8_t intensity);
  void update();
  void setState(DisplayState newState);
  DisplayState getState() { return state; }
  void setCurrentTZ(int tz) { currentTZ = tz; }
  int getCurrentTZ() { return currentTZ; }
  void setTimeString(String timeStr);
  void setTimezoneName(String tzName);
  void setShowTimezone(bool show) { showTimezone = show; }
  bool shouldShowTimezone() { return showTimezone; }
  unsigned long getWaitStart() { return waitStart; }
  void setTimeDisplayDuration(unsigned long duration) { timeDisplayDuration = duration; }
  
private:
  void updateBlinkingColon();
  String formatTimeWithBlinkingColon(String timeStr);
  
  MD_Parola display;
  DisplayState state;
  int currentTZ;
  char scrollBuffer[32];
  unsigned long waitStart;
  unsigned long lastBlinkTime;
  bool colonVisible;
  String currentTimeString;
  bool showTimezone;
  unsigned long timeDisplayDuration;
};

#endif

