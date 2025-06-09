# ⏰ ESP32 Smart Alarm Clock

An intelligent, customizable alarm clock built on the **ESP32-C3** platform, featuring:

- 🕒 Real-time clock with DS1307
- 📟 OLED display (SSD1306)
- 🧠 Motion/gesture input via MPU6050
- 🔊 Buzzer alarm
- 🌈 NeoPixel ring animation
- 🕹️ User input buttons
- 🚨 Motion detection with PIR
- 📏 Distance sensing via HC-SR04

---

## 📷 Features

- **Real-Time Clock (DS1307):** Maintains accurate date and time, even during power-off.
- **OLED Display:** Displays current time, alarm settings, emoji animations, and feedback.
- **MPU6050 Sensor:** Enables shake/tilt-based interactions (e.g., snooze or dismiss alarms).
- **Buzzer Alarm:** Active buzzer rings based on scheduled alarm time.
- **NeoPixel Ring:** LED ring for ambient lighting and visual alarm indication.
- **User Controls:**
  - Buttons for setting time, alarms, and acknowledgment.
  - Joystick or slide switches for input.
- **Motion Sensor (PIR):** Detects motion in room, can trigger idle/wake animations.
- **Distance Sensor (HC-SR04):** Detects proximity to enable/disable PIR.

---

## 🧰 Hardware Used

| Component           | Description                      |
|---------------------|----------------------------------|
| ESP32-C3 DevKitM-1  | Main microcontroller with WiFi   |
| DS1307 RTC Module   | Real-Time Clock                  |
| SSD1306 OLED        | 128x64 Display over I2C          |
| Adafruit MPU6050    | Accelerometer + Gyroscope        |
| HC-SR04             | Ultrasonic Distance Sensor       |
| PIR Sensor          | Motion detection                 |
| Active Buzzer       | Audio alarm indicator            |
| Buttons             | User input                       |

---

## 📦 Wiring Overview

| Device         | SDA (Data) | SCL (Clock) | Power | Notes                     |
|----------------|------------|-------------|-------|---------------------------|
| OLED + RTC     | GPIO 8     | GPIO 9      | 3.3V  | Shared I2C bus            |
| MPU6050        | GPIO 4     | GPIO 5      | 3.3V  | Uses secondary I2C (Wire1)|
| HC-SR04        | Trig: 6    | Echo: 7     | 5V    | Distance measurement       |
| PIR            | GPIO 10    | -           | 5V    | Motion detection           |
| Buzzer         | GPIO 3     | -           | 3.3V  | Active buzzer             |
| NeoPixel Ring  | GPIO 2     | -           | 5V    | Use level shifter if needed|

---

## 🧪 Setup Instructions

1. **Install PlatformIO** (or Arduino IDE with necessary libraries).
2. **Clone the repository**:

