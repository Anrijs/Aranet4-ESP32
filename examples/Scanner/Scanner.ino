/*
 *  This simple example demonstrates how to scan Aranet4 
 *  devices and read sensor data without connecting
 * 
 *  Name:       Scanner.ino
 *  Created:    2022-10-24
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "Aranet4.h"

#define SCAN_DURATION 5 // 5 seconds
#define SCAN_INTERVAL 30 // 30 seconds

NimBLEScan *pScan;

void setup() {
    Serial.begin(115200);
    Serial.println("Init");

    pScan = NimBLEDevice::getScan();
    pScan->setActiveScan(true);

    // Set up bluettoth security and callbacks
    Aranet4::init();
}

void loop() {
    Serial.println("Scanning BT devices...");
    pScan->start(5);

    NimBLEScanResults results = pScan->getResults();
    Serial.printf("Found %u devices\n", results.getCount());
    Serial.println("-----------------------------");

    for (int i = 0; i < results.getCount(); i++) {
        NimBLEAdvertisedDevice adv = results.getDevice(i);
        AranetManufacturerData mfdata;

        if (mfdata.fromAdvertisement(&adv)) {
            Serial.printf("%s", adv.getName().c_str());
            Serial.printf("    v%u.%u.%u\n",
                mfdata.version.major,
                mfdata.version.minor,
                mfdata.version.patch
            );
            if (!mfdata.flags.bits.integrations) {
                Serial.println("Integrations disabled");
            } else {
                switch (mfdata.data.type) {
                case ARANET4:
                    Serial.printf("CO2:          %i ppm\n", mfdata.data.co2);
                    Serial.printf("Temperature:  %.2f C\n", mfdata.data.temperature / 20.0);
                    Serial.printf("Pressure:     %.1f C\n", mfdata.data.pressure / 10.0);
                    Serial.printf("Humidity:     %i %%\n",  mfdata.data.humidity);
                    Serial.printf("Battery:      %i %%\n",  mfdata.data.battery);
                    Serial.printf("Interval:     %i s\n",   mfdata.data.interval);
                    Serial.printf("Ago:          %i s\n",   mfdata.data.ago);
                    break;
                case ARANET2:
                    Serial.printf("Temperature:  %.2f C\n",  mfdata.data.temperature / 20.0);
                    Serial.printf("Humidity:     %.1f %%\n", mfdata.data.humidity / 10.0);
                    Serial.printf("Battery:      %i %%\n",   mfdata.data.battery);
                    Serial.printf("Interval:     %i s\n",    mfdata.data.interval);
                    Serial.printf("Ago:          %i s\n",    mfdata.data.ago);
                    break;
                case ARANET_RADIATION:
                    Serial.printf("Rate:         %.2f uSv/h\n", mfdata.data.radiation_rate / 1000.0);
                    Serial.printf("Total:        %.4f mSv\n",   mfdata.data.radiation_total / 1000000.0);
                    Serial.printf("Battery:      %i %%\n",  mfdata.data.battery);
                    Serial.printf("Interval:     %i s\n",   mfdata.data.interval);
                    Serial.printf("Ago:          %i s\n",   mfdata.data.ago);
                    break;
                case ARANET_RADON:
                    Serial.printf("Concentration %i Bq/m3\n", mfdata.data.radon_concentration / 1000.0);
                    Serial.printf("Temperature:  %.2f C\n", mfdata.data.temperature / 20.0);
                    Serial.printf("Pressure:     %.1f C\n", mfdata.data.pressure / 10.0);
                    Serial.printf("Humidity:     %.1f %%\n", mfdata.data.humidity / 10.0);
                    Serial.printf("Battery:      %i %%\n",  mfdata.data.battery);
                    Serial.printf("Interval:     %i s\n",   mfdata.data.interval);
                    Serial.printf("Ago:          %i s\n",   mfdata.data.ago);
                    break;
                default:
                    Serial.println("Read failed: unknown type");
                    break;
                }
            }
            Serial.println("-----------------------------");
        }
    }

    delay(SCAN_INTERVAL * 1000); // sleep for 30 seconds
}
