#include "edit_screen.h"
#include "screen_manager.h"
#include "profile_data.h"
#include "elements.h"
#include "motor_control.h"
#include "profile_editor_screen.h"
#include "main_screen.h"

Button btnPush = {45, 100, 200, 60, "Push +", lcd.color888(0, 0, 255)};
Button btnPull = {45, 380, 200, 60, "Pull -", lcd.color888(0, 0, 255)};
Button btnBack = {560, 380, 200, 60, "Back", lcd.color888(0, 0, 255)};
Button btnEditProfile = {560, 100, 200, 200, "Edit Profile", lcd.color888(0, 0, 255)};
Button btnTemperatureControlUP = {325, 170, 160, 60, "+", lcd.color888(0, 0, 255)};
Button btnTemperatureControlDOWN = {325, 330, 160, 60, "-", lcd.color888(0, 0, 255)};
Button btnTemperatureControlSTATUS = {290, 410, 230, 40, "STATUS", lcd.color888(0, 0, 255)};

extern void setCurrentStepDuration(int durationSec);

void drawEditScreen() {
  lcd.fillScreen(TFT_BLACK);
  drawHeader("Edit Step", true);
  lcd.fillRoundRect(10, 70, 780, 400, 8, lcd.color888(80, 80, 80));
  lcd.fillRoundRect(280, 80, 250, 60, 8, lcd.color888(0,0,0));
  lcd.fillRoundRect(540, 80, 240, 240, 8, lcd.color888(0,0,0));
  lcd.drawString("Temperature", 410, 110);
  drawTemperatureControl();
  drawPushandPull();
  btnBack.draw();
  btnEditProfile.draw();
}

void drawPushandPull(){
  lcd.fillRoundRect(20, 80, 250, 380, 8, lcd.color888(0,0,0));
  btnPush.draw();
  btnPull.draw();

  const Step* step = ProfileData::getCurrentStep();
  if (step) {
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(TFT_WHITE);
    lcd.setTextSize(15);
    lcd.drawString((String(step->modifier)).c_str(), 150, 280);
  }
}

void drawTemperatureControl() {
  lcd.fillRoundRect(280, 150, 250, 310, 8, lcd.color888(0,0,0));

  static char tempGoal[12];
  snprintf(tempGoal, sizeof(tempGoal), "%.1f C", TemperatureGoal);
  
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(4);
  lcd.drawString(tempGoal, 410, 280);

  if (TemperatureIsConnected) {
    btnTemperatureControlSTATUS.label = "Connected";
    btnTemperatureControlSTATUS.color = lcd.color888(0,180,0);
    btnTemperatureControlSTATUS.draw();
  } else {
    btnTemperatureControlSTATUS.label = "Disconnected";
    btnTemperatureControlSTATUS.color = lcd.color888(180,0,0);
    btnTemperatureControlSTATUS.draw();
  }

  btnTemperatureControlUP.draw();
  btnTemperatureControlDOWN.draw();  

}

void handleEditTouch(int tx, int ty) {
  float value;
  Step* step = ProfileData::getMutableCurrentStep();
  if (!step) return;

  if (btnPush.isTouched(tx, ty)&& !(timerStarted && !isPaused)) {
    step->modifier = std::min(step->modifier + 1, 3);
    setCurrentStepDuration(getEffectiveDuration(*step));
    MotorControl::sendBeep(650, 200);
    drawPushandPull();
  }

  if (btnPull.isTouched(tx, ty)&& !(timerStarted && !isPaused)) {
    step->modifier = std::max(step->modifier - 1, -3);
    setCurrentStepDuration(getEffectiveDuration(*step));
    MotorControl::sendBeep(350, 200);
    drawPushandPull();
  }

  if (btnBack.isTouched(tx, ty)) {
    MotorControl::sendBeep(400, 200);
    switchToScreen(SCREEN_MAIN);
  }

  if (btnTemperatureControlUP.isTouched(tx, ty)) {
    value = TemperatureGoal;
    value += 0.5;
    changeTemperatureGoal(value);
    drawTemperatureControl();
  }

  if (btnTemperatureControlDOWN.isTouched(tx, ty)) {
    value = TemperatureGoal;
    value -= 0.5;
    changeTemperatureGoal(value);
    drawTemperatureControl();
  }

  if (btnEditProfile.isTouched(tx, ty)&& !(timerStarted && !isPaused)) {
    startProfileEditor(true, ProfileData::currentProfile.filename.c_str());
  }
}
