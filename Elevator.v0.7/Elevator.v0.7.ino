// Version 0.7
// Version history
// 0.0 - Enables the MegaMoto board and sets the motor pins to inactive to avoid immediate movement of actuator
// 0.1 - Enables the manual up/down control button
// 0.2 - Adds the ElevatorUp, ElevatorDown, and ElevatorStop functions
// 0.3 - Add reading input from actuator
// 0.4 - Add support for preset buttons
// 0.5 - Add slowing down the elevator as it nears target to improve accuracy
// 0.6 - Add programming support
// 0.7 - Add EEPROM support

// Add button library
#include <Button.h>
#include <EEPROM.h>

// Use the jumpers on the board to select which pins will be used
int EnablePin = 8; // use pin 8 to enable board
int PWMPinA1 = 3;  // motor pin A for actuator
int PWMPinB1 = 11; // motor pin B for actuator

// Declare our buttons and specify the correponding pins
Button ManualUp(9);      // use pin 9 to move elevator up manually
Button ManualDown(10);   // use pin 10 to move elevator down manually
Button PresetMain(6);    // use pin 6 to move elevator to main level
Button PresetStaging(7); // use pin 7 to move elevator to staging level
Button Program(12);      // use pin 12 for programming switch

// Specify our input pin for actuator position
int SensorPin = A3;

// Set other globally used variables
int delaytime = 5*1000; // 5 seconds times 1,000 to convert to milliseconds
int MainLevel;    // will store/read these values from EEPROM for production
int StagingLevel; // will store/read these values from EEPROM for production
int CurrentPos = 0;   // use to read the current position of actuator
int TargetPos = 0;    // use to set the target position of actuator
int Accuracy = 1;    // stopping accuracy of the actuator - must be less than ApproachLimit
int ApproachLimit = 20;   // defines the vicinity of target where the actuator will slow down - must be greater than Accuracy
bool Programming = false; // if true, elevator is in programming mode
bool AutoPilot = false;   // if true, elevator is on autopilot
const int Stopped = 0;
const int GoingUp = 1;
const int GoingDown = 2;
const int Fast = 3;
const int Slow = 4;
int AutoPilotStatus = Stopped; 
int AutoPilotSpeed = Fast;
int FullSpeed = 255;    // full elevator speed
int ApproachSpeed = 60; // slow elevator speed
const int MainAddress = 0;    // EEPROM address for storing the main level
const int StagingAddress = 5; // EEPROM address for storing the staging level

void setup() {
  // put your setup code here, to run once:

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
  Serial.println("Read the following values for main and staging levels:");
  Serial.println(MainLevel);
  Serial.println(StagingLevel);

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
  Serial.println("Enabling MegaMoto shield");
  // delay(delaytime);
  digitalWrite(EnablePin, HIGH);
  Serial.println("MegaMoto shield is now enabled");

}

void loop() {

  CurrentPos = ReadPos();

  if ((AutoPilot) && (ReachedTarget(CurrentPos))) {
      ElevatorStop();
      AutoPilot = false;
      AutoPilotStatus = Stopped;
  }
  if (PresetMain.pressed()) {
    if (Programming) {
      MainLevel = ReadPos();
      EEPROM.put(MainAddress,MainLevel);    
      Serial.print("Main level set to: ");
      Serial.println(MainLevel);
    }
    else {   
      AutoPilot = true;
      TargetPos=MainLevel;
      GoToTarget();      
    }
  }
  if (PresetStaging.pressed()) {
    if (Programming) {
      StagingLevel = ReadPos();
      EEPROM.put(StagingAddress,StagingLevel);     
      Serial.print("Staging level set to: ");
      Serial.println(StagingLevel);
    }
    else {
       AutoPilot = true;
       TargetPos=StagingLevel;
       GoToTarget();
    }
  }
  if (ManualUp.pressed()) {
    AutoPilot = false;
    ElevatorUp(FullSpeed);
  }
  if (ManualDown.pressed()) {
    AutoPilot = false;
    ElevatorDown(FullSpeed);
  }
  if (ManualUp.released() || ManualDown.released()) {
    ElevatorStop();
  } 
  if (Program.pressed()) {
    Programming = true;
    Serial.println("Entering programming mode.");
  }
  if (Program.released()) {
    Programming = false;
    Serial.println("Exiting programming mode.");
  }
}

void ElevatorUp(int Speed) {
  // Activate pin A and deactivate pin B  
  analogWrite(PWMPinA1, Speed);
  analogWrite(PWMPinB1, 0); // speed 0-255

  // Print starting point
  Serial.print("Moving elevator up from position: ");
  Serial.println(ReadPos());
}

void ElevatorDown(int Speed) {
  // Activate pin B and deactivate pin A  
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, Speed); // speed 0-255

  // Print starting point
  Serial.print("Moving elevator down from position: ");
  Serial.println(ReadPos());
}

void ElevatorStop() {
  // Deactivate both pins  
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, 0); // speed 0-255

  // Print stopping point
  Serial.print("Elevator stopped at position: ");
  Serial.println(ReadPos());
}

int ReadPos() {
  return analogRead(SensorPin);
}

void GoToTarget() {

  AutoPilot = true;  
  if (ReachedTarget(CurrentPos)) {
    Serial.println("Elevator already at target.");
    AutoPilot = false;
  }
  else if (TargetPos < CurrentPos) {
    ElevatorDown(FullSpeed);
    AutoPilotStatus = GoingDown;
    AutoPilotSpeed = Fast;
  }
  else if (TargetPos > CurrentPos) {
    ElevatorUp(FullSpeed);
    AutoPilotStatus = GoingUp;
    AutoPilotSpeed = Fast;
  }
}

bool ReachedTarget(int CheckPos) {
  int LimitLow = CheckPos - Accuracy;
  int LimitHigh = CheckPos + Accuracy;
  int ApproachLow = CheckPos - ApproachLimit;
  int ApproachHigh = CheckPos + ApproachLimit;

  if (AutoPilotSpeed == Fast) {
    if ((TargetPos < ApproachHigh) && (TargetPos > ApproachLow)) {
      Serial.println("Elevator nearing target, slowing down.");
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
  else if ((TargetPos < LimitHigh) && (TargetPos > LimitLow)) {
    Serial.println("Elevator reached target.");
    return true;
  }
  else {
    return false;
  }
}
