#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

enum ScreenID {
  SCREEN_MAIN,
  SCREEN_SETTINGS,
  SCREEN_PROFILES,
  SCREEN_EDIT,
  SCREEN_PROFILE_EDITOR
};

extern ScreenID currentScreen;
void switchToScreen(ScreenID screen);
void drawCurrentScreen();

#endif