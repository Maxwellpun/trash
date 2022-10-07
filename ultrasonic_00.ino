#include <Arduino.h>

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Count Trash
const int trigPin1 = 8; 
const int echoPin1 = 7;
unsigned long distance1;
long duration1;

// Trash Level
const int trigPin2 = 2;
const int echoPin2 = 3;
unsigned long distance2;
long duration2;

int trash=0;
float trashLevel=0;

long currentTime;
long prevTime = 0;
long prevTime2 = 0;
int sendTrashLevelInterval=10000; //Interval sending data to MQTT in 
int delayDropInterval=1200;

byte mac[] = {0x12, 0x3A, 0x00, 0xBB, 0x00, 0x09}; //hex
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

const char *mqttServer = "mqtt-io.kku.ac.th";
const int mqttPort = 1991;
const char *mqttUser = "trash";
const char *mqttPass = "1q2w3e4r@trash";
const char *mqttTopic = "smartcity/trash/00001";
char *mqttPayload;

const char *id = mqttTopic;

void print_ip_details()
{
  Serial.print("Ethernet Status : ");
  Serial.println(Ethernet.linkStatus());
  Serial.print("IP Address : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("IP Gateway : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS : ");
  Serial.println(Ethernet.dnsServerIP());
}

void initEthernet()
{
  Serial.println("Ethernet initiation, Just wait a minute.");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Don't receive ip from dhcp server.");
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield connection error!");
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Cable is not connected.");
    }
    else
    {
      Serial.println("Error!");
    }
    while (true)
    {
      delay(200);
    }
  }
  if (Ethernet.linkStatus() == 0)
  {
    print_ip_details();
  }
  else
  {
    Serial.println("Error!");
  }
}

float sendtoMqtt(String key, float value)
{
  String jData = String("{") + String("\"") + key + String("\"") + String(":") + value + String("}");
  char buf[100];
  jData.toCharArray(buf, 100);
  mqttPayload = buf;
  mqttClient.publish(mqttTopic, mqttPayload);
  if (!mqttClient.connected())
  {
    mqttClient.connect(id, mqttUser, mqttPass);
  }
  return 0;
}


void readTrashing() //trashing
{
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = (duration1 /29.1)/2;
  Serial.print("Drop Distance: ");
  Serial.print(distance1);
  Serial.print(" cm. ");
  Serial.println();
  delay(50);
  if (distance1 < 22){
    if (currentTime - prevTime >= delayDropInterval) {
      prevTime = currentTime;
    sendtoMqtt("drop", 1);
    }
  }
  }

void readLevel() //trash level
{
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    duration2 = pulseIn(echoPin2, HIGH);
    distance2 = (duration2 / 29.1) / 2;
    Serial.print("Level: ");
    Serial.print(distance2);
    Serial.print(" cm. ");
    Serial.println();
    delay(50);

    if (currentTime - prevTime2 >= sendTrashLevelInterval)
    {
      prevTime2 = currentTime;
      sendtoMqtt("level", distance2);
    }
}

void setup(){
  Serial.begin(921000);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  initEthernet();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.connect(id, mqttUser, mqttPass);
}


void loop(){
  currentTime = millis();
  readTrashing();
  delay(100);
  readLevel();
  delay(100);
}
