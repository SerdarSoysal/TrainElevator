// Version 0.4
// Version history
// 0.0 - Enables the MegaMoto board and sets the motor pins to inactive to avoid immediate movement of actuator
// 0.1 - Enables the manual up/down control button
// 0.2 - Adds the ElevatorUp, ElevatorDown, and ElevatorStop functions
// 0.3 - Add reading input from actuator
// 0.4 - Add support for preset buttons

// Add button library
#include <Button.h>

// Use the jumpers on the board to select which pins will be used
int EnablePin = 8; // use pin 8 to enable board
int PWMPinA1 = 3;  // motor pin A for actuator
int PWMPinB1 = 11; // motor pin B for actuator

// Declare our buttons and specify the correponding pins
Button ManualUp(9);      // use pin 9 to move elevator up manually
Button ManualDown(10);   // use pin 10 to move elevator down manually
Button PresetMain(6);    // use pin 6 to move elevator to main level
Button PresetStaging(7); // use pin 7 to move elevator to staging level

// Specify our input pin for actuator position
int SensorPin = A3;

// Set other globally used variables
int delaytime = 5*1000; // 5 seconds times 1,000 to convert to milliseconds
int MainLevel = 500;    // will store/read these values from EEPROM for production
int StagingLevel = 300; // will store/read these values from EEPROM for production
int CurrentPos = 0;   // use to read the current position of actuator
int TargetPos = 0;    // use to set the target position of actuator
int Accuracy = 10;    // use to make sure we stop the actuator within the ballpark of target
bool AutoPilot = false; // if true, we are on autopilot

void setup() {
  // put your setup code here, to run once:

  // initialize our buttons
  ManualUp.begin();
  ManualDown.begin();
  PresetMain.begin();
  PresetStaging.begin();
  
  // initialize serial port
  Serial.begin(9600);

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
  }
  if (PresetMain.pressed()) {
    AutoPilot = true;
    TargetPos=MainLevel;
    GoToTarget();
  }
  if (PresetStaging.pressed()) {
    AutoPilot = true;
    TargetPos=StagingLevel;
    GoToTarget();
  }
  if (ManualUp.pressed()) {
    AutoPilot = false;
    ElevatorUp();
  }
  if (ManualDown.pressed()) {
    AutoPilot = false;
    ElevatorDown();
  }
  if (ManualUp.released() || ManualDown.released()) {
    ElevatorStop();
  } 
}

void ElevatorUp() {
  // Activate pin A and deactivate pin B  
  analogWrite(PWMPinA1, 255);
  analogWrite(PWMPinB1, 0); // speed 0-255

  // Print starting point
  Serial.print("Moving elevator up from position: ");
  Serial.println(ReadPos());
}

void ElevatorDown() {
  // Activate pin B and deactivate pin A  
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, 255); // speed 0-255

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
    
  if (ReachedTarget(CurrentPos)) {
    Serial.println("Elevator already at target.");
  }
  else if (TargetPos < CurrentPos) {
     ElevatorDown();
  }
  else if (TargetPos > CurrentPos) {
    ElevatorUp();
  }
}

bool ReachedTarget(int CheckPos) {
  int LimitLow = CheckPos - Accuracy;
  int LimitHigh = CheckPos + Accuracy;

  if ((TargetPos < LimitHigh) && (TargetPos > LimitLow)) {
    Serial.println("Elevator reached target.");
    return true;
  }
  else {
    return false;
  }
}
