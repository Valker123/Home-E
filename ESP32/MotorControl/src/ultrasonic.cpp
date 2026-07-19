#include <Arduino.h>
#include "ultrasonic.h"
#include "motors.h"

void setupUltrasonic() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    randomSeed(analogRead(34));
}

long readDistanceCm() {
  float total = 0;
  int readings = 0;

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); 

    if (duration > 0) {
      float distance = (duration * 0.0343) / 2;
      
      if (distance >= 1.0 && distance <= 400.0) {
        total += distance;
        readings++;
      }
    }
    delay(50); 
  }

  if (readings == 0) {
    return -1; 
  }

  return (long)(total / readings);
  Serial.println("Distance: " + String(total / readings) + " cm");
}

bool ObstacleAvoidance() {
    long distance = readDistanceCm();

    if (distance <= WALL_THRESHOLD_CM && distance > 0) {
        stopMotors();
        delay(500);

        int choice = random (0, 2);

        if (choice == 0){
            turnLeft(200);
            delay(1000);
        }
        else {
            turnRight(200);
            delay(1000);
        }
        delay(500);
        stopMotors();
        return true;
    }
    return false;
}
