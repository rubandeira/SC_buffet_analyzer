#include <Wire.h>
#include <Adafruit_CCS811.h>
#include <SPI.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "MacBook Pro do Tommy";
const char* password = "1223334444";
const char* serverURL = "http://buffetanalyzer.systems/api/sensors";

// Pin definitions
int sda_pin = 21; // GPIO21 as I2C SDA
int scl_pin = 10; // GPIO10 as I2C SCL
#define DHT_PIN 39     // DHT22 data pin
#define DHT_TYPE DHT22 // DHT22 type
#define PWM_PIN 15 // MH-Z19 PIN

// Sensor objects
Adafruit_CCS811 ccs;
DHT dht(DHT_PIN, DHT_TYPE);

// Functions to read DHT sensor data
float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  return t;
}

float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  return h;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Initialize I2C with custom SDA and SCL pins
  Wire.setPins(sda_pin, scl_pin);
  Wire.begin();
  //pinMode(PWM_PIN, INPUT);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  // Initialize CCS811 sensor
  randomSeed(analogRead(0));
  if (!ccs.begin()) {
    Serial.println("Failed to start CCS811 sensor! Please check your wiring.");
    while (1);
  }

  // Wait for CCS811 to stabilize
  while (!ccs.available());
  Serial.println("CCS811 is ready!");

  // Initialize DHT sensor
  dht.begin();

}

void loop() {
  long duration = pulseIn(PWM_PIN, HIGH, 1004000);

  if (duration > 0) {
        // Convert pulse duration to CO2 concentration (in ppm)
        int co2ppm = (duration / 1000) - 2; // Formula from MH-Z19B datasheet
        Serial.print("CO2 Concentration: ");
        Serial.print(co2ppm);
        Serial.println(" ppm");
  } else {
        Serial.println("Error reading PWM signal from MH-Z19B sensor!");
  }

  // Read DHT22 temperature and humidity
  float temperature = readDHTTemperature();
  float humidity = readDHTHumidity();
  float ppm = -1;
  float tvoc = -1;
  int tray_number = random(1, 7);

  // Set environmental data for CCS811 (optional, improves accuracy)
  if (temperature != -1 && humidity != -1) {
    ccs.setEnvironmentalData(humidity, temperature);
  }

  // Read and print CCS811 data
  if (ccs.available()) {
    if (!ccs.readData()) {
      ppm = ccs.geteCO2(); // Read CO2 in ppm
      tvoc = ccs.getTVOC(); // Read TVOC in ppb
      Serial.print("TVOC: ");
      Serial.print(tvoc);
      Serial.println(" ppb");
    } else {
      Serial.println("Error reading from CCS811 sensor!");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(serverURL);  // Specify destination
      http.addHeader("Content-Type", "application/json");
      //Example JSON data
      String jsonPayload = String("{\"tray_number\": ") + tray_number +
                          ", \"ppm\": " + ppm +
                          ", \"tvoc\": " + tvoc +
                          ", \"temperature\": " + temperature + "}";
      Serial.println("Sending payload: " + jsonPayload);

      int httpResponseCode = http.POST(jsonPayload);

      if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response code: " + String(httpResponseCode));
          Serial.println("Server response: " + response);
      } else {
          Serial.println("Error sending POST: " + http.errorToString(httpResponseCode));
      }

      http.end();
  }

  delay(20000);  // Send data every 20 seconds
}
