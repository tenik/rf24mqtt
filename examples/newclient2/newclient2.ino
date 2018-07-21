#include <dht.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include<stdlib.h>
#include "printf.h"
#include <stdio.h>
#include <avr/pgmspace.h>
#include <RF24MQTT.h>


RF24NetworkHeader header_for_send;
// nRF24L01(+) radio attached using Getting Started board 
RF24 radio(9,10);

// Network uses that radio
RF24Network network(radio);

// Address of our node
const uint16_t this_node = 2;

// Address of the other node
const uint16_t gate_node = 0;

RF24MQTT rf24mqtt(network, &onMessage, &onConnect,  gate_node, this_node);

const char devAddress[] PROGMEM = "2";
const char topic1[] PROGMEM = "/dev/2/test";
const char topic2[] PROGMEM = "/dev/2/switch";
const char topicTemp[] PROGMEM = "/dev/2/temp";
const char topicHumm[] PROGMEM = "/dev/2/humm";
const char topicDelay[] PROGMEM = "/dev/2/delay";


#define DHT22_PIN 2
#define pirPIN 3

dht DHT;
int chk;
unsigned long delayValue = 60*1000;

int count = 0;
unsigned long lastTime;
unsigned long lastPingTime;


//=====[ PINS ]=================================================================
int Led = 13;
 
//=====[ VARIABLES ]============================================================
    char Data[32];       // ditto for data size
    int counter;
volatile    int sendAck;
 
//=====[ SUBROUTINES ]==========================================================
 
 void processDHT()
{
  chk = DHT.read22(DHT22_PIN);
/*  switch (chk)
  {
    case DHTLIB_OK:  
                Serial.print("OK,\t"); 
                break;
    case DHTLIB_ERROR_CHECKSUM: 
                Serial.print("Checksum error,\t"); 
                break;
    case DHTLIB_ERROR_TIMEOUT: 
                Serial.print("Time out error,\t"); 
                break;
    default: 
                Serial.print("Unknown error,\t"); 
                break;
  } 
  */
}


void onMessage(char *msg_topic, char *msg_data)
{
 if (!strcmp(msg_topic, getString(topicDelay)))  //topic: delay}
   {
     delayValue = atol(msg_data)*1000;
   }
}

void onConnect()
{
  Serial.println("connected");
  rf24mqtt.sub(getString(topicDelay));
  rf24mqtt.update();
  
  counter = 0;
  lastTime = millis();
  lastPingTime = millis();
  sendAck = 0;
  
}

void connectToGate()
{
  Serial.println("start connect...");
  while (!rf24mqtt.connect())
  {
     Serial.print("res ");
     Serial.println(rf24mqtt.sendError);
     delay(2000);
     Serial.println("retry");
  }
}

void setup()
{
  analogReference(INTERNAL);
  Serial.begin(57600);
  Serial.setTimeout(10);
  printf_begin();

  pinMode(13, OUTPUT);
  //pinMode(pirPIN, INPUT);
  //digitalWrite(13,1);
  
  SPI.begin();
  radio.begin();
  if (radio.setDataRate(RF24_250KBPS))
  {
    Serial.println("250kbps data rate ok!");
  }
  network.begin(/*channel*/ 100, /*node address*/ this_node);  
  
  connectToGate();
}


void loop()
{
  // Pump the network regularly
  rf24mqtt.update();


  if (millis()-lastTime>delayValue)
  {
//     counter++;
//     itoa(counter, Data, 10);
     processDHT();
//     value = DHT.temperature;
     dtostrf(DHT.temperature,-6,1,Data);
     rf24mqtt.pub(getString(topicTemp),Data); 
     rf24mqtt.update();     
     dtostrf(DHT.humidity,-6,1,Data);
     rf24mqtt.pub(getString(topicHumm),Data); 
     rf24mqtt.update();  
     lastTime = millis();
  }
  
}
