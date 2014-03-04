/*
 Copyright (C) 2014 Andrey Osipchuk andreyosipchuk@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include "RF24.h"
#include "RF24Network.h"
#include "RF24MQTT.h"

char stringBuffer[MAX_STRING];
char* getString(const char* str) {
	strcpy_P(stringBuffer, (char*)str);
	return stringBuffer;
}

RF24MQTT::RF24MQTT(RF24Network& _network, RF24MQTT_onMsgFunc _onMsg, RF24MQTT_onConnectFunc _onConnect ,uint16_t _gate_node, uint16_t _this_node): network(_network), onMsg(_onMsg), onConnect(_onConnect),gate_node(_gate_node), this_node(_this_node)
{
  delayValue = 5*1000;
  timeOutDelay = 2*1000; //acknowledgement time
  pingDelay = 5*1000;    // time between ping commands

}

void RF24MQTT::RFParser(void)
{
    RF24NetworkHeader header;
    
    memset(Buffer, 0, sizeof(Buffer));   // Clear contents of Buffer
    network.read(header,&Buffer,sizeof(Buffer));

    switch(header.type)
    {
      case MSG_CONNACK://conack
        connAck = 1;
        break;
      case MSG_PINGRESP://pingresp
        pingResp = 1;
        break;
      case MSG_PUB://publish
        // ��� ���� ���������� ��������� ���������
        //Serial.print("incoming:"); 
        //Serial.println(Buffer);         
        strcpy(Topic,strtok(Buffer," "));
        strcpy(Data,strtok(NULL," \n\0"));

        onMsg(Topic, Data);
        break;
    } 
    memset(Buffer, 0, sizeof(Buffer));   // Clear contents of Buffer
}

int RF24MQTT::connect(void)
{
   connAck = 0;
   sendError = 0;
   header_for_send.type = MSG_CONNECT;
   header_for_send.to_node = gate_node;
   header_for_send.from_node = this_node;
   
   int res =  network.write(header_for_send, payload, 1);
   if (!res) 
   { 
     sendError = 1;
     return false;
   }
   unsigned long t = millis();
   while (!connAck && millis()-t < timeOutDelay)
   {
     update();   
#ifdef MQTTDEBUG    
     //Serial.print(".");
#endif
   }
#ifdef MQTTDEBUG    
     //Serial.println("");
#endif
   
   if (connAck)
   {
     onConnect(); 
   }
   return connAck; // �������� ������������� ��������
}

int RF24MQTT::ping(void)
{
   pingResp = 0;
   sendError = 0;
   header_for_send.type = MSG_PING;
   header_for_send.to_node = gate_node;
   header_for_send.from_node = this_node;
   
   int res =  network.write(header_for_send, payload, 1);
   if (!res) 
   { 
     sendError = 1;
     return false;
   }
   unsigned long t = millis();   
   while (!pingResp && millis()-t < timeOutDelay)
   {
     update();   
   }
   return pingResp; // �������� ������������� pinga
}

int RF24MQTT::sub(char *topic)
{
   header_for_send.type = MSG_SUB;
   header_for_send.to_node = gate_node;
   header_for_send.from_node = this_node;
   
   memset(Buffer, 0, sizeof(Buffer));   // Clear contents of Buffer
   strcpy(Buffer, topic);

#ifdef MQTTDEBUG
   Serial.print("sub:");
   Serial.println(Buffer); 
#endif   

   return network.write(header_for_send, topic, strlen(topic)+1);
    
}

int RF24MQTT::pub(char *topic, char *data)
{
   header_for_send.type = MSG_PUB;
   header_for_send.to_node = gate_node;
   header_for_send.from_node = this_node;
   
   memset(Buffer, 0, sizeof(Buffer));   // Clear contents of Buffer
   strcpy(Buffer, topic);
   strcat(Buffer," ");
   strcat(Buffer, data);

#ifdef MQTTDEBUG
   Serial.print("pub:");
   Serial.println(Buffer); 
#endif
      
   return network.write(header_for_send, Buffer, strlen(Buffer)+1);

}

int RF24MQTT::update()
{
  network.update();

  if ( network.available() ) // something come for us
  {
      RFParser();
  }
  
}
