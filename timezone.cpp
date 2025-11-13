#include "timezone.h"

#include <time.h>

TimezoneManager::TimezoneManager() {
  // Initialize timezone data
  // UTC
  timezones[0] = {"UTC", 0, false};
  
  // Central Standard Time (CST) - UTC-6
  timezones[1] = {"CST", -21600, true};
  
  // Eastern Standard Time (EST) - UTC-5
  timezones[2] = {"EST", -18000, true};
  
  // Pacific Standard Time (PST) - UTC-8
  timezones[3] = {"PST", -28800, true};
  
  // Iran Standard Time (IRST) - UTC+3:30
  timezones[4] = {"IRST", 12600, false};
  
  // Extra slot (can be used for future timezones)
  timezones[5] = {"", 0, false};
  
  // Default enabled timezones
  tzEnabled[0] = false; // UTC
  tzEnabled[1] = false; // CST
  tzEnabled[2] = true;  // EST
  tzEnabled[3] = false; // PST
  tzEnabled[4] = true;  // IRST
  tzEnabled[5] = false; // Extra
}

void TimezoneManager::init() {
  // Initialization if needed
}

bool TimezoneManager::isDSTActive() {
  // US DST rules: Second Sunday in March to First Sunday in November
  // Get current UTC time
  time_t now = time(nullptr);
  if (now == 0) return false; // Time not set yet
  
  struct tm* timeinfo = gmtime(&now);
  int month = timeinfo->tm_mon + 1; // 1-12
  int day = timeinfo->tm_mday;
  int wday = timeinfo->tm_wday; // 0=Sunday, 6=Saturday
  
  // Calculate second Sunday in March
  // Find first Sunday: if March 1 is Sunday (wday=0), first Sunday is day 1
  // Otherwise, first Sunday = (8 - wday) % 7, but we need to work from March 1
  // Simplified: calculate from the current day's weekday
  // We need to know what day of week March 1 is, but we can work backwards
  
  if (month > 3 && month < 11) {
    return true; // April through October: DST active
  } else if (month == 3) {
    // March: DST starts on second Sunday
    // Calculate days since March 1
    int daysSinceMarch1 = day - 1;
    // Calculate what day of week March 1 was
    // Current wday - (daysSinceMarch1 % 7) gives us March 1's weekday
    int march1Wday = (wday - (daysSinceMarch1 % 7) + 7) % 7;
    // First Sunday is at day: (7 - march1Wday) % 7, but if march1Wday is 0, it's day 1
    int firstSunday = march1Wday == 0 ? 1 : (8 - march1Wday);
    int secondSunday = firstSunday + 7;
    return day >= secondSunday;
  } else if (month == 11) {
    // November: DST ends on first Sunday
    int daysSinceNov1 = day - 1;
    int nov1Wday = (wday - (daysSinceNov1 % 7) + 7) % 7;
    int firstSunday = nov1Wday == 0 ? 1 : (8 - nov1Wday);
    return day < firstSunday;
  }
  
  return false; // December through February: DST not active
}

bool TimezoneManager::isDST(int tzIndex) {
  if (tzIndex < 0 || tzIndex >= TZ_COUNT) return false;
  if (!timezones[tzIndex].usesDST) return false;
  return isDSTActive();
}

long TimezoneManager::getCurrentOffset(int tzIndex) {
  if (tzIndex < 0 || tzIndex >= TZ_COUNT) return 0;
  
  long offset = timezones[tzIndex].standardOffset;
  
  // Add 1 hour (3600 seconds) if DST is active
  if (isDST(tzIndex)) {
    offset += 3600;
  }
  
  return offset;
}

const char* TimezoneManager::getTimezoneName(int index) {
  if (index < 0 || index >= TZ_COUNT) return "";
  return timezones[index].name;
}

int TimezoneManager::getEnabledCount() {
  int count = 0;
  for (int i = 0; i < TZ_COUNT; i++) {
    if (tzEnabled[i] && strlen(timezones[i].name) > 0) {
      count++;
    }
  }
  return count;
}

int TimezoneManager::nextEnabledTZ(int start) {
  int enabledCount = getEnabledCount();
  if (enabledCount == 0) return start;
  
  for (int i = 1; i <= TZ_COUNT; i++) {
    int idx = (start + i) % TZ_COUNT;
    if (tzEnabled[idx] && strlen(timezones[idx].name) > 0) {
      return idx;
    }
  }
  return start;
}
