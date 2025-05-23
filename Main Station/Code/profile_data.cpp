#include "profile_data.h"
#include "screen_manager.h"
#include "main_screen.h"

Profile ProfileData::currentProfile = {};
int ProfileData::currentStepIndex = 0;

Step* ProfileData::getMutableCurrentStep() {
  if (currentStepIndex >= 0 && currentStepIndex < currentProfile.steps.size()) {
    return &currentProfile.steps[currentStepIndex];
  }
  return nullptr;
}

bool ProfileData::loadProfileFromFile(const String& filename) {
  String path = filename;
  if (!filename.startsWith("/")) {
    path = "/" + filename;
  }

  File file = SD.open(path);
  if (!file) {
    Serial.printf("[ProfileData] Failed to open profile file: %s\n", path.c_str());
    return false;
  }

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.print("[ProfileData] JSON parse failed: ");
    Serial.println(error.c_str());
    return false;
  }

  currentProfile.name = doc["name"].as<const char*>();
  currentProfile.filename = std::string(path.c_str()); 
  currentProfile.steps.clear();

  JsonArray stepsArray = doc["steps"].as<JsonArray>();
  for (JsonObject stepObj : stepsArray) {
    Step s;
    s.number = stepObj["step"];
    s.name = stepObj["nameofstep"].as<const char*>();
    s.duration = stepObj["duration"];
    s.modifier = 0;
    currentProfile.steps.push_back(s);
  }
  currentStepIndex = 0;
  Serial.printf("[ProfileData] Loaded '%s' with %d step(s)\n", currentProfile.name.c_str(), currentProfile.steps.size());

  if (!currentProfile.steps.empty()) {
    resetTimerState(getEffectiveDuration(currentProfile.steps[0]));
  }

  return true;
}

void ProfileData::resetStepIndex() {
  currentStepIndex = 0;
}

bool ProfileData::nextStep() {
  if (currentStepIndex + 1 < currentProfile.steps.size()) {
    currentStepIndex++;
    return true;
  }
  return false;
}

bool ProfileData::prevStep() {
  if (currentStepIndex > 0) {
    currentStepIndex--;
    return true;
  }
  return false;
}

const Step* ProfileData::getCurrentStep() {
  if (currentStepIndex < currentProfile.steps.size()) {
    return &currentProfile.steps[currentStepIndex];
  }
  return nullptr;
}

const char* ProfileData::getProfileName() {
  return currentProfile.name.c_str();
}
