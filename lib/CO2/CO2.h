#include <Arduino.h>

byte co2request[9] =  { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
byte co2response[9];

byte co2Header[2] = { 0xFF, 0x86 };

void SwitchToCO2(){
  delay(50);
  Serial.pins(2, 3);
  Serial.setTimeout(300);
  delay(50);
}

void SwitchToUSB(){
  delay(50);
  Serial.pins(1, 3);
  delay(50);
}

bool CheckCrc(){
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=co2response[i];
  crc = 255 - crc;
  crc++;

  if (co2response[8] == crc) {
    return true;
  }

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
  return false;
}

void SendRequest(){
  Serial.readString(); // dump buffer to /dev/null
  Serial.write(co2request, 9);
  Serial.flush();
}

int ReadCO2PPM(){
  Serial.flush();

  SwitchToCO2();
  SendRequest();
  memset(co2response, 0, 9);
  int tries = 5;
  while (!Serial.find(co2Header, 2) && tries >= 0) {
    SendRequest();
    tries--;
  }
  if(tries < 0){
    SwitchToUSB();
    return -1;
  }

  //copy header
  for(int i = 0; i < 2; i++) co2response[i] = co2Header[i];

  Serial.readBytes(co2response + 2, 7);
  SwitchToUSB();


  if ( CheckCrc() ) {
    unsigned int responseHigh = (unsigned int) co2response[2];
    unsigned int responseLow = (unsigned int) co2response[3];
    int ppm = (responseHigh << 8) + responseLow;
    return ppm;
  }else{
    return -1;
  }
}
