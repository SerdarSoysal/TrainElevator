// Version 0.8
// Version history
// 0.0 - Enables the MegaMoto board and sets the motor pins to inactive to avoid immediate movement of actuator
// 0.1 - Enables the manual up/down control button
// 0.2 - Adds the ElevatorUp, ElevatorDown, and ElevatorStop functions
// 0.3 - Add reading input from actuator
// 0.4 - Add support for preset buttons
// 0.5 - Add slowing down the elevator as it nears target to improve accuracy
// 0.6 - Add programming support
// 0.7 - Add EEPROM support
// 0.8 - Commented out the debugging messages and added more comments

/*
 * by Serdar Soysal
 * 
 * Big thanks to code samples at arduino.cc, Firgelli Automations, Progressive Automations, and Shift Automation.
 * 
 * This code uses an Arduino Uno board to control a position feedback linear actuator using various tactile switches for control.
 * The goal is to use this as a control mechanism for a train elevator.
 * This implementation uses two levels: main and staging. It is possible to extend the code to use more levels by adding the necessary switches and controls.
 * The implementation has the following details:
 *   - Preset pushbuttons for both levels
 *   - Ability to move the elevator manually using a momentary SPDT switch
 *   - Ability to program either preset pushbutton to the current location of the actuator
 *   - The preset values are stored in the EEPROM memory to avoid losing them each time the board is turned off
 * 
 * Potential improvements:
 *   - Add a potentiometer input for controlling the speed of the actuator
 *   - Add digital display output for elevator levels
 */

// Add additional libraries
#include <Button.h>
#include <EEPROM.h>

// Use the jumpers on the board to select which pins will be used
int EnablePin = 8; // use pin 8 to enable board
int PWMPinA1 = 3;  // motor pin A for actuator
int PWMPinB1 = 11; // motor pin B for actuator
// switch these two pins if the elevator moves in the opposite direction than you expect

// Declare our buttons and specify the correponding pins
Button ManualUp(9);      // use pin 9 to move elevator up manually
Button ManualDown(10);   // use pin 10 to move elevator down manually
Button PresetMain(6);    // use pin 6 to move elevator to main level
Button PresetStaging(7); // use pin 7 to move elevator to staging level
Button Program(12);      // use pin 12 for programming switch

// Specify our input pin for actuator position
int SensorPin = A3;

// Set other globally used variables
// int delaytime = 5*1000; // 5 seconds times 1,000 to convert to milliseconds
int MainLevel;    // will store/read these values from EEPROM for production
int StagingLevel; // will store/read these values from EEPROM for production
int CurrentPos = 0;   // use to read the current position of actuator
int TargetPos = 0;    // use to set the target position of actuator
int Accuracy = 1;    // stopping accuracy of the actuator - must be at least 15 less than ApproachLimit
int ApproachLimit = 20;   // defines the vicinity of target where the actuator will slow down - must be at least 15 greater than Accuracy
bool Programming = false; // if true, elevator is in programming mode
bool AutoPilot = false;   // if true, elevator is on autopilot
const int Stopped = 0;    // elevator autopilot status
const int GoingUp = 1;    // elevator autopilot status
const int GoingDown = 2;  // elevator autopilot status
const int Fast = 3;       // elevator autopilot speed
const int Slow = 4;       // elevator autopilot speed
int AutoPilotStatus = Stopped;  // start at stopped level
int AutoPilotSpeed = Fast;      // default speed is Fast
int FullSpeed = 255;    // full elevator speed - adjust based on your actuator
int ApproachSpeed = 60; // slow elevator speed - adjust based on your actuator
const int MainAddress = 0;    // EEPROM address for storing the main level
const int StagingAddress = 5; // EEPROM address for storing the staging level

void setup() {
  // initialize serial port
  Serial.begin(9600);

  // initialize our buttons
  ManualUp.begin();
  ManualDown.begin();
  PresetMain.begin();
  PresetStaging.begin();
  Program.begin();
 
  // read values from EEPROM
  EEPROM.get(MainAddress,MainLevel);
  EEPROM.get(StagingAddress,StagingLevel);
  // Serial.println("Read the following values for main and staging levels:");
  // Serial.println(MainLevel);
  // Serial.println(StagingLevel);

  // initialize MegaMoto board 
  pinMode(EnablePin, OUTPUT); //Enable the board
  pinMode(PWMPinA1, OUTPUT);
  pinMode(PWMPinB1, OUTPUT);  //Set motor outputs

  // set our sensor pin to read
  pinMode(SensorPin, INPUT);

  // make sure we don't move the actuator upon start
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, 0);

  // enable the megamoto shield
  // Serial.println("Enabling MegaMoto shield");
  // delay(delaytime);
  digitalWrite(EnablePin, HIGH);
  // Serial.println("MegaMoto shield is now enabled");

}

