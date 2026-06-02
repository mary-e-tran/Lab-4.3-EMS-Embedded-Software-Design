#include <ADXL335SelfTest.h>

const uint8_t X_PIN = A2;
const uint8_t Y_PIN = A1;
const uint8_t Z_PIN = A0;

// Change this to the Arduino pin connected to the MOSFET self-test control.
const uint8_t SELF_TEST_PIN = 4;

// The PCB MOSFET inverts the self-test signal, so the control pin is active-low.
ADXL335SelfTest adxlSelfTest(X_PIN, Y_PIN, Z_PIN, SELF_TEST_PIN, 5.0f, false);

void setup() {
  Serial.begin(9600);
  adxlSelfTest.begin();

  // Keep this conservative first, then tune from real measurements.
  adxlSelfTest.setMinimumDeltaVolts(0.10f, 0.10f, 0.10f);

  Serial.println(F("ADXL335 self-test example"));
  Serial.println(F("Keep the sensor still while the test runs."));
}

void loop() {
  ADXL335SelfTestResult result = adxlSelfTest.run(32);
  adxlSelfTest.printResult(Serial, result);
  Serial.println();

  delay(5000);
}
