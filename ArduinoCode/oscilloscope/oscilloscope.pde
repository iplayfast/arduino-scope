/*
  Analog input, serial output
 
 Reads an analog input pin, prints the results to the serial monitor.
 
 The circuit:

 * potentiometer connected to analog pin 0.
   Center pin of the potentiometer goes to the analog pin.
   side pins of the potentiometer go to +5V and ground
 
 created over and over again
 by Tom Igoe and everyone who's ever used Arduino
 
 */
 
 void setup() {
  Serial.begin(115200); 
 }
 
 void loop() {
  // read the analog input into a variable:
  unsigned long time = millis();
   int analogValue1 = analogRead(0);
   int analogValue2 = analogRead(1);
   // print the result:
   Serial.print(time);
   Serial.print(" ");
   Serial.print(analogValue1);
   Serial.print(" ");
   Serial.println(analogValue2);
   // wait 10 milliseconds for the analog-to-digital converter
   // to settle after the last reading:
   //if (time & 0xff000 ==0)  delay(10);
 }
