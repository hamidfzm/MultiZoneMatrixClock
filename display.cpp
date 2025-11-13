#include "display.h"
#include "config.h"

DisplayManager::DisplayManager() : 
  display(HARDWARE_TYPE, CS_PIN, MAX_DEVICES),
  state(SHOW_TZ_SCROLL),
  currentTZ(0),
  waitStart(0),
  lastBlinkTime(0),
  colonVisible(true),
  showTimezone(true),
  timeDisplayDuration(STATIC_TIME_DURATION_DEFAULT) {
  scrollBuffer[0] = '\0';
}

void DisplayManager::begin(uint8_t intensity) {
  display.begin();
  display.setIntensity(intensity);
  display.setTextAlignment(PA_CENTER);
  display.displayClear();
}

void DisplayManager::setIntensity(uint8_t intensity) {
  display.setIntensity(intensity);
}

String DisplayManager::formatTimeWithBlinkingColon(String timeStr) {
  // Find the colon position and replace with space or colon based on blink state
  int colonPos = timeStr.indexOf(':');
  if (colonPos >= 0) {
    String result = timeStr;
    result.setCharAt(colonPos, colonVisible ? ':' : ' ');
    return result;
  }
  return timeStr;
}

void DisplayManager::updateBlinkingColon() {
  unsigned long now = millis();
  if (now - lastBlinkTime >= BLINK_INTERVAL) {
    colonVisible = !colonVisible;
    lastBlinkTime = now;
    
    // Update display if we're in static time mode
    if (state == SHOW_TIME_STATIC && currentTimeString.length() > 0) {
      String formatted = formatTimeWithBlinkingColon(currentTimeString);
      formatted.toCharArray(scrollBuffer, sizeof(scrollBuffer));
      display.displayClear();
      display.print(scrollBuffer);
    }
  }
}

void DisplayManager::setTimeString(String timeStr) {
  currentTimeString = timeStr;
  timeStr.toCharArray(scrollBuffer, sizeof(scrollBuffer));
  
  // If we're in static mode, update the display immediately
  if (state == SHOW_TIME_STATIC) {
    String formatted = formatTimeWithBlinkingColon(timeStr);
    formatted.toCharArray(scrollBuffer, sizeof(scrollBuffer));
    display.displayClear();
    display.print(scrollBuffer);
    lastBlinkTime = millis(); // Reset blink timer
  }
}

void DisplayManager::setTimezoneName(String tzName) {
  tzName.toCharArray(scrollBuffer, sizeof(scrollBuffer));
  // If we're in scroll state, start the vertical scroll animation
  if (state == SHOW_TZ_SCROLL) {
    display.displayClear();
    display.displayScroll(scrollBuffer, PA_CENTER, PA_SCROLL_UP, 50);
  }
}

void DisplayManager::setState(DisplayState newState) {
  // Only reset waitStart if we're transitioning to a new state
  if (state != newState) {
    state = newState;
    if (state == SHOW_TZ_WAIT || state == SHOW_TIME_STATIC) {
      waitStart = millis();
    }
  }
}

void DisplayManager::update() {
  // Update blinking colon
  if (state == SHOW_TIME_STATIC) {
    updateBlinkingColon();
  }
  
  switch (state) {
    case SHOW_TZ_SCROLL:
      // Check if animation is complete (displayAnimate returns true when done)
      if (display.displayAnimate()) {
        // Animation complete, transition to wait state
        waitStart = millis();
        state = SHOW_TZ_WAIT;
      }
      break;

    case SHOW_TZ_WAIT:
      if (millis() - waitStart >= WAIT_DURATION) {
        String timeStr = currentTimeString;
        timeStr.toCharArray(scrollBuffer, sizeof(scrollBuffer));

        if (strlen(scrollBuffer) > 5) {
          display.displayScroll(scrollBuffer, PA_CENTER, PA_SCROLL_LEFT, 90);
          state = SHOW_TIME_LTR;
        } else {
          display.displayClear();
          // Format with blinking colon
          String formatted = formatTimeWithBlinkingColon(timeStr);
          formatted.toCharArray(scrollBuffer, sizeof(scrollBuffer));
          display.print(scrollBuffer);
          waitStart = millis();
          lastBlinkTime = millis();
          colonVisible = true;
          state = SHOW_TIME_STATIC;
        }
      }
      break;

    case SHOW_TIME_LTR:
      if (display.displayAnimate()) {
        display.displayScroll(scrollBuffer, PA_CENTER, PA_SCROLL_RIGHT, 90);
        state = SHOW_TIME_RTL;
      }
      break;

    case SHOW_TIME_RTL:
      if (display.displayAnimate()) {
        state = SHOW_TZ_SCROLL;
      }
      break;

    case SHOW_TIME_STATIC:
      // Check if we should transition to next timezone
      if (millis() - waitStart >= timeDisplayDuration) {
        // This will be handled by main loop
      }
      // Blinking is handled in updateBlinkingColon()
      break;
  }
}

