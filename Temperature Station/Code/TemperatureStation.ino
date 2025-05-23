#include <WiFi.h>
#include <esp_now.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4  
uint8_t receiverMAC[] = { 0x18, 0x8B, 0x0E, 0xFE, 0xF9, 0xD8 };

typedef struct struct_message {
  float temperature;
} struct_message;

struct_message outgoingData;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  Serial.print("Sender MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C) {
    outgoingData.temperature = tempC;
    Serial.print("Sending temperature: ");
    Serial.println(tempC);

    esp_now_send(receiverMAC, (uint8_t *)&outgoingData, sizeof(outgoingData));
  } else {
    Serial.println("DS18B20 not found or disconnected");
  }

  delay(30000);  
}
