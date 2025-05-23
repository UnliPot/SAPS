#include "sd_settings.h"
#include "settings_screen.h"
#include "display.h"       
#include "screen_manager.h" 
#include "elements.h"
#include "sd_settings.h"
#include "motor_control.h"
#include "main_screen.h"

const int screenW = 800;
const int screenH = 480;
const int headerH = 40;

Button btnBrgUP;
Button btnBrgDOWN;
Button btnSpeedUP;
Button btnSpeedDOWN;
Button btnMotorSTART;
Button btnMotorSTOP;


void drawSettingsScreen() {
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Settings", true);
  lcd.fillRoundRect(10, 70, 780, 400, 8, lcd.color888(80,80,80));
  drawBrightnessBlock(0, 0, 200, 380);
  drawMotorSpeedBlock(220, 0, 200, 380);
  drawMotorSpeed(220, 0, 200, 380);
  drawBrightnessPercent(0,0,200,380);
  drawMotorControl(440, 0, 320, 380);
}

void drawBrightnessBlock(int bx, int by, int w, int h) {
  btnBrgUP = {20+bx+w/2-35, 80+by+h/2-(50+50), 70, 50, "U", lcd.color888(255,50,0)};
  btnBrgDOWN = {20+bx+w/2-35, 80+by+h/2+(50), 70, 50, "D", lcd.color888(255,50,0)};

  lcd.fillRoundRect(20+bx, 80+by, w, h, 8, lcd.color888(0,0,0));
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Brightness", 20+bx+w/2, by+110 );

  btnBrgUP.draw();
  btnBrgDOWN.draw();
}

void drawMotorControl(int bx, int by, int w, int h) {
  btnMotorSTART = {20+bx+w/2-130, 80+by+h/2-(50+50), 260, 100, "START", lcd.color888(255,50,0)};
  btnMotorSTOP = {20+bx+w/2-130, 80+by+h/2+(50), 260, 100, "STOP", lcd.color888(255,50,0)};

  lcd.fillRoundRect(20+bx, 80+by, w, h, 8, lcd.color888(0,0,0));
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Motor Control", 20+bx+w/2, by+110 );

  btnMotorSTART.draw();
  btnMotorSTOP.draw();
}

void drawMotorSpeedBlock(int bx, int by, int w, int h) {
  btnSpeedUP = {20+bx+w/2-35, 80+by+h/2-(50+50), 70, 50, "U", lcd.color888(255,50,0)};
  btnSpeedDOWN = {20+bx+w/2-35, 80+by+h/2+(50), 70, 50, "D", lcd.color888(255,50,0)};

  lcd.fillRoundRect(20+bx, 80+by, w, h, 8, lcd.color888(0,0,0));
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Motor Speed", 20+bx+w/2, by+110 );

  btnSpeedUP.draw();
  btnSpeedDOWN.draw();
}

void drawBrightnessPercent(int bx, int by, int w, int h) {
  int x = 20 + bx + w / 2;
  int y = 80 + by + h / 2;

  float percent = SDSettings::brightnessLevel / 2.5;
  String percentText = String((int)percent) + "%";

  lcd.fillRect(x - 40, y - 10, 80, 20, TFT_BLACK);  
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString(percentText, x, y);
}

void drawMotorSpeed(int bx, int by, int w, int h) {
  int x = 20 + bx + w / 2;
  int y = 80 + by + h / 2;

  float percent = SDSettings::motorSpeed;
  String percentText = String((int)percent) + "%";

  lcd.fillRect(x - 40, y - 10, 80, 20, TFT_BLACK);
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString(percentText, x, y);
}

void handleSettingsTouch(int tx, int ty) {
  if (btnBrgUP.isTouched(tx, ty)) {
  setAndSaveBrightness(SDSettings::brightnessLevel + 25);
  Serial.println("Brightness UP"); 
  drawBrightnessPercent(0,0,200,380);
  MotorControl::sendBeep(450, 150);
  return;
  }

  if (btnBrgDOWN.isTouched(tx, ty)) {
    setAndSaveBrightness(SDSettings::brightnessLevel - 25);
    Serial.println("Brightness DOWN");
    drawBrightnessPercent(0,0,200,380);
    MotorControl::sendBeep(450, 150);
  return;
  }

  if (btnSpeedUP.isTouched(tx, ty)) {
  setAndSaveSpeed(SDSettings::motorSpeed + 2);
  Serial.println("Speed UP"); 
  drawMotorSpeed(220,0,200,380);
  return;
  }

  if (btnSpeedDOWN.isTouched(tx, ty)) {
  setAndSaveSpeed(SDSettings::motorSpeed - 2);
  Serial.println("Speed Down"); 
  drawMotorSpeed(220,0,200,380);
  return;
  }

  if (btnMotorSTART.isTouched(tx, ty)) {
  Serial.println("Motor -> START");
  MotorControl::sendSpeed(SDSettings::motorSpeed);
  MotorControl::sendEnable(true); 
  return;
  }

  if (btnMotorSTOP.isTouched(tx, ty)) {
  Serial.println("Motor -> STOP");
  MotorControl::sendEnable(false);
  pauseTimer();
  return;
  }
}

void setAndSaveBrightness(int value) {
  SDSettings::brightnessLevel = constrain(value, 25, 250);
  lcd.setBrightness(SDSettings::brightnessLevel);
  SDSettings::saveSettings();
}

void setAndSaveSpeed(int value) {
  SDSettings::motorSpeed = constrain(value, 0, 100);
  MotorControl::sendSpeed(value);
  SDSettings::saveSettings();
}
