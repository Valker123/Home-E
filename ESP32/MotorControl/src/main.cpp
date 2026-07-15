#include <Arduino.h>

const int ENA = 25;
const int ENB = 33;
const int IN1 = 13;
const int IN2 = 14;
const int IN3 = 26;
const int IN4 = 27;

const int pwmFreq = 1000;
const int pwmResolution = 8;
const int leftMotorChannel = 0;
const int rightMotorChannel = 1;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(leftMotorChannel, pwmFreq, pwmResolution);
  ledcSetup(rightMotorChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, leftMotorChannel);
  ledcAttachPin(ENB, rightMotorChannel);

  // Set direction to Forward
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void loop() {
  // Check if Python sent a message
  if (Serial.available() > 0){
    String line = Serial.readStringUntil('\n');
    line.trim();
    
    // If "start", turn motors on full speed
    if (line == "start"){
      Serial.println("Starting motors...");
      ledcWrite(leftMotorChannel, 255);
      ledcWrite(rightMotorChannel, 255);
    }
    // If "stop", turn motors off immediately
    else if (line == "stop"){
      Serial.println("Stopping motors...");
      ledcWrite(leftMotorChannel, 0);
      ledcWrite(rightMotorChannel, 0);
    }
  }
}