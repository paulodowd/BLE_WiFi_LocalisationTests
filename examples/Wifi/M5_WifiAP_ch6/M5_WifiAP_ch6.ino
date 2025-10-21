#include <WiFi.h>

const char* ssid = "MyCore2AP";      // Your AP name
const char* password = "12345678";   // Minimum 8 chars for WPA2

void setup() {
  Serial.begin(115200);
  delay(100);

  // Set Wi-Fi mode to Access Point
  WiFi.mode(WIFI_AP);

  // Configure and start the AP on channel 6
  bool result = WiFi.softAP(ssid, password, 11);  // 3rd arg = channel
  if(result) {
    Serial.println("Access Point started successfully!");
    Serial.printf("SSID: %s, Channel: %d\n", ssid, 11);
    Serial.printf("IP address: %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("Failed to start Access Point.");
  }
}

void loop() {
  // Optionally monitor connected clients
  Serial.printf("Clients connected: %d\n", WiFi.softAPgetStationNum());
  delay(5000);
}
