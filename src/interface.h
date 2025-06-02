// interface.h
#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "clock.h"
#include "weather.h"
#include "emoji.h"
#include "sensors.h"

extern int timeZoneOffset;
extern std::string weatherRegion;
extern bool menuselecting;

// Interface core
void startInterface();
void showInterface();
void interfaceLoop();

// Actions
void toggleAlarm();
void changeTimeZone();
void changeWeatherRegion();
void showEmoji(int index);

// Callbacks implemented in main.cpp
void displayText(int x, int y, const std::string &text);
void displayEmoji(int x, int y, const std::string &emoji);
int getUserInput(); // 1=OK, 2=Next, 3=Prev

#endif
