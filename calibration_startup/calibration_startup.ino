// =====================================================================
// calibration_startup.ino - integration demo for the calibration module
// ---------------------------------------------------------------------
// This sketch is the minimal harness for testing runCalibration().
// To integrate into the final firmware:
//   1. Copy calibration.h + calibration.ino into the main sketch folder
//   2. Add `#include "calibration.h"` at the top
//   3. In setup(): analogReference(EXTERNAL); Serial.begin(9600);
//                  runCalibration();
//   4. Use the globals zeroGX / zeroGY / zeroGZ and the inline toG()
//      helper anywhere acceleration is needed.
// =====================================================================
#include "calibration.h"

unsigned long lastPrint = 0;
const unsigned long PRINT_INTERVAL_MS = 500;

void setup() {
  // AREF is hard-wired to the 3.3V rail on the PCB.
  // analogReference(EXTERNAL) MUST be called before the first analogRead.
  analogReference(EXTERNAL);

  Serial.begin(9600);
  delay(500);   // let the Serial monitor attach

  runCalibration();

  // [DISPLAY] switch to main step-counting screen
  Serial.println(F("== SETUP COMPLETE =="));
  lastPrint = millis();
}

void loop() {
  if (millis() - lastPrint < PRINT_INTERVAL_MS) return;
  lastPrint = millis();

  int rawX = analogRead(xOut);
  int rawY = analogRead(yOut);
  int rawZ = analogRead(zOut);

  float gX = toG(rawX, zeroGX, SENSITIVITY_X);
  float gY = toG(rawY, zeroGY, SENSITIVITY_Y);
  float gZ = toG(rawZ, zeroGZ, SENSITIVITY_Z);

  Serial.print(F("raw X:")); Serial.print(rawX);
  Serial.print(F(" Y:"));    Serial.print(rawY);
  Serial.print(F(" Z:"));    Serial.print(rawZ);
  Serial.print(F("  | g X:")); Serial.print(gX, 2);
  Serial.print(F(" Y:"));      Serial.print(gY, 2);
  Serial.print(F(" Z:"));      Serial.print(gZ, 2);
  Serial.print(F("  cal:"));
  if (calibrationSkipped) Serial.println(F("SKIP"));
  else                    Serial.println(calibrationOk ? F("OK") : F("FAIL-DEFAULT"));
}
