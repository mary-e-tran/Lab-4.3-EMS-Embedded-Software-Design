#ifndef ADXL335_SELF_TEST_H
#define ADXL335_SELF_TEST_H

#include <Arduino.h>

enum ADXL335SelfTestAxis {
  ADXL335_AXIS_X = 0,
  ADXL335_AXIS_Y = 1,
  ADXL335_AXIS_Z = 2,
  ADXL335_AXIS_COUNT = 3
};

enum ADXL335ExpectedPolarity {
  ADXL335_POLARITY_EITHER = 0,
  ADXL335_POLARITY_POSITIVE = 1,
  ADXL335_POLARITY_NEGATIVE = -1
};

struct ADXL335SelfTestResult {
  uint16_t normalRaw[ADXL335_AXIS_COUNT];
  uint16_t selfTestRaw[ADXL335_AXIS_COUNT];
  int16_t deltaRaw[ADXL335_AXIS_COUNT];
  float deltaVolts[ADXL335_AXIS_COUNT];
  bool axisPassed[ADXL335_AXIS_COUNT];
  bool passed;
};

class ADXL335SelfTest {
public:
  ADXL335SelfTest(uint8_t xPin,
                  uint8_t yPin,
                  uint8_t zPin,
                  uint8_t selfTestPin,
                  float adcReferenceVoltage = 5.0f,
                  bool selfTestActiveHigh = true);

  void begin();

  void setMinimumDeltaVolts(float xMinVolts, float yMinVolts, float zMinVolts);
  void setExpectedPolarity(ADXL335ExpectedPolarity xPolarity,
                           ADXL335ExpectedPolarity yPolarity,
                           ADXL335ExpectedPolarity zPolarity);
  void setTimings(uint16_t settleDelayMs, uint8_t sampleDelayMs);

  ADXL335SelfTestResult run(uint8_t sampleCount = 32);
  void printResult(Stream &output, const ADXL335SelfTestResult &result) const;

private:
  uint8_t _axisPins[ADXL335_AXIS_COUNT];
  uint8_t _selfTestPin;
  float _adcReferenceVoltage;
  bool _selfTestActiveHigh;
  float _minimumDeltaVolts[ADXL335_AXIS_COUNT];
  ADXL335ExpectedPolarity _expectedPolarity[ADXL335_AXIS_COUNT];
  uint16_t _settleDelayMs;
  uint8_t _sampleDelayMs;

  void setSelfTestEnabled(bool enabled) const;
  uint16_t readAverageRaw(uint8_t pin, uint8_t sampleCount) const;
  float rawDeltaToVolts(int16_t deltaRaw) const;
  bool axisPasses(uint8_t axis, float deltaVolts) const;
};

#endif
