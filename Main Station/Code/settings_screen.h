#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "display.h"
#include "elements.h"

void drawSettingsScreen();
void drawBrightnessBlock(int bx, int by, int w, int h);
void drawBrightnessPercent(int bx, int by, int w, int h);
void handleSettingsTouch(int tx, int ty);
void setAndSaveBrightness(int value);
void drawMotorSpeed(int bx, int by, int w, int h);
void drawMotorSpeedBlock(int bx, int by, int w, int h);
void setAndSaveSpeed(int value);
void drawMotorControl(int bx, int by, int w, int h);

#endif