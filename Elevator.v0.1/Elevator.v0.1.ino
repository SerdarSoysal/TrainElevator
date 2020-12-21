// Version 0.1
// Version history
// 0.0 - Enables the MegaMoto board and sets the motor pins to inactive to avoid immediate movement of actuator
// 0.1 - Enables the manual up/down control button

// Add button library
#include <Button.h>

// Use the jumpers on the board to select which pins will be used
int EnablePin = 8; // use pin 8 to enable board
int PWMPinA1 = 11; // motor pin A for actuator
int PWMPinB1 = 3;  // motor pin B for actuator

// Declare our buttons and specify the correponding pins
Button ManualUp(9);    // use pin 9 to move elevator up manually
Button ManualDown(10); // use pin 10 to move elevator down manually

// Set other globally used variables
int delaytime = 5*1000; // 5 seconds times 1,000 to convert to milliseconds

void setup() {
  // put your setup code here, to run once:

  // initialize our buttons
  ManualUp.begin();
  ManualDown.begin();
  
  // initialize serial port
  Serial.begin(9600);

  // initialize MegaMoto board
  pinMode(EnablePin, OUTPUT); //Enable the board
  pinMode(PWMPinA1, OUTPUT);
  pinMode(PWMPinB1, OUTPUT);  //Set motor outputs

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
  // put your main code here, to run repeatedly:

  if (ManualUp.pressed()) {
    Serial.println("Moving elevator up manually");
  }
  if (ManualDown.pressed()) {
    Serial.println("Moving elevator down manually");
  }
}
