#include "sd_settings.h"
#include "motor_control.h"
#include <FS.h>
#include <SD.h>
#include <ArduinoJson.h>

namespace SDSettings {

int brightnessLevel = 250;
bool screenIsSleeping = false;
int motorSpeed = 100;

const char* SETTINGS_PATH = "/settings.json";

int nextCustomProfileIndex() {
  File file;
  int index = 1;
  char filename[32];

  while (true) {
    snprintf(filename, sizeof(filename), "/custom_profile_%d.json", index);
    file = SD.open(filename);
    if (!file) break;
    file.close();
    index++;
  }

  return index;
}

bool begin() {
  if (!SD.begin()) {
    Serial.println("[SD] Card mount failed");
    return false;
  }
  Serial.println("[SD] Card initialized");
  return true;
}

bool loadSettings() {
  if (!SD.exists(SETTINGS_PATH)) {
    Serial.println("[SD] Settings file not found, creating default");
    return saveSettings();
  }

  File file = SD.open(SETTINGS_PATH);
  if (!file) {
    Serial.println("[SD] Failed to open settings file");
    return false;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("[SD] Failed to parse settings: ");
    Serial.println(error.c_str());
    return false;
  }

  if (doc.containsKey("brightness")) {
    brightnessLevel = doc["brightness"].as<int>();
    brightnessLevel = constrain(brightnessLevel, 25, 250);
  }

  if (doc.containsKey("motorSpeed")) {
    motorSpeed = doc["motorSpeed"].as<int>();
    motorSpeed = constrain(motorSpeed, 0, 100);
  }

  Serial.println("[SD] Settings loaded");
  return true;
}

bool saveSettings() {
  StaticJsonDocument<256> doc;
  doc["brightness"] = brightnessLevel;
  doc["motorSpeed"] = motorSpeed;

  File file = SD.open(SETTINGS_PATH, FILE_WRITE);
  if (!file) {
    Serial.println("[SD] Failed to open settings file for writing");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("[SD] Failed to write settings to file");
    file.close();
    return false;
  }

  file.close();
  Serial.println("[SD] Settings saved");
  return true;
}

}

