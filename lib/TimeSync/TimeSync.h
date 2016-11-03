#include <NTPClient.h>
#include <WiFiUdp.h>

#include <TimeLib.h>

#include <Wire.h>
#include <DS3232RTC.h>

void PrintWireStatus(byte status)
{
  if(status == 0){
    Serial.println("success");
  }
  if(status == 1){
    Serial.println("data too long to fit in transmit buffer");
  }
  if(status == 2){
    Serial.println("received NACK on transmit of address");
  }
  if(status == 3){
    Serial.println("received NACK on transmit of data");
  }
  if(status == 4){
    Serial.println("other error");
  }
}

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", (2) * SECS_PER_HOUR);
DS3232RTC RTC;

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}


void displayTime(tmElements_t &tm)
{
  Serial.print(dayShortStr(tm.Wday));
  Serial.write(' ');
  print2digits(tm.Hour);
  Serial.write(':');
  print2digits(tm.Minute);
  Serial.write(':');
  print2digits(tm.Second);
  Serial.write(' ');
  print2digits(tm.Day);
  Serial.write(' ');
  Serial.print(monthShortStr(tm.Month));
  Serial.write(' ');
  Serial.print(tmYearToCalendar(tm.Year));
  Serial.println();
}

void displayTime(){
  time_t t = now();
  tmElements_t tm;
  breakTime(t, tm);
  displayTime(tm);
}

time_t getNtpTime()
{
  return timeClient.getEpochTime();
}

uint8_t dec2bcd(uint8_t n)
{
    return n + 6 * (n / 10);
}

#define MaxTries 3

bool SyncTime()
{
  RTC.squareWave(SQWAVE_NONE);

  if(WiFi.getMode() == WIFI_OFF){
    Serial.println("WiFi is off. No NTP time!");
    setSyncProvider(RTC.get);
    Serial.print("Current RTC time: ");
    displayTime();
    return true;
  }else{
    Serial.println("Syncing time...");

    timeClient.begin();
    int tries = 0;
    while(true) {
      tries++;
      if(tries > MaxTries){
        break;
      }
      Serial.println("Updating NTP time...");
      if(timeClient.forceUpdate()){
        break;
      }
      delay(1000);
    }
    if(tries > MaxTries){
      Serial.println("Failed to update NTP time");
      setSyncProvider(RTC.get);
      Serial.print("Current RTC time: ");
      displayTime();
      return true;
    }

    Serial.print("Got NTP time: ");

    time_t ntpTime = timeClient.getEpochTime();
    setTime(ntpTime);
    tmElements_t tm;
    breakTime(ntpTime, tm);
    displayTime(tm);
  }

  tries = 0;
  while(true) {
    tries++;
    if(tries > MaxTries){
      break;
    }

    Serial.print("Updating RTC time... ");
    byte setResult = RTC.write(tm);
    PrintWireStatus(setResult);
    if(setResult == 0){
      break;
    }
    delay(1000);
    breakTime(now(), tm);
  }
  if(tries > MaxTries){
    Serial.println("Failed to update RTC time");
    setSyncProvider(getNtpTime);
  }else{
    setSyncProvider(RTC.get);
  }

  return true;
}
