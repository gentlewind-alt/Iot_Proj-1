  #include "alarm.h"
  #include <Arduino.h>
  #include <RTClib.h>  // Needed for DateTime

  extern Adafruit_SSD1306 display;

  #define BUZZER_PIN 7  // example buzzer pin, change as needed

  bool alarmEnabled = false;
  int alarmHour = 7;
  int alarmMinute = 0;

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
    if (now.hour() != alarmHour || now.minute() != alarmMinute) {
      lastMin = -1;
      return;
    }
    if (now.minute() == lastMin) return;

    lastMin = now.minute();

    Serial.println("🔔 Alarm Triggered!");
      buzzerTone(10, 50, 2000);
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
