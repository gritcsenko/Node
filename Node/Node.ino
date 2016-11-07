#include <Arduino.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

#include <ESP8266mDNS.h>

#include <SI7021.h>
#include <Adafruit_BMP085.h>

#include "..\lib\CO2\CO2.h"
#include "..\lib\Storage\Storage.h"
#include "..\lib\Settings\Settings.h"
#include "..\lib\OTA\OTA.h"
#include "..\lib\Wifi\Wifi.h"
#include "..\lib\TimeSync\TimeSync.h"
#include "..\lib\NRF\NRF.h"

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

  double P = bmp.readPressure() / 100.0;
  Serial.print("Temp: "); Serial.print(bmp.readTemperature()); // Celsius
  Serial.print("\tPressure: "); Serial.println(P); // Gecto Pascals

  double V = 1.0;
  double Rv = 461.5;

  double T = 273.15 + t;
  double e_omega_t = 6.112*exp((17.62*T - 4812.903)/(T - 30.03));
  double f = 1.0016 + 0.00000315*P - 0.074/P;
  double e_omega = e_omega_t*f;
  Serial.print("e omega[hPa]: "); Serial.println(e_omega);
  double e = RH*e_omega/100;
  double AH = e/(Rv*T);
  double m = AH*V*1000000.0;

  Serial.print("Water mass[mg]: "); Serial.println(m);

  int ppm = ReadCO2PPM();
  if(ppm < 0){
    Serial.println("No CO2 valid response");
  }else{
    Serial.print("CO2: ");
    Serial.print(ppm);
    Serial.println(" ppm");
  }

  Serial.println();
}
