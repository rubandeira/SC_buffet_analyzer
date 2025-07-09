void wifi_setup() {
  WiFi.mode(WIFI_STA);  // Modo estação

  IPAddress local_IP(192, 168, 137, 101);   // IP fixo desejado
  IPAddress gateway(192, 168, 137, 1);      // Gateway (geralmente 137.1 em hotspot Windows)
  IPAddress subnet(255, 255, 255, 0);       // Sub-rede
  IPAddress dns(8, 8, 8, 8);                // DNS (Google)

  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("❌ Falha ao configurar IP estático");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println(F("[WIFI] Connecting..."));
}

void wifi_connection_manager(){

  static boolean wifi_connected = false;
  static long wifi_disconnected_time = 0;

   // Check for WiFi
  if(WiFi.status() != WL_CONNECTED){
    // Wifi disconnected
   
    if(wifi_connected){
      // Acknowledge disconnection
      wifi_connected = false;
      wifi_disconnected_time = millis();
      Serial.println(F("[WIFI] Disconnected"));
    }

    // Force reconnect if fail
    if(millis() - wifi_disconnected_time > WIFI_CONNECTION_TIMEOUT){
      wifi_disconnected_time = millis();
      Serial.println(F("[WIFI] Connection timeout, resetting connection..."));
      WiFi.disconnect(true);
      wifi_setup();
    }
  }
  else {
    // Wifi connected
    
    if(!wifi_connected){
      // Acknowledge wifi connection
      wifi_connected = true;
      Serial.print(F("[WIFI] connected, IP: "));
      Serial.println(WiFi.localIP());
    }
  }
}
