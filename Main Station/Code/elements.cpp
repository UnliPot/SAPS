#include "elements.h"
#include "screen_manager.h"
#include "motor_control.h"
#include "main_screen.h"


const int screenW = 800;
const int screenH = 480;
const int headerH = 60;

float Temperature;
float TemperatureGoal = 35.5;
bool TemperatureIsConnected = false;

bool screenIsSleeping = false;
bool waitForTouchRelease = false;

void Button::draw() {
  lcd.fillRoundRect(x, y, w, h, 8, color);
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(2);
  lcd.drawString(label, x + w / 2, y + h / 2);
}

bool Button::isTouched(int tx, int ty) {
  return tx >= x && tx <= x + w && ty >= y && ty <= y + h;
}

Button btnTemperature = {640, 10, 100, 40, "----", lcd.color888(0, 0, 0)};
Button btnHome = {10, 10, 80, 40, "Home", lcd.color888(0, 180, 0)};
Button btnSleep = {750, 10, 40, 40, "C", lcd.color888(0, 0, 0)};

void changeTemperatureGoal(float value) {
  if (value < -10.0f) value = -10.0f;
  else if (value > 85.0f) value = 85.0f;
  TemperatureGoal = value;
}

void drawHeader(const char* title, bool showBack) {
  lcd.fillRect(0, 0, 800, 60, lcd.color888(80,80,80)); //x,y,w,h,color
  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE, lcd.color888(80,80,80));
  lcd.setTextSize(3);
  lcd.drawString(title, 400, 30);
  btnHome.draw();
  btnSleep.draw();

  if (TemperatureIsConnected){
    static char tempLabel[12];
    snprintf(tempLabel, sizeof(tempLabel), "%.1f C", Temperature);
    btnTemperature.label = tempLabel;

    float delta = fabs(Temperature - TemperatureGoal);

    if (delta > 5.0f) {
      btnTemperature.color = lcd.color888(180, 0, 0);
    }else if (delta > 1.0f) {
      btnTemperature.color = lcd.color888(180, 90, 0);
    } else {
      btnTemperature.color = lcd.color888(0, 180, 0);
    }

    btnTemperature.draw();
  }
}

bool handleHeaderTouch(int tx, int ty) {
  if (btnHome.isTouched(tx, ty)) {
    lastDrawnSeconds = -1;
    switchToScreen(SCREEN_MAIN); 
    Serial.println("Home Button Touched");
    MotorControl::sendBeep(450, 150);
    delay(100);
    return true;

  }else if(btnSleep.isTouched(tx, ty)){
    lcd.sleep();
    Serial.println("Sleep Button Touched");
    screenIsSleeping = true;
    lcd.setBrightness(0);
    MotorControl::sendBeep(1660, 200);
    delay(100);
    MotorControl::sendBeep(1240, 200);
    delay(100);
    MotorControl::sendBeep(830, 200);
    delay(100);
    MotorControl::sendBeep(930, 200);
    waitForTouchRelease = true;
    return true;
  }
   return false;
}


