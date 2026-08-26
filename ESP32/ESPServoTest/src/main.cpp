#include <Arduino.h>
#include <ESP32Servo.h>

Servo lightSwitchServo;
const int servoSignal = 13;

// Starting guesses — adjust these as you find the right values
int LightSwitchOnAngle  = 500;
int LightSwitchOffAngle = 1500;

enum LightState {
  LIGHT_SWITCH_ON,
  LIGHT_SWITCH_OFF
};

LightState currentLightState = LIGHT_SWITCH_OFF;

void turnLightOn() {
  lightSwitchServo.writeMicroseconds(LightSwitchOnAngle);
  currentLightState = LIGHT_SWITCH_ON;
  Serial.print("Light: ON (");
  Serial.print(LightSwitchOnAngle);
  Serial.println("us)");
}

void turnLightOff() {
  lightSwitchServo.writeMicroseconds(LightSwitchOffAngle);
  currentLightState = LIGHT_SWITCH_OFF;
  Serial.print("Light: OFF (");
  Serial.print(LightSwitchOffAngle);
  Serial.println("us)");
}

void printHelp() {
  Serial.println("---------------------------------------------");
  Serial.println("Commands:");
  Serial.println("  on          -> move to current ON angle");
  Serial.println("  off         -> move to current OFF angle");
  Serial.println("  <number>    -> jog servo directly to that microsecond value (e.g. 1200)");
  Serial.println("  seton       -> save last jogged value as the ON angle");
  Serial.println("  setoff      -> save last jogged value as the OFF angle");
  Serial.println("  help        -> show this again");
  Serial.println("---------------------------------------------");
}

int lastJoggedValue = 1500;

void setup() {
  Serial.begin(115200);
  delay(500);

  ESP32PWM::allocateTimer(0);

  lightSwitchServo.writeMicroseconds(1500);
  lightSwitchServo.attach(servoSignal, 300, 2500);

  delay(1000);

  Serial.println("Servo test ready.");
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();

    if (input.length() == 0) {
      return;
    }

    if (input == "on") {
      turnLightOn();
    } else if (input == "off") {
      turnLightOff();
    } else if (input == "help") {
      printHelp();
    } else if (input == "seton") {
      LightSwitchOnAngle = lastJoggedValue;
      Serial.print("Saved ON angle = ");
      Serial.println(LightSwitchOnAngle);
    } else if (input == "setoff") {
      LightSwitchOffAngle = lastJoggedValue;
      Serial.print("Saved OFF angle = ");
      Serial.println(LightSwitchOffAngle);
    } else {
      // Try to parse it as a raw microsecond value
      bool isNumber = true;
      for (unsigned int i = 0; i < input.length(); i++) {
        if (!isDigit(input[i])) {
          isNumber = false;
          break;
        }
      }

      if (isNumber) {
        int value = input.toInt();
        if (value >= 500 && value <= 2500) {
          lastJoggedValue = value;
          lightSwitchServo.writeMicroseconds(value);
          Serial.print("Jogged servo to ");
          Serial.print(value);
          Serial.println("us");
        } else {
          Serial.println("Value out of range (500-2500). Ignored for safety.");
        }
      } else {
        Serial.println("Unrecognized command. Type 'help' for options.");
      }
    }
  }
}