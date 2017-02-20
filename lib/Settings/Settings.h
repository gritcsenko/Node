#include <SD.h>
#include <ArduinoJson.h>

const char* settingsFileName = "/settings.json";


DynamicJsonBuffer jsonRootBuffer;

JsonObject* LoadSettings(const char* fileName)
{
  if(!SD.exists(fileName))
  {
    Serial.print("Settings file ");
    Serial.print(fileName);
    Serial.println(" does not exists");
    return NULL;
  }

  File settingsFile = SD.open(fileName, FILE_READ);
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

JsonObject* LoadSettings(void)
{
    return LoadSettings(settingsFileName);
}


bool SaveSettings(JsonObject& settingsRoot, const char* fileName)
{
  File settingsFile = SD.open(fileName, FILE_WRITE);
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

bool SaveSettings(JsonObject& settingsRoot)
{
  return SaveSettings(settingsRoot, settingsFileName);
}
