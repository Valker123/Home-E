#include <Arduino.h>
#include <WiFiMulti.h>

#define WIFI_SSID "TP-Link_25CF"
#define WIFI_PASSWORD "Happylife!505"

WiFiMulti wifiMulti;
;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(921600);
  pinMode(LED_BUILTIN, OUTPUT);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, WiFi.status() == WL_CONNECTED);
}
