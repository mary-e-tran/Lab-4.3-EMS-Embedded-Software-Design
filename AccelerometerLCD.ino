//Included Libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"


// initialise
enum MODE { ST, CAL, STP, WLK };
MODE currentState = CAL;

//Pin definitions
const int x_out = A2;
const int y_out = A1;
const int z_out = A0;
#define TFT_DC 7
#define TFT_CS 10
//TFT_SCL 13
//TFT_SDA 11
//TFT_RES 8
//TFT_BLK 9 or just 3.3V
const int switchButtonPin = 2;
const int debugButtonPin = 3;

int switchButtonState;
int debugButtonState;
int prevSwitchButtonState = LOW;
int prevDebugButtonState = LOW;
int debounceDelay = 50;

uint32_t debounceTime = 0;
int x_adc_value, y_adc_value, z_adc_value;
double x_g_value, y_g_value, z_g_value;
double roll, pitch, yaw;
// Variables for display timing and logic simulations
unsigned long prevDisplayMillis = 0;
const long displayInterval = 1500; // Refreshes the display every 1.5 seconds
//Variables for step count and the walking pace detection tracking
int stepsInLastSecond = 0;
int sweepnumber = 0;
int stepsPerSecond[15];
int buttonState;
int lastButtonState = LOW;
unsigned long lastSweepTime = 0;


//Creation of LCD object to control
Adafruit_GC9A01A tft(TFT_CS, TFT_DC);


void setup() {
  pinMode(switchButtonPin, INPUT);
  pinMode(debugButtonPin, INPUT);
  tft.begin();

  lastSweepTime = millis();
  updateDisplay();
}

void loop() {
  stateToggle();
  debugToggle();
  accelerometerInterpreting();

  unsigned long currentMillis = millis();
  if (currentMillis - prevDisplayMillis >= displayInterval) {
    updateDisplay();
  }
}

