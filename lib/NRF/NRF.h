#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>

RF24 radio(D3, D0);
const uint64_t pipeIn =  0xE8E8F0F0E1LL;
//const short num_channels = 128;
//short values[num_channels];

bool InitNRF()
{
  Serial.println("Initializing NRF...");

  if(!radio.begin()){
    Serial.println("NRF begin failed");
  }
  radio.openReadingPipe(1, pipeIn);
  //radio.maskIRQ(false,false,false);

  Serial.println( "Setting PA level" );
  uint8_t paLevel = radio.getPALevel();
  if(paLevel != RF24_PA_MIN){
    radio.setPALevel(RF24_PA_MIN);
  }

  Serial.println( "Setting data rate" );
  if( !radio.setDataRate( RF24_250KBPS ) ) {
    Serial.println( "Data rate 250KBPS set FAILED!!" ) ;
  }

  Serial.println( "Enabling dynamic payloads" ) ;
  radio.enableDynamicPayloads();
  Serial.println( "Setting auto ack" ) ;
  radio.setAutoAck(true);
  Serial.println( "Powering up" ) ;
  radio.powerUp();
  Serial.println( "Starting to listen" ) ;
  radio.startListening();
  Serial.println( "NRF initiated" ) ;

  radio.printDetails();
  return true;
}
