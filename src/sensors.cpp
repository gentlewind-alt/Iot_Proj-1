#include <U8g2lib.h>
#include "emoji.h"
#include "sensors.h"

#define PIR_PIN 6

int pirState = LOW;
unsigned long lastMotionTime = 0;
bool inIdleAnimation = false;
const unsigned long idleTimeout = 1 * 60 * 1000; // 1 minutes

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

void setupMotionSensor() {
  pinMode(PIR_PIN, INPUT);
  lastMotionTime = millis();
}

void motionSensorLoop() {
  int pirVal = digitalRead(PIR_PIN);
  if (
      digitalRead(0) == LOW || // UP_BUTTON
      digitalRead(1) == LOW || // DOWN_BUTTON
      digitalRead(2) == LOW || // PREV_BUTTON
      digitalRead(4) == LOW || // NEXT_BUTTON
      digitalRead(5) == LOW || // OK_BUTTON
      digitalRead(3) == LOW    // RST_PIN
  ) {
    lastMotionTime = millis();
    if (inIdleAnimation) {
      Serial.println("✨ Exiting idle mode due to button press.");
      inIdleAnimation = false;
      u8g2.clearBuffer();
      u8g2.sendBuffer();
    }
  }
  // === Motion Detected ===
  if (pirVal == HIGH) {
    if (pirState == LOW) {
      Serial.println("🚶 Motion detected!");
      pirState = HIGH;
    }

    lastMotionTime = millis();

    if (inIdleAnimation) {
      Serial.println("✨ Exiting idle mode due to motion.");
      inIdleAnimation = false;
      u8g2.clearBuffer();
      u8g2.sendBuffer();
    }
  }
  // === No Motion ===
  else {
    if (pirState == HIGH) {
      Serial.println("🛑 Motion ended!");
      pirState = LOW;
    }

    if (!inIdleAnimation && (millis() - lastMotionTime > idleTimeout)) {
      Serial.println("💤 No motion or button for 1 min. Entering idle animation.");
      inIdleAnimation = true;
    }
  }
  // === Idle Animation ===
  if (inIdleAnimation) {
    static int animFrame = 0;
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, epd_bootanimation_allArray[animFrame]);
    u8g2.sendBuffer();
    animFrame++;
    if (animFrame >= 60) animFrame = 0;
    delay(100); // slows the animation
  }
}
