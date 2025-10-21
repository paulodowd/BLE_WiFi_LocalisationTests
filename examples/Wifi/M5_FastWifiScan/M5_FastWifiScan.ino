#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

// If set to 0, scans all channels
#define SCAN_CHANNEL 2

unsigned long start_time;
unsigned long dt;
unsigned long last_scan;

// Event handler for Wi-Fi scan done
void WiFiScanEventHandler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    // Get number of APs found
    uint16_t apCount = 0;
    esp_wifi_scan_get_ap_num(&apCount);
    if (apCount == 0) {
      //      Serial.println("No networks found on channel 6.");
      resetScan();
      return;
    }

    // Allocate memory and fetch AP records
    wifi_ap_record_t *apList = (wifi_ap_record_t*)malloc(sizeof(wifi_ap_record_t) * apCount);
    if (!apList) return;
    esp_wifi_scan_get_ap_records(&apCount, apList);

    //    Serial.printf("Scan done! Found %d networks on channel 6:\n", apCount);
    for (int i = 0; i < apCount; i++) {

      if (apList[i].primary == SCAN_CHANNEL) {

	// Here, just checking 2 characters to lock on to
	// the expected SSID
        if ( apList[i].ssid[7 ] == 'e' && apList[i].ssid[8] == 'n' ) {


          if ( millis() - start_time > 30000 ) {
            while (true) {
              Serial.println("end");
              delay(1000);
            }
          }
          dt = millis() - last_scan;
          Serial.print( dt );
          Serial.print(",");
          Serial.println( apList[i].rssi );
          last_scan = millis();
//                    Serial.printf("%d: SSID: %s, RSSI: %d dBm, Channel: %d\n", i, apList[i].ssid, apList[i].rssi, apList[i].primary);
          //          Serial.print( (char)apList[i].ssid[7] );
          //          Serial.print( "," );
          //          Serial.println( (char)apList[i].ssid[8] );

        }
      }
    }

    free(apList);
    resetScan();

  }
}

void resetScan() {


  // Clear cached results and restart scan
  WiFi.scanDelete();  // ensures old APs don't appear in next scan

  // Start a new single-channel scan immediately
  wifi_scan_config_t scanConfig = {0};
  scanConfig.channel = SCAN_CHANNEL;
  scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  scanConfig.scan_time.active.min = 30;
  scanConfig.scan_time.active.max = 60;
  esp_wifi_scan_start(&scanConfig, false);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // --- Core initialization to prevent 12308 error ---
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  // --- Wi-Fi init ---
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  // --- Register scan done event handler ---
  esp_event_handler_instance_t instance;
  esp_event_handler_instance_register(
    WIFI_EVENT,
    WIFI_EVENT_SCAN_DONE,
    &WiFiScanEventHandler,
    NULL,
    &instance
  );

  last_scan = millis();
  dt = 0;
  start_time = millis();

  // --- Start first scan on channel 6 ---
  WiFi.scanDelete();  // clear any previous cached results
  wifi_scan_config_t scanConfig = {0};
  scanConfig.channel = SCAN_CHANNEL;
  scanConfig.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  scanConfig.scan_time.active.min = 30;
  scanConfig.scan_time.active.max = 60;
  esp_wifi_scan_start(&scanConfig, false);

  Serial.println("Started true single-channel scan on channel 6...");
}

void loop() {
  // Main loop can handle other tasks
  delay(1000);
}
