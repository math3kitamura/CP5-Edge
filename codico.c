/* 
------------------ FIAP --------------------
EDGE COMPUTING & COMPUTER SYSTEMS
Participantes:
Prof. Paulo Marcotti PF2150
Matheus Kitamura Gurther RM563205
Matheus Da Costa Barroso RM561308
Victor Oliveira Alves RM565723
--------------------------------------------
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 15
#define DHTTYPE DHT22
#define LDR_PIN 36
#define TRIG_PIN 5
#define ECHO_PIN 18

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ⚠️ Orion rodando no Docker local
const char* serverName = "https://unosmotic-shawana-huffy.ngrok-free.dev/v2/entities/lamp010/attrs";


void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println(" conectado!");
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.034 / 2.0;
}

void loop() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  int ldrValue = analogRead(LDR_PIN);
  float distance = getDistance();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Leitura DHT inválida.");
    delay(2000);
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String json = "{";
    json += "\"temperature\": {\"value\": " + String(temp, 2) + ", \"type\": \"Number\"},";
    json += "\"humidity\": {\"value\": " + String(hum, 2) + ", \"type\": \"Number\"},";
    json += "\"luminosity\": {\"value\": " + String(ldrValue) + ", \"type\": \"Number\"},";
    json += "\"distance\": {\"value\": " + String(distance, 2) + ", \"type\": \"Number\"}";
    json += "}";

    int code = http.POST(json);
    Serial.print("POST /attrs -> HTTP ");
    Serial.println(code);
    http.end();
  } else {
    Serial.println("WiFi desconectado.");
  }

  delay(5000);
}