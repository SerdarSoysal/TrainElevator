# TrainElevator
Arduino code for controlling a position feedback linear actuator used for a train elevator on my model railroad.

Big thanks to code samples at arduino.cc, Firgelli Automations, Progressive Automations, and Shift Automation which were all super helpful for me learning how to control the actuator the way I needed.
 
This code uses an Arduino Uno board to control a position feedback linear actuator using various tactile switches for control. The goal is to use this as a control mechanism for a train elevator.

This implementation uses two levels: main and staging. It is possible to extend the code to use more levels by adding the necessary switches and controls. The implementation has the following details:
 - Preset pushbuttons for both levels
 - Ability to move the elevator manually using a momentary SPDT switch
 - Ability to program either preset pushbutton to the current location of the actuator
 - The preset values are stored in the EEPROM memory to avoid losing them each time the board is turned off
  
 Potential improvements in the future (just ideas right now - nothing planned):
 - Add a potentiometer input for controlling the speed of the actuator
 - Add digital display output for elevator levels
 
 