void loop() {

  CurrentPos = ReadPos();

  // if the elevator is moving on autopilot because a preset button is pressed AND the target position is reached, stop the elevator
  if ((AutoPilot) && (ReachedTarget(CurrentPos))) {
      ElevatorStop();
      AutoPilot = false;
      AutoPilotStatus = Stopped;
  }

  // check to see if the preset button for main level is pressed
  if (PresetMain.pressed()) {
    // if the elevator is in programming mode, then we should read the current position, set the variable for main level to that value and also store it in the EEPROM
    if (Programming) {
      MainLevel = ReadPos();
      EEPROM.put(MainAddress,MainLevel);    
      // Serial.print("Main level set to: ");
      // Serial.println(MainLevel);
    }
    // otherwise, switch to autopilot mode and go to the main level
    else {   
      AutoPilot = true;
      TargetPos=MainLevel;
      GoToTarget();      
    }
  }

  // check to see if the preset button for staging level is pressed
  if (PresetStaging.pressed()) {
    // if the elevator is in programming mode, then we should read the current position, set the variable for staging level to that value and also store it in the EEPROM
    if (Programming) {
      StagingLevel = ReadPos();
      EEPROM.put(StagingAddress,StagingLevel);     
      // Serial.print("Staging level set to: ");
      // Serial.println(StagingLevel);
    }
    // otherwise, switch to autopilot mode and go to the staging level
    else {
       AutoPilot = true;
       TargetPos=StagingLevel;
       GoToTarget();
    }
  }

  // check to see either of the manual control buttons are pressed
  // if they are, interrupt any autopilot action and then start moving the elevator full speed in the direction of the button pressed
  if (ManualUp.pressed()) {
    AutoPilot = false;
    ElevatorUp(FullSpeed);
  }
  if (ManualDown.pressed()) {
    AutoPilot = false;
    ElevatorDown(FullSpeed);
  }

  // check to see if both manual control buttons are released, if they are, stop the elevator
  if (ManualUp.released() || ManualDown.released()) {
    ElevatorStop();
  } 

  // enter or exit programming mode based on the status of the programming button
  if (Program.pressed()) {
    Programming = true;
    // Serial.println("Entering programming mode.");
  }
  if (Program.released()) {
    Programming = false;
    // Serial.println("Exiting programming mode.");
  }
}

void ElevatorUp(int Speed) {
  // Activate pin A and deactivate pin B  
  analogWrite(PWMPinA1, Speed);
  analogWrite(PWMPinB1, 0); // speed 0-255

  // Print starting point
  // Serial.print("Moving elevator up from position: ");
  // Serial.println(ReadPos());
}

void ElevatorDown(int Speed) {
  // Activate pin B and deactivate pin A  
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, Speed); // speed 0-255

  // Print starting point
  // Serial.print("Moving elevator down from position: ");
  // Serial.println(ReadPos());
}

void ElevatorStop() {
  // Deactivate both pins  
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, 0); // speed 0-255

  // Print stopping point
  // Serial.print("Elevator stopped at position: ");
  // Serial.println(ReadPos());
}

int ReadPos() {
  return analogRead(SensorPin);
}

void GoToTarget() {
  // this routine is called when one of the preset buttons is pressed
  // switch to autopilot mode
  AutoPilot = true;  
  if (ReachedTarget(CurrentPos)) {
    // if the elevator is already at target, then don't move and switch off autopilot
    // Serial.println("Elevator already at target.");
    AutoPilot = false;
  }
  else if (TargetPos < CurrentPos) {
    // the target position is less than the current position, so the elevator needs to start going down
    ElevatorDown(FullSpeed);
    AutoPilotStatus = GoingDown;
    AutoPilotSpeed = Fast;
  }
  else if (TargetPos > CurrentPos) {
    // the target position is higher than the current position, so the elevator needs to start going up
    ElevatorUp(FullSpeed);
    AutoPilotStatus = GoingUp;
    AutoPilotSpeed = Fast;
  }
}

bool ReachedTarget(int CheckPos) {
  // this routine checks to see if the elevator has reached its target
  // to prevent overshooting, hesitation etc, especially with fast moving actuators, the routine will assume that the elevator has reached its target if the position feedback is within a range of the target
  // this range is controlled by the global variable Accuracy.
  // it uses a similar approach for when to slow down the actuator using the global variable ApproachLimit
  // slowing down the actuator as it nears the target allows increased accuracy

  // set the +/- range for both limit and approach
  int LimitLow = CheckPos - Accuracy;
  int LimitHigh = CheckPos + Accuracy;
  int ApproachLow = CheckPos - ApproachLimit;
  int ApproachHigh = CheckPos + ApproachLimit;

  // if the elevator is moving fast, we should check to see if we are nearing the target
  if (AutoPilotSpeed == Fast) {
    if ((TargetPos < ApproachHigh) && (TargetPos > ApproachLow)) {
      // elevator is nearing target, so switch to slow speed and set the actuator control pins accordingly
      // Serial.println("Elevator nearing target, slowing down.");
      AutoPilotSpeed = Slow;
      if (AutoPilotStatus == GoingDown) {
        ElevatorDown(ApproachSpeed);
      }
      else if (AutoPilotStatus == GoingUp) {
        ElevatorUp(ApproachSpeed);
      }
    }
    return false;
  }
  // elevator is moving slow, so we should check to see if we are at target
  else if ((TargetPos < LimitHigh) && (TargetPos > LimitLow)) {
    // Serial.println("Elevator reached target.");
    return true;
  }
  else {
    return false;
  }
}
