#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

bool InitWifiSta(JsonObject& settingsRoot)
{
  JsonObject& wifi_sta = settingsRoot["wifi"]["sta"].as<JsonObject&>();

  Serial.print("Connecting to WIFI ");
  Serial.print(wifi_sta["ssid"].as<char*>());
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_sta["ssid"].as<char*>(), wifi_sta["password"].as<char*>());

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  return true;
}
