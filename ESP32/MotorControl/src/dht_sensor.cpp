#include <Arduino.h>
#include <DHT.h>
#include "dht_sensor.h"

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void setupDHT() {
    dht.begin();
}

float readTemperatureF() {
    float tempF = dht.readTemperature(true);  // true = Fahrenheit
    return tempF;  // returns NAN if read failed
}

float readHumidity() {
    float hum = dht.readHumidity();
    return hum;  // returns NAN if read failed
}

