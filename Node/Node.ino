#include <Arduino.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

#include <ESP8266mDNS.h>

#include <SI7021.h>
#include <Adafruit_BMP085.h>

#include <ArduinoJson.h>

using namespace ArduinoJson::Internals;

#include "..\lib\CO2\CO2.h"
#include "..\lib\Storage\Storage.h"
#include "..\lib\Settings\Settings.h"
#include "..\lib\OTA\OTA.h"
#include "..\lib\Wifi\Wifi.h"
#include "..\lib\TimeSync\TimeSync.h"
#include "..\lib\NRF\NRF.h"

SI7021 sensor;
Adafruit_BMP085 bmp;
StaticJsonBuffer<200> jsonBuffer;


void setup() {

  Serial.begin(9600);

  delay(1000);
  Serial.println("Starting up");

  Wire.begin();
  SPI.begin();

  InitSD();

  if (InitWifi()) {
    InitOTA();
  }else{
    //Serial.println("Rebooting...");
    //delay(5000);
    //ESP.restart();
  }

  if(!SyncTime())
  {
    Serial.println("Failed to sync time!");
  }

  if(!InitNRF()){
    Serial.println("NRF Initialization failed");
  }

  Serial.print("Initializing SI7021 sensor... ");
  sensor.begin(SDA, SCL);
  if(sensor.sensorExists()){
    Serial.print("found, id: ");
    Serial.println(sensor.getDeviceId());
  }else{
    Serial.println("NOT found");
  }

  Serial.print("Initializing BMP085/BMP180 sensor... ");
  if (bmp.begin()) {
    Serial.println("Ok");
  }else{
    Serial.println("Could not find a valid BMP085/BMP180 sensor");
  }

  Serial.println("Setup complete");
}

time_t lastTime = 0;

void loop() {
  if(WiFi.getMode() != WIFI_OFF){
    ArduinoOTA.handle();
  }
/*
  byte b;
  if ( radio.isValid() && radio.available() ) {
    bool done = false;
    while(!done){
      radio.read(&b, sizeof(b));
      Serial.write(b);
    }
    Serial.write("\n\r");
  }
*/
//  time_t currentTime = RTC.get();
//  time_t currentTime = timeClient.getEpochTime();
  time_t currentTime = now();
  if(currentTime - lastTime < 10){
    return;
  }
  lastTime = currentTime;

  Serial.print("Internal: ");
  displayTime();

  tmElements_t tm;
  RTC.read(tm);
  Serial.print("RTC:      ");
  displayTime(tm);
  Serial.print("Timer Temp: "); Serial.println(RTC.temperature()/4.0);

  si7021_thc thc = sensor.getTempAndRH();
  double t = thc.celsiusHundredths / 100.0;
  int RH = thc.humidityPercent;
  Serial.print("Temp: "); Serial.print(t);
  Serial.print("\tHum: "); Serial.println(RH);

  int32_t p = bmp.readPressure();
  double P = p / 100.0;
  double bmpTemp = bmp.readTemperature();
  Serial.print("Temp: "); Serial.print(bmpTemp); // Celsius
  Serial.print("\tPressure: "); Serial.println(P); // Gecto Pascals

  int humudutyTempWeight = 1;
  int pressureTempWeight = 1;
  double temperature = (t * humudutyTempWeight + bmpTemp * pressureTempWeight) / (humudutyTempWeight + pressureTempWeight);

  double V = 1.0;
  double Rv = 461.5;

  double T = 273.15 + temperature;
  double e_omega_t = 6.112*exp((17.62*T - 4812.903)/(T - 30.03));
  double f = 1.0016 + 0.00000315*P - 0.074/P;
  double e_omega = e_omega_t*f;
  Serial.print("e omega[hPa]: "); Serial.println(e_omega);
  double e = RH*e_omega/100;
  double AH = e/(Rv*T);
  double m = AH*V*100000.0;

  Serial.print("Water mass[mg]: "); Serial.println(m);

  int ppm = ReadCO2PPM();
  if(ppm < 0){
    Serial.println("No CO2 valid response");
  }else{
    Serial.print("CO2: ");
    Serial.print(ppm);
    Serial.println(" ppm");
  }

  JsonObject& root = jsonBuffer.createObject();
  root["time"] = currentTime;
  root["temperature"] = temperature;
  root["pressure"] = P;
  root["humidity"] = RH;
  root.printTo(Serial);
  char dirName[12];
  sprintf(dirName, 12, "/%.4i/%.2i/%.2i", tmYearToCalendar(tm.Year), tm.Month, tm.Day)
  char fileName[25];
  sprintf(fileName, 25, "/%s/%.2i.csv", dirName, tm.Hour)

  File dataFile;
  if(!SD.exists(fileName)){
    if(!SD.mkdir(dirName)){
      Serial.printf("Failed to create directory %s\r\n", dirName);
      return;
    }
    dataFile = SD.open(buffer, O_WRITE | O_CREAT);
  }else{
    dataFile = SD.open(buffer, O_WRITE);
  }

  dataFile.seek(dataFile.size());
  dataFile.printf("%i;%i;%i;%.2f;%i;%i\r\n", currentTime, thc.celsiusHundredths, thc.humidityPercent, bmpTemp, p, ppm);
  dataFile.close();
  Serial.println();
}
