#define UART_RX_PIN 14

// Motor driver pins
const int EN_PIN     = 4;
const int NSLEEP_PIN = 5;
const int PHASE_PIN  = 6;
const int NFAULT_PIN = 7;

// Piezo Speaker Pin
const int PIEZO_PIN = 11;

enum MotorState {
  MOTOR_STATE_IDLE,
  MOTOR_STATE_SPEED_UP,
  MOTOR_STATE_RUNNING,
  MOTOR_STATE_SLOW_DOWN
};

MotorState currentMotorState = MOTOR_STATE_IDLE;

// Timing
unsigned long phaseStartTime = 0;
const unsigned long RUNNING_DURATION      = 4000;
const unsigned long IDLE_DURATION         = 200;
const unsigned long SPEED_CHANGE_DURATION = 1000;
const unsigned long SPEED_STEP_INTERVAL   = SPEED_CHANGE_DURATION / 10;

int motorSpeedTargetPWM = 128;
int currentSpeedPWM     = 0;
unsigned long speedStepStartTime = 0;
int speedStepCount = 0;

bool directionForward = true;

bool motorEnabled = false;
bool faultDetected = false;
bool speedChanged = false;

void startBeep(int pin, int frequency, int duration_ms) {
  tone(pin, frequency, duration_ms);
}
void stopBeep() { noTone(PIEZO_PIN); }
void beepStartup() {
  tone(PIEZO_PIN, 988, 150); delay(180); // B5
  tone(PIEZO_PIN, 1319, 150); delay(180); // E6
  tone(PIEZO_PIN, 1760, 200); delay(220); // A6
  tone(PIEZO_PIN, 988, 100); delay(120); // B5
  tone(PIEZO_PIN, 1319, 200); delay(220); // E6
}
void twoBeepsStart() { startBeep(PIEZO_PIN, 1500, 150); delay(200); startBeep(PIEZO_PIN, 1500, 150); }
void longBeepStop() { startBeep(PIEZO_PIN, 800, 500); }
void shortLowBeepSpeed() { startBeep(PIEZO_PIN, 400, 100); }
void longHighBeepFault() { startBeep(PIEZO_PIN, 2000, 1000); }

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, UART_RX_PIN, -1);
  pinMode(EN_PIN, OUTPUT);
  pinMode(NSLEEP_PIN, OUTPUT);
  pinMode(PHASE_PIN, OUTPUT);
  pinMode(NFAULT_PIN, INPUT_PULLUP);
  pinMode(PIEZO_PIN, OUTPUT);

  digitalWrite(NSLEEP_PIN, HIGH);
  digitalWrite(EN_PIN, LOW);
  digitalWrite(PHASE_PIN, LOW);
  beepStartup();
  Serial.println("[MotorESP] Ready for UART control on GPIO9");
  currentMotorState = MOTOR_STATE_IDLE;
  phaseStartTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  if (Serial1.available()) {
    String msg = Serial1.readStringUntil('\n');
    msg.trim();

    if (msg == "START") {
      if (!motorEnabled || faultDetected) {
        motorEnabled = true;
        faultDetected = false;
        currentMotorState = MOTOR_STATE_SPEED_UP;
        phaseStartTime = currentMillis;
        speedStepStartTime = currentMillis;
        speedStepCount = 0;
        currentSpeedPWM = 0;
        directionForward = true;
        digitalWrite(PHASE_PIN, LOW);
        analogWrite(EN_PIN, currentSpeedPWM);
        twoBeepsStart();
        Serial.println("[MotorESP] Motor START sequence initiated (Speed Up).");
      }
    } else if (msg == "STOP") {
      if (motorEnabled || faultDetected) {
        motorEnabled = false;
        faultDetected = false;
        currentMotorState = MOTOR_STATE_IDLE;
        analogWrite(EN_PIN, 0);
        currentSpeedPWM = 0;
        longBeepStop();
        Serial.println("[MotorESP] Motor STOP.");
      }
    } else if (msg.startsWith("SPEED:")) {
      int val = msg.substring(6).toInt();
      val = constrain(val, 0, 100);
      motorSpeedTargetPWM = map(val, 0, 100, 0, 255);
      shortLowBeepSpeed();
      speedChanged = true;
      Serial.printf("[MotorESP] Target speed set to %d%% (PWM %d)\n", val, motorSpeedTargetPWM);
    } else if (msg.startsWith("BEEP:")) {
      int colon = msg.indexOf(':', 5);
      if (colon > 5) {
        int freq = msg.substring(5, colon).toInt();
        int dur = msg.substring(colon + 1).toInt();
        tone(PIEZO_PIN, freq, dur);
        Serial.printf("[MotorESP] External BEEP: freq %d Hz, duration %d ms\n", freq, dur);
      }
    }
  }

  if (!motorEnabled || faultDetected) {
    if (currentMotorState != MOTOR_STATE_IDLE) {
      currentMotorState = MOTOR_STATE_IDLE;
      analogWrite(EN_PIN, 0);
      currentSpeedPWM = 0;
      Serial.println("[MotorESP] Transitioning to IDLE due to stop or fault.");
    }
    return;
  }

  switch (currentMotorState) {
    case MOTOR_STATE_IDLE:
      if (currentMillis - phaseStartTime >= IDLE_DURATION) {
        directionForward = !directionForward;
        digitalWrite(PHASE_PIN, directionForward ? LOW : HIGH);
        currentMotorState = MOTOR_STATE_SPEED_UP;
        phaseStartTime = currentMillis;
        speedStepStartTime = currentMillis;
        speedStepCount = 0;
        currentSpeedPWM = 0;
        analogWrite(EN_PIN, currentSpeedPWM);
        Serial.printf("[MotorESP] IDLE done. Direction: %s. Starting SPEED_UP.\n", directionForward ? "FORWARD" : "REVERSE");
      }
      break;

    case MOTOR_STATE_SPEED_UP:
      if (currentMillis - speedStepStartTime >= SPEED_STEP_INTERVAL) {
        speedStepStartTime += SPEED_STEP_INTERVAL;
        speedStepCount++;
        currentSpeedPWM = map(speedStepCount, 0, 10, 0, motorSpeedTargetPWM);
        currentSpeedPWM = constrain(currentSpeedPWM, 0, motorSpeedTargetPWM);
        analogWrite(EN_PIN, currentSpeedPWM);
        if (speedStepCount >= 10 || (currentMillis - phaseStartTime) >= SPEED_CHANGE_DURATION) {
          currentSpeedPWM = motorSpeedTargetPWM;
          analogWrite(EN_PIN, currentSpeedPWM);
          currentMotorState = MOTOR_STATE_RUNNING;
          phaseStartTime = currentMillis;
          speedStepCount = 0;
          Serial.printf("[MotorESP] SPEED_UP done. Entering RUNNING at PWM %d.\n", currentSpeedPWM);
        }
      }
      break;

    case MOTOR_STATE_RUNNING:
      if (speedChanged) {
        analogWrite(EN_PIN, motorSpeedTargetPWM);
        currentSpeedPWM = motorSpeedTargetPWM;
        speedChanged = false;
        Serial.printf("[MotorESP] SPEED changed during RUNNING to PWM %d.\n", motorSpeedTargetPWM);
      }
      if (currentMillis - phaseStartTime >= RUNNING_DURATION) {
        currentMotorState = MOTOR_STATE_SLOW_DOWN;
        phaseStartTime = currentMillis;
        speedStepStartTime = currentMillis;
        speedStepCount = 0;
        Serial.println("[MotorESP] RUNNING done. Entering SLOW_DOWN.");
      }
      break;

    case MOTOR_STATE_SLOW_DOWN:
      if (currentMillis - speedStepStartTime >= SPEED_STEP_INTERVAL) {
        speedStepStartTime += SPEED_STEP_INTERVAL;
        speedStepCount++;
        currentSpeedPWM = map(speedStepCount, 0, 10, motorSpeedTargetPWM, 0);
        currentSpeedPWM = constrain(currentSpeedPWM, 0, motorSpeedTargetPWM);
        analogWrite(EN_PIN, currentSpeedPWM);
        if (speedStepCount >= 10 || (currentMillis - phaseStartTime) >= SPEED_CHANGE_DURATION) {
          currentSpeedPWM = 0;
          analogWrite(EN_PIN, 0);
          currentMotorState = MOTOR_STATE_IDLE;
          phaseStartTime = currentMillis;
          speedStepCount = 0;
          Serial.println("[MotorESP] SLOW_DOWN done. Entering IDLE.");
        }
      }
      break;
  }
}
