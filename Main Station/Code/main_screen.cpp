#include "main_screen.h"
#include "screen_manager.h"
#include "elements.h"
#include "motor_control.h"
#include "profile_data.h"
#include "edit_screen.h"
#include "sd_settings.h"

const int screenW = 800;
const int screenH = 480;
const int headerH = 60;

Button btnSettings = {610, 390, 160, 60, "Settings", lcd.color888(0, 0, 255)};
Button btnStart = {550, 120, 220, 80, "Start", lcd.color888(0, 155, 0)};
Button btnStop = {550, 260, 220, 80, "Pause", lcd.color888(255, 10, 0)};
Button btnProfiles = {30, 390, 220, 60, "Profiles", lcd.color888(0, 0, 255)};
Button btnEdit = {310, 390, 220, 60, "Edit", lcd.color888(0, 0, 255)};
Button btnNextstep = {460, 290, 60, 60, ">", lcd.color888(255, 0, 0)};
Button btnPrevstep = {40, 290, 60, 60, "<", lcd.color888(255, 0, 0)};

unsigned long startTimeMs = 0;
int totalDuration = 0;
int lastDrawnSeconds = -1;
bool isPaused = false;
bool timerStarted = false;
bool stepCompleted = false;
unsigned long pausedAtMs = 0;
unsigned long completedAtMs = 0;

void startTimer(int durationSec) {
  startTimeMs = millis();
  totalDuration = durationSec;
  isPaused = false;
  lastDrawnSeconds = -1;
  timerStarted = true;
  stepCompleted = false;
}

void pauseTimer() {
  if (!isPaused) {
    isPaused = true;
    pausedAtMs = millis();
  }
}

void resumeTimer() {
  if (isPaused) {
    unsigned long pausedDuration = millis() - pausedAtMs;
    startTimeMs += pausedDuration;
    isPaused = false;
  }
}

int getRemainingSeconds() {
  if (!timerStarted) return totalDuration;
  long remaining;
  if (isPaused) {
    remaining = totalDuration - ((pausedAtMs - startTimeMs) / 1000);
  } else {
    remaining = totalDuration - ((millis() - startTimeMs) / 1000);
  }
  return (remaining < 0) ? 0 : remaining;
}

void setCurrentStepDuration(int durationSec) {
  totalDuration = durationSec;
  lastDrawnSeconds = -1;
  timerStarted = false;
}

void resetTimerState(int durationSec) {
  totalDuration = durationSec;
  startTimeMs = 0;
  pausedAtMs = 0;
  isPaused = false;
  timerStarted = false;
  lastDrawnSeconds = -1;
  stepCompleted = false;
}

void updateTimer() {
  if (!timerStarted || isPaused) return;
  int currentRemaining = getRemainingSeconds();
  if (currentRemaining != lastDrawnSeconds) {
    lastDrawnSeconds = currentRemaining;
    if (currentRemaining > 0) {
      if (currentScreen == SCREEN_MAIN && !screenIsSleeping) {
        drawTimerCountdown(currentRemaining);
      }
    } else {
      if (!stepCompleted) {
        MotorControl::sendEnable(false);
        if (currentScreen == SCREEN_MAIN && !screenIsSleeping) {
          lcd.fillRoundRect(40, 110, 480, 160, 8, lcd.color888(0, 0, 0));
          lcd.setTextDatum(middle_center);
          lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
          lcd.setTextSize(5);
          lcd.drawString("Step Done", 270, 180);
        }
        stepCompleted = true;
        completedAtMs = millis();
        timerStarted = false;
      } else if (millis() - completedAtMs > 3000) {
        if (ProfileData::nextStep()) {
          Step* step = ProfileData::getMutableCurrentStep();
          if (step) {
            totalDuration = getEffectiveDuration(*step);
            lastDrawnSeconds = -1;
            timerStarted = false;
            if (currentScreen == SCREEN_MAIN && !screenIsSleeping) {
              drawMainScreen();
            }
          }
        } else {
          timerStarted = false;
        }
      }
    }
  }
}

