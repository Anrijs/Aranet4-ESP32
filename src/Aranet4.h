/*
 *  Name:       Aranet4.h
 *  Created:    2021-04-17
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#ifndef __ARANET4_H
#define __ARANET4_H

#include "Arduino.h"
#include <NimBLEDevice.h>

#define ARANET4_MANUFACTURER_ID 0x0702

typedef uint16_t ar4_err_t;

// ar4_err_t Status/Error codes
#define AR4_OK           0 // ar4_err_t value indicating success (no error)
#define AR4_FAIL        -1 // Generic ar4_err_t code indicating failure

#define AR4_ERR_NO_GATT_SERVICE    0x01
#define AR4_ERR_NO_GATT_CHAR       0x02
#define AR4_ERR_NO_CLIENT          0x03
#define AR4_ERR_NOT_CONNECTED      0x04

// Aranet4 specific codes
#define AR4_PARAM_TEMPERATURE   1
#define AR4_PARAM_HUMIDITY      2
#define AR4_PARAM_PRESSURE      3
#define AR4_PARAM_CO2           4
#define AR4_PARAM_HUMIDITY2     5

#define AR4_NO_DATA_FOR_PARAM   -1

#define AR4_PACKING_ARANET2     0
#define AR4_PACKING_ARANET4     1

// Aranet4 param flags
#define AR4_PARAM_TEMPERATURE_FLAG   1 << (AR4_PARAM_TEMPERATURE - 1)
#define AR4_PARAM_HUMIDITY_FLAG      1 << (AR4_PARAM_HUMIDITY - 1)
#define AR4_PARAM_PRESSURE_FLAG      1 << (AR4_PARAM_PRESSURE - 1)
#define AR4_PARAM_CO2_FLAG           1 << (AR4_PARAM_CO2 - 1)
#define AR4_PARAM_HUMIDITY2_FLAG     1 << (AR4_PARAM_HUMIDITY2 - 1)

#define AR2_PARAM_FLAGS  AR4_PARAM_TEMPERATURE_FLAG | AR4_PARAM_HUMIDITY2_FLAG
#define AR4_PARAM_FLAGS  AR4_PARAM_TEMPERATURE_FLAG | AR4_PARAM_HUMIDITY_FLAG | AR4_PARAM_PRESSURE_FLAG | AR4_PARAM_CO2_FLAG

// Service UUIDs
static NimBLEUUID UUID_Aranet4_Old  ("f0cd1400-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4      ("fce0");
static NimBLEUUID UUID_Generic      ("1800");
static NimBLEUUID UUID_Common       ("180a");

// Read / Aranet service
static NimBLEUUID UUID_Aranet2_CurrentReadings     ("f0cd1504-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_CurrentReadings     ("f0cd1503-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_CurrentReadingsDet  ("f0cd3001-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_Interval            ("f0cd2002-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_SecondsSinceUpdate  ("f0cd2004-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_TotalReadings       ("f0cd2001-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_Cmd                 ("f0cd1402-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_Notify_History      ("f0cd2003-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Aranet4_Subscribe_History   ("2902");

// Read / Generic servce
static NimBLEUUID UUID_Generic_DeviceName ("2a00");

//Read / Common servce
static NimBLEUUID UUID_Common_Manufacturer ("2a29");
static NimBLEUUID UUID_Common_Model        ("2a24");
static NimBLEUUID UUID_Common_Serial       ("2a25");
static NimBLEUUID UUID_Common_HwRev        ("2a27");
static NimBLEUUID UUID_Common_FwRev        ("2a26");
static NimBLEUUID UUID_Common_SwRev        ("2a28");
static NimBLEUUID UUID_Common_Battery      ("2a19");

#pragma pack(push, 1)
typedef struct AranetData {
    uint8_t  packing = 0;
    uint16_t co2 = 0;
    uint16_t temperature = 0;
    uint16_t pressure = 0;
    uint16_t humidity = 0;
    uint8_t  battery = 0;
    uint8_t  status = 0;
    uint16_t interval = 0;
    uint16_t ago = 0;

    bool parseFromAdvertisement(uint8_t* data) {
        packing = data[9];
        if (packing == AR4_PACKING_ARANET2) {
            // aranet2
            memcpy(&temperature, (uint8_t*) data + 12, 2);
            memcpy(&humidity,    (uint8_t*) data + 16, 2);
            memcpy(&interval,    (uint8_t*) data + 21, 2);
            memcpy(&ago,         (uint8_t*) data + 23, 2);

            battery = data[19];
            status = data[20];

            return true;
        } else if (packing == AR4_PACKING_ARANET4) {
            // aranet4
            memcpy(&co2,         (uint8_t*) data + 10, 2);
            memcpy(&temperature, (uint8_t*) data + 12, 2);
            memcpy(&pressure,    (uint8_t*) data + 14, 2);
            memcpy(&interval,    (uint8_t*) data + 19, 2);
            memcpy(&ago,         (uint8_t*) data + 21, 2);

            humidity = data[16];
            battery = data[17];
            status = data[18];

            return true;
        }

        return false;
    }

    ar4_err_t parseFromGATT(uint8_t* data, uint8_t packing) {
        this->packing = packing;
        if (packing == AR4_PACKING_ARANET2) {
            // aranet2
            memcpy(&temperature, (uint8_t*) data + 7, 2);
            memcpy(&humidity,    (uint8_t*) data + 9, 2);
            memcpy(&interval,    (uint8_t*) data + 2, 2);
            memcpy(&ago,         (uint8_t*) data + 4, 2);

            battery = data[6];
            status = data[11];

            return AR4_OK;
        } else if (packing == AR4_PACKING_ARANET4) {
            // aranet4
            memcpy(&co2,         (uint8_t*) data + 0, 2);
            memcpy(&temperature, (uint8_t*) data + 2, 2);
            memcpy(&pressure,    (uint8_t*) data + 4, 2);
            memcpy(&interval,    (uint8_t*) data + 9, 2);
            memcpy(&ago,         (uint8_t*) data + 11, 2);

            humidity = data[6];
            battery = data[7];
            status = data[8];

            return AR4_OK;
        }

        return AR4_FAIL;
    }

    uint16_t getCO2() {
        if (packing == AR4_PACKING_ARANET4) return co2;
        return -1;
    }

    float getTemperature() {
        return temperature / 20.0;
    }

    float getPressure() {
        if (packing == AR4_PACKING_ARANET4) return pressure / 10.0;
        return -1.0;
    }

    float getHumidity() {
        if (packing == AR4_PACKING_ARANET4) return humidity;
        return humidity / 10.0;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct AranetManufacturerData {
    uint16_t manufacturer_id;
    uint8_t disconnected : 1,
            __unknown1   : 1,
            calib_state  : 2,
            dfu_mode     : 1,
            integrations : 1,
            __unknown2   : 2;
    struct {
        uint8_t  patch;
        uint8_t  minor;
        uint16_t major;
    } version;
    uint8_t hw_rev;
    uint8_t __unknown3;
    uint8_t packing;
    AranetData data;

    bool fromAdvertisement(NimBLEAdvertisedDevice* adv) {
        std::string strManufacturerData = adv->getManufacturerData();
        int cLength = strManufacturerData.length();

        uint8_t cManufacturerData[100];
        strManufacturerData.copy((char *) cManufacturerData, cLength, 0);

        // check manufacturer id
        if(*(uint16_t*) cManufacturerData != ARANET4_MANUFACTURER_ID) return false;

        // copy data
        if (cLength > sizeof(AranetManufacturerData)) cLength = sizeof(AranetManufacturerData);
        memcpy(this, (void*) cManufacturerData, cLength); // -2 to drop id

        return true;
    }
};
#pragma pack(pop)

// Small version of AranetData for history
typedef struct AranetDataCompact {
    uint16_t co2 = 0;
    uint16_t temperature = 0;
    uint16_t pressure = 0;
    uint16_t humidity = 0;
};

class Aranet4Callbacks : public NimBLEClientCallbacks {
    uint32_t onPassKeyRequest() {
        return onPinRequested();
    }

    virtual uint32_t onPinRequested() = 0;
};

class Aranet4 {
public:
    Aranet4(Aranet4Callbacks* callbacks);
    ~Aranet4();
    static void init(uint16_t mtu = 247);
    ar4_err_t connect(NimBLEAdvertisedDevice* adv, bool secure = true);
    ar4_err_t connect(NimBLEAddress addr, bool secure = true);
    ar4_err_t connect(uint8_t* addr, bool secure = true, uint8_t type = BLE_ADDR_RANDOM);
    ar4_err_t connect(String addr, bool secure = true, uint8_t type = BLE_ADDR_RANDOM);
    ar4_err_t secureConnection();
    void      disconnect();
    void      setConnectTimeout(uint8_t time);
    bool      isConnected();

    AranetData  getCurrentReadings();
    uint16_t    getSecondsSinceUpdate();
    uint16_t    getTotalReadings();
    uint16_t    getInterval();
    String      getName();
    String      getSwVersion();
    String      getFwVersion();
    String      getHwVersion();

    ar4_err_t   writeCmd(uint8_t* data, uint16_t len);

    int         getHistoryCO2(int start, uint16_t count, uint16_t* data);
    int         getHistoryTemperature(int start, uint16_t count, uint16_t* data);
    int         getHistoryPressure(int start, uint16_t count, uint16_t* data);
    int         getHistoryHumidity(int start, uint16_t count, uint16_t* data);
    int         getHistoryHumidity2(int start, uint16_t count, uint16_t* data);
    int         getHistory(int start, uint16_t count, AranetDataCompact* data, uint8_t params = AR4_PARAM_FLAGS);
    ar4_err_t   getStatus();

    bool isAranet4();
    bool isAranet2();
private:
    NimBLEClient* pClient = nullptr;
    ar4_err_t status = AR4_OK;

    NimBLERemoteService* getAranetService();

    ar4_err_t getValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid, uint8_t* data, uint16_t* len);;
    ar4_err_t getValue(NimBLERemoteService* service, NimBLEUUID charUuid, uint8_t* data, uint16_t* len);;
    String    getStringValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid);
    String    getStringValue(NimBLERemoteService* service, NimBLEUUID charUuid);
    uint16_t  getU16Value(NimBLEUUID serviceUuid, NimBLEUUID charUuid);
    uint16_t  getU16Value(NimBLERemoteService* service, NimBLEUUID charUuid);

    // History stuff
    int       getHistoryByParam(int start, uint16_t count, uint16_t* data, uint8_t param);
    ar4_err_t subscribeHistory(uint8_t* cmd);

    static QueueHandle_t historyQueue;
    static void historyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
};

#endif
