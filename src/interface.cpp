#include "interface.h"
#include <Arduino.h>
#include "alarm.h"
#include "weather.h"
#include "clock.h"
#include "emoji.h"
#include <vector>
#include <string>

// Add extern declaration for emojis vector if defined in emoji.cpp
//extern std::vector<std::string> emojis;

extern int selectedTimeZoneIndex;
extern Adafruit_SSD1306 display;
extern RTC_DS1307 rtc;

// === Menu Options ===
enum MenuOption {
  MENU_CLOCK,
  MENU_SET_ALARM,
  MENU_ALARM_TOGGLE,
  MENU_SET_REGION,
 // MENU_SHOW_EMOJI,
  MENU_SET_TIMEZONE,
  MENU_COUNT
};

// === Global Variables ===
static bool okButtonPressed = false;
static unsigned long okPressStart = 0;
static int currentMenu = 0;
static unsigned long lastActionTime = 0;
static bool editingAlarm = false;
static bool editingMinute = false;
static unsigned long lastRtcRead = 0;
static DateTime now;

//int currentEmoji = 0;  // <<< Moved here for proper scoping if not global in emoji.cpp

// === Time Formatters ===
static std::string formatTime(int h, int m) {
  char b[6]; snprintf(b, sizeof(b), "%02d:%02d", h, m);
  return std::string(b);
}
static std::string formatTZ(int off) {
  char b[8]; snprintf(b, sizeof(b), "UTC%+d", off);
  return std::string(b);
}

// === Interface Entry Point ===
void startInterface() {
  DateTime now = rtc.now();
  const TimeZone& tz = timeZones[selectedTimeZoneIndex];
}

// === UI Summary Screen ===
void showInterface() {
  DateTime now = rtc.now();
  const TimeZone& tz = timeZones[selectedTimeZoneIndex];
  displayText(0, 20, "1. Toggle Alarm");
  displayText(0, 30, "2. Set Alarm Time");
  displayText(0, 40, ("3. Timezone " + std::string(tz.name)).c_str());
  displayText(0, 50, "4. Weather: " + weatherRegion);
}

// === Helpers ===
void toggleAlarm() { alarmEnabled = !alarmEnabled; }
void setAlarmTime(int h, int m) { alarmHour = h; alarmMinute = m; }

// === Main Menu Loop ===
void interfaceLoop() {
  display.clearDisplay();
  DateTime nowIST = rtc.now();
  const TimeZone& tz = timeZones[selectedTimeZoneIndex];

  if (millis() - lastRtcRead > 500) {
    now = rtc.now();
    lastRtcRead = millis();
  }

  switch (currentMenu) {
    case MENU_ALARM_TOGGLE: {
      std::string toggleStatus = alarmEnabled ? "ENABLED" : "DISABLED";
      std::string timeStr = formatTime(alarmHour, alarmMinute);
      displayText(0, 30, "Alarm Toggle");
      displayText(0, 40, "Alarm: " + toggleStatus);
      displayText(0, 50, "Time: " + timeStr);
      int input = getUserInput();
      if (millis() - lastActionTime > 250) {
        if (input == 1) toggleAlarm();
        else if (input == 2) currentMenu = (currentMenu + 1) % MENU_COUNT;
        else if (input == 3) currentMenu = (currentMenu - 1 + MENU_COUNT) % MENU_COUNT;
        lastActionTime = millis();
      }
    } break;

    case MENU_SET_ALARM: {
      std::string label = editingAlarm
        ? (editingMinute ? "Set Minute:" : "Set Hour:")
        : "Alarm Time:";
      std::string timeStr = formatTime(alarmHour, alarmMinute);
      displayText(0, 20, label);
      displayText(0, 30, timeStr);
      int in = getUserInput();
      if (millis() - lastActionTime > 300) {
        switch (in) {
          case 1:  // OK
            if (!editingAlarm) editingAlarm = true, editingMinute = false;
            else if (!editingMinute) editingMinute = true;
            else editingAlarm = false;
            break;
          case 2: case 3:
            currentMenu = (in == 2)
              ? (currentMenu + 1) % MENU_COUNT
              : (currentMenu - 1 + MENU_COUNT) % MENU_COUNT;
            break;
          case 4:
            if (editingAlarm) (!editingMinute ? alarmHour : alarmMinute)++;
            break;
          case 5:
            if (editingAlarm) (!editingMinute ? alarmHour : alarmMinute)--;
            break;
        }
        alarmHour = (alarmHour + 24) % 24;
        alarmMinute = (alarmMinute + 60) % 60;
        lastActionTime = millis();
      }
    } break;

    case MENU_SET_TIMEZONE: {
      std::string tzStr = std::string("Timezone: ") + tz.name + " (UTC" + (tz.offsetHours >= 0 ? "+" : "") + std::to_string(tz.offsetHours) + ")";
      displayText(0, 10, tzStr);
      displayText(0, 30, "Next -> press OK");
      int input = getUserInput();
      if (millis() - lastActionTime > 250) {
        if (input == 1) {
          display.clearDisplay();
          changeTimeZone();
          DateTime now = rtc.now();
          showClockPage(now);
        } else if (input == 2)
          currentMenu = (currentMenu + 1) % MENU_COUNT;
        else if (input == 3)
          currentMenu = (currentMenu - 1 + MENU_COUNT) % MENU_COUNT;
        lastActionTime = millis();
      }
    } break;

    case MENU_SET_REGION:
      displayText(0, 20, "City:");
      displayText(0, 30, currentWeather);
      break;

    case MENU_CLOCK:
      displayText(0, 20, "Clock");
      showClockPage(now);
      break;

    // case MENU_SHOW_EMOJI: {
    //   showEmoji(currentEmoji);
    //   int input = getUserInput();
    //   if (millis() - lastActionTime > 300) {
    //     if (input == 1 || input == 2) {
    //       currentEmoji = (currentEmoji + 1) % emojis.size();
    //     } else if (input == 3) {
    //       currentMenu = (currentMenu - 1 + MENU_COUNT) % MENU_COUNT;
    //     }
    //     lastActionTime = millis();
    //   }
    // } break;
  }

  int in = getUserInput();
  if (millis() - lastActionTime > 250) {
    if (in == 1) {
      switch (currentMenu) {
        case MENU_ALARM_TOGGLE: toggleAlarm(); break;
        case MENU_SET_ALARM: alarmHour = (alarmHour + 1) % 24; break;
        case MENU_SET_TIMEZONE: changeTimeZone(); break;
        case MENU_SET_REGION: changeWeatherRegion(); break;
        case MENU_CLOCK: showClockPage(now); break;
      }
    } else if (in == 2) currentMenu = (currentMenu + 1) % MENU_COUNT;
    else if (in == 3) currentMenu = (currentMenu - 1 + MENU_COUNT) % MENU_COUNT;
    lastActionTime = millis();
  }

  display.display();
}
