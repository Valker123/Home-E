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
int counter = 0;

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

void startMotors(){
  Serial.printf("starting motors\n");

  ledcWrite(leftMotorChannel, 128);
  ledcWrite(rightMotorChannel, 128);
  delay(3000);

  // Full speed
  ledcWrite(leftMotorChannel, 255);
  ledcWrite(rightMotorChannel, 255);
  delay(3000);
}

void stopMotors(){
  Serial.printf("stopping motors\n");
  ledcWrite(leftMotorChannel, 0);
  ledcWrite(rightMotorChannel, 0);
  delay(2000);
}

void loop() {
  // Serial.println("Starting motor routine... , counter: " + String(counter));  
  // counter += 1;
  // delay(1000);
 // motorRoutine();
  if (Serial.available()){
    String line = Serial.readStringUntil('\n');
    line.trim();
    Serial.printf("received: %s\n", line);
    if (line == "start"){
      startMotors();
    }
    else if (line == "stop"){
      stopMotors();
    }
  }
}

