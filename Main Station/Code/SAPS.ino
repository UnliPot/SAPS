//SAPS (Semi-Automatic Processing Station)
// Version 1.0.0
// Created by Jakub

#include "display.h"
#include "screen_manager.h"
#include "main_screen.h"
#include "settings_screen.h"
#include "elements.h"
#include "sd_settings.h"
#include "motor_control.h"
#include "profiles_screen.h"
#include "profile_data.h"
#include "edit_screen.h"
#include "profile_editor_screen.h"
#include "espnow_reciever.h"
//#include <WiFi.h> include only for macaddress snooping

static bool lastConnectionState = TemperatureIsConnected;
bool touchAlreadyHandled = false;
bool screenJustWokeUp = false;
bool wasTouchingLast = false;
bool blockTouches = false;
#define UART_TX_PIN 38

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.setRotation(2);
  lcd.setColorDepth(16);
  lcd.setSwapBytes(true);

  //WiFi.mode(WIFI_STA); //only for mac address snooping, if not to be used delete or command out

  if (!SDSettings::begin()) {
    Serial.println("SD init failed.");
  }
  if (!SDSettings::loadSettings()) {
    Serial.println("Using default settings.");
  }

  lcd.setBrightness(SDSettings::brightnessLevel);
  Serial1.begin(9600, SERIAL_8N1, -1, UART_TX_PIN);
  switchToScreen(SCREEN_MAIN);

  setupEspNow();

}

void loop() {
  int tx, ty;
  bool isTouchingNow = lcd.getTouch(&tx, &ty);

  updateTimer();
  updateTemperatureConnection();

  if(TemperatureIsConnected != lastConnectionState){
    lastConnectionState = TemperatureIsConnected;
    switchToScreen(currentScreen);
  }

  if (blockTouches) {
    if (!isTouchingNow && wasTouchingLast) {
      blockTouches = false;        // Finger was lifted — now allow touch again
      waitForTouchRelease = false; // Also clear wait-for-release just in case
      Serial.println("Touch released, input unblocked");
    }
    wasTouchingLast = isTouchingNow;
    return;  
  }

  if (isTouchingNow && screenIsSleeping) {
    lcd.wakeup();
    lcd.setBrightness(SDSettings::brightnessLevel);
    Serial.println("screen is woken up!");
    screenIsSleeping = false;
    screenJustWokeUp = true;
    blockTouches = true;
    MotorControl::sendBeep(930, 150);
    delay(100);
    MotorControl::sendBeep(1240, 150);
    wasTouchingLast = isTouchingNow;
    return;
  }

  if (isTouchingNow && !screenIsSleeping && !blockTouches) {
    if (!touchAlreadyHandled) {
      Serial.println("Touch!");

      if (handleHeaderTouch(tx, ty)) {
        touchAlreadyHandled = true;
        wasTouchingLast = isTouchingNow;
        return;
      }

      switch (currentScreen) {
        case SCREEN_MAIN: handleMainTouch(tx, ty); break;
        case SCREEN_SETTINGS: handleSettingsTouch(tx, ty); break;
        case SCREEN_PROFILES: handleProfilesTouch(tx, ty); break;
        case SCREEN_EDIT: handleEditTouch(tx, ty); break;
        case SCREEN_PROFILE_EDITOR: handleProfileEditorTouch(tx, ty); break;
      }

      touchAlreadyHandled = true;
    }
  } else {
    touchAlreadyHandled = false;
  }

  wasTouchingLast = isTouchingNow;

  if (screenJustWokeUp) {
    lastDrawnSeconds = -1;
    drawCurrentScreen();
    screenJustWokeUp = false;
  }
  //only for mac address snooping, if not in use delete or command out
  //Serial.print("ESP32 MAC Address: ");
  //Serial.println(WiFi.macAddress());

}
