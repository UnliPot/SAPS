#pragma once

#include <Arduino.h>

namespace SDSettings {

extern int brightnessLevel;
extern bool screenIsSleeping;
extern int motorSpeed;

int nextCustomProfileIndex();

bool begin();
bool loadSettings();
bool saveSettings();

}
