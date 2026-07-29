#include <Arduino.h>
#include "motors.h"
#include "ultrasonic.h"
#include "dht_sensor.h"


enum RobotState {
  STATE_USER_CONTROLLED,
  STATE_AUTONOMOUS
};

RobotState currentState = STATE_USER_CONTROLLED;

void setup() {
  setupMotors();
  setupUltrasonic();
  setupDHT();
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
    else if (command == 'R') { 
      float tempF = readTemperatureF();
      float hum = readHumidity();
      if (isnan(tempF) || isnan(hum)) {
        Serial.println("TEMP:ERROR");
      } else {
        Serial.print("TEMP:");
        Serial.print(tempF);
        Serial.print(",HUM:");
        Serial.println(hum);
      }
      return;
    }

    if (currentState == STATE_USER_CONTROLLED) {
      switch (command) {
        case 'W': driveForward(150);  break;
        case 'A': turnLeft(150);      break;
        case 'S': driveBackward(150); break; 
        case 'D': turnRight(150);     break;
        default:                      break; 
      }
      return;
    }
  }

  if (currentState == STATE_AUTONOMOUS) {
  long distance = readDistanceCm();
  

  bool avoided = ObstacleAvoidance();
  if (!avoided) {
    driveForward(150);
  }
  delay(100); // 100ms delay keeps serial clean and readable
}
}