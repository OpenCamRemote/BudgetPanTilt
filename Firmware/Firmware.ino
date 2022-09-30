/*
 * OpenCamRemote BudgetPanTilt
 * A Stepper Pan/Tilt program for Arduino
 */

// CONFIG
// should the serial connection be in computer-friendly mode? (true for usage with OpenCamRemote software, false for manual use with a terminal or terminal emulator)
boolean computerFriendlySerial = true;


// PROGRAM
#include <TinyStepper_28BYJ_48.h>

// declare the stepper motors
TinyStepper_28BYJ_48 pan;
TinyStepper_28BYJ_48 tilt;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data
int panAmount = 0;
int panAmountDeg = 0;
int tiltAmount = 0;
int tiltAmountDeg = 0;

boolean newData = false;

boolean buttonReady = true;

void setup() {
  // Set up the button
  pinMode(12, INPUT_PULLUP);
  
  // connect pins 2/8,3/9,4/10,5/11 to IN1,IN2,IN3,IN4 on ULN2003 board
  pan.connectToPins(8,9,10,11);
  tilt.connectToPins(2,3,4,5);
  
  // let's set a custom speed
  pan.setSpeedInStepsPerSecond(500);
  tilt.setSpeedInStepsPerSecond(500);

  // now let's set up a serial connection and print some stepper info to the console
  Serial.begin(9600);

  // introduction messages
  if(!computerFriendlySerial){
    while(!Serial.read()) {}
    Serial.println("Camera Remote Pan/Tilt");
    Serial.println("V1.0");
    Serial.println();
    Serial.println("Syntax:");
    Serial.println("<pan, tilt>");
    Serial.println();
  }
}

void loop() {
  // we need to call run() during loop() 
  // in order to keep the stepper moving
  // if we are using non-blocking moves
  pan.processMovement();
  tilt.processMovement();

  if(pan.motionComplete()){
    pan.disableMotor();
  }
  if(tilt.motionComplete()){
    tilt.disableMotor();
  }

  // home mount position if home button is pressed
  if(digitalRead(12) == LOW){
    if(buttonReady){
      buttonReady = false;
      // 90 degrees is 512 steps
      pan.setupMoveInSteps(512);
      tilt.setupMoveInSteps(512);
      if(computerFriendlySerial){
        Serial.println("home");
      } else {
        Serial.println("Homing Camera");
        Serial.println();
      }
    }
  } else {
    buttonReady = true;
  }
  
  recvWithStartEndMarkers();
  if(newData) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    moveToPos();
    newData = false;
  }
}

void recvWithStartEndMarkers() { // TBH, I'm not even sure how most of this works. It's just a modified version of this example code https://forum.arduino.cc/t/serial-input-basics-updated/382007/3#example-5-receiving-and-parsing-several-pieces-of-data-3
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void parseData() {      // split the data into its parts
  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars,","); // get the new pan
  panAmountDeg = atoi(strtokIndx);
  panAmount = map(panAmountDeg, 0, 360, 0, 2048); // convert to integer

  strtokIndx = strtok(NULL, ","); // get the new tilt
  tiltAmountDeg = atoi(strtokIndx);
  tiltAmount = map(tiltAmountDeg, 0, 360, 0, 2048); // convert to integer
}

void moveToPos() {
  if(!computerFriendlySerial){
    Serial.print("Pan: "); Serial.println(panAmountDeg);
    Serial.print("Tilt: "); Serial.println(tiltAmountDeg);
    Serial.println();
  }
    
  pan.setupMoveInSteps(panAmount);
  tilt.setupMoveInSteps(tiltAmount);
}
