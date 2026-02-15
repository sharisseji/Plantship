#include <Arduino.h>

// put function declarations here:

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  delay(2000);
  Serial.println("Booted!");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Moisture Sensor Value:");
  Serial.println(analogRead(15));
  delay(1000); 
}



// #include <Arduino.h>

// void setup() {
//   USBSerial.begin(115200);
//   delay(2000);
//   USBSerial.println("USBSerial OK");
// }

// void loop() {
//   USBSerial.println("tick");
//   delay(500);
// }