void drawMainScreen() {
  lcd.fillScreen(TFT_BLACK);
  drawHeader(ProfileData::getProfileName(), true);
  lcd.fillRoundRect(10, 70, 780, 400, 8, lcd.color888(80, 80, 80));
  lcd.fillRoundRect(30, 90, 500, 280, 8, lcd.color888(0, 0, 0));
  lcd.fillRoundRect(40, 290, 460, 60, 8, lcd.color888(80, 80, 80));

  btnSettings.draw();
  btnStart.draw();
  btnStop.draw();
  btnProfiles.draw();

  if (strlen(ProfileData::getProfileName()) > 0) {
    btnEdit.draw();
  }

  btnNextstep.draw();
  btnPrevstep.draw();

  const Step* step = ProfileData::getCurrentStep();
  if (step) {
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(TFT_WHITE, lcd.color888(80, 80, 80));
    lcd.setTextSize(3);
    lcd.drawString(step->name.c_str(), 270, 320);
  }
  lastDrawnSeconds = getRemainingSeconds() + 1;
  drawTimerCountdown(getRemainingSeconds());
}

void drawTimerCountdown(int secondsRemaining) {
  int minutes = secondsRemaining / 60;
  int seconds = secondsRemaining % 60;
  lcd.fillRoundRect(40, 110, 480, 160, 8, lcd.color888(0, 0, 0));
  lcd.setCursor(62.5, 180);
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(15);
  lcd.printf("%02d:%02d", minutes, seconds);
}

void handleMainTouch(int tx, int ty) {
  if (btnSettings.isTouched(tx, ty)) {
    switchToScreen(SCREEN_SETTINGS);
    MotorControl::sendBeep(840, 150);
    Serial.println("Settings Pressed");
  }

  if (btnStart.isTouched(tx, ty)) {
    Serial.println("Start Pressed");
    MotorControl::sendSpeed(SDSettings::motorSpeed);
    const Step* step = ProfileData::getCurrentStep();
    if (!step) return;

    if (isPaused) {
      resumeTimer();
      MotorControl::sendEnable(true);
    } else {
      startTimer(getEffectiveDuration(*step));
      MotorControl::sendEnable(true);
    }
  }

  if (btnStop.isTouched(tx, ty)) {
    Serial.println("Stop Pressed");
    pauseTimer();
    stepCompleted = false;
    MotorControl::sendEnable(false);
  }

  if (btnProfiles.isTouched(tx, ty) && !(timerStarted && !isPaused)) {
    Serial.println("Profiles touched");
    switchToScreen(SCREEN_PROFILES);
    MotorControl::sendBeep(450, 150);
  }

  if (btnEdit.isTouched(tx, ty) && strlen(ProfileData::getProfileName()) > 0) {
    Serial.println("Edit Profile touched");
    switchToScreen(SCREEN_EDIT);
    MotorControl::sendBeep(450, 150);
  }

  if (btnNextstep.isTouched(tx, ty) && !(timerStarted && !isPaused)) {
    if (ProfileData::nextStep()) {
      Step* step = ProfileData::getMutableCurrentStep();
      if (step) {
        step->modifier = 0;
        totalDuration = getEffectiveDuration(*step);
        lastDrawnSeconds = -1;
        isPaused = false;
        timerStarted = false;
        stepCompleted = false;
      }
      drawMainScreen();
    }
  }

  if (btnPrevstep.isTouched(tx, ty) && !(timerStarted && !isPaused)) {
    if (ProfileData::prevStep()) {
      Step* step = ProfileData::getMutableCurrentStep();
      if (step) {
        step->modifier = 0;
        totalDuration = getEffectiveDuration(*step);
        lastDrawnSeconds = -1;
        isPaused = false;
        timerStarted = false;
        stepCompleted = false;
      }
      drawMainScreen();
    }
  }
}
