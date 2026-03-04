enum MODE {CAL, STPCNT, WLKPC};
unsigned long lastStep;
int stepCount = 0;

MODE CurrentSt;

void setup() {
  CurrentSt = CAL;
  lastStep = millis();
  // put your setup code here, to run once:
}
void loop() {
  if ((CurrentSt == STPCNT) || (CurrentSt == WLKPC)) {
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
  if (CurrentSt == STPCNT) {
    //Display step count and other relevant info
  }
  else if (CurrentSt == WLKPC) {
    //Display walking pace; may want to display step count also, but shouldnt be required
  }
  else if (CurrentSt == CAL) {
    //Calibration routine
  }
  else {
    //Inform user that system is not in a designated mode; this is effectively just a catch in case a mode was added but system display settings haven't yet.
  }
  //If any other modes are added, add them in as separate modes in the enum and continue using elifs. Keep overall structure.
}