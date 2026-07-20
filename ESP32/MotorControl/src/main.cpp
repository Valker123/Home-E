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
    
    if (command == 'M') {
      currentState = STATE_USER_CONTROLLED;
      stopMotors(); 
    } 
    else if (command == 'T') {
      currentState = STATE_AUTONOMOUS;
    }
    else if (command == 'S') {
      stopMotors(); // Force instant stop in any mode
      if (currentState == STATE_AUTONOMOUS) {
        currentState = STATE_USER_CONTROLLED; // Drop back to manual on stop command
      }
    }
    
    if (currentState == STATE_USER_CONTROLLED) {
      switch (command) {
        case 'W': driveForward(150); break;
        case 'A': turnLeft(150);     break;
        case 'S': driveBackward(150); break;
        case 'D': turnRight(150);    break;
        case 'V': stopMotors();      break;
        default:                     break; 
      }
    }
  }

  // Only run autonomous navigation if state is active
  if (currentState == STATE_AUTONOMOUS) {
    bool avoided = ObstacleAvoidance();
    if (!avoided) {
      driveForward(150);
    }
  }
}

// void loop() {
//   // Check if Python sent a message
//   if (Serial.available() > 0){
//     String line = Serial.readStringUntil('\n');
//     line.trim();
    
//     // If "start", turn motors on full speed
//     if (line == "start"){
//       Serial.println("Starting motors...");
//       ledcWrite(leftMotorChannel, 255);
//       ledcWrite(rightMotorChannel, 255);
//     }
//     // If "stop", turn motors off immediately
//     else if (line == "stop"){
//       Serial.println("Stopping motors...");
//       ledcWrite(leftMotorChannel, 0);
//       ledcWrite(rightMotorChannel, 0);
//     }
//     if (line == "autonomous"){
//       Serial.println("Autonomous mode started")
//     }
//   }
// }