#include "interface.h"

extern int selectedTimeZoneIndex;
extern Adafruit_SSD1306 display;
extern RTC_DS1307 rtc;


// Add this extern so main.cpp and interface.cpp share the same variable
bool menuSelecting = false;
int currentMenu = MENU_CLOCK; // Default to clock
static int selectedMenuIndex = MENU_CLOCK;

// === Global Variables ===
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

// === Helpers ===
void toggleAlarm() { alarmEnabled = !alarmEnabled; }
const char* getMenuName(int index) {
  switch (index) {
    case MENU_CLOCK: return "Clock";
    case MENU_ALARM: return "Alarm";
    case MENU_SET_REGION: return "Weather Region";
    case MENU_SET_TIMEZONE: return "Timezone";
    default: return "Unknown";
  }
}

// === Main Menu Loop ===
void interfaceLoop() {
    if (menuSelecting) {
        display.clearDisplay();
        displayText(0, 0, "Select Menu:");

        for (int i = 0; i < MENU_COUNT; ++i) {
            std::string line = (i == selectedMenuIndex ? "> " : "  ");
            line += getMenuName(i);
            displayText(0, 12 + i * 10, line.c_str());
        }

        int input = getUserInput();  // 1=OK, 2=Down, 3=Up,  // 4=Prev

        if (millis() - lastActionTime > 200) {
            if (input == 1) {  // OK
                currentMenu = selectedMenuIndex;
                menuSelecting = false;
            } else if (input == 2) {  // Down
                selectedMenuIndex = (selectedMenuIndex + 1) % MENU_COUNT;
            } else if (input == 3) {  // Up
                selectedMenuIndex = (selectedMenuIndex - 1 + MENU_COUNT) % MENU_COUNT;
            } else if (input == 4) {  // Prev button pressed
                // Go back to default clock screen
                menuSelecting = false;
                currentMenu = MENU_CLOCK;
            }
            lastActionTime = millis();
        }

        display.display();
        return;  // skip rest of loop while selecting
    }

    // --- RST_PIN handling: Go back to menu if pressed ---
    int input = getUserInput();
    if (input == 6) { // RST_PIN must be defined and pinMode set in main.cpp
        menuSelecting = true;
        delay(300); // Debounce
        return;
    }

    display.clearDisplay();
    DateTime nowIST = rtc.now();
    const TimeZone& tz = timeZones[selectedTimeZoneIndex];

    if (millis() - lastRtcRead > 500) {
        now = rtc.now();
        lastRtcRead = millis();
    }

    switch (currentMenu) {
        case MENU_ALARM: {
            // Draw a rounded box for the alarm time (centered, 128x64 screen)
            int boxX = 10, boxY = 8, boxW = 108, boxH = 48;
            display.drawRoundRect(boxX, boxY, boxW, boxH, 8, SSD1306_WHITE);

            // Label: "Alarm" or "Set Hour"/"Set Minute"
            std::string label = editingAlarm
                ? (editingMinute ? "Set Minute" : "Set Hour")
                : "Alarm";
            display.setTextSize(1);
            int labelX = boxX + (boxW - label.length() * 6) / 2;
            int labelY = boxY + 4;
            display.setCursor(labelX, labelY);
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.print(label.c_str());

            // Alarm time, large and centered
            std::string timeStr = formatTime(alarmHour, alarmMinute);
            display.setTextSize(2);
            int timeW = 12 * 5; // 5 chars, 12px each at size 2
            int timeX = boxX + (boxW - timeW) / 2;
            int timeY = boxY + 16;
            display.setCursor(timeX, timeY);
            display.print(timeStr.c_str());
            display.setTextSize(1);

            // Alarm status (ON/OFF) as a small badge in the top-right of the box
            int badgeW = 28, badgeH = 12;
            int badgeX = boxX + boxW - badgeW - 4;
            int badgeY = boxY + 4;
            display.fillRoundRect(badgeX, badgeY, badgeW, badgeH, 4, alarmEnabled ? SSD1306_WHITE : SSD1306_BLACK);
            display.setTextColor(alarmEnabled ? SSD1306_BLACK : SSD1306_WHITE, alarmEnabled ? SSD1306_WHITE : SSD1306_BLACK);
            display.setCursor(badgeX + 4, badgeY + 2);
            display.print(alarmEnabled ? "ON" : "OFF");
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

            // Footer: instructions
            display.fillRect(0, 58, 128, 6, SSD1306_BLACK);
            display.setCursor(4, 59);
            if (!editingAlarm)
                display.print("OK: Edit  |  Prev: Back");
            else if (!editingMinute)
                display.print("UP/DOWN: Hour  OK: Next");
            else
                display.print("UP/DOWN: Min  OK: Done");

            // Handle input
            int in = getUserInput();
            if (millis() - lastActionTime > 300) {
                switch (in) {
                    case 1:  // OK
                        if (!editingAlarm && !alarmEnabled) { 
                           editingAlarm = true;
                           editingMinute = false; 
                           alarmEnabled = true;}
                        else if (!editingMinute) { editingMinute = true; alarmEnabled = true; }
                        else { editingAlarm = false; editingMinute = false; } // Done editing
                        break;
                    case 3: // UP
                        if (editingAlarm) {
                            if (!editingMinute) alarmHour = (alarmHour + 1) % 24;
                            else alarmMinute = (alarmMinute + 1) % 60;
                        }
                        break;
                    case 2: // DOWN
                        if (editingAlarm) {
                            if (!editingMinute) alarmHour = (alarmHour + 23) % 24;
                            else alarmMinute = (alarmMinute + 59) % 60;
                        }
                        break;
                }
                lastActionTime = millis();
            }
            break;
        }

          case MENU_SET_TIMEZONE: {
            // Draw a rounded box for the timezone info (centered, 128x64 screen)
            int boxW = 110, boxH = 36;
            int boxX = (128 - boxW) / 2;
            int boxY = (64 - boxH) / 2;
            display.drawRoundRect(boxX, boxY, boxW, boxH, 8, SSD1306_WHITE);

            // Format timezone string, truncate if needed to fit
            char offsetStr[8];
            snprintf(offsetStr, sizeof(offsetStr), "%+.2f", tz.offsetHours);
            std::string tzStr = std::string(tz.name) + " (UTC" + offsetStr + ")";
            if (tzStr.length() > 18) tzStr = tzStr.substr(0, 18);

            // Display timezone string inside the box, centered
            display.setTextSize(2);
            int labelX = boxX + (boxW - (tzStr.length() * 12) / 2) / 2;
            int labelY = boxY + 10;
            display.setCursor(labelX, labelY);
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.print(tzStr.c_str());
            display.setTextSize(1);

            // Footer: instructions
            display.fillRect(0, 56, 128, 8, SSD1306_BLACK);
            display.setCursor(8, 58);
            int input = getUserInput();
            if (millis() - lastActionTime > 250) {
                if (input == 1) {
                    display.clearDisplay();
                    changeTimeZone();
                    DateTime now = rtc.now();
                    showClockPage(now);
                    toggleAlarmStatus();
                    display.display();
                }
                lastActionTime = millis();
            }
            break;
        }

        case MENU_SET_REGION: {
            // City name bar at the top
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.fillRect(0, 0, 128, 12, SSD1306_BLACK);
            display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
            display.setCursor(4, 2);
            display.print(weatherRegion.c_str());

            // Weather info box
            int boxW = 120, boxH = 24;
            int boxX = (128 - boxW) / 2;
            int boxY = 20;
            display.drawRoundRect(boxX, boxY, boxW, boxH, 6, SSD1306_WHITE);

            // Weather icon (left side of box)
            int iconX = boxX + 4;
            int iconY = boxY + 4;
            if (currentWeather.find("rain") != std::string::npos || currentWeather.find("Rain") != std::string::npos) {
                display.fillCircle(iconX + 6, iconY + 8, 5, SSD1306_WHITE);
                display.fillTriangle(iconX + 6, iconY + 2, iconX, iconY + 14, iconX + 12, iconY + 14, SSD1306_WHITE);
            } else if (currentWeather.find("cloud") != std::string::npos || currentWeather.find("Cloud") != std::string::npos) {
                display.fillCircle(iconX + 8, iconY + 8, 7, SSD1306_WHITE);
                display.fillCircle(iconX + 16, iconY + 10, 5, SSD1306_WHITE);
                display.fillRect(iconX + 8, iconY + 10, 10, 6, SSD1306_WHITE);
            } else {
                display.drawCircle(iconX + 10, iconY + 10, 7, SSD1306_WHITE);
                for (int i = 0; i < 8; ++i) {
                    float angle = i * 3.14159 / 4;
                    int x1 = iconX + 10 + cos(angle) * 9;
                    int y1 = iconY + 10 + sin(angle) * 9;
                    int x2 = iconX + 10 + cos(angle) * 13;
                    int y2 = iconY + 10 + sin(angle) * 13;
                    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
                }
            }

            // Weather string, scaled to fit one line (max 16 chars at size 1, 8 at size 2)
            std::string weatherShort = currentWeather.substr(0, 16);
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            int textX = boxX + 28;
            int textY = boxY + 7;
            display.setCursor(textX, textY);
            display.print(weatherShort.c_str());

            // Footer: hint for next city
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.fillRect(0, 56, 128, 8, SSD1306_BLACK);
            display.setCursor(8, 58);

            int input = getUserInput();
            if (millis() - lastActionTime > 250) {
                if (input == 3 || input == 2) { // Up/Down
                    changeWeatherRegion();
                }
                lastActionTime = millis();
            }
            break;
        }

        case MENU_CLOCK: {
            DateTime now = rtc.now();
            display.clearDisplay();
            showClockPage(now);
            toggleAlarmStatus();
            display.display();
            break;
        }
    }

    int in = getUserInput();
    if (millis() - lastActionTime > 250) {
        if (in == 1) {
            switch (currentMenu) {
                case MENU_ALARM: toggleAlarm(); break;
                case MENU_SET_TIMEZONE: changeTimeZone(); break;
                case MENU_SET_REGION: changeWeatherRegion(); break;
                case MENU_CLOCK: showClockPage(now); break;
            }
        }
        lastActionTime = millis();
    }
    display.display();
    // --- Allow Going Back to Menu ---
    if (millis() - lastActionTime > 500) {
        if (in == 1) {  // Use 6 or any other code for "back to menu"
            menuSelecting = true;
        }
        lastActionTime = millis();
    }
}