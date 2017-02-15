#include <SD.h>
#include <ArduinoJson.h>

const char* settingsFileName = "settings.json";


DynamicJsonBuffer jsonRootBuffer;

JsonObject* LoadSettings(const char* fileName)
{
  if(!SD.exists(fileName))
  {
    return NULL;
  }

  File settingsFile = SD.open(fileName, O_READ);

  JsonObject& root = jsonRootBuffer.parseObject(settingsFile);

  settingsFile.close();

  return &root;
}

JsonObject* LoadSettings(void)
{
    return LoadSettings(settingsFileName);
}
