#include "ADXL335SelfTest.h"

#include <math.h>

namespace {
const char *axisName(uint8_t axis) {
  switch (axis) {
    case ADXL335_AXIS_X:
      return "X";
    case ADXL335_AXIS_Y:
      return "Y";
    case ADXL335_AXIS_Z:
      return "Z";
    default:
      return "?";
  }
}
}

ADXL335SelfTest::ADXL335SelfTest(uint8_t xPin,
                                 uint8_t yPin,
                                 uint8_t zPin,
                                 uint8_t selfTestPin,
                                 float adcReferenceVoltage,
                                 bool selfTestActiveHigh)
    : _axisPins{xPin, yPin, zPin},
      _selfTestPin(selfTestPin),
      _adcReferenceVoltage(adcReferenceVoltage),
      _selfTestActiveHigh(selfTestActiveHigh),
      _minimumDeltaVolts{0.10f, 0.10f, 0.10f},
      _expectedPolarity{ADXL335_POLARITY_EITHER,
                        ADXL335_POLARITY_EITHER,
                        ADXL335_POLARITY_EITHER},
      _settleDelayMs(100),
      _sampleDelayMs(2) {
}

void ADXL335SelfTest::begin() {
  pinMode(_selfTestPin, OUTPUT);
  setSelfTestEnabled(false);
}

void ADXL335SelfTest::setMinimumDeltaVolts(float xMinVolts,
                                           float yMinVolts,
                                           float zMinVolts) {
  _minimumDeltaVolts[ADXL335_AXIS_X] = fabs(xMinVolts);
  _minimumDeltaVolts[ADXL335_AXIS_Y] = fabs(yMinVolts);
  _minimumDeltaVolts[ADXL335_AXIS_Z] = fabs(zMinVolts);
}

void ADXL335SelfTest::setExpectedPolarity(ADXL335ExpectedPolarity xPolarity,
                                          ADXL335ExpectedPolarity yPolarity,
                                          ADXL335ExpectedPolarity zPolarity) {
  _expectedPolarity[ADXL335_AXIS_X] = xPolarity;
  _expectedPolarity[ADXL335_AXIS_Y] = yPolarity;
  _expectedPolarity[ADXL335_AXIS_Z] = zPolarity;
}

void ADXL335SelfTest::setTimings(uint16_t settleDelayMs,
                                 uint8_t sampleDelayMs) {
  _settleDelayMs = settleDelayMs;
  _sampleDelayMs = sampleDelayMs;
}

ADXL335SelfTestResult ADXL335SelfTest::run(uint8_t sampleCount) {
  ADXL335SelfTestResult result;
  result.passed = true;

  if (sampleCount == 0) {
    sampleCount = 1;
  }

  setSelfTestEnabled(false);
  delay(_settleDelayMs);
  for (uint8_t axis = 0; axis < ADXL335_AXIS_COUNT; axis++) {
    result.normalRaw[axis] = readAverageRaw(_axisPins[axis], sampleCount);
  }

  setSelfTestEnabled(true);
  delay(_settleDelayMs);
  for (uint8_t axis = 0; axis < ADXL335_AXIS_COUNT; axis++) {
    result.selfTestRaw[axis] = readAverageRaw(_axisPins[axis], sampleCount);
  }

  setSelfTestEnabled(false);

  for (uint8_t axis = 0; axis < ADXL335_AXIS_COUNT; axis++) {
    result.deltaRaw[axis] =
        static_cast<int16_t>(result.selfTestRaw[axis]) -
        static_cast<int16_t>(result.normalRaw[axis]);
    result.deltaVolts[axis] = rawDeltaToVolts(result.deltaRaw[axis]);
    result.axisPassed[axis] = axisPasses(axis, result.deltaVolts[axis]);
    result.passed = result.passed && result.axisPassed[axis];
  }

  return result;
}

void ADXL335SelfTest::printResult(Stream &output,
                                  const ADXL335SelfTestResult &result) const {
  output.println(F("ADXL335 self-test result"));
  output.print(F("Overall: "));
  output.println(result.passed ? F("PASS") : F("FAIL"));

  for (uint8_t axis = 0; axis < ADXL335_AXIS_COUNT; axis++) {
    output.print(axisName(axis));
    output.print(F(": normal="));
    output.print(result.normalRaw[axis]);
    output.print(F(" selfTest="));
    output.print(result.selfTestRaw[axis]);
    output.print(F(" deltaRaw="));
    output.print(result.deltaRaw[axis]);
    output.print(F(" deltaV="));
    output.print(result.deltaVolts[axis], 3);
    output.print(F(" "));
    output.println(result.axisPassed[axis] ? F("PASS") : F("FAIL"));
  }
}

void ADXL335SelfTest::setSelfTestEnabled(bool enabled) const {
  const bool pinHigh = enabled ? _selfTestActiveHigh : !_selfTestActiveHigh;
  digitalWrite(_selfTestPin, pinHigh ? HIGH : LOW);
}

uint16_t ADXL335SelfTest::readAverageRaw(uint8_t pin,
                                         uint8_t sampleCount) const {
  uint32_t total = 0;

  for (uint8_t sample = 0; sample < sampleCount; sample++) {
    total += analogRead(pin);
    if (_sampleDelayMs > 0) {
      delay(_sampleDelayMs);
    }
  }

  return static_cast<uint16_t>((total + (sampleCount / 2)) / sampleCount);
}

float ADXL335SelfTest::rawDeltaToVolts(int16_t deltaRaw) const {
  return (static_cast<float>(deltaRaw) * _adcReferenceVoltage) / 1023.0f;
}

bool ADXL335SelfTest::axisPasses(uint8_t axis, float deltaVolts) const {
  if (fabs(deltaVolts) < _minimumDeltaVolts[axis]) {
    return false;
  }

  if (_expectedPolarity[axis] == ADXL335_POLARITY_POSITIVE) {
    return deltaVolts > 0.0f;
  }

  if (_expectedPolarity[axis] == ADXL335_POLARITY_NEGATIVE) {
    return deltaVolts < 0.0f;
  }

  return true;
}
