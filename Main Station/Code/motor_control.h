#pragma once
#include <Arduino.h>

namespace MotorControl {
  void sendEnable(bool enable);                 // true = start, false = stop
  void sendSpeed(uint8_t percent);              // 0–100%
  void sendBeep(uint16_t freq, uint16_t dur);   //send beep
}
