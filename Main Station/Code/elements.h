#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include "display.h"

extern bool screenIsSleeping;
extern bool waitForTouchRelease;
extern bool TemperatureIsConnected;
extern float Temperature;
extern float TemperatureGoal;

struct Button {
  int x, y, w, h;
  const char* label;
  uint32_t color;

  void draw();
  bool isTouched(int tx, int ty);
};

void changeTemperatureGoal(float value);
void drawHeader(const char* title, bool showBack = false);
bool handleHeaderTouch(int tx, int ty);

#endif
