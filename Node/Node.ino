#include <Arduino.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

#include <ESP8266mDNS.h>

#include <SI7021.h>
#include <Adafruit_BMP085.h>

#include "..\lib\Storage\Storage.h"
#include "..\lib\Settings\Settings.h"
#include "..\lib\OTA\OTA.h"
#include "..\lib\Wifi\Wifi.h"
#include "..\lib\TimeSync\TimeSync.h"
#include "..\lib\NRF\NRF.h"

byte co2request[9] =  { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
byte co2response[9];

SI7021 sensor;
Adafruit_BMP085 bmp;

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
    Serial println("Ok");
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
  Serial.print("Temp: "); Serial.print(thc.celsiusHundredths / 100.0);
  Serial.print("\tHum: "); Serial.println(thc.humidityPercent);

  int32_t pressure = bmp.readPressure();
  Serial.print("Temp: "); Serial.print(bmp.readTemperature()); // Celsius
  Serial.print("\tPressure: "); Serial.println(pressure); // Pascals

  Serial.flush();
  size_t count = 0;
  while (count < 9) {
    delay(50);
    Serial.pins(2, 3);
    delay(50);
    Serial.write(co2request, 9);
    Serial.flush();
    memset(co2response, 0, 9);
    count = Serial.readBytes(co2response, 9);
    delay(50);
    Serial.pins(1, 3);
    delay(50);
  }

  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=co2response[i];
  crc = 255 - crc;
  crc++;

  if ( !(co2response[0] == 0xFF && co2response[1] == 0x86 && co2response[8] == crc) ) {
    Serial.print("0x");
    Serial.print(co2response[0],HEX);
    Serial.print(co2response[1],HEX);
    Serial.print(co2response[2],HEX);
    Serial.print(co2response[3],HEX);
    Serial.print(co2response[4],HEX);
    Serial.print(co2response[5],HEX);
    Serial.print(co2response[6],HEX);
    Serial.print(co2response[7],HEX);
    Serial.print(co2response[8],HEX);
    Serial.println();
    Serial.println("CRC error: " + String(crc));
  } else {
    unsigned int responseHigh = (unsigned int) co2response[2];
    unsigned int responseLow = (unsigned int) co2response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    Serial.print("CO2: ");
    Serial.print(ppm);
    Serial.println(" ppm");
  }
  Serial.println();
}
