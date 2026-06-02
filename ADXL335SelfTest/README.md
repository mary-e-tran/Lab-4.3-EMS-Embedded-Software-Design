# ADXL335SelfTest

Small Arduino library for checking whether an ADXL335 responds when its
`ST`/self-test input is asserted.

The test works by:

1. Reading averaged normal `X/Y/Z` ADC values.
2. Turning on the ADXL335 self-test input.
3. Reading averaged self-test `X/Y/Z` ADC values.
4. Comparing the voltage change on each axis against configurable thresholds.

This is designed for the existing sketch pin order:

```cpp
X -> A2
Y -> A1
Z -> A0
```

You must also provide the Arduino digital pin connected to the MOSFET self-test
control circuit. The current PCB inverts this signal, so the examples below use
active-low self-test control.

## Basic Use

```cpp
#include <ADXL335SelfTest.h>

const uint8_t SELF_TEST_PIN = 4; // Change to your actual MOSFET control pin.

ADXL335SelfTest adxlSelfTest(A2, A1, A0, SELF_TEST_PIN, 5.0f, false);

void setup() {
  Serial.begin(9600);
  adxlSelfTest.begin();
}

void loop() {
  ADXL335SelfTestResult result = adxlSelfTest.run();

  if (result.passed) {
    Serial.println("ADXL335 self-test passed");
  } else {
    Serial.println("ADXL335 self-test failed");
  }

  adxlSelfTest.printResult(Serial, result);
  delay(5000);
}
```

## Settings for MOS

The PCB transistor driver makes the self-test signal active-low, so construct
the class with `false` as the final argument:

```cpp
ADXL335SelfTest adxlSelfTest(A2, A1, A0, SELF_TEST_PIN, 5.0f, false);
```

If the Arduino ADC reference is not 5 V, pass the correct reference voltage and
keep the final argument as `false`:

```cpp
ADXL335SelfTest adxlSelfTest(A2, A1, A0, SELF_TEST_PIN, 3.3f, false);
```

Start with the default `0.10 V` minimum axis delta. After testing real hardware,
tune the threshold using:

```cpp
adxlSelfTest.setMinimumDeltaVolts(0.10f, 0.10f, 0.10f);
```

This self-test only apply while user on rest or device on a flatsurface or manually calibrated. The check compares before/after
readings, so movement during the test can make the result noisy.
