#ifndef ALARM_H
#define ALARM_H

#include <adafruit_ssd1306.h>
#include <RTClib.h>

extern bool alarmEnabled;
extern int alarmHour;
extern int alarmMinute;

void checkAndTriggerAlarm(const DateTime& now);  // ✅ Exact signature
void buzzerTone(uint8_t pin, uint16_t duration, uint16_t interval);

#endif
