/*
 *  This simple example demonstrates how to connect to Aranet4 
 *  device and read sensor data
 * 
 *  Name:       BasicRead.ino
 *  Created:    2021-04-18
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "Aranet4.h"

// Create custom callback to allow PIN code input.
// In this example, when PIN is requested, you must enter it in serial console.
class MyAranet4Callbacks: public Aranet4Callbacks {
    uint32_t onPinRequested() {
      Serial.println("PIN Requested. Enter PIN in serial console.");
      while(Serial.available() == 0)
          vTaskDelay(500 / portTICK_PERIOD_MS);

      return  Serial.readString().toInt();
    }

    void onConnected() {
      Serial.println("Device conencted");
    }

    void onFailed(uint8_t code) {
        Serial.print("Connection failed: ");
        switch (code) {
            case AR4_ERR_NOT_CONNECTED: Serial.println("not connected"); break;
            case AR4_ERR_UNAUTHORIZED: Serial.println("unauthorized"); break;
            default: Serial.println("unknown)"); break;
        }
    }

    void onDisconnected() {
        Serial.println("Aranet4 disconencted");
    }
};
 
void setup() {
    Serial.begin(115200);
    Serial.println("Init");

    // Address can be string or byte array
    // uint8_t addr[] = {0xc00 0x01, 0x02, 0x03, 0x04, 0x05 };
    String addr = "00:01:02:03:04:05"; // Put your Aranet4 MAC address here

    Aranet4 ar4;
    ar4.init(new MyAranet4Callbacks());

    Serial.println("Connecting...");
    if (ar4.connect(addr) == AR4_OK) {
        AranetData data = ar4.getCurrentReadings();
        
        if (ar4.getStatus() == AR4_OK) {
            Serial.println("Aranet4 read OK");
            Serial.printf("CO2:          %i ppm\n", data.co2);
            Serial.printf("Temperature:  %.2f C\n", data.temperature / 20.0);
            Serial.printf("Pressure:     %.1f C\n", data.pressure / 10.0);
            Serial.printf("Humidity:     %i %%\n",  data.humidity);
            Serial.printf("Battery:      %i %%\n",  data.battery);
            Serial.printf("Interval:     %i s\n",   data.interval);
            Serial.printf("Ago:          %i s\n",   data.ago);
        } else {
           Serial.printf("Aranet4 read failed: (%i)\n", ar4.getStatus());
        }
    }

    ar4.disconnect();
}

void loop() {

}