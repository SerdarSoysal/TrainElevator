# TrainElevator
Arduino code for controlling a position feedback linear actuator used for a train elevator on my model railroad.

Big thanks to code samples at arduino.cc, Firgelli Automations, Progressive Automations, and Shift Automation which were all super helpful for me learning how to control the actuator the way I needed.
 
This code uses an Arduino Uno board to control a position feedback linear actuator using various tactile switches for control. There is also a Progressive Automations MegaMoto Plus board to drive the actuator. The goal is to use this as a control mechanism for a train elevator.

This implementation uses two levels: main and staging. It is possible to extend the code to use more levels by adding the necessary switches and controls. The implementation has the following details:
 - Preset pushbuttons for both levels
 - Ability to move the elevator manually using a momentary SPDT switch
 - Ability to program either preset pushbutton to the current location of the actuator
 - The preset values are stored in the EEPROM memory to avoid losing them each time the board is turned off
  
 Potential improvements in the future (just ideas right now - nothing planned):
 - Add a potentiometer input for controlling the speed of the actuator
 - Add digital display output for elevator levels
 
 ## Version history
- 0.0 - Enables the MegaMoto board and sets the motor pins to inactive to avoid immediate movement of actuator
- 0.1 - Enables the manual up/down control button
- 0.2 - Adds the ElevatorUp, ElevatorDown, and ElevatorStop functions
- 0.3 - Add reading input from actuator
- 0.4 - Add support for preset buttons
- 0.5 - Add slowing down the elevator as it nears target to improve accuracy
- 0.6 - Add programming support
- 0.7 - Add EEPROM support
- 0.8 - Commented out the debugging messages and added more comments

 
