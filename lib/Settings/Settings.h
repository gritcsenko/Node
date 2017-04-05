#include <SdFat.h>
#include <ArduinoJson.h>

#include "..\lib\SPIFFS\SPIFFS.h"

String settingsFileName = "/settings.json";

const size_t bufferSize = 2*JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(8) + 330; // calculate buffer size here https://bblanchon.github.io/ArduinoJson/assistant/
char settingsBuf[1024];

JsonObject& LoadSDSettings(SdFat SD, String fileName)
{
  //if(!SD.exists(fileName))
  //{
  //  Serial.print("Settings file ");
  //  Serial.print(fileName);
  //  Serial.println(" does not exists");
  //  return NULL;
  //}

  Serial.print("Opening settings file ");
  Serial.print(fileName);
  Serial.println(" ...");
  File settingsFile = SD.open(fileName.c_str(), FILE_READ);
  if(!settingsFile)
  {
    Serial.print("Settings file ");
    Serial.print(fileName);
    Serial.println(" cannot be opened for reading");
    return JsonObject::invalid();
  }

  Serial.print("Preparing buffer (");
  Serial.print(sizeof(settingsBuf));
  Serial.println(")...");
  memset(settingsBuf, '\0', sizeof(settingsBuf));

  Serial.println("Reading JSON...");
  int size = settingsFile.read(settingsBuf, sizeof(settingsBuf));
  settingsFile.close();

  Serial.print("Read ");
  Serial.print(size);
  Serial.println(" bytes");
  Serial.println(settingsBuf);

  Serial.println("Parsing JSON...");

  DynamicJsonBuffer jsonRootBuffer(bufferSize);
  return jsonRootBuffer.parseObject(settingsBuf);
}

bool SaveSDSettings(SdFat SD, JsonObject& settingsRoot, String fileName)
{
  File settingsFile = SD.open(fileName.c_str(), FILE_WRITE);
  if (!settingsFile) {
    Serial.print("Settings file ");
    Serial.print(fileName);
    Serial.println(" cannot be opened for writing");
    return false;
  }

  settingsRoot.printTo(settingsFile);

  settingsFile.close();

  return true;
}

JsonObject& LoadSettings(SdFat SD, String fileName)
{
  Serial.println("Mounting SPIFFS...");
  if (spiffs::Mount()) {
    Serial.println("Loading SPIFFS config...");
    JsonObject& spiffsRoot = spiffs::LoadSettings(settingsFileName);
    if(!spiffsRoot.success())
    {
      Serial.print("Settings file ");
      Serial.print(settingsFileName);
      Serial.println(" contains wrong JSON");
    }else{
      return spiffsRoot;
    }
  }else{
    Serial.println("Failed to mount SPIFFS");
  }

  Serial.println("Loading SD config...");
  JsonObject& sdRoot = LoadSDSettings(SD, fileName);
  if(!sdRoot.success())
  {
    Serial.println("Rebooting...");
    delay(5000);
    ESP.restart();
  }

  return sdRoot;
}

JsonObject& LoadSettings(SdFat SD)
{
    return LoadSettings(SD, settingsFileName);
}
