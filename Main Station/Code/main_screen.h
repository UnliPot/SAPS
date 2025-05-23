#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include "display.h"
#include "elements.h"

extern int lastDrawnSeconds;
extern bool timerStarted;
extern bool isPaused;

void drawMainScreen();
void handleMainTouch(int tx, int ty);
void drawTimerCountdown(int secondsRemaining);
void updateTimer();
void startTimer(int durationSec);
void setCurrentStepDuration(int durationSec);
void pauseTimer();
void resetTimerState(int durationSec);

#endif
