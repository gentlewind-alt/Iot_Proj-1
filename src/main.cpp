#include "interface.h"

// ===Bus addresses and I2C Setup===
// === Pin Definitions ===
#define UP_BUTTON       0
#define DOWN_BUTTON     1
#define NEXT_BUTTON     4
#define PREV_BUTTON     2
#define OK_BUTTON       5
#define I2C_SDA         8
#define I2C_SCL         9
#define NEOPIXEL_PIN    10
#define BUZZER_PIN      7
#define PIR_PIN         6
#define RST_PIN         3

// === Display and RTC Setup ===
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// === Global Display and RTC Objects ===
Adafruit_SSD1306 display(128, 64, &Wire);
RTC_DS1307 rtc;
// === NeoPixel Setup ===
Adafruit_NeoPixel pixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// == Function Prototypes ===
void displayText(int x, int y, const std::string &text) {
  display.setCursor(x, y);
  display.print(text.c_str());
}

void displayEmoji(int x, int y, const std::string &emoji) {
  display.setCursor(x, y);
  display.print(emoji.c_str());
}

void lightPixel(uint32_t color) {
  pixel.setPixelColor(0, color);
  pixel.show();
}

// === Button Handling ===
int getUserInput() {
  if (digitalRead(RST_PIN) == LOW) {
    lightPixel(pixel.Color(255, 255, 0));
    return 6;
  }
  if (digitalRead(OK_BUTTON) == LOW) {
    lightPixel(pixel.Color(0, 255, 0));
    return 1;
  }
  if (digitalRead(NEXT_BUTTON) == LOW) {
    lightPixel(pixel.Color(0, 0, 255));
    return 2;
  }
  if (digitalRead(PREV_BUTTON) == LOW) {
    lightPixel(pixel.Color(255, 0, 0));
    return 3;
  }
  if (digitalRead(UP_BUTTON) == LOW) { //black  
    return 4;
  }
  if (digitalRead(DOWN_BUTTON) == LOW) { //yellow
    return 5;
  }
  return 0;
}

const char* ssid = "Wokwi-GUEST";
const char* password = "";

void playEmojiBootAnimation() {
  for (int i = 0; i < 541; i++) {   // assuming 90 frames total
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, epd_bootanimation_allArray[i]);
    u8g2.sendBuffer();
  }
}

void setup() {

  u8g2.begin();
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  Serial.println(ssid);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    Serial.println("Status Code: " + String(WiFi.status()));
  }

  fetchWeather();

  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);

   Wire.begin(I2C_SDA, I2C_SCL);

  if (!rtc.begin()) {
    Serial.println("❌ RTC not found! Check wiring.");
    while (1);
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();

  // === Emoji Boot Expression ===
  // playEmojiBootAnimation();
  Serial.println("✅ Emoji boot animation complete!");
  // setupMotionSensor();

  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);
  pinMode(OK_BUTTON, INPUT_PULLUP);
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(RST_PIN, INPUT_PULLUP);

  pixel.begin();
  pixel.setBrightness(100);
  lightPixel(pixel.Color(30, 129, 176));
}

void loop() {
  motionSensorLoop(); // Handles motion + idle animation
  if (!inIdleAnimation) { // Only do clock & interface work if not idle
    if (rtc.isrunning()) {
      DateTime now = rtc.now();
      checkAndTriggerAlarm(now);
      showClockPage(now);   // ⏰ check and ring alarm
    } else {
      Serial.println("⚠ RTC not running or not responding");
    }
    interfaceLoop(); // Draw the default clock screen at boot
  }
}