// =====================================================================
// calibration.h - Step Tracker startup calibration
// ---------------------------------------------------------------------
// Drop this pair (calibration.h + calibration.ino) into any sketch.
// In your main setup(): call runCalibration() once after Serial.begin
// and analogReference(EXTERNAL). The result lives in the globals below.
// =====================================================================
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>   // for analogRead, millis, F(), pinMode, etc.

// ---------- Pin allocation (matches BasicCode.ino / Accel.ino) -------
const int xOut = A2;   // ADXL335 Xout
const int yOut = A1;   // ADXL335 Yout
const int zOut = A0;   // ADXL335 Zout

// Hold this button during the 2 s lead-in to skip calibration and
// force the lab-default constants. Matches user-control_v1 pin.
const int skipBtnPin = 2;

// ---------- Lab-measured constants at VS = AREF = 3.3V ---------------
// Source: device measurements (1g vs 0g per axis, VS = 3.3V)
// LSB = 3.3V / 1023 = 3.223 mV/count
//
//   Axis  Vat1g     Vat0g     Sens (mV/g)   0g count   counts/g
//     X   1.930 V   1.604 V       326          497       101.2
//     Y   1.895 V   1.556 V       339          483       105.2
//     Z   1.928 V   1.584 V       344          491       106.7

const int ZERO_G_X_DEFAULT = 497;
const int ZERO_G_Y_DEFAULT = 483;
const int ZERO_G_Z_DEFAULT = 491;

const float SENSITIVITY_X = 101.2f;
const float SENSITIVITY_Y = 105.2f;
const float SENSITIVITY_Z = 106.7f;

// Validity window: +/-20% of each default zero-g count
const int VALID_LOW_X  = 398;
const int VALID_HIGH_X = 597;
const int VALID_LOW_Y  = 386;
const int VALID_HIGH_Y = 580;
const int VALID_LOW_Z  = 393;
const int VALID_HIGH_Z = 589;

// ---------- Calibration outputs (populated by runCalibration) --------
extern int  zeroGX;
extern int  zeroGY;
extern int  zeroGZ;
extern bool calibrationOk;     // true = measured, false = defaults used
extern bool calibrationSkipped;// true = user held button to skip

// ---------- Public API -----------------------------------------------
// Call once from setup() after Serial.begin and analogReference(EXTERNAL).
void runCalibration();

// Convert raw ADC reading to acceleration in g using calibrated zero-g.
// Inline so it can sit in a tight inner loop without function-call cost.
inline float toG(int raw, int zeroG, float sens) {
  return (raw - zeroG) / sens;
}

#endif // CALIBRATION_H
