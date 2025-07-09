#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h> //Required to compile on Russian 'ArduinoDroid' IDE but not required on Arduino IDE
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "base64.h"

/*
 * BOARD SETTINGS
 * 
 * Board type: ESP32 Wrover Module
 * 
 */

// Getting WIFI_SSID and WIFI_PASSWORD from external credentials file
//#include "jtektiot_credentials.h";
const char* WIFI_SSID = "HotSpot";
const char* WIFI_PASSWORD = "soufixe123";

// Intervalo
unsigned long lastSendTime = 0;
const unsigned long interval = 15000;


// Wifi settings
#define HOSTNAME "camera"
#define WIFI_CONNECTION_TIMEOUT 5000

// Camera pinout
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define FLASH_GPIO_NUM     4
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCameraServer();

void sendImageHTTP() {
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    delay(150);

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
      Serial.println("❌ Falha ao capturar imagem");
      digitalWrite(FLASH_GPIO_NUM, LOW);
      return;
    }

    // Codificar imagem em Base64
    String base64Image = base64::encode(fb->buf, fb->len);
    Serial.println("📷 Imagem convertida para base64");

    HTTPClient http;
    String url = "http://192.168.137.248:8080/buffet-food-quality-cam-01/properties/photo";
    http.begin(url);
    http.addHeader("Content-Type", "text/plain");

    // Enviar imagem em base64 por PUT
    int httpResponseCode = http.PUT((uint8_t*)base64Image.c_str(), base64Image.length());

    if (httpResponseCode > 0) {
      Serial.printf("📤 Enviado com sucesso. Código: %d\n", httpResponseCode);
    } else {
      Serial.printf("❌ Erro ao enviar imagem. Código: %d\n", httpResponseCode);
    }

    http.end();
    esp_camera_fb_return(fb);
    digitalWrite(FLASH_GPIO_NUM, LOW);
  } else {
    Serial.println("❌ WiFi não conectado.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  digitalWrite(FLASH_GPIO_NUM, LOW);  // Garante que começa desligado
  Serial.println();
  camera_init();
  wifi_setup();
  startCameraServer();
  ota_setup();
}
void loop() {
  ArduinoOTA.handle();

  // Verifica se passaram 11 minutos (660000 ms)
  if (millis() - lastSendTime >= 30000) {
    lastSendTime = millis();
    sendImageHTTP();
  }

  delay(100);  // Evita uso 100% da CPU
}

