/*
 *  This example demonstrates how to connect to Aranet4
 *  device and retrieve measurement history
 * 
 *  Name:       HistoryRead.ino
 *  Created:    2021-04-27
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "Aranet4.h"

// We will request up to 25 measurements
#define MEASUREMENT_COUNT 25

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

    Serial.println("Connecting...");
    if (ar4.connect(addr) == AR4_OK) {
        // Create store for measuremetns
        AranetDataCompact data[MEASUREMENT_COUNT];
        AranetType type = ar4.getType();
        uint16_t count = ar4.getTotalReadings();

        Serial.printf("Total logs: %i\n", count);

        // Function requres start index and point count
        // Oldest log has index 1. Newest log index should be same as total log count.

        // We will request 25 latest measurements. This means start index will be [Total count] - [25] + [1]
        int start = count - MEASUREMENT_COUNT + 1;
        uint16_t params = AR4_PARAM_FLAGS;

        if (type == ARANET2) {
            params = AR2_PARAM_FLAGS;
        } else if (type == ARANET_RADIATION) {
            params = ARR_PARAM_FLAGS;
        }

        int recvd = ar4.getHistory(start, MEASUREMENT_COUNT, data, params);

        // Print fortmated data
        for(int i=0;i<recvd;i++) {
            if (type == ARANET4) {
                Serial.printf("#%i: \t%i ppm \t%.1f C \t%.1f hPa \t%i %%\n",i+1,
                    data[i].aranet4.co2,
                    data[i].aranet4.temperature/20.0,
                    data[i].aranet4.pressure/10.0,
                    data[i].aranet4.humidity);
            } else if (type == ARANET2) {
                Serial.printf("#%i: \t%.1f C \t%.1f %%\n",i+1,
                    data[i].aranet4.temperature/20.0,
                    data[i].aranet4.humidity/10.0);
            } else if (type == ARANET_RADIATION) {
                Serial.printf("#%i: \t%.2f uSv/h\t%.4f mSv\n",i+1,
                    data[i].aranetr.rad_dose_rate / 100.0,
                    data[i].aranetr.rad_dose_integral / 1000000.0);
            }
        }
    } else {
        Serial.println("Failed to conenct.");
    }
    ar4.disconnect();
}

void loop() {

}