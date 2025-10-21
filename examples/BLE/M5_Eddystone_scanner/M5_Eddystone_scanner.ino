/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
   Changed to a beacon scanner to report iBeacon, EddystoneURL and EddystoneTLM beacons by beegee-tokyo
*/

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>

int scanTime = 1; //In seconds
BLEScan *pBLEScan;


#define BEACON_NAME "Unique-Name"
float detected_rssi = 0.0;
unsigned long dt;
bool valid_rssi = false;    // we need to catch if we found the rssi on
                            // a recent scan or not
unsigned long last_scan;
unsigned long start_time;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

      // Paul:
      // We should be able to catch "Unique-Name", useful for debugging
      // bug maybe comment out later.
//      if (advertisedDevice.haveName()) {      
//        Serial.print("Device name: ");
//        Serial.println(advertisedDevice.getName().c_str());
//        Serial.println(""); 
//      }

      // Paul:
      // Maybe useful to quickly call return if this is
      // not the unique beacon we care about?
      // Commenting out for now.
      //      if (advertisedDevice.haveServiceUUID()) {
      //        BLEUUID devUUID = advertisedDevice.getServiceUUID();
      //        Serial.print("Found ServiceUUID: ");
      //        Serial.println(devUUID.toString().c_str());
      //        Serial.println("");
      //      }

      // Paul:
      // Which type? Don't care about this, so commenting out.
      //      if (advertisedDevice.haveManufacturerData() == true) {
      //        std::string strManufacturerData = advertisedDevice.getManufacturerData();
      //
      //        uint8_t cManufacturerData[100];
      //        strManufacturerData.copy((char *)cManufacturerData, strManufacturerData.length(), 0);
      //
      //        if (strManufacturerData.length() == 25 && cManufacturerData[0] == 0x4C && cManufacturerData[1] == 0x00) {
      //          Serial.println("Found an iBeacon!");
      //          BLEBeacon oBeacon = BLEBeacon();
      //          oBeacon.setData(strManufacturerData);
      //          Serial.printf("iBeacon Frame\n");
      //          Serial.printf("ID: %04X Major: %d Minor: %d UUID: %s Power: %d\n", oBeacon.getManufacturerId(), ENDIAN_CHANGE_U16(oBeacon.getMajor()), ENDIAN_CHANGE_U16(oBeacon.getMinor()), oBeacon.getProximityUUID().toString().c_str(), oBeacon.getSignalPower());
      //        }
      //        else
      //        {
      //          Serial.println("Found another manufacturers beacon!");
      //          Serial.printf("strManufacturerData: %d ", strManufacturerData.length());
      //          for (int i = 0; i < strManufacturerData.length(); i++)
      //          {
      //            Serial.printf("[%X]", cManufacturerData[i]);
      //          }
      //          Serial.printf("\n");
      //        }
      //      }

      uint8_t *payLoad = advertisedDevice.getPayload();
      // search for Eddystone Service Data in the advertising payload
      // *payload shall point to eddystone data or to its end when not found
      const uint8_t serviceDataEddystone[3] = {0x16, 0xAA, 0xFE}; // it has Eddystone BLE UUID
      const size_t payLoadLen = advertisedDevice.getPayloadLength();
      uint8_t *payLoadEnd = payLoad + payLoadLen - 1; // address of the end of payLoad space
      while (payLoad < payLoadEnd) {
        if (payLoad[1] == serviceDataEddystone[0] && payLoad[2] == serviceDataEddystone[1] && payLoad[3] == serviceDataEddystone[2]) {
          // found!
          payLoad += 4;
          break;
        }
        payLoad += *payLoad + 1;  // payLoad[0] has the field Length
      }

      // Eddystone Service Data and respective BLE UUID were found
      if (payLoad < payLoadEnd) {

        // Paul:
        // Not using URL beacon type, so this is redundant. Will
        // comment out the bulk of this.
        if (*payLoad == 0x10) {

          // Paul: we don't actually care about the data.
          // commenting out.
//          Serial.println("Found an EddystoneURL beacon!");
//          BLEEddystoneURL foundEddyURL = BLEEddystoneURL();
//          uint8_t URLLen = *(payLoad - 4) - 3;  // Get Field Length less 3 bytes (type and UUID)
//          foundEddyURL.setData(std::string((char*)payLoad, URLLen));
//          std::string bareURL = foundEddyURL.getURL();
//          if (bareURL[0] == 0x00)
//          {
//            // dumps all bytes in advertising payload
//            Serial.println("DATA-->");
//            uint8_t *payLoad = advertisedDevice.getPayload();
//            for (int idx = 0; idx < payLoadLen; idx++)
//            {
//              Serial.printf("0x%02X ", payLoad[idx]);
//            }
//            Serial.println("\nInvalid Data");
//            return;
//          }
//
//          Serial.printf("Found URL: %s\n", foundEddyURL.getURL().c_str());
//          Serial.printf("Decoded URL: %s\n", foundEddyURL.getDecodedURL().c_str());
//          Serial.printf("TX power %d\n", foundEddyURL.getPower());  
//          Serial.print(" RSSI: " ); Serial.println( advertisedDevice.getRSSI() );
//          Serial.println("\n");

          valid_rssi = true;
          detected_rssi = (float)advertisedDevice.getRSSI();
          dt = millis() - last_scan;
          last_scan = millis();
        
        } else if (*payLoad == 0x20) {
          
          // Paul: 
          // commenting out, don't care about the data
//          Serial.println("Found an EddystoneTLM beacon!");
//          Serial.print(" RSSI: " ); Serial.println( advertisedDevice.getRSSI() );
//          BLEEddystoneTLM eddystoneTLM;
//          eddystoneTLM.setData(std::string((char*)payLoad, 14));
//          Serial.printf("Reported battery voltage: %dmV\n", eddystoneTLM.getVolt());
//          Serial.printf("Reported temperature: %.2f°C (raw data=0x%04X)\n", eddystoneTLM.getTemp(), eddystoneTLM.getRawTemp());
//          Serial.printf("Reported advertise count: %d\n", eddystoneTLM.getCount());
//          Serial.printf("Reported time since last reboot: %ds\n", eddystoneTLM.getTime());
//          Serial.println("\n");
//          Serial.print(eddystoneTLM.toString().c_str());
//          Serial.println("\n");
        }
      }
    }
};

void setup()
{
  Serial.begin(115200);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster

  // Paul: you might be able to tweak these to get a 
  // faster scan time.
  // Scan Interval  How often the ESP32 starts a new scan window. Measured in units of 0.625 ms.
  // Scan Window How long the radio actively listens during each scan interval. Also in units of 0.625 ms.
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99 ); // less or equal setInterval value

  last_scan = millis();
  start_time = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  valid_rssi = false;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  if( millis() - start_time > 30000 ) {
    while(true) {
      Serial.println("End");
      delay(1000);
      
    }
  }

  if( valid_rssi ) {
    Serial.print( dt );
    Serial.print(",");
    Serial.println( detected_rssi, 2 );
  }

  
//  Serial.print("Found: ");
//  Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
