#include <Arduino.h>
#include "motors.h"
#include "ultrasonic.h"

enum RobotState {
  STATE_USER_CONTROLLED,
  STATE_AUTONOMOUS
};

RobotState currentState = STATE_USER_CONTROLLED;

void setup() {
  setupMotors();
  setupUltrasonic();
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    if (command == '\n' || command == '\r') return;

    if (command == 'M') {
      currentState = STATE_USER_CONTROLLED;
      stopMotors(); 
      return;
    } 
    else if (command == 'T') {
      currentState = STATE_AUTONOMOUS;
      return;
    }
    else if (command == 'V') { // 'V' is Stop & Reset to Manual
      stopMotors();
      currentState = STATE_USER_CONTROLLED;
      return;
    }

    if (currentState == STATE_USER_CONTROLLED) {
      switch (command) {
        case 'W': driveForward(150);  break;
        case 'A': turnLeft(150);      break;
        case 'S': driveBackward(150); break; // Now works without triggering state reset!
        case 'D': turnRight(150);     break;
        default:                      break; 
      }
      return
    }
  }

  // --- AUTONOMOUS MODE ---
  if (currentState == STATE_AUTONOMOUS) {
    bool avoided = ObstacleAvoidance();
    if (!avoided) {
      driveForward(150);
    }
    delay(30); // Gives the ESP32 CPU time to listen for new serial commands
  }
}