#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include "secret.h"
#include <OneWire.h>//serch by name
#include <DallasTemperature.h>//serch by name


//----------secret.h----------------
//const char ssid[] = "";
//const char pass[] = "";
//const char* mqtt_server = "1.1.1.1";
//const char *subscribeTopic = "{realm}/{clientId}/attribute/{attributeName}/{assetId}";//example: master/client123/attribute/subscribeAttribute/6xIa9MkpZuR7slaUGB6OTZ
//const char *publicTopic = "{realm}/{clientID}/writeattributevalue/{attributeName}/{assetID}";//example : master/client123/writeattributevalue/writeAttribute/6xIa9MkpZuR7slaUGB6OTZ
//const char* username = "master:username"; 
//const char* mqttpass = "*****"; 
//const char* clientID = "";
//#define K_PODGONA 100
//----------secret.h----------------

WiFiClientSecure net;
MQTTClient client;

unsigned long lastMillis = 0;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

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
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  connect();
}

void loop() {

loopAndReconnect();
  if (millis() - lastMillis > 1000*10) {
    lastMillis = millis();
    sensors.requestTemperatures(); 
    float temp = sensors.getTempCByIndex(0);
    String temperatureStr = String(temp);
    loopAndReconnect();
    client.publish(publicTopic, temperatureStr.c_str());
    //delay(100);
    Serial.print("Temperature : "); Serial.println(temp); 
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
