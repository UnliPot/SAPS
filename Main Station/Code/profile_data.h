#pragma once

#include <ArduinoJson.h>
#include <SD.h>
#include <vector>
#include <string>
#include <cmath>

struct Step {
  int number;
  std::string name;
  int duration; // seconds
  int modifier = 0;
};

struct Profile {
  std::string name;
  std::string filename; 
  std::vector<Step> steps;
};

namespace ProfileData {
  extern Profile currentProfile;
  extern int currentStepIndex;

  bool loadProfileFromFile(const String& filename);
  void resetStepIndex();
  bool nextStep();
  bool prevStep();
  const Step* getCurrentStep();
  Step* getMutableCurrentStep();
  const char* getProfileName();
}

inline int getEffectiveDuration(const Step& step) {
  return round(step.duration * pow(1.33, step.modifier));
} 