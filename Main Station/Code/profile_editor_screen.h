#ifndef PROFILE_EDITOR_SCREEN_H
#define PROFILE_EDITOR_SCREEN_H

#include <Arduino.h>

// If `editMode` is true, loads from filename, else creates a new empty profile
void startProfileEditor(bool editMode, const String& filename);

void handleProfileEditorTouch(int tx, int ty);

void drawProfileEditorScreen();
void drawStepTimer();

#endif
