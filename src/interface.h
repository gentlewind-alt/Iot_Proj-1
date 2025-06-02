// interface.h
#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <Adafruit_SSD1306.h>

extern int timeZoneOffset;
extern std::string weatherRegion;

// Interface core
void startInterface();
void showInterface();
void interfaceLoop();

// Actions
void toggleAlarm();
void setAlarmTime(int hour, int minute);
void changeTimeZone();
void changeWeatherRegion();
void showEmoji(int index);

// Callbacks implemented in main.cpp
void displayText(int x, int y, const std::string &text);
void displayEmoji(int x, int y, const std::string &emoji);
int getUserInput(); // 1=OK, 2=Next, 3=Prev

#endif
