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

#define AR4_NO_DATA_FOR_PARAM   -1

// Service UUIDs
static NimBLEUUID UUID_Aranet4  ("f0cd1400-95da-4f4b-9ac8-aa55d312af0c");
static NimBLEUUID UUID_Generic  ("1800");
static NimBLEUUID UUID_Common   ("180a");

// Read / Aranet service
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
    uint16_t co2 = 0;
    uint16_t temperature = 0;
    uint16_t pressure = 0;
    uint8_t  humidity = 0;
    uint8_t  battery = 0;
    uint8_t  unkn = 0;
    uint16_t interval = 0;
    uint16_t ago = 0;
};
#pragma pack(pop)

// Small version of AranetData for history
typedef struct AranetDataCompact {
    uint16_t co2 = 0;
    uint16_t temperature = 0;
    uint16_t pressure = 0;
    uint8_t  humidity = 0;
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
    static void init();
    ar4_err_t connect(NimBLEAdvertisedDevice* adv, bool secure = true);
    ar4_err_t connect(NimBLEAddress addr, bool secure = true);
    ar4_err_t connect(uint8_t* addr, uint8_t type = BLE_ADDR_RANDOM, bool secure = true);
    ar4_err_t connect(String addr, uint8_t type = BLE_ADDR_RANDOM, bool secure = true);
    ar4_err_t secureConnection();
    void      disconnect();

    AranetData  getCurrentReadings();
    uint16_t    getSecondsSinceUpdate();
    uint16_t    getTotalReadings();
    uint16_t    getInterval();
    String      getName();
    String      getSwVersion();
    String      getFwVersion();
    String      getHwVersion();

    ar4_err_t   writeCmd(uint8_t* data, uint16_t len);

    int         getHistoryCO2(uint16_t start, uint16_t count, uint16_t* data);
    int         getHistoryTemperature(uint16_t start, uint16_t count, uint16_t* data);
    int         getHistoryPressure(uint16_t start, uint16_t count, uint16_t* data);
    int         getHistoryHumidity(uint16_t start, uint16_t count, uint16_t* data);
    int         getHistory(uint16_t start, uint16_t count, AranetDataCompact* data);
    ar4_err_t   getStatus();
private:
    NimBLEClient* pClient = nullptr;
    ar4_err_t status = AR4_OK;

    ar4_err_t getValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid, uint8_t* data, uint16_t* len);
    String    getStringValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid);
    uint16_t  getU16Value(NimBLEUUID serviceUuid, NimBLEUUID charUuid);

    // History stuff
    int       getHistoryByParam(uint16_t start, uint16_t count, uint16_t* data, uint8_t param);
    ar4_err_t subscribeHistory(uint8_t* cmd);

    static QueueHandle_t historyQueue;
    static void historyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
};

#endif
