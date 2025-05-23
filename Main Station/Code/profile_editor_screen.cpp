#include "profile_editor_screen.h"
#include "screen_manager.h"
#include "profile_data.h"
#include "elements.h"
#include "sd_settings.h"
#include "motor_control.h"
#include "profiles_screen.h"
#include <FS.h>

const int screenW = 800;
const int screenH = 480;
const int headerH = 60;

std::vector<Step> editSteps;
bool editingExisting = false;
String loadedFilename = "";
int selectedEditStep = 0;
bool awaitingDeleteConfirm = false;

Button btnSave = {40, 390, 150, 70, "Save", lcd.color888(0, 180, 0)};
Button btnBackEditor = {610, 390, 150, 70, "Back", lcd.color888(0, 0, 255)};
Button btnNext = {660, 80, 100, 60, "Next", lcd.color888(0, 0, 255)};
Button btnPrev = {40, 80, 100, 60, "Prev", lcd.color888(0, 0, 255)};
Button btnDelete = {540, 170, 200, 80, "Delete Step", lcd.color888(200, 0, 0)};
Button btnRemoveProfile = {540, 280, 200, 80, "Remove Profile", lcd.color888(200, 0, 100)};

Button btnMinUp = {60, 160, 200, 40, "^", lcd.color888(0, 0, 255)};
Button btnMinDown = {60, 330, 200, 40, "v", lcd.color888(0, 0, 255)};
Button btnSecUp = {290, 160, 200, 40, "^", lcd.color888(0, 0, 255)};
Button btnSecDown = {290, 330, 200, 40, "v", lcd.color888(0, 0, 255)};

void drawProfileEditorScreen() {
  lcd.fillScreen(TFT_BLACK);
  drawHeader(editingExisting ? loadedFilename.c_str() : "Create Profile", true);

  lcd.fillRoundRect(10, 70, 780, 400, 8, lcd.color888(80, 80, 80));
  lcd.fillRoundRect(40, 80, 720, 60, 8, lcd.color888(0, 0, 0));
  lcd.fillRoundRect(520, 150, 240, 230, 8, lcd.color888(0, 0, 0));

  btnBackEditor.draw();
  btnSave.draw();
  btnNext.draw();
  btnPrev.draw();
  btnDelete.draw();
  btnRemoveProfile.draw();

  lcd.setTextDatum(middle_center);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  lcd.drawString("Step " + String(selectedEditStep + 1), screenW / 2, 110);

  btnNext.label = (selectedEditStep == editSteps.size() - 1) ? "+ Step" : "Next";
  btnNext.draw();

  drawStepTimer();

  if (awaitingDeleteConfirm) {
    lcd.setTextDatum(middle_center);
    lcd.setTextColor(TFT_YELLOW);
    lcd.setTextSize(3);
    lcd.fillRoundRect(40, 150, 470, 230, 8, lcd.color888(0, 0, 0));
    lcd.drawString("Confirm delete profile?", 280, 235);
    lcd.drawString("Tap Remove again", 280, 295);
  }
}

void drawStepTimer() {
  lcd.fillRoundRect(40, 150, 470, 230, 8, lcd.color888(0, 0, 0));
  int totalSec = editSteps[selectedEditStep].duration;
  int min = totalSec / 60;
  int sec = totalSec % 60;

  char minStr[4];
  char secStr[3];
  sprintf(minStr, "%02d", min);
  sprintf(secStr, "%02d", sec);

  lcd.setTextSize(10);
  lcd.setTextColor(totalSec == 0 ? TFT_RED : TFT_WHITE);
  lcd.drawString(minStr, 160, 265);
  lcd.drawString(":", 280, 265);
  lcd.drawString(secStr, 390, 265);

  btnMinUp.draw();
  btnMinDown.draw();
  btnSecUp.draw();
  btnSecDown.draw();
}

