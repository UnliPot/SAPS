#include "motor_control.h"

namespace {
  const uint8_t SLAVE_ADDR = 0x08;
}

namespace MotorControl {

void sendEnable(bool enable) {
  if (enable){
    Serial.println("enable true send");
    Serial1.println("START");
  }
  if (!enable) {
    Serial.println("enable false send");
    Serial1.println("STOP");
  }
}

void sendSpeed(uint8_t percent) {
  Serial.println("speed noted send");
  Serial1.println("SPEED:" + String(percent));
}

void sendBeep(uint16_t freq, uint16_t dur) {
  Serial.print("sendBeep: ");
  Serial.print(freq);
  Serial.print(" Hz for ");
  Serial.print(dur);
  Serial.println(" ms");
  Serial1.println("BEEP:" + String(freq) + ":" + String(dur));
}

}
