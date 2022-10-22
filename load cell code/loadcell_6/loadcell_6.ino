#include <HX711.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <String.h>

byte mac[] = {0x12, 0x3A, 0x00, 0xBB, 0x06, 0x12};
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
const char* mqttServer = "mqtt-io.kku.ac.th";
const int mqttPort = 1991;
const char* mqttUser = "trash";
const char* mqttPass = "1q2w3e4r@trash";
const char* mqttTopic = "smartcity/trash/00006";
char* mqttPayload = "";

char *id = "20006"; // 2000x is for trash ID
unsigned long currentTime;
unsigned long prevTime = 0;
const int interval = 20000; //Interval time

float calibration_factor = 45471.00; 
#define zero_factor 8492602
#define DOUT  8
#define CLK   7
#define DEC_POINT  2

float offset=0;
float get_units_kg();
HX711 scale(DOUT, CLK);

void setup() 
{
  Serial.begin(9600);
    Serial.println("Ethernet initiation, Just wait a minute.");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Don't receive ip from dhcp server.");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield connection error!");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Cable is not connected.");
    } else {
      Serial.println("Error!");
    }
    while(true){
      delay(1);
    }
  }
  if(Ethernet.linkStatus() == 0) { 
    print_ip_details();
  } else {
    Serial.println("Error!");
  }
  mqttClient.setServer(mqttServer,mqttPort);
  mqttClient.connect(id, mqttUser, mqttPass);
  Serial.println("Load Cell");
  scale.set_scale(calibration_factor); 
  scale.set_offset(zero_factor);   
}

void loop() 
{ 
  currentTime = millis();
  DynamicJsonDocument doc(1024);
  Serial.print("Reading: ");
  String data = String(get_units_kg()+offset, DEC_POINT);
  Serial.print(data);
  Serial.println(" kg");
  if (currentTime - prevTime >= interval) {
    prevTime = currentTime;
  String jData =  String("{\"weight\": ") + data + String("}");
  char buf[100];
  jData.toCharArray(buf, 100);
  mqttPayload = buf;
  mqttClient.publish(mqttTopic, mqttPayload);
  if (!mqttClient.connected()) {
    mqttClient.connect(id, mqttUser, mqttPass);
    Serial.println("Send to MQTT!");
  }
     
  }
}

float get_units_kg()
{
  return(scale.get_units()*0.453592);
}

void print_ip_details() {
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
