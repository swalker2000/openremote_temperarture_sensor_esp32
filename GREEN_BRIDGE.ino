#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "secret.h"


//----------secret.h----------------
//const char ssid[] = "";
//const char pass[] = "";
//const char* mqtt_server = "1.1.1.1";
//const char *subscribeTopic = "{realm}/{clientId}/attribute/{attributeName}/{assetId}";//example: master/client123/attribute/subscribeAttribute/6xIa9MkpZuR7slaUGB6OTZ
//const char *publicTopic = "{realm}/{clientID}/writeattributevalue/{attributeName}/{assetID}";//example : master/client123/writeattributevalue/writeAttribute/6xIa9MkpZuR7slaUGB6OTZ
//const char* username = "master:username"; 
//const char* mqttpass = "*****"; 
//const char* clientID = "";
//----------secret.h----------------

WiFiClientSecure net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  net.setInsecure();
  while (!client.connect(clientID, username, mqttpass)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  client.subscribe(subscribeTopic);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  client.begin(mqtt_server, 8883, net);
  client.onMessage(messageReceived);
  //pinMode(25, INPUT);
  connect();
}

void loop() {

loopAndReconnect();
  if (millis() - lastMillis > 1000*60) {
    lastMillis = millis();
    int value = analogRead(34);
    
    Serial.print("Value : "); Serial.print(value);
    int temp = getTemperatureFromAnalogRead(value);
    Serial.print(", temperature : "); Serial.println(temp);
    String temperatureStr = String(temp);
    loopAndReconnect();
    client.publish(publicTopic, temperatureStr.c_str());
    //delay(100);
    //Serial.println("Published"); 
  }
}

void loopAndReconnect()
{
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }
}

  #define MAX_READ  4095
  #define TEMP_STEP  5
  #define MIN_TEMP -55
  #define ZERRO 273//абсолютный 0
  //все хначения умножены на MAX_READ (4095)
  long rtTable[] = {394349, 274406, 193161, 137797, 99345,//-55 -35
                    72482,  53399,  39750,  29865,  22658,//-30 -1
                    17330,  13370,  10397,  8149,   6433, //-5 15
                    5115,   4095,   3299,   2674,   2181, //20 40
                    1789,   1475,   1223,   1019,   853,  //45 65
                    717,    606,    515,    439,    376,  //70 90
                    323,    278,    241,    209,    182,  //95 115
                    159,    140,    123,    109,    96,   //120 140
                    85,     76,     68};                  //145 155

int rtTableLength =43;



int getTemperatureFromAnalogRead(int valueLow)
{
  int valueHigh = MAX_READ-valueLow;
  long rtValue = long(MAX_READ*long(valueLow))/long(valueHigh);
  for (int i=0; i<rtTableLength-1; i++)
  {
    if (rtValue<rtTable[i])
    {
        int stepRt = rtTable[i]- rtTable[i+1];
        int deltaValue = rtTable[i]- rtValue;
        int dt=(deltaValue*TEMP_STEP)/stepRt;
        int t = MIN_TEMP+TEMP_STEP*i+dt;
        return t;
    }
  }
  return 160;


}

