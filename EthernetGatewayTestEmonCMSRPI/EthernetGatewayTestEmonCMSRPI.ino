// Simplified Moteino Gateway test
#include <RFM69.h>    //with KiwiSinceBirth mods
#include <SPI.h>

#include "apikey.h" 
//use tabs to add apikey.h file or uncomment this line
//#define APIKEY "xxxxxxxxxxxxxxxxx" //emoncms Account--> "Write API Key"




#define NODEID        1    //unique for each node on same network
#define NETWORKID     200  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200

#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

RFM69 radio;

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         temp;   //temperature
  int           moisture; //soil moisture
} Payload;
Payload theData;

void setup() {
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  //Serial.println(buff);
}

byte ackCount=0;
void loop() {

  if (radio.receiveDone())
  {
    
    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
    
    
    Serial.print(theData.nodeId);Serial.print(' ');
    Serial.print(theData.uptime);Serial.print(' ');
    Serial.print(theData.temp);Serial.print(' ');
    Serial.print(radio.RSSI);Serial.print(' ');
    Serial.print(theData.moisture);
    Serial.println();

    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      //Serial.print(" - ACK sent.");

    }
    //Serial.println();
    Blink(LED,3);
  }
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
