#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

bool InitWifiSta(JsonObject& settingsRoot)
{
  WiFi.mode(WIFI_OFF);

  JsonObject& wifi_sta = settingsRoot["sta"].as<JsonObject&>();

  Serial.print("Connecting to WIFI ");
  Serial.print(wifi_sta["ssid"].as<char*>());
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_sta["ssid"].as<char*>(), wifi_sta["password"].as<char*>());

  int tryCounter = 20;
  while(!WiFi.isConnected() && tryCounter-- > 0)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println(".");

  if(!WiFi.isConnected())
  {
    Serial.println("Connection Failed!");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  Serial.print("localIP: ");
  Serial.println(WiFi.localIP());
  Serial.print("macAddress: ");
  Serial.println(WiFi.macAddress());
  Serial.print("subnetMask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("gatewayIP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("hostname: ");
  Serial.println(WiFi.hostname());
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("psk: ");
  Serial.println(WiFi.psk());
  Serial.print("BSSID: ");
  Serial.println(WiFi.BSSIDstr());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  return true;
}
