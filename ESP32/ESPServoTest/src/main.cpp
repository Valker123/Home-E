#include <Arduino.h>
#include <ESP32Servo.h>

Servo lightSwitchServo;

const int servoSignal = 13;
enum LightState {
  LIGHT_SWITCH_ON
  LIGHT_SWITCH_OFF
}

LightState currentLightState = LIGHT_SWITCH_OFF

const int LightSwitchOnAngle = 500
const int LightSwitchOffAngle = 2500

void setup() {
  ESP32PWM::allocateTimer(0);
  
  lightSwitchServo.writeMicroseconds(500);
  lightSwitchServo.attach(servoSignal, 500, 2500);
  
  delay(1000);
}

void turnLightOn {
  lightSwitchServo.writeMicroseconds(LightSwitchOnAngle);
  currentLightState = LIGHT_SWITCH_ON;
  Serial.println("Light: ON")
}

void turnLightOff {
  lightSwitchServo.writeMicroseconds(LightSwitchOffAngle);
  currentLightState = LIGHT_SWITCH_OFF;
  Serial.println("Light: OFF")
}



void loop() {
  for (int pos = 500; pos <= 2500; pos += 20) {
    lightSwitchServo.writeMicroseconds(pos);
    delay(20);
  }

  delay(500);

  for (int pos = 2500; pos >= 500; pos -= 20) {
    lightSwitchServo.writeMicroseconds(pos);
    delay(20);
  }

  delay(500);
}