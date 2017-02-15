#include <ArduinoJson.h>
#include <ArduinoOTA.h>

bool InitOTA(JsonObject& settingsRoot)
{
  JsonObject& wifi_ota = settingsRoot["wifi"]["ota"].as<JsonObject&>();
  if(!wifi_ota.success())
  {
      return false;
  }
  ArduinoOTA.setPort(wifi_ota["port"].as<int>());
  ArduinoOTA.setHostname(wifi_ota["hostName"].as<char*>());
  ArduinoOTA.onStart([]() {
    String type;
    //if (ArduinoOTA.getCommand() == U_FLASH)
    //  type = "sketch";
    //else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if(progress % 2 == 0){
      digitalWrite(1, HIGH);
    }else{
      digitalWrite(1, LOW);
    }
    printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    digitalWrite(LED_BUILTIN, HIGH);
    printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Host name: ");
  Serial.println(ArduinoOTA.getHostname());
  return true;
}
