/*
 Copyright (C) 2014 Andrey Osipchuk andreyosipchuk@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * @file RF24mqtt.h
 *
 * Class declaration for RF24mqtt 
 */

#ifndef __RF24MQTT_H__
#define __RF24MQTT_H__

#define MQTTDEBUG

const int bSize = 64;
const int dSize = 32;

#define MSG_CONNECT 'c'
#define MSG_CONNACK 'a'
#define MSG_PING 'r'
#define MSG_PINGRESP 'q'
#define MSG_PUB 'p'
#define MSG_SUB 's'


#define MAX_STRING 60

char* getString(const char* str);

typedef void (*RF24MQTT_onMsgFunc)(char *msg_topic, char *msg_data);
typedef void (*RF24MQTT_onConnectFunc)(void);

class RF24MQTT
{
private:

protected:

public:
  RF24NetworkHeader header_for_send;
  RF24Network& network;
  RF24MQTT_onMsgFunc onMsg;
  RF24MQTT_onConnectFunc onConnect;
  // Address of our node
  const uint16_t this_node;

  // Address of the other node
  const uint16_t gate_node;

  char payload[dSize];
  volatile int  connAck;
  volatile int  pingResp;
  volatile int  sendError;
  unsigned long delayValue;
  unsigned long timeOutDelay; //acknowledgement time
  unsigned long pingDelay;    // time between ping commands

  unsigned long lastPingTime;

  char Buffer[bSize];  // Serial buffer
  char Data[dSize];       // ditto for data size
  char Topic[dSize];       // ditto for data size
  int ByteCount;
  int counter;

  void RFParser(void);


  RF24MQTT(RF24Network& _network, RF24MQTT_onMsgFunc _onMsg, RF24MQTT_onConnectFunc _onConnect ,uint16_t _gate_node, uint16_t _this_node);

  int update();
  int connect(void);
  int ping(void);
  int sub(char *topic);
  int pub(char *topic, char *data);

};

#endif
