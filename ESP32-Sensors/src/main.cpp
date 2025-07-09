#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_CCS811.h>
#include <WebServer.h>
#define GREEN_LED_PIN  42
#define RED_LED_PIN    41



// ---------------------- CONFIG ----------------------

const char* ssid = "HotSpot";
const char* password = "soufixe123";
const char* serverUrl = "http://192.168.137.248:8080/buffet-food-quality-analyzer-01/properties/sensorDataReceived";
const char* hostname = "esp32-sensores";

// ---------------------- SENSORS ----------------------
#define DHT_PIN 39
#define DHT_TYPE DHT22
#define PWM_PIN 15
#define SDA_PIN 21
#define SCL_PIN 10

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_CCS811 ccs;
WebServer server(80);  // HTTP server na porta 80

float lastTemperature = -1;
float lastHumidity = -1;
int lastCO2 = -1;
int lastTVOC = -1;

// ---------------------- TIMER ----------------------
unsigned long lastSendTime = 0;
const unsigned long interval = 10000; // 10 seconds

// ---------------------- OTA ----------------------
void setupOTA() {
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() { Serial.println("Start OTA"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd OTA"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });
  ArduinoOTA.begin();
}
//-------------------Webserver---------------------
void handleSensorData() {
  String json = "{";
  json += "\"temperature\":" + String(lastTemperature, 2) + ",";
  json += "\"humidity\":" + String(lastHumidity, 2) + ",";
  json += "\"co2\":" + String(lastCO2) + ",";
  json += "\"tvoc\":" + String(lastTVOC);
  json += "}";
  
  server.send(200, "application/json", json);
}
void handleRedOn() {
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  server.send(200, "application/json", "{\"status\":\"red_on\"}");
}

void handleGreenOn() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  server.send(200, "application/json", "{\"status\":\"green_on\"}");
}

// ---------------------- INIT ----------------------
void setup() {
  Serial.begin(115200);
pinMode(GREEN_LED_PIN, OUTPUT);
pinMode(RED_LED_PIN, OUTPUT);

// Por exemplo: começa com verde ligado
digitalWrite(GREEN_LED_PIN, HIGH);
digitalWrite(RED_LED_PIN, LOW);
// ⚠️ Configurar IP fixo
IPAddress local_IP(192, 168, 137, 100);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

WiFi.config(local_IP, gateway, subnet, dns);

WiFi.begin(ssid, password);
Serial.print("Ligando ao WiFi");
while (WiFi.status() != WL_CONNECTED) {
  Serial.print(".");
  delay(500);
}
Serial.println("\n✅ WiFi conectado. IP: " + WiFi.localIP().toString());


  // 2. Inicializar os sensores
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  dht.begin();
  if (!ccs.begin()) {
    Serial.println("❌ Falha ao iniciar CCS811. Reinicia o ESP.");
    while (true) delay(1000);
  }
  while (!ccs.available()) delay(100);

  // ✅ 3. Iniciar o servidor HTTP DEPOIS do Wi-Fi estar OK
  server.on("/api/last-reading", handleSensorData);
  server.on("/api/toggle-on", HTTP_POST, handleRedOn);
  server.on("/api/toggle-off", HTTP_POST, handleGreenOn);

  server.begin();
  Serial.println("🌐 Servidor HTTP ativo na porta 80");

  // 4. OTA
  setupOTA();
}

// ---------------------- SENSORS ----------------------
float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("⚠️ Falha ao ler temperatura");
    return -1;
  }
  return t;
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("⚠️ Falha ao ler humidade");
    return -1;
  }
  return h;
}

void updateSensorValues() {
  float temp = readDHTTemperature();
  float hum = readDHTHumidity();
  float tvoc = -1;
  float co2 = -1;

  if (temp != -1 && hum != -1) {
    ccs.setEnvironmentalData(hum, temp);
  }

  if (ccs.available()) {
    if (!ccs.readData()) {
      tvoc = ccs.getTVOC();
      co2 = ccs.geteCO2();
    } else {
      Serial.println("⚠️ Erro ao ler CCS811");
    }
  } else {
    Serial.println("⚠️ CCS811 não disponível. Reiniciando sensor...");
    ccs.begin();
  }

  // Atualiza sempre os valores, mesmo que inválidos
  lastTemperature = temp;
  lastHumidity = hum;
  lastCO2 = co2;
  lastTVOC = tvoc;
}

// ---------------------- HTTP SEND ----------------------

void sendHttpSensorData(float temperature, float humidity, int co2, int tvoc) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi não conectado. Abortando envio.");
    return;
  }

  HTTPClient http;

  http.begin(serverUrl);  // Define o endpoint
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"sensorId\":\"wot:dev:buffet-food-quality-analyzer-01\",";
  payload += "\"temperature\":" + String(temperature, 2) + ",";
  payload += "\"humidity\":" + String(humidity, 2) + ",";
  payload += "\"co2\":" + String(co2) + ",";
  payload += "\"tvoc\":" + String(tvoc);
  payload += "}";

  int httpResponseCode = http.PUT(payload);

  if (httpResponseCode > 0) {
    Serial.print("✅ HTTP PUT enviado. Código: ");
    //Serial.println(httpResponseCode);
    //Serial.println("Resposta: " + http.getString());
  } else {
    Serial.print("❌ Erro no envio HTTP. Código: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}


// ---------------------- LOOP ----------------------

void loop() {
  // Atualiza OTA e responde a pedidos HTTP
  ArduinoOTA.handle();
  server.handleClient();

  // Envio periódico de dados
  if (millis() - lastSendTime >= interval) {
    lastSendTime = millis();

    float temp = readDHTTemperature();
    float hum = readDHTHumidity();
    float tvoc = -1;
    float co2 = -1;

    // Atualiza os dados ambientais no CCS811
    if (temp != -1 && hum != -1) {
      ccs.setEnvironmentalData(hum, temp);
    }

    // Lê os dados do sensor CCS811
    if (ccs.available()) {
      if (!ccs.readData()) {
        tvoc = ccs.getTVOC();
        co2 = ccs.geteCO2();
      } else {
        Serial.println("⚠️ Erro ao ler CCS811");
      }
    } else {
      Serial.println("⚠️ CCS811 não disponível. Reiniciando sensor...");
      ccs.begin();
    }

    // Se todos os dados forem válidos, envia e atualiza as variáveis globais
    if (co2 > 0 && tvoc >= 0 && temp != -1 && hum != -1) {
      lastTemperature = temp;
      lastHumidity = hum;
      lastCO2 = co2;
      lastTVOC = tvoc;

      sendHttpSensorData(temp, hum, co2, tvoc);
    } else {
      Serial.println("❌ Dados inválidos. Envio cancelado.");
    }
  }

  delay(10);  // Pequeno delay para estabilidade do loop
}
