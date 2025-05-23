#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "elements.h"
#include "screen_manager.h"

// MAC address of the sender
uint8_t senderMAC[] = {0x14, 0x2B, 0x2F, 0xEB, 0xC2, 0x84};

typedef struct struct_message {
  float temperature;
} struct_message;

struct_message incomingData;

unsigned long lastReceivedTime = 0;
const unsigned long timeoutDuration = 180000; 

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingDataRaw, int len) {
  if (len == sizeof(struct_message)) {
    memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
    Temperature = incomingData.temperature;
    TemperatureIsConnected = true;
    lastReceivedTime = millis();

    Serial.print("Received from: ");
    for (int i = 0; i < 6; ++i) {
      Serial.printf("%02X", recv_info->src_addr[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();

    Serial.print("Temperature: ");
    Serial.println(Temperature);
    switchToScreen(currentScreen);
  } else {
    Serial.println("Invalid data size received");
  }
}

void setupEspNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, senderMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(senderMAC)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
    }
  }
}

void updateTemperatureConnection() {
  if (millis() - lastReceivedTime > timeoutDuration) {
    TemperatureIsConnected = false;
  }
}