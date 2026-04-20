#include <math.h>

enum MODE {CAL, STP, WLKPC};
unsigned long stpTime;
int steps = 0;

const int xOut = A2; /* connect x_out of module to A1 of UNO board */
const int yOut = A1; /* connect y_out of module to A2 of UNO board */
const int zOut = A0; /* connect z_out of module to A3 of UNO board */
int xPrev = 0;
int yPrev = 0;
int zPrev = 0;

const int stpThreshold = 0.4;
const int stpDebounceDelay = 60;
unsigned long stpDebounceTime = 0;

//Variables required for step detection/counter
bool counting = false;
int stpPeriodLower = 300; //Change as neccessary
int stpPeriodUpper = 1000;

double refPitch = 0;
double pitch = 0;
double refRoll = 0;
double roll = 0;
double thresholdPR = 1;

double refX = 0;
double refY = 0;
double refZ = 0;
double thresholdXYZ = 1;


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
  stpTime = millis();
  // put your setup code here, to run once:
}

void stepCountR() { // if valid input from accelerometer, steps++; 

  float pitchChange = abs(refPitch - pitch);
  float rollChange = abs(refRoll - roll);

  if(currentState == STP || currentState == WLKPC) {
    if(counting) { // able to chain count steps 
      if((millis() - stpTime >= stpPeriodLower) && (millis() - stpTime <= stpPeriodUpper)) { // while within timeframe (natural buffer + time limit )
        if(pitchChange > thresholdPR || rollChange > thresholdPR) { // if threshold breached, wip
          refPitch = pitch;    // > set breach as new
          refRoll = roll;      //   reference point
          steps++;             // > increments steps
          stepsInLastSecond++;
          stpTime = millis();  // > reset timeframe
        } 
      }
      else if (millis() - stpTime > stpPeriodUpper) {counting = false;} // break if timeframe exceeded without peak
    }
    else {
      if (pitchChange > thresholdPR || rollChange > thresholdPR) { // if threshold breached not in chain counting state, set to chain counting state and reset timeframe
        refPitch = pitch;
        refRoll = roll;
        stpTime = millis();
        counting = true;
      }
    }
  }
}

void stepCountXYZ() {
  float xChange = abs(refX - xOut);
  float yChange = abs(refY - yOut);
  float zChange = abs(refZ - zOut);

  if(currentState == STP || currentState == WLKPC) {
    if(counting) { // able to chain count steps 
      if((millis() - stpTime >= stpPeriodLower) && (millis() - stpTime <= stpPeriodUpper)) { // while within timeframe (natural buffer + time limit )
        if(xChange > thresholdXYZ || yChange > thresholdXYZ || zChange > thresholdXYZ) { // if threshold breached, wip
          refX = xOut;    // > set breach as
          refY = yOut;    //   new reference
          refZ = zOut;    //   point
          steps++;             // > increments steps
          stepsInLastSecond++;
          stpTime = millis();  // > reset timeframe
        } 
      }
      else if (millis() - stpTime > stpPeriodUpper) {counting = false;} // break if timeframe exceeded without peak
    }
    else {
      if(xChange > thresholdXYZ || yChange > thresholdXYZ || zChange > thresholdXYZ) { // if threshold breached not in chain counting state, set to chain counting state and reset timeframe
        refX = xOut;
        refY = yOut;
        refZ = zOut;
        stpTime = millis();
        counting = true;
      }
    }
  }
}


int walkingPaceDetection(int returnType) { //The function works to track the steps that occured over the last 15 seconds, storing it in an array
  if (millis() > lastSweepTime + 1000){ //Walking pace is redetermined once per second so it won't bump around as much for variable walking paces
    stepsPerSecond[sweepnumber] = stepsInLastSecond; //Replaces an element in the array with the most recent steps per second, cycling through them all
    stepsInLastSecond = 0; 
    sweepnumber++;
    if (sweepnumber == 15) { //As there are only 15 elements in the array, cycles back to repeat the cycle
      sweepnumber = 0;
    }
    int tally = 0;
    for (int i = 0; i<=14; i++) {
      tally += stepsPerSecond[i];
    } //Tallies the total number of steps that occurred each second over the last 15 seconds

    lastSweepTime = millis(); //Timer reset
    if (tally >= 35) {walkingPace = 2;} //If steps for the cycle is 35 or higher (140/min), returns the user is running
    else if (tally <= 5) {walkingPace = 0;} //If steps for the cycle is 5 or lower (20/min), returns the user is stationary. Not 0 to account for possible noise or random movements.
    else {walkingPace = 1;} //If between the two, returns the user is walking
    if (returnType == 1) {return walkingPace;} //If return type is set to 1, return walking pace (standard mode)
    else if (returnType == 2) {return tally;} //Allows for the tally to be returned if desired alternatively
    else {return 0;} //Catch in case neither is used, so it can be easily detected.
  }
}



void loop() {
  if ((currentState == STP) || (currentState == WLKPC)) {

    walkingPaceDetection(1);
    if (millis () - stpTime < 3000) {
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