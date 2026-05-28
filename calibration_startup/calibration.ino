// =====================================================================
// calibration.ino - implementation of runCalibration()
// See calibration.h for the public API and constants.
// =====================================================================
#include "calibration.h"

// ---------- Definitions of the public globals ------------------------
int  zeroGX = ZERO_G_X_DEFAULT;
int  zeroGY = ZERO_G_Y_DEFAULT;
int  zeroGZ = ZERO_G_Z_DEFAULT;
bool calibrationOk      = false;
bool calibrationSkipped = false;

// ---------- Internal sampling parameters -----------------------------
static const int          N_SAMPLES          = 50;     // 0.5 s of data
static const int          SAMPLE_INTERVAL_MS = 10;     // 100 Hz
static const unsigned long SETTLE_MS         = 2000;   // hold-still lead-in
static const unsigned long SKIP_HOLD_MS      = 800;    // continuous hold needed to skip

// Poll the skip button during the settle wait. Returns true if the
// button stays pressed for at least SKIP_HOLD_MS continuously.
// Non-blocking: returns as soon as the full SETTLE_MS has elapsed.
static bool waitWithSkipCheck() {
  unsigned long start    = millis();
  unsigned long heldFrom = 0;       // 0 = not currently held
  while (millis() - start < SETTLE_MS) {
    // Active-LOW with INPUT_PULLUP -> pressed reads LOW.
    bool pressed = (digitalRead(skipBtnPin) == LOW);
    if (pressed) {
      if (heldFrom == 0) heldFrom = millis();
      if (millis() - heldFrom >= SKIP_HOLD_MS) return true;
    } else {
      heldFrom = 0;
    }
    delay(10);
  }
  return false;
}

// Load lab defaults into the live globals.
static void loadDefaults() {
  zeroGX = ZERO_G_X_DEFAULT;
  zeroGY = ZERO_G_Y_DEFAULT;
  zeroGZ = ZERO_G_Z_DEFAULT;
  calibrationOk = false;
}

void runCalibration() {

  // Skip button uses internal pull-up: button wires Nano D2 -> GND.
  pinMode(skipBtnPin, INPUT_PULLUP);

  // --- Stage 1: announce start --------------------------------------
  // [DISPLAY] show "Calibrating..." screen, hint "hold BTN to skip"
  Serial.println(F("== CALIBRATION START =="));
  Serial.println(F("Hold BTN (D2) for ~1s to skip and use defaults."));

  // --- Stage 2: settle period (with skip check) ---------------------
  // [DISPLAY] show "Hold device flat and still" + 2 s countdown
  Serial.println(F("Hold device flat and still..."));
  if (waitWithSkipCheck()) {
    // [DISPLAY] show "Skipped - defaults loaded"
    Serial.println(F("Skip held -> using lab defaults, no sampling."));
    loadDefaults();
    calibrationSkipped = true;
    Serial.print(F("zeroG X:")); Serial.print(zeroGX);
    Serial.print(F(" Y:"));      Serial.print(zeroGY);
    Serial.print(F(" Z:"));      Serial.println(zeroGZ);
    return;
  }
  calibrationSkipped = false;

  // --- Stage 3: collect samples -------------------------------------
  // [DISPLAY] show progress bar while sampling
  Serial.print(F("Sampling"));
  long sumX = 0, sumY = 0, sumZ = 0;
  for (int i = 0; i < N_SAMPLES; i++) {
    sumX += analogRead(xOut);
    sumY += analogRead(yOut);
    sumZ += analogRead(zOut);
    if ((i % 5) == 0) Serial.print('.');
    delay(SAMPLE_INTERVAL_MS);
  }
  Serial.println();

  // --- Stage 4: average ---------------------------------------------
  int avgX = sumX / N_SAMPLES;
  int avgY = sumY / N_SAMPLES;
  int avgZ = sumZ / N_SAMPLES;

  Serial.print(F("Averages -> X:")); Serial.print(avgX);
  Serial.print(F("  Y:"));           Serial.print(avgY);
  Serial.print(F("  Z:"));           Serial.println(avgZ);

  // --- Stage 5: validate each axis ----------------------------------
  bool xOk = (avgX >= VALID_LOW_X) && (avgX <= VALID_HIGH_X);
  bool yOk = (avgY >= VALID_LOW_Y) && (avgY <= VALID_HIGH_Y);
  bool zOk = (avgZ >= VALID_LOW_Z) && (avgZ <= VALID_HIGH_Z);

  Serial.print(F("X: ")); Serial.println(xOk ? F("PASS") : F("FAIL"));
  Serial.print(F("Y: ")); Serial.println(yOk ? F("PASS") : F("FAIL"));
  Serial.print(F("Z: ")); Serial.println(zOk ? F("PASS") : F("FAIL"));

  // --- Stage 6: accept or fall back ---------------------------------
  // [DISPLAY] show OK / FAIL banner
  if (xOk && yOk && zOk) {
    zeroGX = avgX;
    zeroGY = avgY;
    zeroGZ = avgZ;
    calibrationOk = true;
    Serial.println(F("Calibration OK - using measured values"));
  } else {
    loadDefaults();
    Serial.println(F("Calibration FAILED - using lab defaults"));
    Serial.println(F("(check device was stationary and flat)"));
  }

  // --- Stage 7: report final constants in use -----------------------
  Serial.print(F("zeroG X:")); Serial.print(zeroGX);
  Serial.print(F(" Y:"));      Serial.print(zeroGY);
  Serial.print(F(" Z:"));      Serial.println(zeroGZ);
  Serial.print(F("sens  X:")); Serial.print(SENSITIVITY_X);
  Serial.print(F(" Y:"));      Serial.print(SENSITIVITY_Y);
  Serial.print(F(" Z:"));      Serial.println(SENSITIVITY_Z);
}
