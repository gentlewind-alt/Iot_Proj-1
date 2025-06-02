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

// Function declarations
void changeTimeZone();
void setTimeZoneIndex(int index);
void showClockPage(const DateTime& nowIST);
void updateClockDisplay(const DateTime& now, const TimeZone& tz);

#endif
