#define FS_NO_GLOBALS

#include <ArduinoJson.h>
#include <FS.h>

namespace spiffs
{

  DynamicJsonBuffer jsonRootBuffer;

  bool Mount()
  {
    return SPIFFS.begin();
  }

  bool SaveSettings(JsonObject& settingsRoot, const char* fileName)
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

  JsonObject* LoadSettings(const char* fileName)
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
