#include <Arduino.h>
#include "motors.h"

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

void setupMotors(){ 
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    ledcSetup(leftMotorChannel, pwmFreq, pwmResolution);
    ledcSetup(rightMotorChannel, pwmFreq, pwmResolution);
    ledcAttachPin(ENA, leftMotorChannel);
    ledcAttachPin(ENB, rightMotorChannel);

    stopMotors();
}

void setMotorSpeed(int leftSpeed, int rightSpeed) {
    ledcWrite(leftMotorChannel, leftSpeed);
    ledcWrite(rightMotorChannel, rightSpeed);
}

void stopMotors() {
    setMotorSpeed(0,0); 
}

void driveForward(int speed) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    setMotorSpeed(speed, speed);
}

void driveBackward(int speed) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    setMotorSpeed(speed, speed);
}

void turnLeft(int speed) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    setMotorSpeed(speed, speed);
}

void turnRight(int speed) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    setMotorSpeed(speed, speed);
}