#include <Arduino.h>

#include <SD.h>

#include <ESP8266mDNS.h>

#include <ArduinoOTA.h>

#include <SI7021.h>


#include "..\lib\Wifi\Wifi.h"
#include "..\lib\TimeSync\TimeSync.h"
#include "..\lib\NRF\NRF.h"

byte co2request[9] =  { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
byte co2response[9];

Sd2Card card;
SdVolume volume;
SdFile root;

SI7021 sensor;

void setup() {

  Serial.begin(9600);
  Wire.begin();
  SPI.begin();

  delay(1000);
  Serial.println();

  if (!InitWifi()) {
    Serial.println("Rebooting...");
    delay(5000);
    ESP.restart();
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

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("esp8266");
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

  Serial.println("Initializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, D8)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }


  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);

  uint32_t bpc = volume.blocksPerCluster();
  uint32_t clusters = volume.clusterCount();

  Serial.print("Clusters: ");
  Serial.println(clusters, DEC);
  Serial.print("Blocks: ");
  Serial.println(bpc * clusters, DEC);
  Serial.print("Blocks per cluster: ");
  Serial.println(bpc, DEC);
  Serial.println("Block size: 512");
  Serial.print("Cluster size: ");
  Serial.println(bpc * 512, DEC);
  Serial.println();


  volumesize = bpc;        // clusters are collections of blocks
  volumesize *= clusters;  // we'll have a lot of clusters
  volumesize *= 512;       // SD card blocks are always 512 bytes
  Serial.print("Volume size (bytes): ");
  Serial.println(volumesize);
  Serial.print("Volume size (Kbytes): ");
  float kb = volumesize / 1024.0;
  Serial.println(kb);
  Serial.print("Volume size (Mbytes): ");
  float mb = volumesize / (1024.0 * 1024);
  Serial.println(mb);
  Serial.print("Volume size (Gbytes): ");
  float gb = volumesize / (1024.0 * 1024 * 1024);
  Serial.println(gb);


  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

  Serial.println("Setup complete");
}

time_t lastTime = 0;

void loop() {
  ArduinoOTA.handle();
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
