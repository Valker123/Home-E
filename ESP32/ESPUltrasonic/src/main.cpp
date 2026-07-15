#include <Arduino.h>

#define TRIG_PIN 32
#define ECHO_PIN 34

float getDistance() {
  // Make sure trigger starts LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10 microsecond trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Wait for echo (timeout after 30ms)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo received
  if (duration == 0) {
    return -1;
  }

  // Calculate distance in cm
  float distance = duration * 0.0343 / 2;

  // Reject impossible values
  if (distance < 1 || distance > 400) {
    return -1;
  }

  return distance;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("HC-SR04 Test Starting...");
}

void loop() {
  float total = 0;
  int readings = 0;

  // Take 5 readings and average them
  for (int i = 0; i < 5; i++) {
    float distance = getDistance();

    if (distance != -1) {
      total += distance;
      readings++;
    }

    delay(50);
  }

  if (readings > 0) {
    float averageDistance = total / readings;

    Serial.print("Distance: ");
    Serial.print(averageDistance);
    Serial.println(" cm");
  } 
  else {
    Serial.println("No valid reading");
  }

  delay(300);
}