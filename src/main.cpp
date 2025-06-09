#include "interface.h"

unsigned long lastNTPSync = 0;
const unsigned long ntpSyncInterval = 6UL * 60UL * 60UL * 1000UL; // 6 hours
// === Global Display and RTC Objects ===
Adafruit_SSD1306 display(128, 64, &Wire);
RTC_DS1307 rtc;
Adafruit_MPU6050 mpu;
// === NeoPixel Setup ===

// == Function Prototypes ===
void displayText(int x, int y, const std::string &text) {
  display.setCursor(x, y);
  display.print(text.c_str());
}

// Remove lightPixel and all NeoPixel usage
// void lightPixel(uint32_t color) {
//   pixel.setPixelColor(0, color);
//   pixel.show();
// }

// === Button Handling ===
int getUserInput() {
  if (digitalRead(RST_PIN) == LOW) {
    // lightPixel(pixel.Color(255, 255, 0));
    return 6;
  }
  if (digitalRead(OK_BUTTON) == LOW) {
    // lightPixel(pixel.Color(0, 255, 0));
    return 1;
  }
  if (digitalRead(NEXT_BUTTON) == LOW) {
    // lightPixel(pixel.Color(0, 0, 255));
    return 2;
  }
  if (digitalRead(PREV_BUTTON) == LOW) {
    // lightPixel(pixel.Color(255, 0, 0));
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

const char* ssid = "Airtel_mahe_2477";
const char* password = "Air@15514";

void playEmojiBootAnimation() {
  for (int i = 0; i < 60; i++) {   // assuming 90 frames total
    display.clearDisplay();;
    display.drawBitmap(0, 0, boot_allArray[i], 128, 64, SSD1306_WHITE);
    display.display();
    delay(100); // Adjust delay for speed of animation
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  // Initialize WiFi
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

  if (WiFi.status() == WL_CONNECTED) {
    syncRTCWithNTP();
  }

  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!mpu.begin(0x69)) {
    Serial.println("❌ MPU6050 not found at 0x69");
    while (1) delay(10);
  }
  Serial.println("✅ MPU6050 initialized");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();

  // === Emoji Boot Expression ===
  playEmojiBootAnimation();
  Serial.println("✅ Emoji boot animation complete!");
  // setupMotionSensor();

  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);
  pinMode(OK_BUTTON, INPUT_PULLUP);
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  pinMode(RST_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // pixel.begin();
  // pixel.setBrightness(100);
  // lightPixel(pixel.Color(30, 129, 176));
}

void loop(){
  motionSensorLoop(); // Handles motion + idle animation
  if (inIdleAnimation) {
    // Ensure the idle animation is actually drawn to the display
    orienConfig();
    display.display();
    return;
  }
  // Only do clock & interface work if not idle
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    checkAndTriggerAlarm(now);
    showClockPage(now);   // ⏰ check and ring alarm
  } else {
    Serial.println("⚠ RTC not running or not responding");
  }
  interfaceLoop(); // Draw the default clock screen at boot
  if (WiFi.status() == WL_CONNECTED && millis() - lastNTPSync > ntpSyncInterval) {
    syncRTCWithNTP();
    lastNTPSync = millis();
  }
}