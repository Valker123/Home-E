#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ---------------- WiFi / MQTT config ----------------
const char* WIFI_SSID     = "TP-Link_25CF";
const char* WIFI_PASSWORD = "Happylife!505";

const char* MQTT_BROKER   = "192.168.0.249";   // e.g. "192.168.1.50" — the Pi's LAN IP
const int   MQTT_PORT     = 1883;
const char* MQTT_TOPIC    = "home-light-set";
const char* MQTT_CLIENT_ID = "esp32-light-switch";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ---------------- Servo config ----------------
Servo lightSwitchServo;
const int servoSignal = 13;

enum LightState {
  LIGHT_SWITCH_ON,
  LIGHT_SWITCH_OFF
};

LightState currentLightState = LIGHT_SWITCH_OFF;


const int LightSwitchOnAngle  = 500;
const int LightSwitchOffAngle = 1100;

void turnLightOn() {
  lightSwitchServo.writeMicroseconds(LightSwitchOnAngle);
  currentLightState = LIGHT_SWITCH_ON;
  Serial.println("Light: ON");
}

void turnLightOff() {
  lightSwitchServo.writeMicroseconds(LightSwitchOffAngle);
  currentLightState = LIGHT_SWITCH_OFF;
  Serial.println("Light: OFF");
}

// ---------------- WiFi ----------------
void setupWifi() {
  delay(10);
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

// ---------------- MQTT ----------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  message.toLowerCase();

  Serial.print("MQTT message on [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (message == "on") {
    turnLightOn();
  } else if (message == "off") {
    turnLightOff();
  } else {
    Serial.print("Unrecognized payload: ");
    Serial.println(message);
  }
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
      mqttClient.subscribe(MQTT_TOPIC);
      Serial.print("Subscribed to: ");
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  lightSwitchServo.writeMicroseconds(1500);
  lightSwitchServo.attach(servoSignal, 500, 2500);
  delay(1000);

  setupWifi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
}