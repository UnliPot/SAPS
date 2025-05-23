#include "screen_manager.h"
#include "main_screen.h"
#include "settings_screen.h"
#include "profiles_screen.h"
#include "edit_screen.h"
#include "profile_editor_screen.h"

ScreenID currentScreen = SCREEN_MAIN;

void switchToScreen(ScreenID screen) {
  currentScreen = screen;
  drawCurrentScreen();
}

void drawCurrentScreen() {
  switch (currentScreen) {
    case SCREEN_MAIN:
      drawMainScreen();
      break;
    case SCREEN_SETTINGS:
      drawSettingsScreen();
      break;
    case SCREEN_PROFILES:
      drawProfilesScreen();
      break;
    case SCREEN_EDIT:
      drawEditScreen();
      break;
    case SCREEN_PROFILE_EDITOR:
      drawProfileEditorScreen();
      break;
  }
}
