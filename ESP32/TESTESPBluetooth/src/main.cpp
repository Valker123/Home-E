#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "TP-Link_25CF";
const char* password = "Happylife!505";

const char* mqtt_broker = "192.168.1.XXX";  // <-- your Pi's IP address
const int mqtt_port = 1883;

const char* topic_set = "home/light/set";     // Pi -> ESP32 (commands)
const char* topic_state = "home/light/state"; // ESP32 -> Pi (status updates)

WiFiClient espClient;
PubSubClient client(espClient);

void setLight(bool on) {
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
  client.publish(topic_state, on ? "on" : "off");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Received on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (message == "on") {
    setLight(true);
  } else if (message == "off") {
    setLight(false);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (client.connect("ESP32LightClient")) {
      Serial.println("MQTT connected");
      client.subscribe(topic_set);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
}