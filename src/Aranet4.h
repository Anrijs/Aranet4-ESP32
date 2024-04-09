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
#define AR4_PARAM_TEMPERATURE              1
#define AR4_PARAM_HUMIDITY                 2
#define AR4_PARAM_PRESSURE                 3
#define AR4_PARAM_CO2                      4
#define AR4_PARAM_HUMIDITY2                5
#define AR4_PARAM_RADIATION_PULSES         6
#define AR4_PARAM_RADIATION_DOSE           7
#define AR4_PARAM_RADIATION_DOSE_RATE      8
#define AR4_PARAM_RADIATION_DOSE_INTEGRAL  9
#define AR4_PARAM_MAX                      10

#define AR4_NO_DATA_FOR_PARAM   -1

// Aranet param flags
#define AR4_PARAM_TEMPERATURE_FLAG   1 << (AR4_PARAM_TEMPERATURE - 1)
#define AR4_PARAM_HUMIDITY_FLAG      1 << (AR4_PARAM_HUMIDITY - 1)
#define AR4_PARAM_PRESSURE_FLAG      1 << (AR4_PARAM_PRESSURE - 1)
#define AR4_PARAM_CO2_FLAG           1 << (AR4_PARAM_CO2 - 1)
#define AR4_PARAM_HUMIDITY2_FLAG     1 << (AR4_PARAM_HUMIDITY2 - 1)
#define AR4_PARAM_RADIATION_PULSES_FLAG          1 << (AR4_PARAM_RADIATION_PULSES - 1)
#define AR4_PARAM_RADIATION_DOSE_FLAG            1 << (AR4_PARAM_RADIATION_DOSE - 1)
#define AR4_PARAM_RADIATION_DOSE_RATE_FLAG       1 << (AR4_PARAM_RADIATION_DOSE_RATE - 1)
#define AR4_PARAM_RADIATiON_DOSE_INTEGRAL_FLAG   1 << (AR4_PARAM_RADIATION_DOSE_INTEGRAL - 1)


#define AR2_PARAM_FLAGS  AR4_PARAM_TEMPERATURE_FLAG | AR4_PARAM_HUMIDITY2_FLAG
#define AR4_PARAM_FLAGS  AR4_PARAM_TEMPERATURE_FLAG | AR4_PARAM_HUMIDITY_FLAG | AR4_PARAM_PRESSURE_FLAG | AR4_PARAM_CO2_FLAG
#define ARR_PARAM_FLAGS  AR4_PARAM_RADIATION_DOSE_RATE_FLAG | AR4_PARAM_RADIATiON_DOSE_INTEGRAL_FLAG

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
static NimBLEUUID UUID_Aranet4_History             ("f0cd2005-95da-4f4b-9ac8-aa55d312af0c");
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

enum AranetType {
    ARANET4 = 0,
    ARANET2 = 1,
    ARANET_RADIATION = 2,
    UNKNOWN = 255
};

