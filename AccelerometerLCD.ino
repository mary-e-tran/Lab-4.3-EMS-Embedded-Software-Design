//Included Libraries
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_GC9A01A.h"


// initialise
enum MODE { ST, CAL, STP, WLK };
MODE currentState = CAL;
bool initCalComplete = false;

//Pin definitions
const int x_out = A0;
const int y_out = A2;
const int z_out = A1;
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

//Variables for keeping the readings from the accelerometer present.
int x_adc_value, y_adc_value, z_adc_value;
double x_g_value, y_g_value, z_g_value;
double roll, pitch, yaw;

// Variables for display timing and logic simulations
unsigned long prevDisplayMillis = 0;
const long displayInterval = 1500; // Refreshes the display every 1.5 seconds

//Variables for step count and the walking pace detection tracking
unsigned long stpTime;
int steps = 0;
const int stpThreshold = 0.4;
const int stpDebounceDelay = 60;
unsigned long stpDebounceTime = 0;
//Variables required for step detection/counter
bool counting = false;
int stpPeriodLower = 300; //Change as neccessary
int stpPeriodUpper = 1000;

double refPitch = 0;
double refRoll = 0;
double thresholdPR = 1000;

double refX = 0;
double refY = 0;
double refZ = 0;
double thresholdXYZ = 1;

int stepsInLastSecond = 0;
int sweepnumber = 0;
String walkingPace = "";
int stepsPerSecond[15];
unsigned long lastSweepTime = 0;

double mockCalPercent = 0.00;

//Creation of LCD object to control
Adafruit_GC9A01A tft(TFT_CS, TFT_DC);


