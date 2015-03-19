#include <LowPower.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 7
// Simplified Moteino Node Test

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
int  resolution = 9;

#define NODEID        9    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

#define SERIAL_BAUD   9600

int TRANSMITPERIOD = 1000; //transmit a packet to gateway so often (in ms)
char payload[30];
boolean requestACK = false;
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
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  
  pinMode(A1, OUTPUT);
  pinMode(A1, LOW);
  
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);
  
}

long lastPeriod = 0;
void loop() {

  digitalWrite(A1, HIGH); // Turn on Soil Moisture Sensor
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    Blink(LED,3);
    Serial.println();
  }



sensors.requestTemperatures();
//Serial.println(sensors.getTempCByIndex(0));  

      int temperatureReading = sensors.getTempCByIndex(0);
      int soilMoisture = analogRead(A0);
      digitalWrite(A1, LOW); //Turn off Soil Moisture sensor
      theData.nodeId = NODEID;
      theData.uptime = millis();
      theData.temp = temperatureReading;
      theData.moisture = soilMoisture;
      
      if (radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData)),2,100)
      {
        Serial.println(F(" ok!"));
      }
      else {
        Serial.println(F(" nothing..."));
      }
    Blink(LED,3);

  Serial.flush();
  radio.sleep();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