void handleProfileEditorTouch(int tx, int ty) {
  if (btnBackEditor.isTouched(tx, ty)) {
    switchToScreen(SCREEN_PROFILES);
    return;
  }

  if (btnSave.isTouched(tx, ty)) {
    String filename;
    if (editingExisting && loadedFilename.length() > 0) {
      filename = loadedFilename;
      Serial.printf("[Save] Editing existing. loadedFilename = '%s'\n", filename.c_str());
      if (SD.exists(filename)) {
        Serial.printf("[Save] File exists. Deleting '%s'\n", filename.c_str());
        if (!SD.remove(filename)) {
          Serial.printf("[Save] Failed to delete '%s'\n", filename.c_str());
        }
      } else {
        Serial.printf("[Save] File does not exist: '%s'\n", filename.c_str());
      }
    } else {
      int nextIndex = SDSettings::nextCustomProfileIndex();
      filename = "/custom_profile_" + String(nextIndex) + ".json";
      Serial.printf("[Save] Creating new file: '%s'\n", filename.c_str());
    }

    DynamicJsonDocument doc(4096);
    doc["name"] = "Profile";
    doc["filename"] = filename;
    JsonArray steps = doc.createNestedArray("steps");
    for (int i = 0; i < editSteps.size(); ++i) {
      JsonObject step = steps.createNestedObject();
      step["step"] = i + 1;
      step["nameofstep"] = "Step " + String(i + 1);
      step["duration"] = editSteps[i].duration;
    }
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
      serializeJson(doc, file);
      file.close();
      Serial.printf("[Save] Successfully wrote to '%s'\n", filename.c_str());
    } else {
      Serial.printf("[Save] Failed to open file '%s' for writing\n", filename.c_str());
    }

    loadProfiles();
    drawProfilesScreen();
    delay(100);
    switchToScreen(SCREEN_PROFILES);
    return;
  }

  if (btnRemoveProfile.isTouched(tx, ty) && editingExisting) {
    if (!awaitingDeleteConfirm) {
      awaitingDeleteConfirm = true;
      drawProfileEditorScreen();
      return;
    } else {
      awaitingDeleteConfirm = false;
      Serial.printf("[Delete] Attempting to remove: '%s'\n", loadedFilename.c_str());
      if (loadedFilename.length() > 0 && SD.exists(loadedFilename)) {
        if (SD.remove(loadedFilename)) {
          Serial.printf("[Delete] Successfully deleted profile: '%s'\n", loadedFilename.c_str());
        } else {
          Serial.printf("[Delete] Failed to delete profile: '%s'\n", loadedFilename.c_str());
        }
      } else {
        Serial.printf("[Delete] File does not exist: '%s'\n", loadedFilename.c_str());
      }
      loadedFilename = "";
      editingExisting = false;
      loadProfiles();
      drawProfilesScreen();
      delay(100);
      switchToScreen(SCREEN_PROFILES);
      return;
    }
  } else {
    awaitingDeleteConfirm = false;
  }

  if (btnNext.isTouched(tx, ty)) {
    if (selectedEditStep == editSteps.size() - 1) {
      Step s = {static_cast<int>(editSteps.size() + 1), "", 60, 0};
      editSteps.push_back(s);
      MotorControl::sendBeep(700, 100);
    } else {
      MotorControl::sendBeep(500, 100);
    }
    selectedEditStep = min(static_cast<int>(editSteps.size() - 1), selectedEditStep + 1);
    drawProfileEditorScreen();
    return;
  }
  if (btnPrev.isTouched(tx, ty)) {
    if (selectedEditStep > 0) {
      selectedEditStep--;
      MotorControl::sendBeep(400, 100);
      drawProfileEditorScreen();
    }
    return;
  }
  if (btnDelete.isTouched(tx, ty)) {
    if (!editSteps.empty()) {
      MotorControl::sendBeep(300, 200);
      editSteps.erase(editSteps.begin() + selectedEditStep);
      if (selectedEditStep >= editSteps.size()) selectedEditStep = editSteps.size() - 1;
      if (editSteps.empty()) {
        Step s = {1, "", 60, 0};
        editSteps.push_back(s);
        selectedEditStep = 0;
      }
      drawProfileEditorScreen();
    }
    return;
  }

  Step& s = editSteps[selectedEditStep];
  if (btnMinUp.isTouched(tx, ty)) { s.duration = min(s.duration + 60, 5999); MotorControl::sendBeep(600, 50); }
  if (btnMinDown.isTouched(tx, ty)) { s.duration = max(s.duration - 60, 0); MotorControl::sendBeep(600, 50); }
  if (btnSecUp.isTouched(tx, ty)) { s.duration = min(s.duration + 1, 5999); MotorControl::sendBeep(600, 50); }
  if (btnSecDown.isTouched(tx, ty)) { s.duration = max(s.duration - 1, 0); MotorControl::sendBeep(600, 50); }

  drawStepTimer();
}

void startProfileEditor(bool editMode, const String& filename) {
  editingExisting = editMode;
  loadedFilename = filename.startsWith("/") ? filename : ("/" + filename);
  Serial.printf("[StartEditor] loadedFilename set to: '%s'\n", loadedFilename.c_str());
  selectedEditStep = 0;
  editSteps.clear();
  awaitingDeleteConfirm = false;

  if (editMode) {
    const auto& steps = ProfileData::currentProfile.steps;
    for (const Step& s : steps) {
      editSteps.push_back(s);
    }
  } else {
    Step s = {1, "", 60, 0};
    editSteps.push_back(s);
  }

  switchToScreen(SCREEN_PROFILE_EDITOR);
}
