#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

bool InitWifi(JsonObject& settingsRoot)
{
  Serial.print("Connecting to WIFI ");
  Serial.print(settingsRoot["wifi"]["ssid"].as<char*>());
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(settingsRoot["wifi"]["ssid"].as<char*>(), settingsRoot["wifi"]["password"].as<char*>());

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  return true;
}
