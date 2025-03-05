#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_client.h"

// Credenciais Wi-Fi
const char* ssid = "MacBook Pro do Tommy";
const char* password = "1223334444";

// Configuração dos pinos da câmera (modelo AI-Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
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
#define LED_GPIO_NUM      4  // LED do flash

// URL da API (usando HTTP)
const char* serverUrl = "http://buffetanalyzer.systems/api/upload";

// Função para capturar e enviar uma imagem
void takePictureAndSend(int trayNumber) {
    // Liga o flash
    pinMode(LED_GPIO_NUM, OUTPUT);
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(100);  // Permite que o flash estabilize

    // Captura uma foto
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("❌ Erro ao capturar a imagem");
        digitalWrite(LED_GPIO_NUM, LOW);
        return;
    }

    // Configuração do cliente HTTP (HTTP, sem TLS)
    esp_http_client_config_t config = {};
    config.url = serverUrl;
    config.method = HTTP_METHOD_POST;
    // Removemos os parâmetros TLS, pois estamos usando HTTP

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Cabeçalho multipart/form-data
    const char* boundary = "----esp32boundary";
    esp_http_client_set_header(client, "Content-Type", 
        (String("multipart/form-data; boundary=") + boundary).c_str());

    // Construção do corpo da requisição
    String bodyStart = String("--") + boundary + "\r\n"
                       "Content-Disposition: form-data; name=\"tray_number\"\r\n\r\n" +
                       String(trayNumber) + "\r\n" +
                       "--" + boundary + "\r\n"
                       "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n"
                       "Content-Type: image/jpeg\r\n\r\n";
    String bodyEnd = String("\r\n--") + boundary + "--\r\n";

    size_t bodyLength = bodyStart.length() + fb->len + bodyEnd.length();

    // Envia a requisição
    esp_http_client_open(client, bodyLength);
    esp_http_client_write(client, bodyStart.c_str(), bodyStart.length());
    esp_http_client_write(client, (const char*)fb->buf, fb->len);
    esp_http_client_write(client, bodyEnd.c_str(), bodyEnd.length());

    // Executa a requisição
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        Serial.println("✅ Imagem enviada com sucesso!");
    } else {
        Serial.printf("❌ Falha ao enviar a imagem: %s\n", esp_err_to_name(err));
    }

    // Limpa os recursos
    esp_http_client_cleanup(client);
    esp_camera_fb_return(fb);

    // Desliga o flash
    digitalWrite(LED_GPIO_NUM, LOW);
}

// Função de configuração
void setup() {
    Serial.begin(115200);
    Serial.println("🔄 Inicializando...");

    // Configura o LED do flash
    pinMode(LED_GPIO_NUM, OUTPUT);
    digitalWrite(LED_GPIO_NUM, LOW);

    // Conecta ao Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ Conectado ao Wi-Fi!");

    // Configura a câmera
    camera_config_t cam_config;
    cam_config.ledc_channel = LEDC_CHANNEL_0;
    cam_config.ledc_timer = LEDC_TIMER_0;
    cam_config.pin_d0 = Y2_GPIO_NUM;
    cam_config.pin_d1 = Y3_GPIO_NUM;
    cam_config.pin_d2 = Y4_GPIO_NUM;
    cam_config.pin_d3 = Y5_GPIO_NUM;
    cam_config.pin_d4 = Y6_GPIO_NUM;
    cam_config.pin_d5 = Y7_GPIO_NUM;
    cam_config.pin_d6 = Y8_GPIO_NUM;
    cam_config.pin_d7 = Y9_GPIO_NUM;
    cam_config.pin_xclk = XCLK_GPIO_NUM;
    cam_config.pin_pclk = PCLK_GPIO_NUM;
    cam_config.pin_vsync = VSYNC_GPIO_NUM;
    cam_config.pin_href = HREF_GPIO_NUM;
    cam_config.pin_sccb_sda = SIOD_GPIO_NUM;
    cam_config.pin_sccb_scl = SIOC_GPIO_NUM;
    cam_config.pin_pwdn = PWDN_GPIO_NUM;
    cam_config.pin_reset = RESET_GPIO_NUM;
    cam_config.xclk_freq_hz = 20000000;
    cam_config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        cam_config.frame_size = FRAMESIZE_UXGA;
        cam_config.jpeg_quality = 10;
        cam_config.fb_count = 2;
    } else {
        cam_config.frame_size = FRAMESIZE_SVGA;
        cam_config.jpeg_quality = 12;
        cam_config.fb_count = 1;
    }

    if (esp_camera_init(&cam_config) != ESP_OK) {
        Serial.println("❌ Falha ao inicializar a câmera");
        while (true);
    }
    Serial.println("✅ Câmera inicializada com sucesso!");
}

// Loop principal
void loop() {
    int trayNumber = 1;  // Número da bandeja
    takePictureAndSend(trayNumber);
    delay(20000);  // Aguarda 20 segundos antes de capturar novamente
}