#include "weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  // For JSON parsing

std::string currentWeather = "Loading...";
const char* apiKey = "e19f470114e4a4f4580249def7bd3643";  // ← INSERT YOUR KEY HERE

// List of up to 5 city names or coordinates
const char* cityList[] = {
  "Tokyo", "Delhi", "London", "Paris", "New York",
  "Berlin", "Moscow", "Seoul", "Dhenkanal", "Mumbai"
};
const int cityCount = sizeof(cityList) / sizeof(cityList[0]);

int selectedCityIndex = 0;
std::string weatherRegion = cityList[selectedCityIndex];

void changeWeatherRegion() {
  selectedCityIndex = (selectedCityIndex + 1) % cityCount;
  weatherRegion = cityList[selectedCityIndex];
  fetchWeather();  // auto-update when changed
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    currentWeather = "No WiFi";
    return;
  }

  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += weatherRegion.c_str();
  url += "&appid=";
  url += apiKey;
  url += "&units=metric";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);

    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      float temp = doc["main"]["temp"];
      currentWeather = weatherRegion + ": " + std::to_string(temp) + " C";
    } else {
      currentWeather = "Parse Err";
    }
  } else {
    currentWeather = "HTTP Error";
  }

  http.end();
}