void setup() {
  pinMode(switchButtonPin, INPUT);
  pinMode(debugButtonPin, INPUT);
  tft.begin();

  lastSweepTime = millis();
  stpTime = millis();
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

void stepCountR() { // if valid input from accelerometer, steps++; 
  float pitchChange = abs(refPitch - pitch);
  float rollChange = abs(refRoll - roll);

  if(counting) { // able to chain count steps 
    if((millis() - stpTime >= stpPeriodLower) && (millis() - stpTime <= stpPeriodUpper)) { // while within timeframe (natural buffer + time limit )
      if(pitchChange > thresholdPR || rollChange > thresholdPR) { // if threshold breached, wip
        refPitch = pitch;    // > set breach as new
        refRoll = roll;      //   reference point
        steps++;             // > increments steps
        stpTime = millis();  // > reset timeframe
        stepsInLastSecond++;
      } 
    }
    else if (millis() - stpTime > stpPeriodUpper) {counting = false;} // break if timeframe exceeded without peak
  }
  else {
    if(pitchChange > thresholdPR || rollChange > thresholdPR) { // if threshold breached not in chain counting state, set to chain counting state and reset timeframe
      refPitch = pitch;
      refRoll = roll;
      stpTime = millis();
      counting = true;
    }
  }
}

void stepCountXYZ() {
  float xChange = abs(refX - x_adc_value);
  float yChange = abs(refY - y_adc_value);
  float zChange = abs(refZ - z_adc_value);
  float pitchChange = abs(refPitch - pitch);
  float rollChange = abs(refRoll - roll);

  if(counting) { // able to chain count steps 
    if((millis() - stpTime >= stpPeriodLower) && (millis() - stpTime <= stpPeriodUpper)) { // while within timeframe (natural buffer + time limit )
      if(xChange > thresholdXYZ || yChange > thresholdXYZ || zChange > thresholdXYZ) { // if threshold breached, wip
        refX = x_adc_value;    // > set breach as
        refY = y_adc_value;    //   new reference
        refZ = z_adc_value;    //   point
        steps++;             // > increments steps
        stpTime = millis();  // > reset timeframe
        stepsInLastSecond++;
      } 
    }
    else if (millis() - stpTime > stpPeriodUpper) {counting = false;} // break if timeframe exceeded without peak
  }
  else {
    if (pitchChange > thresholdPR || rollChange > thresholdPR) { // if threshold breached not in chain counting state, set to chain counting state and reset timeframe
      refX = x_adc_value;
      refY = y_adc_value;
      refZ = z_adc_value;
      stpTime = millis();
      counting = true;
      }
  }
}

String detectPace() {
  if (millis() > lastSweepTime + 1000){
    stepsPerSecond[sweepnumber] = stepsInLastSecond;
    stepsInLastSecond = 0;
    sweepnumber++;
    if (sweepnumber == 15) {
      sweepnumber = 0;
    }
    int tally = 0;
    for (int i = 0; i<=14; i++) {
      tally += stepsPerSecond[i];
    }
    if (tally >= 35) {walkingPace = "RUNNING";}
    else if (tally <= 5) {walkingPace = "STATIONARY";}
    else {walkingPace = "WALKING";}

    lastSweepTime = millis();
    return walkingPace;
  }
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

    if (currentState == STP) {
      // Dynamic text centering configuration
      tft.setTextSize(3);
      tft.setTextColor(GC9A01A_WHITE);

      // Crude centering calculation: character width is roughly (6 * textSize) pixels
      String stepStr = String(steps);
      int16_t textWidth = stepStr.length() * 18; 
      
      tft.setCursor(cx - (textWidth / 2), cy - 10);
      tft.print(stepStr);
      
      // Visual text sub-label
      tft.setTextSize(1);
      tft.setCursor(cx - 15, cy + 20);
      tft.print("STEPS");
    }
   else {
      // Replace the existing WLK block with this:
      tft.setTextSize(2);
      tft.setTextColor(GC9A01A_WHITE);

      String pace = detectPace();

      // Each char at textSize 2 is 12px wide, 16px tall
      int16_t textWidth = pace.length() * 12;

      tft.setCursor(cx - (textWidth / 2), cy + 8);
      tft.print(pace);

      tft.setTextSize(1);
      int16_t labelWidth = 12 * 4; // "PACE" = 5 chars
      tft.setCursor(cx - (labelWidth / 2), cy - 14);
      tft.print("PACE");
    }
    // Simulate stepping for proof of concept

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

    if (currentState == CAL) { //Replace once correct callibration code is created
      int mockCalPercentDisp = round(mockCalPercent);
      String calStr = String(mockCalPercentDisp) + "%";
      int16_t textWidth = calStr.length() * 12;
      
      tft.setCursor(cx - (textWidth / 2), cy - 15);
      tft.print(calStr);
      
      tft.setTextSize(1);
      tft.setCursor(cx - 42, cy + 10);
      tft.print("Calibrating...");

      tft.setCursor(cx - 42, cy + 25);
      tft.print("Rotate left");

      // Simulate calibration progression
      mockCalPercent += 9.3;
      if (mockCalPercent > 100) {
        mockCalPercent = 0;
        if (initCalComplete == false) {
          initCalComplete = true;
          currentState = STP;
        }
      }
    } 
    else if (currentState == ST) { //Does not currently read the ST pin as we didnt connect it on the pcb; will display the raw adc values though.
      // Print live Accelerometer outputs
      tft.setTextSize(2);
      tft.setTextColor(GC9A01A_YELLOW);
      
      tft.setCursor(cx - 30, cy - 30);
      tft.print("X: "); tft.println(x_g_value);
      tft.setCursor(cx - 30, cy - 5);
      tft.print("Y: "); tft.println(y_g_value);
      tft.setCursor(cx - 30, cy + 20);
      tft.print("Z: "); tft.println(z_g_value);
    }
  }
  unsigned long currentMillis = millis();
  prevDisplayMillis = currentMillis;
}



void stateToggle() {  
  if (initCalComplete) {
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
}

void debugToggle() {
  if (initCalComplete) {
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
              //Reset data from user mode whilst in debug so itll be clean when leaving. 
              steps = 0;
              stepsInLastSecond = 0;
              sweepnumber = 0;
              for (int i = 0; i<=14; i++) {
                stepsPerSecond[i] = 0;
              }
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
  
}