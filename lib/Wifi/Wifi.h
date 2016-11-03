#include <ESP8266WiFi.h>
const char *ssid     = "AsusRT-N56U";
const char *password = "R2d2c3po!";

bool InitWifi()
{
  Serial.print("Connecting to WIFI ");
  Serial.print(ssid);
  Serial.println("...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed!");
    WiFi.mode(WIFI_OFF);
    return false;
  }

  return true;
}
