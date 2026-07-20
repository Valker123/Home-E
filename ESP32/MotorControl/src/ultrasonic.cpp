#include <Arduino.h>
#include "ultrasonic.h"
#include "motors.h"

#define TRIG_PIN 32
#define ECHO_PIN 34

void setupUltrasonic() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  randomSeed(analogRead(0)); 
}

// Single function that handles triggering, timing, and 5-sample averaging
float readDistanceCm() {
  float total = 0;
  int readings = 0;

  for (int i = 0; i < 5; i++) {
    // 1. Send trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // 2. Measure echo duration
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    // 3. Convert and validate reading
    if (duration > 0) {
      float distance = duration * 0.0343 / 2.0;
      if (distance >= 1.0 && distance <= 400.0) {
        total += distance;
        readings++;
      }
    }

    delay(50);
  }

  return (readings > 0) ? (total / readings) : -1.0;
}

bool ObstacleAvoidance() {
  float avgDistance = readDistanceCm();

  if (avgDistance != -1.0) {
    Serial.print("Distance: ");
    Serial.print(avgDistance);
    Serial.println(" cm");
  } else {
    Serial.println("No valid reading");
  }

  // Obstacle detection (< 20 cm)
  if (avgDistance > 0 && avgDistance < 20.0) {
    stopMotors();
    delay(100);

    // Random turn: 0 = Left, 1 = Right
    if (random(0, 2) == 0) {
      Serial.println("Obstacle! Turning LEFT...");
      turnLeft(300);
    } else {
      Serial.println("Obstacle! Turning RIGHT...");
      turnRight(300);
    }

    delay(400);
    stopMotors();
    return true;
  }

  return false;
}