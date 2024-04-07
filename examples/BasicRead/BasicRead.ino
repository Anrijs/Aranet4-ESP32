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

// Address can be string or byte array
// uint8_t addr[] = {0xc00 0x01, 0x02, 0x03, 0x04, 0x05 };
String addr = "00:01:02:03:04:05"; // Put your Aranet MAC address here

// Create custom callback to allow PIN code input.
// In this example, when PIN is requested, you must enter it in serial console.
class MyAranet4Callbacks: public Aranet4Callbacks {
    uint32_t onPinRequested() {
        Serial.println("PIN Requested. Enter PIN in serial console.");
        while(Serial.available() == 0)
            vTaskDelay(500 / portTICK_PERIOD_MS);
        return  Serial.readString().toInt();
    }
};

Aranet4 ar4(new MyAranet4Callbacks());

void setup() {
    Serial.begin(115200);
    Serial.println("Init");

    // Set up bluettoth security and callbacks
    Aranet4::init();
}

void loop() {
    long sleep = 1000 * 60; // 1 minute

    Serial.println("Connecting...");

    bool secure = true;
    if (ar4.connect(addr) == AR4_OK) {
        AranetData data = ar4.getCurrentReadings();
        
        if (ar4.getStatus() == AR4_OK) {
            switch (data.type) {
            case ARANET4:
                Serial.println("Aranet2 read OK");
                Serial.printf("CO2:          %i ppm\n", data.co2);
                Serial.printf("Temperature:  %.2f C\n", data.temperature / 20.0);
                Serial.printf("Pressure:     %.1f C\n", data.pressure / 10.0);
                Serial.printf("Humidity:     %i %%\n",  data.humidity);
                Serial.printf("Battery:      %i %%\n",  data.battery);
                Serial.printf("Interval:     %i s\n",   data.interval);
                Serial.printf("Ago:          %i s\n",   data.ago);
                break;
            case ARANET2:
                Serial.println("Aranet4 read OK");
                Serial.printf("Temperature:  %.2f C\n", data.temperature / 20.0);
                Serial.printf("Humidity:     %.1f %%\n", data.humidity / 10.0);
                Serial.printf("Battery:      %i %%\n",  data.battery);
                Serial.printf("Interval:     %i s\n",   data.interval);
                Serial.printf("Ago:          %i s\n",   data.ago);
                break;
            case ARANET_RADIATION:
                Serial.println("Aranet Radiation read OK");
                Serial.printf("Rate:         %.2f uSv/h\n", data.radiation_rate / 1000.0);
                Serial.printf("Total:        %.4f mSv\n",   data.radiation_total / 1000000.0);
                Serial.printf("Battery:      %i %%\n",  data.battery);
                Serial.printf("Interval:     %i s\n",   data.interval);
                Serial.printf("Ago:          %i s\n",   data.ago);
                break;
            default:
                Serial.println("Aranet read failed: unknown type");
                break;
            }

            sleep = (data.interval - data.ago) * 1000;
        } else {
           Serial.printf("Aranet read failed: (%i)\n", ar4.getStatus());
        }
    }

    ar4.disconnect();

    Serial.printf("Waiting %i seconds for next measurement\n", sleep / 1000);
    delay(sleep); // Wait until next measurement
}