#pragma pack(push, 1)
typedef struct AranetData {
    AranetType type = UNKNOWN;
    uint16_t co2 = 0;
    uint16_t temperature = 0;
    uint16_t pressure = 0;
    uint16_t humidity = 0;
    uint8_t  battery = 0;
    uint8_t  status = 0;
    uint16_t interval = 0;
    uint16_t ago = 0;

    uint32_t radiation_pulses = 0;
    uint32_t radiation_rate = 0;
    uint64_t radiation_total = 0;
    uint64_t radiation_duration = 0;

    bool parseFromAdvertisement(uint8_t* data, AranetType type) {
        int dp = 0;

        this->type = type;
        switch (type) {
        case ARANET4:
            memcpy(&co2,         (uint8_t*) data + 8, 2);
            memcpy(&temperature, (uint8_t*) data + 10, 2);
            memcpy(&pressure,    (uint8_t*) data + 12, 2);
            memcpy(&interval,    (uint8_t*) data + 17, 2);
            memcpy(&ago,         (uint8_t*) data + 19, 2);
            humidity = data[14];
            battery = data[15];
            status = data[16];
            return true;
        case ARANET2:
            memcpy(&temperature, (uint8_t*) data + 10, 2);
            memcpy(&humidity,    (uint8_t*) data + 14, 2);
            memcpy(&interval,    (uint8_t*) data + 19, 2);
            memcpy(&ago,         (uint8_t*) data + 21, 2);
            battery = data[17];
            status = data[18];
            return true;
        case ARANET_RADIATION:
            // Preclear. Advertisement uses smaller datatypes than GATT
            radiation_rate = 0;
            radiation_total = 0;
            radiation_duration = 0;

            memcpy(&radiation_total,    (uint8_t*) data + 6, 4);
            memcpy(&radiation_duration, (uint8_t*) data + 10, 4);
            memcpy(&radiation_rate,     (uint8_t*) data + 14, 2);
            battery = data[17];
            memcpy(&interval,    (uint8_t*) data + 21, 2);
            memcpy(&ago,         (uint8_t*) data + 19, 2);
            return true;
        }

        // bad type
        return false;
    }

    ar4_err_t parseFromGATT(uint8_t* data, AranetType type) {
        this->type = type;

        switch (type) {
        case ARANET4:
            memcpy(&co2,         (uint8_t*) data + 0, 2);
            memcpy(&temperature, (uint8_t*) data + 2, 2);
            memcpy(&pressure,    (uint8_t*) data + 4, 2);
            memcpy(&interval,    (uint8_t*) data + 9, 2);
            memcpy(&ago,         (uint8_t*) data + 11, 2);

            humidity = data[6];
            battery = data[7];
            status = data[8];

            return AR4_OK;
        case ARANET2:
            memcpy(&temperature, (uint8_t*) data + 7, 2);
            memcpy(&humidity,    (uint8_t*) data + 9, 2);
            memcpy(&interval,    (uint8_t*) data + 2, 2);
            memcpy(&ago,         (uint8_t*) data + 4, 2);

            battery = data[6];
            status = data[11];

            return AR4_OK;
        case ARANET_RADIATION:
            memcpy(&interval,    (uint8_t*) data + 2, 2);
            memcpy(&ago,         (uint8_t*) data + 4, 2);
            battery = data[6];

            memcpy(&radiation_rate, (uint8_t*) data + 7, 4);
            memcpy(&radiation_total, (uint8_t*) data + 11, 8);
            memcpy(&radiation_duration, (uint8_t*) data + 19, 8);
            status = data[27];

            return AR4_OK;
        }

        // bad type
        return AR4_FAIL;
    }

    uint16_t getCO2() {
        if (type == ARANET4) return co2;
        return -1;
    }

    float getTemperature() {
        if (type == ARANET4 || type == ARANET2) return temperature / 20.0;
        return -1.0;
    }

    float getPressure() {
        if (type == ARANET4) return pressure / 10.0;
        return -1.0;
    }

    float getHumidity() {
        if (type == ARANET2) return humidity / 10.0;
        if (type == ARANET4) return humidity;
        return -1.0;
    }

    uint32_t getRadiationRate() {
        if (type == ARANET_RADIATION) return radiation_rate;
        return 0;
    }

    uint64_t getRadiationTotal() {
        if (type == ARANET_RADIATION) return radiation_total;
        return 0;
    }

    uint64_t getRadiationDuration() {
        if (type == ARANET_RADIATION) return radiation_duration;
        return 0;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct AranetManufacturerData {
    uint16_t manufacturer_id;
    union {
        uint8_t all;
        struct {
            uint8_t disconnected : 1;
            uint8_t __unknown1   : 1;
            uint8_t calib_state  : 2;
            uint8_t dfu_mode     : 1;
            uint8_t integrations : 1;
            uint8_t __unknown2   : 2;
        } bits;
    } flags;
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

        if (cLength < 8) return false; // not enough data
        if (cLength > 100) cLength = 50; // trim

        uint8_t cManufacturerData[50];
        strManufacturerData.copy((char *) &manufacturer_id, 2, 0);
        strManufacturerData.copy((char *) cManufacturerData, cLength, 2);

        // check manufacturer id
        if(manufacturer_id != ARANET4_MANUFACTURER_ID) return false;

        int idx = 1;
        // TODO: Check by name
        if (cLength == 9 || cLength == 24) {
            idx = 0;
            data.type = AranetType::ARANET4;
        } else if (cManufacturerData[0] == 1) {
            data.type = AranetType::ARANET2;
        } else if (cManufacturerData[0] == 2) {
            data.type = AranetType::ARANET_RADIATION;
        } else {
            data.type = AranetType::UNKNOWN;
            return false;
        }

        this->flags.all = cManufacturerData[idx + 0];
        this->version.patch = cManufacturerData[idx + 1];
        this->version.minor = cManufacturerData[idx + 2];
        this->version.major = cManufacturerData[idx + 3] | (cManufacturerData[idx + 4] << 8);
        this->hw_rev = cManufacturerData[idx + 4];
        this->__unknown3 = cManufacturerData[idx + 6];
        this->packing = cManufacturerData[idx + 7];

        if (this->flags.bits.integrations) {
            data.parseFromAdvertisement(cManufacturerData, data.type);
        }

        return true;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct AranetHistoryHeader {
    uint8_t  param = 0;
    uint16_t interval = 0;
    uint16_t total_readings = 0;
    uint16_t ago = 0;
    uint16_t start = 0;
    uint8_t count = 0;
};
#pragma pack(pop)

// Small version of AranetData for history
typedef union AranetDataCompact {
    struct {
        uint16_t co2;
        uint16_t temperature;
        uint16_t pressure;
        uint16_t humidity;
        uint32_t _reserved;
    } aranet4;
    struct  {
        uint16_t rad_pulses;
        uint16_t rad_dose;
        uint16_t rad_dose_rate;
        uint64_t rad_dose_integral;
    } aranetr;

    void set(uint8_t param, uint64_t value) {
        switch (param) {
        case AR4_PARAM_TEMPERATURE:
            aranet4.temperature = value; break;
        case AR4_PARAM_HUMIDITY:
        case AR4_PARAM_HUMIDITY2:
            aranet4.humidity = value; break;
        case AR4_PARAM_PRESSURE:
            aranet4.pressure = value; break;
        case AR4_PARAM_CO2:
            aranet4.co2 = value; break;
        case AR4_PARAM_RADIATION_PULSES:
            aranetr.rad_pulses = value; break;
        case AR4_PARAM_RADIATION_DOSE:
            aranetr.rad_dose = value; break;
        case AR4_PARAM_RADIATION_DOSE_RATE:
            aranetr.rad_dose_rate = value; break;
        case AR4_PARAM_RADIATION_DOSE_INTEGRAL:
            aranetr.rad_dose_integral = value; break;
        }
    }
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
    int         getHistory(uint16_t start, uint16_t count, AranetDataCompact* data, uint16_t params = AR4_PARAM_FLAGS);
    int         getHistoryV1(int start, uint16_t count, AranetDataCompact* data, uint8_t params = AR4_PARAM_FLAGS);
    int         getHistoryV2(uint16_t start, uint16_t count, AranetDataCompact* data, uint16_t params = AR4_PARAM_FLAGS);
    ar4_err_t   getStatus();

    AranetType getType();

    bool isAranet4();
    bool isAranet2();
    bool isAranetRadiation();
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
    int       getHistoryByParamV1(int start, uint16_t count, uint16_t* data, uint8_t param);
    int       getHistoryByParamV2(uint16_t start, uint16_t count, AranetDataCompact* data, size_t size, uint8_t param);
    int       getHistoryChunk(uint16_t start, uint8_t count, AranetDataCompact* data, uint8_t param);
    ar4_err_t subscribeHistory(uint8_t* cmd);

    static QueueHandle_t historyQueue;
    static void historyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
};

#endif
