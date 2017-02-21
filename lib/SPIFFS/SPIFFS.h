#define FS_NO_GLOBALS

#include <ArduinoJson.h>
#include <FS.h>

namespace spiffs
{
  const size_t bufferSize = 3*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(8) + 330;
  DynamicJsonBuffer jsonRootBuffer(bufferSize);

  bool Mount()
  {
    return SPIFFS.begin();
  }

  bool SaveSettings(JsonObject& settingsRoot, String fileName)
  {
    fs::File settingsFile = SPIFFS.open(fileName, "w");
    if (!settingsFile) {
      Serial.print("Settings file ");
      Serial.print(fileName);
      Serial.println(" be opened for writing");
      return false;
    }

    settingsRoot.printTo(settingsFile);

    settingsFile.close();

    return true;
  }

  JsonObject* LoadSettings(String fileName)
  {
    if(!SPIFFS.exists(fileName))
    {
      Serial.print("Settings file ");
      Serial.print(fileName);
      Serial.println(" does not exists");
      return NULL;
    }

    fs::File settingsFile = SPIFFS.open(fileName, "r");
    if(!settingsFile)
    {
      Serial.print("Settings file ");
      Serial.print(fileName);
      Serial.println(" exists but cannot be opened");
      return NULL;
    }

    JsonObject& root = jsonRootBuffer.parseObject(settingsFile);

    settingsFile.close();

    return &root;
  }
}
