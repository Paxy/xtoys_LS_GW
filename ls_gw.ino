#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>

 char* ssid = "<<wifi ssid>>";
 char* password = "<<wifi psk>>";

 char* mqttServer = "<<mqtt domain name>>";
 int mqttPort = 1883;
 char* mqttUser = "<<mqtt user>>";
 char* mqttPassword = "<<mqtt password>>";
 const char* xtoys_topic="<<mqtt topic for ex xtoys/gw/>>";

static uint16_t companyId = 0xFFF0;

#define MANUFACTURER_DATA_PREFIX 0x6D, 0xB6, 0x43, 0xCE, 0x97, 0xFE, 0x42, 0x7C

uint8_t manufacturerDataList[][11] = {
    // Stop all channels
    {MANUFACTURER_DATA_PREFIX, 0xE5, 0x15, 0x7D},
    // Set all channels to speed 1
    {MANUFACTURER_DATA_PREFIX, 0xE4, 0x9C, 0x6C},
    // Set all channels to speed 2
    {MANUFACTURER_DATA_PREFIX, 0xE7, 0x07, 0x5E},
    // Set all channels to speed 3
    {MANUFACTURER_DATA_PREFIX, 0xE6, 0x8E, 0x4F},
    // Stop 1st channel (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xD5, 0x96, 0x4C},
    // Set 1st channel to speed 1 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xD4, 0x1F, 0x5D},
    // Set 1st channel to speed 2 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xD7, 0x84, 0x6F},
    // Set 1st channel to speed 3 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xD6, 0x0D, 0x7E},
    // Stop 2nd channel (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xA5, 0x11, 0x3F},
    // Set 2nd channel to speed 1 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xA4, 0x98, 0x2E},
    // Set 2nd channel to speed 2 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xA7, 0x03, 0x1C},
    // Set 2nd channel to speed 3 (only for toys with 2 channels)
    {MANUFACTURER_DATA_PREFIX, 0xA6, 0x8A, 0x0D},
};

const char *deviceName = "MuSE_Advertiser";

WiFiClient espClient;
PubSubClient client(espClient);

void connectMQTT(){
  if(!client.connected()){
    while (!client.connected()) {
    Serial.println(F("Connecting to MQTT..."));
 
    if (client.connect("LS-GW", mqttUser, mqttPassword )) {
 
      Serial.println(F("connected"));  
 
    } else {
 
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  client.subscribe(xtoys_topic);  
  advertiseManufacturerData(0);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if((char)payload[2]=='1'){
    if((char)payload[0]=='0') advertiseManufacturerData(4);
    else if((char)payload[0]=='1') advertiseManufacturerData(5);
    else if((char)payload[0]=='2') advertiseManufacturerData(6);
    else if((char)payload[0]=='3') advertiseManufacturerData(7);
  }else if((char)payload[2]=='2'){
    if((char)payload[0]=='0') advertiseManufacturerData(8);
    else if((char)payload[0]=='1') advertiseManufacturerData(9);
    else if((char)payload[0]=='2') advertiseManufacturerData(10);
    else if((char)payload[0]=='3') advertiseManufacturerData(11);
  }
  String data = String((char*)payload);
  Serial.printf("Got data: %s\n",data);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE...");
  NimBLEDevice::init(deviceName);

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println(F("Connection Failed! Rebooting..."));
    delay(5000);
    ESP.restart();
  }


  connectMQTT();

}

void advertiseManufacturerData(uint8_t index) {
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->stop();

  uint8_t *manufacturerData = manufacturerDataList[index];

  Serial.print("Advertising index: ");
  Serial.print(index);
  Serial.print(", data: ");
  for (int i = 0; i < 11; i++) {
    Serial.print(manufacturerDataList[index][i], HEX);
    if (i < 10) {
      Serial.print(", ");
    }
  }
  Serial.println();
  Serial.flush(); // Flush to ensure data is sent before delay

  pAdvertising->setManufacturerData(std::string((char *)&companyId, 2) + std::string((char *)manufacturerData, 11));

  // Set properties: scannable, connectable, and use legacy advertising
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setMinPreferred(0x02);

  // Start advertising
  pAdvertising->start();
}

void loop() {
  connectMQTT();  
  client.loop();
}
