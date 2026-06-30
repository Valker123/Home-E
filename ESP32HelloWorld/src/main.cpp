#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(921600);
  Serial.println("Bruh");
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Hello, my name is Vincent");
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(5000);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}