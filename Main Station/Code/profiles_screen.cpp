#include "profiles_screen.h"
#include "display.h"
#include "screen_manager.h"
#include "elements.h"
#include "profile_data.h"
#include "main_screen.h"
#include "motor_control.h"
#include "profile_editor_screen.h"

const int screenW = 800;
const int screenH = 480;
const int headerH = 60;

struct ProfileEntry {
  String filename;
  String label;
};

std::vector<ProfileEntry> profiles;
int currentPage = 0;
const int PROFILES_PER_PAGE = 4;
std::vector<Button> profileButtons;
Button nextPageButton;
Button prevPageButton;
Button createProfileButton;

void loadProfiles() {
  profiles.clear();

  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("[Profiles] Failed to open root directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = file.name();
    if (!file.isDirectory() && name.endsWith(".json") && !name.endsWith("/settings.json") && !name.equals("settings.json")) {
      ProfileEntry entry;
      entry.filename = name;
      entry.label = name.substring(name.lastIndexOf('/') + 1);
      profiles.push_back(entry);
    }
    file.close();
    file = root.openNextFile();
  }

  root.close();
  Serial.printf("[Profiles] %d profile(s) loaded.\n", profiles.size());
}

void drawProfilesScreen() {
  lcd.clear(TFT_BLACK);
  drawHeader("Select Profile", true);

  loadProfiles();
  profileButtons.clear();

  int startIdx = currentPage * PROFILES_PER_PAGE;
  int y = headerH + 20;

  for (int i = 0; i < PROFILES_PER_PAGE; ++i) {
    int idx = startIdx + i;
    if (idx >= profiles.size()) break;

    Button btn = {
      .x = 100,
      .y = y,
      .w = 600,
      .h = 60,
      .label = profiles[idx].label.c_str(),
      .color = lcd.color888(50, 50, 50)
    };
    btn.draw();
    profileButtons.push_back(btn);
    y += 70;
  }

  bool hasPrev = currentPage > 0;
  bool hasNext = (currentPage + 1) * PROFILES_PER_PAGE < profiles.size();

  if (hasPrev) {
    prevPageButton = { 100, 400, 290, 50, "Previous Page", lcd.color888(0, 128, 255) };
  } else {
    prevPageButton = { 100, 400, 290, 50, "Create Profile", lcd.color888(0, 200, 0) };
  }
  prevPageButton.draw();

  if (hasNext) {
    nextPageButton = { 410, 400, 290, 50, "Next Page", lcd.color888(0, 0, 255) };
  } else {
    nextPageButton = { 410, 400, 290, 50, "Create Profile", lcd.color888(0, 200, 0) };
  }
  nextPageButton.draw();
}

void handleProfilesTouch(int tx, int ty) {
  for (int i = 0; i < profileButtons.size(); ++i) {
    if (profileButtons[i].isTouched(tx, ty)) {
      int profileIndex = currentPage * PROFILES_PER_PAGE + i;
      Serial.printf("[Profile] Selected: %s\n", profiles[profileIndex].filename.c_str());
      MotorControl::sendBeep(600, 150);
      delay(100);
      MotorControl::sendBeep(600, 150);

      if (ProfileData::loadProfileFromFile(profiles[profileIndex].filename)) {
        ProfileData::resetStepIndex();
        const Step* step = ProfileData::getCurrentStep();
        if (step){
          setCurrentStepDuration(step->duration);
          lastDrawnSeconds = -1;
        }
        switchToScreen(SCREEN_MAIN);
      }
      return;
    }
  }

  if (String(nextPageButton.label) == "Next Page" && nextPageButton.isTouched(tx, ty)) {
    currentPage++;
    MotorControl::sendBeep(400, 150);
    drawProfilesScreen();
    return;
  }

  if (String(prevPageButton.label) == "Previous Page" && prevPageButton.isTouched(tx, ty)) {
    currentPage--;
    MotorControl::sendBeep(400, 150);
    drawProfilesScreen();
    return;
  }

  if (String(nextPageButton.label) == "Create Profile" && nextPageButton.isTouched(tx, ty)) {
    Serial.println("[Profile] Create Profile triggered from next button space");
    startProfileEditor(false, "");
    return;
  }

  if (String(prevPageButton.label) == "Create Profile" && prevPageButton.isTouched(tx, ty)) {
    Serial.println("[Profile] Create Profile triggered from prev button space");
    startProfileEditor(false, "");
    return;
  }
}
