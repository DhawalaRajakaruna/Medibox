#include arduino.h
#include <Wire.h>

#define BUZZER 18

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  digitalWrite(BUZZER, 1000);
  delay(1000);
  noTone(BUZZER);
  delay(1000);
}