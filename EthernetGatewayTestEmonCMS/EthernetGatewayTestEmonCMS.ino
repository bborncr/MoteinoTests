// Simplified Moteino Gateway test
#include <RFM69.h>    //with KiwiSinceBirth mods
#include <SPI.h>
#include <Ethernet.h>
#include <utility/w5100.h> //with KiwiSinceBirth mods
#include "apikey.h" 
//use tabs to add apikey.h file or uncomment this line
//#define APIKEY "xxxxxxxxxxxxxxxxx" //emoncms Account--> "Write API Key"


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };
//IPAddress server (192,168,10,176);
char server[] = "energia.crcibernetica.com";
IPAddress ip(192,168,10,178); //ip to use in case of DHCP failure
EthernetClient client;

#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack
#define SERIAL_BAUD   9600

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
  float         temp;   //temperature maybe?
} Payload;
Payload theData;

void setup() {
  W5100.select(7); //selects pin 7 as SS for Ethernet module (KiwiSinceBirth mod)
  Serial.begin(SERIAL_BAUD);
  delay(10);
  Serial.println(APIKEY);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
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
      
    
    
    client.stop();
      if(client.connect(server,80))

{

    client.print("GET /input/post.json?node=");  // make sure there is a [space] between GET and /input
    client.print(theData.nodeId);
    client.print("&csv=");
    client.print(theData.uptime);
    client.print(",");
    client.print(theData.temp);
    client.print("&apikey=");
    client.print(APIKEY);         //assuming MYAPIKEY is a char or string
    client.println(" HTTP/1.1");   //make sure there is a [space] BEFORE the HTTP
    client.println(F("Host: energia.crcibernetica.com"));
    client.print(F("User-Agent: Arduino-ethernet"));
    client.println(F("Connection: close"));
    client.println();
} else {
        Serial.println("Connection Failed");
      }
    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");

    }
    Serial.println();
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