
# Aranet4 ESP32 client
This library allows you to read data from Aranet4 devices using Bluetooth.

## Requirements
To save memory, this library is using NimBLE bluetooth stack. Install it from library manager or get it here: [h2zero/NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)

## Usage example
1. Create bluetooth callback, to allow pairing. In this example, when PIN is requested, you must enter it in serial console:
```cpp
class MyAranet4Callbacks: public Aranet4Callbacks {
    uint32_t onPinRequested() {
        Serial.println("PIN Requested. Enter PIN in serial console.");
        while(Serial.available() == 0)
            vTaskDelay(500 / portTICK_PERIOD_MS);
        return  Serial.readString().toInt();
    }
};
```
2. Initialize ESP32 bluetooth device and security profile in `setup()` function:
```cpp
Aranet4::init(new  MyAranet4Callbacks());
```
3. Connect to device and read data:
```cpp
String addr =  "00:01:02:03:04:05";
Aranet4 ar4;
...
if (ar4.connect(addr) == AR4_OK) {
    AranetData data =  ar4.getCurrentReadings();
	Serial.printf("CO2: %i ppm\n", data.co2);
	Serial.printf("Temperature: %.2f C\n", data.temperature  /  20.0);
	Serial.printf("Pressure: %.1f C\n", data.pressure  /  10.0);
	Serial.printf("Humidity: %i  %%\n", data.humidity);
	Serial.printf("Battery: %i  %%\n", data.battery);
	Serial.printf("Interval: %i s\n", data.interval);
	Serial.printf("Ago: %i s\n", data.ago);
} else {
    Serial.println("read failed");
}
ar4.disconnect();
```

Measurement data is stored in original binary format. This means temperature must be divided by 20 and pressure must be divided by 10 to get correct values.
