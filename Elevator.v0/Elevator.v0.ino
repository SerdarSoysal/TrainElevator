//Use the jumpers on the board to select which pins will be used
int EnablePin = 8;
int PWMPinA1 = 11; // motor pin A for actuator
int PWMPinB1 = 3;  // motor pin B for actuator
int delaytime = 5*1000; // 5 seconds times 1,000 to convert to milliseconds

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(EnablePin, OUTPUT);//Enable the board
  pinMode(PWMPinA1, OUTPUT);
  pinMode(PWMPinB1, OUTPUT);//Set motor outputs
  // make sure we don't move the actuator upon start
  analogWrite(PWMPinA1, 0);
  analogWrite(PWMPinB1, 0);
  // enable the megamoto shield
  Serial.println("We are now about to enable the board");
  delay(delaytime);
  digitalWrite(EnablePin, HIGH);
  Serial.println("MegaMoto shield is now enabled");
}

void loop() {
  // put your main code here, to run repeatedly:

}
