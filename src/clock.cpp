#include "clock.h"
#include "interface.h" // for getUserInput()
#include <Arduino.h>
#include <RTClib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// List of standard time zones you want to cycle through:
const TimeZone timeZones[] = {
  {"IST", +5.5},
  {"UTC", 0.0},
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

bool alarmEnabled = false;
int alarmHour = 7;
int alarmMinute = 0;

volatile bool alarmBeeping = false;
unsigned long snoozeUntil = 0;

void buzzerTone(uint8_t pin, uint16_t freqHz, uint16_t msDuration) {
  pinMode(pin, OUTPUT);

  uint32_t interval = 500000 / freqHz;  // μs half-period
  uint32_t cycles = (freqHz * msDuration) / 1000;

  noInterrupts();
  for (uint32_t i = 0; i < cycles; ++i) {
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);  // HIGH
    delayMicroseconds(interval);
    REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);  // LOW
    delayMicroseconds(interval);
  }
  interrupts();
}

// Alarm functions
void checkAndTriggerAlarm(const DateTime& now) {
    static int lastMin = -1;
    if (!alarmEnabled) return;

    // Snooze logic
    if (snoozeUntil && now.unixtime() < snoozeUntil) return;

    if (now.hour() != alarmHour || now.minute() != alarmMinute) {
        lastMin = -1;
        return;
    }
    if (now.minute() == lastMin) return;

    lastMin = now.minute();

     Serial.println("🔔 Alarm Triggered!");
    alarmBeeping = true;
    unsigned long startBeep = millis();
    unsigned long snoozeSet = 0;
    while (millis() - startBeep < 60000) // Beep for 60 seconds or until snooze
    {
        buzzerTone(BUZZER_PIN, 2000, 30); // Short beep (30ms)
        delay(30); // Small pause between beeps

        // Check for snooze button
        if (getUserInput() == 5) { // DOWN button
            // Use current time for snooze, not the original 'now'
            snoozeUntil = rtc.now().unixtime() + 30; // 5 min snooze
            Serial.println("😴 Snoozed for 5 minutes!");
            snoozeSet = 1;
            break;
        }
        // Optionally, allow other code to run here (yield, etc.)
    }
    alarmBeeping = false;
}

void displayAlarmSettings() {
  display.clearDisplay();

  char buf[32];
  snprintf(buf, sizeof(buf), "Alarm: %s", alarmEnabled ? "ON" : "OFF");
  display.setCursor(0, 40);
  display.print(buf);

  snprintf(buf, sizeof(buf), "Time: %02d:%02d", alarmHour, alarmMinute);
  display.setCursor(0, 50);
  display.print(buf);

  display.display();
}

// --- Clock Display Functions ---
// Update displayed time adjusted by current timezone
void showClockPage(const DateTime& nowUtc) {
    const TimeZone& tz = timeZones[selectedTimeZoneIndex];
    DateTime adjustedTime = nowUtc + TimeSpan((int)(tz.offsetHours * 3600));

    // Show timezone name
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TZ: ");
    display.print(tz.name);

    // Show time in large font, centered
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", adjustedTime.hour(), adjustedTime.minute(), adjustedTime.second());
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(timeBuf, 0, 0, &x1, &y1, &w, &h);
    int textX = (128 - w) / 2;
    int textY = 20;
    display.setCursor(textX, textY);
    display.print(timeBuf);

    // Draw a thin line below the time
    display.drawLine(0, textY + h + 2, 127, textY + h + 2, WHITE);

    // Show date below the line, small font, centered
    display.setTextSize(1);
    char dateBuf[17];
    snprintf(dateBuf, sizeof(dateBuf), "%02d/%02d/%04d", adjustedTime.day(), adjustedTime.month(), adjustedTime.year());
    display.getTextBounds(dateBuf, 0, 0, &x1, &y1, &w, &h);
    int dateX = (128 - w) / 2;
    int dateY = textY + h + 15; // 18px below the time
    display.setCursor(dateX, dateY);
    display.print(dateBuf);
}

void toggleAlarmStatus() {
  // Toggle alarm status if OK button is pressed
  if (getUserInput() == 1) { // Assuming 1 is OK button
    alarmEnabled = !alarmEnabled;
    delay(200); // Debounce
  }

  // Show alarm status and time at the bottom
  char alarmBuf[32];
  snprintf(alarmBuf, sizeof(alarmBuf), "Alarm: %s %02d:%02d", alarmEnabled ? "ON" : "OFF", alarmHour, alarmMinute);

  display.setCursor(0, 54);
  display.print(alarmBuf);
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

void syncRTCWithNTP() {
  configTime(0, 0, "pool.ntp.org"); // Use UTC; adjust offset if needed
  struct tm timeinfo;
  const int maxRetries = 5;
  int attempt = 0;
  bool success = false;

  while (attempt < maxRetries) {
    if (getLocalTime(&timeinfo)) {
      rtc.adjust(DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      ));
      Serial.println("✅ RTC synced with NTP!");
      success = true;
      break;
    } else {
      Serial.println("❌ Failed to get time from NTP, retrying...");
      delay(1000); // Wait 1 second before retrying
      attempt++;
    }
  }

  if (!success) {
    Serial.println("❌ Failed to sync RTC with NTP after retries.");
  }
}