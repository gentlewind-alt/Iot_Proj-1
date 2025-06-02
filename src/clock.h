#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>

// Your display instance (extern, declared elsewhere)
extern Adafruit_SSD1306 display;
// selected timezone index
extern int selectedTimeZoneIndex;
// Define the TimeZone struct fully here — this is the key fix
struct TimeZone {
  const char* name;
  float offsetHours;
};
// Declare extern timeZones array and tzCount
extern const TimeZone timeZones[];
extern const int tzCount;
extern int alarmHour;      // Alarm hour
extern int alarmMinute;    // Alarm minute
extern bool alarmEnabled;  // Alarm enabled state
extern RTC_DS1307 rtc; // RTC instance
// Function declarations
void changeTimeZone();
void setTimeZoneIndex(int index);
// Modular display functions
void showClockPage(const DateTime& nowUtc);
void toggleAlarmStatus();
void buzzerTone(uint8_t pin, uint16_t freqHz, uint16_t msDuration);
void checkAndTriggerAlarm(const DateTime& now);
void displayAlarmSettings();

#endif
