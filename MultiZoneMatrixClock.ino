#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EEPROM.h>
#include <time.h>

#include "config.h"
#include "timezone.h"
#include "display.h"
#include "webserver.h"

// Global objects
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
ESP8266WebServer server(80);

TimezoneManager tzManager;
DisplayManager displayManager;
Settings settings;
WebServerManager webManager(&server, &tzManager, &settings, &displayManager);

void saveSettings() {
  settings.magic = EEPROM_MAGIC;
  for (int i = 0; i < TZ_COUNT; i++) {
    settings.enabled[i] = tzManager.tzEnabled[i] ? 1 : 0;
  }
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void loadSettings() {
  EEPROM.get(0, settings);
  if (settings.magic != EEPROM_MAGIC) {
    // Default settings
    settings.intensity = 5;
    settings.use12Hour = 0;
    settings.debugEnabled = 0;
    settings.timeDisplayDuration = 10; // Default 10 seconds
    for (int i = 0; i < TZ_COUNT; i++) {
      settings.enabled[i] = tzManager.tzEnabled[i] ? 1 : 0;
    }
    saveSettings();
  } else {
    // Load enabled timezones from EEPROM
    for (int i = 0; i < TZ_COUNT; i++) {
      tzManager.tzEnabled[i] = settings.enabled[i] != 0;
    }
    // Initialize defaults for new fields if they're 0 (first time loading after update)
    if (settings.timeDisplayDuration == 0) {
      settings.timeDisplayDuration = 10;
    }
  }
  // Update display manager with time display duration
  displayManager.setTimeDisplayDuration(settings.timeDisplayDuration * 1000); // Convert to milliseconds
}

String getShortTime() {
  String full = timeClient.getFormattedTime(); // HH:MM:SS
  String hh = full.substring(0, 2);
  String mm = full.substring(3, 5);
  int hour = hh.toInt();
  int minute = mm.toInt();
  
  if (settings.use12Hour) {
    bool isPM = hour >= 12;
    hour = hour % 12;
    if (hour == 0) hour = 12;
    // Format with zero padding
    String hourStr = hour < 10 ? "0" + String(hour) : String(hour);
    String minuteStr = minute < 10 ? "0" + String(minute) : String(minute);
    return hourStr + ":" + minuteStr + (isPM ? " PM" : " AM");
  } else {
    // Already padded from getFormattedTime, but ensure it's correct
    return hh + ":" + mm;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize timezone manager
  tzManager.init();
  
  // Load settings from EEPROM (this will initialize debug flag)
  loadSettings();
  
  // Only log if debug is enabled
  if (settings.debugEnabled) {
    Serial.println("Multi-Zone Matrix Clock Starting...");
  }
  
  // Initialize display
  displayManager.begin(settings.intensity);
  displayManager.setShowTimezone(tzManager.getEnabledCount() > 1);
  
  // Connect to WiFi
  WiFiManager wm;
  wm.autoConnect("MultiZoneClock");
  
  if (settings.debugEnabled) {
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  // Initialize NTP client
  timeClient.begin();
  
  // Find first enabled timezone
  int firstTZ = tzManager.nextEnabledTZ(-1);
  if (firstTZ >= 0) {
    displayManager.setCurrentTZ(firstTZ);
    long offset = tzManager.getCurrentOffset(firstTZ);
    timeClient.setTimeOffset(offset);
  }
  
  // Initialize web server
  webManager.begin();
  
  // Start with first timezone display
  int enabledCount = tzManager.getEnabledCount();
  if (enabledCount > 0) {
    if (enabledCount > 1 && displayManager.shouldShowTimezone()) {
      displayManager.setState(SHOW_TZ_SCROLL);
      String tzName = String(tzManager.getTimezoneName(firstTZ));
      displayManager.setTimezoneName(tzName); // This will start the scroll
    } else {
      // Single timezone mode - go straight to time
      String timeStr = getShortTime();
      displayManager.setTimeString(timeStr);
      displayManager.setState(SHOW_TIME_STATIC);
    }
  }
  
  if (settings.debugEnabled) {
    Serial.println("Setup complete!");
  }
}

void loop() {
  server.handleClient();
  
  // Update NTP client periodically (it's smart enough to only sync when needed)
  static unsigned long lastNTPUpdate = 0;
  if (millis() - lastNTPUpdate >= NTP_UPDATE_INTERVAL) {
    timeClient.update();
    lastNTPUpdate = millis();
  }
  
  // Update time offset only when timezone changes (for DST and timezone switching)
  static int lastTZ = -1;
  int currentTZ = displayManager.getCurrentTZ();
  if (currentTZ != lastTZ) {
    long currentOffset = tzManager.getCurrentOffset(currentTZ);
    timeClient.setTimeOffset(currentOffset);
    lastTZ = currentTZ;
  }
  
  DisplayState state = displayManager.getState();
  
  switch (state) {
    case SHOW_TZ_SCROLL:
      displayManager.update();
      if (displayManager.getState() == SHOW_TZ_WAIT) {
        // Prepare time string
        String timeStr = getShortTime();
        displayManager.setTimeString(timeStr);
      }
      break;
      
    case SHOW_TZ_WAIT:
      displayManager.update();
      break;
      
    case SHOW_TIME_LTR:
      displayManager.update();
      break;
      
    case SHOW_TIME_RTL:
      displayManager.update();
      if (displayManager.getState() == SHOW_TZ_SCROLL) {
        // Move to next timezone
        int nextTZ = tzManager.nextEnabledTZ(currentTZ);
        displayManager.setCurrentTZ(nextTZ);
        
        // Update timezone display setting based on count
        int enabledCount = tzManager.getEnabledCount();
        displayManager.setShowTimezone(enabledCount > 1);
        
        if (displayManager.shouldShowTimezone()) {
          String tzName = String(tzManager.getTimezoneName(nextTZ));
          displayManager.setTimezoneName(tzName);
        } else {
          // Single timezone mode - skip timezone name, go straight to time
          displayManager.setTimezoneName("");
          String timeStr = getShortTime();
          displayManager.setTimeString(timeStr);
          displayManager.setState(SHOW_TZ_WAIT);
        }
      }
      break;
      
    case SHOW_TIME_STATIC: {
      displayManager.update();
      // Update time string periodically for single timezone mode
      int enabledCount = tzManager.getEnabledCount();
      if (enabledCount == 1) {
        // Single timezone mode - just update time every second
        static unsigned long lastTimeUpdate = 0;
        if (millis() - lastTimeUpdate >= 1000) {
          String timeStr = getShortTime();
          displayManager.setTimeString(timeStr);
          displayManager.setState(SHOW_TIME_STATIC); // Reset the state to update display
          lastTimeUpdate = millis();
        }
      } else if (millis() - displayManager.getWaitStart() >= (unsigned long)settings.timeDisplayDuration * 1000) {
        // Multiple timezones - move to next timezone
        int nextTZ = tzManager.nextEnabledTZ(currentTZ);
        displayManager.setCurrentTZ(nextTZ);
        
        displayManager.setState(SHOW_TZ_SCROLL);
        String tzName = String(tzManager.getTimezoneName(nextTZ));
        displayManager.setTimezoneName(tzName); // This will start the scroll
      }
      break;
    }
  }
  
  // Small delay to prevent overwhelming the system
  delay(10);
}
