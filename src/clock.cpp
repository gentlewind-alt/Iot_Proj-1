#include "clock.h"
#include <Arduino.h>
#include <RTClib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// List of standard time zones you want to cycle through:
const TimeZone timeZones[] = {
  {"IST", 0.0},
  {"UTC", -5.5},
  {"PST", -13.5},
  {"MST", -12.5},
  {"CST", -11.5},
  {"EST", -10.5},
  {"GMT", -5.5},
  {"CET", -4.5},
  {"EET", -3.5},
  {"MSK", -2.5},
  {"CST(China)", 2.5},
  {"JST", 3.5},
  {"AEST", 4.5},
  {"AEDT", 5.5},
  {"NZST", 6.5},
};
const int tzCount = sizeof(timeZones) / sizeof(timeZones[0]);

int selectedTimeZoneIndex = 0;

// Forward declaration
void updateClockDisplay(const DateTime& now, const TimeZone& tz);

// Update displayed time adjusted by current timezone
void showClockPage(const DateTime& nowUtc) {
  const TimeZone& tz = timeZones[selectedTimeZoneIndex];

  // Adjust UTC now by offset hours
  // DateTime supports add/subtract seconds, so convert hours to seconds
  DateTime nowIST = nowUtc + TimeSpan(19800); // IST is UTC+5.5
  DateTime adjustedTime = nowUtc + TimeSpan((int)(tz.offsetHours * 3600)); 

  updateClockDisplay(adjustedTime, tz);
}

void updateClockDisplay(const DateTime& now, const TimeZone& tz) {
  display.clearDisplay();

  // Show timezone name
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Timezone: ");
  display.print(tz.name);

  // Show time
  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
  display.setCursor(0, 10);
  display.print(timeBuf);

  // Show date
  char dateBuf[17];
  snprintf(dateBuf, sizeof(dateBuf), "Date: %02d/%02d/%04d", now.day(), now.month(), now.year());
  display.setCursor(0, 20);
  display.print(dateBuf);

  display.display();
}

// Cycle to next time zone in list
void changeTimeZone() {
  selectedTimeZoneIndex = (selectedTimeZoneIndex + 1) % tzCount;
}

// Optionally, a function to set timezone index manually:
void setTimeZoneIndex(int index) {
  if (index >= 0 && index < tzCount) {
    selectedTimeZoneIndex = index;
  }
}
