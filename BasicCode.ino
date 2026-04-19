#include <math.h>

enum MODE {CAL, STP, WLKPC};
unsigned long lastStep;
int stepCount = 0;

const int xOut = A2; /* connect x_out of module to A1 of UNO board */
const int yOut = A1; /* connect y_out of module to A2 of UNO board */
const int zOut = A0; /* connect z_out of module to A3 of UNO board */
int xPrev = 0;
int yPrev = 0;
int zPrev = 0;

const int stpThreshold = 0.4;
const int stpDebounceDelay = 60;
unsigned long stpDebounceTime = 0;

//Variables required for walking pace detection sequence
int stepsInLastSecond = 0;
int sweepnumber = 0;
int stepsPerSecond[15]; //Array storing the last 15 steps; not 
unsigned long lastSweepTime = 0;
int walkingPace = 0;

MODE currentState;

void setup() {
  Serial.begin(9600);
  //currentState = CAL;
  lastStep = millis();
  // put your setup code here, to run once:
}

void stepCounting() { // if valid input from accelerometer, steps++;
  
  int xChange = abs(xOut - xPrev);
  int yChange = abs(yOut - yPrev);
  int zChange = abs(zOut - zPrev);

  if(currentState == STP || currentState == WLKPC) {                                      // is current MODE is step counting OR walking pace identification ?
    if(xChange > stpThreshold || yChange > stpThreshold || zChange > stpThreshold) {    // are any value changes significant enought to be considered a step ?
      xPrev = xOut;
      yPrev = yOut;
      zPrev = zOut;
      
      if(millis() - stpDebounceTime > stpDebounceDelay) {                               // are values changes persistent ?
        xChange = abs(xOut - xPrev);
        yChange = abs(yOut - yPrev);
        zChange = abs(zOut - zPrev);

        if(xChange > stpThreshold || yChange > stpThreshold || zChange > stpThreshold) {
          stepCount++;
        }
      }
    }
  }
}
//Track when each step over the last 15 seconds occured using an array
//If 35 or more steps have occurred over the last 15 seconds
//  (equating to 140/minute)
//WlkPcDet will return a 2, indicating that the current
//  walking pace is running
//else if 5 or less steps have occurred over the last 15 seconds
//  (equating to around 20/minute)
  //WlkPcDet will retrn a 0, indicating that the current
  //  walking pace is stationary
  //  Note that the number of steps being checked for is not 0;
  //  this is to account for where the device is moving a small
  //  amount, but steps are not really occurring
//else (only occurring if steps in the last 15 seconds are
//  between 5 and 35)
  //WlkPcDet will return a 1, indicating that the current
  //  walking pace is walking
int walkingPaceDetection() {
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

    lastSweepTime = millis();
    if (tally >= 35) {walkingPace = 2;}
    else if (tally <= 5) {walkingPace = 0;}
    else {walkingPace = 1;}
    return walkingPace;
  }
}



void loop() {
  if ((currentState == STP) || (currentState == WLKPC)) {
    if (millis () - lastStep < 3000) {
      //if step occurs
        //lastStep = millis();
        //stepCount++;

      //Integrate acceleration over each axis into displacement
      // Utilise displacement over last 15 seconds, multiplying by 4 to get km/min; use 1/n to convert to actual pace

    }
  }

}

void Display() {
  if (currentState == STP) {
    //Display step count and other relevant info
  }
  else if (currentState == WLKPC) {
    

    //Display walking pace; may want to display step count also, but shouldnt be required
  }
  else if (currentState == CAL) {
    //Calibration routine
  }
  else {
    //Inform user that system is not in a designated mode; this is effectively just a catch in case a mode was added but system display settings haven't yet.
  }
  //If any other modes are added, add them in as separate modes in the enum and continue using elifs. Keep overall structure.
}