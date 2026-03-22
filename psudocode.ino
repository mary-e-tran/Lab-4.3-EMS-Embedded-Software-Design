
void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

void stepCount() {
  // values from ADC pins (connected to x output, y output, or z output) are mapped to range

  // if current state is STPCNT (step counting) or WLKPC (walking pace)
    // if mapped value > n
      // if significant change in mapped value occurs (to distinguish peak from mere orientation change) (??)
        // increment 'steps (int)' variable

}

void display() {  
  switch(currentState) {
    case ST: {
      // display self-test stats (difference in voltage for each output) (comfirm process with teammate)
      break;
    }
    case CAL: {
      // while calibration in progress (calActive = true)
        //display "calibration in progess..." OR give instructions for calibration (comfirm calibration process with teammate)
      
      // when complete, display "calibration finished" or switch to other mode automatically (see userControl)
      break;
    }
    case STPCNT: {
      // display "Steps: (int steps)"
      break;
    }
    case WLKPC: {
      // "Walking pace: (walking pace)"
      // *** note: walking pace return (string or numerical) is controlled by walking pace subroutine
      break;
    }
  }
}

void userControl() {  
  switch(currentState) {
    case ST: {
      // if button tapped (falling edge triggers change)
        // set current system state to STPCNT (step counting)
      // else if button held (3 s)
        // set current system state to CAL (calibration)
      break;
    }
    case CAL: {
      // if calibration is complete (calActive = false) with no input for 3 s OR if button tapped (falling edge triggers change)
        // set current system state to STPCNT (step counting)
      // else if button double tapped
        // set current system state to ST (self-test)
      break;
    }
    case STPCNT: {
      // if button tapped (falling edge triggers change)
        // set current system state to WLKPC (walking pace identification)
      // else if button held (3 s)
        // set current system state to CAL (calibration)
      // else if button double tapped
        // set current system state to ST (self-test)
      break;
    }
    case WLKPC: {
      // if button tapped (falling edge triggers change)
        // set current system state to STPCNT (step counting)
      // else if button held (3 s)
        // set current system state to CAL (calibration)
      // else if button double tapped
        // set current system state to ST (self-test)
      break;
    }
  }
}