#include <SD.h>
#include <ArduinoJson.h>

String settingsFileName = "/settings.jsn";

const size_t bufferSize = 3*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(8) + 330;
//DynamicJsonBuffer jsonRootBuffer(bufferSize); // calculate buffer size here https://bblanchon.github.io/ArduinoJson/assistant/
//StaticJsonBuffer<bufferSize> jsonRootBuffer;
DynamicJsonBuffer jsonRootBuffer;
char settingsBuf[1024];

JsonObject* LoadSettings(String fileName)
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
  File settingsFile = SD.open(fileName, FILE_READ);
  if(!settingsFile)
  {
    Serial.print("Settings file ");
    Serial.print(fileName);
    Serial.println(" cannot be opened for reading");
    return NULL;
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

  JsonObject& rootJson = jsonRootBuffer.parseObject(settingsBuf);
  return &rootJson;
}

JsonObject* LoadSettings(void)
{
    return LoadSettings(settingsFileName);
}


bool SaveSettings(JsonObject& settingsRoot, String fileName)
{
  File settingsFile = SD.open(fileName, FILE_WRITE);
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

bool SaveSettings(JsonObject& settingsRoot)
{
  return SaveSettings(settingsRoot, settingsFileName);
}
