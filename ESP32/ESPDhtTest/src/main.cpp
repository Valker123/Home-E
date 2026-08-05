#include <DHT.h>

#define DHTPIN 4        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("DHT22 Test");
  dht.begin();
}

void loop() {
  delay(5000); // Wait 5 seconds between readings

  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();       // Celsius
  float tempF = dht.readTemperature(true);   // Fahrenheit

  // Check if any reads failed
  if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(tempC);
  Serial.print("°C / ");
  Serial.print(tempF);
  Serial.println("°F");
}