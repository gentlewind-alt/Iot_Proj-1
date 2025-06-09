// interface.h
#ifndef INTERFACE_H
#define INTERFACE_H

#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MPU6050.h"
#include "I2Cdev.h"

#include "clock.h"
#include "weather.h"
#include "emoji.h"
#include "sensors.h"

#define UP_BUTTON       0
#define DOWN_BUTTON     1
#define NEXT_BUTTON     4
#define PREV_BUTTON     2
#define OK_BUTTON       5
#define I2C_SDA         8
#define I2C_SCL         9
#define BUZZER_PIN      10
#define PIR_PIN         7
#define RST_PIN         3
#define TRIG_PIN        18
#define ECHO_PIN        19

extern int timeZoneOffset;
extern std::string weatherRegion;
extern bool menuselecting;

// Interface core
void interfaceLoop();

// Actions
void toggleAlarm();
void changeTimeZone();
void changeWeatherRegion();

// Callbacks implemented in main.cpp
void displayText(int x, int y, const std::string &text);
int getUserInput(); // 1=OK, 2=Next, 3=Prev

enum MenuOption {
  MENU_SELECTOR = -1,
  MENU_CLOCK,
  MENU_ALARM,
  MENU_SET_REGION,
  MENU_SET_TIMEZONE,
  MENU_COUNT
};// === Menu Options ===


extern int currentMenu;
extern Adafruit_MPU6050 mpu;

#endif