unsigned long testText() {
  tft.fillScreen(GC9A01A_BLACK);
  unsigned long start = micros(); 
  tft.setCursor(0, 0);
  tft.setTextColor(GC9A01A_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(GC9A01A_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(GC9A01A_GREEN);
  tft.setTextSize(5);
  tft.println("BASIC");
  tft.setTextSize(2);
  tft.println("have a read of this");
  tft.setTextSize(1);
  tft.println("isn't actually doing much.");
  tft.println("will update for displaying Accelerometer values later");
  return micros() - start;
}

void accelerometerInterpreting() {
  x_adc_value = analogRead(x_out); /* Digital value of voltage on x_out pin */ 
  y_adc_value = analogRead(y_out); /* Digital value of voltage on y_out pin */ 
  z_adc_value = analogRead(z_out); /* Digital value of voltage on z_out pin */

  x_g_value = ( ( ( (double)(x_adc_value * 5)/1024) - 1.665 ) / 0.335 ); /* Acceleration in x-direction in g units */ 
  y_g_value = ( ( ( (double)(y_adc_value * 5)/1024) - 1.675 ) / 0.343 ); /* Acceleration in y-direction in g units */ 
  z_g_value = ( ( ( (double)(z_adc_value * 5)/1024) - 1.693 ) / 0.3415 ); /* Acceleration in z-direction in g units */ 

  roll = ( ( (atan2(y_g_value,z_g_value) * 180) / 3.14 ) + 180 ); /* Formula for roll */
  pitch = ( ( (atan2(z_g_value,x_g_value) * 180) / 3.14 ) + 90 ); /* Formula for pitch */
}

// Custom function to handle dynamic mode drawing
void updateDisplay() {
  tft.fillScreen(GC9A01A_BLACK); // Clear screen safely before drawing
  
  int cx = tft.width() / 2;
  int cy = tft.height() / 2;
  int maxRadius = min(tft.width(), tft.height()) / 2;

  // --- STATE DEPENDENT GRAPHICS ---
  if (currentState == STP || currentState == WLK) {
    // 3 circles along the outer edge: Green -> Blue -> Green
    tft.drawCircle(cx, cy, maxRadius - 2, GC9A01A_GREEN);
    tft.drawCircle(cx, cy, maxRadius - 7, GC9A01A_BLUE);
    tft.drawCircle(cx, cy, maxRadius - 12, GC9A01A_GREEN);

    // Dynamic text centering configuration
    tft.setTextSize(3);
    tft.setTextColor(GC9A01A_WHITE);
    
    // Crude centering calculation: character width is roughly (6 * textSize) pixels
    String stepStr = String(mockStepCount);
    int16_t textWidth = stepStr.length() * 18; 
    
    tft.setCursor(cx - (textWidth / 2), cy - 10);
    tft.print(stepStr);
    
    // Visual text sub-label
    tft.setTextSize(1);
    tft.setCursor(cx - 15, cy + 20);
    tft.print("STEPS");

    // Simulate stepping for proof of concept
    if(currentState == WLK) {
      mockStepCount += 3;
      tft.setTextSize(1);
      tft.setCursor(cx - 65, cy + 35);
      tft.print("Current Pace: RUNNING");
    }

  } 
  else if (currentState == CAL || currentState == ST) {
    // 2 circles along the outer edge: Orange -> Red
    // Note: Adafruit_GFX doesn't have a built-in GC9A01A_ORANGE macro, 
    // so we build it via color565(R, G, B) -> Red max, Green mid, Blue zero
    uint16_t orangeColor = tft.color565(255, 165, 0);
    
    tft.drawCircle(cx, cy, maxRadius - 2, orangeColor);
    tft.drawCircle(cx, cy, maxRadius - 5, GC9A01A_RED);
    tft.drawCircle(cx, cy, maxRadius - 8, orangeColor);

    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);

    if (currentState == CAL) {
      String calStr = String(mockCalPercent) + "%";
      int16_t textWidth = calStr.length() * 12;
      
      tft.setCursor(cx - (textWidth / 2), cy - 15);
      tft.print(calStr);
      
      tft.setTextSize(1);
      tft.setCursor(cx - 42, cy + 10);
      tft.print("Calibrating...");

      tft.setCursor(cx - 42, cy + 25);
      tft.print("Rotate left");

      // Simulate calibration progression
      mockCalPercent += 5;
      if (mockCalPercent > 100) mockCalPercent = 0;
    } 
    else if (currentState == ST) {
      // Print live Accelerometer outputs
      int x_val = analogRead(x_out);
      int y_val = analogRead(y_out);
      int z_val = analogRead(z_out);

      tft.setTextSize(2);
      tft.setTextColor(GC9A01A_YELLOW);
      
      tft.setCursor(cx - 30, cy - 30);
      tft.print("X: "); tft.println(x_val);
      tft.setCursor(cx - 30, cy - 5);
      tft.print("Y: "); tft.println(y_val);
      tft.setCursor(cx - 30, cy + 20);
      tft.print("Z: "); tft.println(z_val);
    }
  }
  prevDisplayMillis = currentMillis;
}



void stateToggle() {  
  int read = digitalRead(switchButtonPin);

  if(read != prevSwitchButtonState) {
    debounceTime = millis();
  }

  if((millis() - debounceTime) > debounceDelay) {
    if (read != switchButtonState) {
      switchButtonState = read;
      
      if (switchButtonState == HIGH) {
        switch(currentState) {
          case ST: {
            currentState = CAL;
            break;
          }
          case CAL: {
            currentState = ST;
            break;
          }
          case STP: {
            currentState = WLK;
            break;
          }
          case WLK: {
            currentState = STP;
            break;
          }
        }
      }
    }
  }
  prevSwitchButtonState = read;
}

void debugToggle() {  
  int read = digitalRead(debugButtonPin);

  if(read != prevDebugButtonState) {
    debounceTime = millis();
  }

  if((millis() - debounceTime) > debounceDelay) {
    if (read != debugButtonState) {
      debugButtonState = read;
      
      if (debugButtonState == HIGH) {
        switch(currentState) {
          case ST:
          case CAL: {
            currentState = STP;
            break;
          }
          case STP:
          case WLK: {
            currentState = CAL;
            break;
          }
        }
      }
    }
  }
  prevDebugButtonState = read;
}