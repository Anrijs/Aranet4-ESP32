/*
 *  Name:       Aranet4.h
 *  Created:    2021-04-17
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#ifndef __ARANET4_H
#define __ARANET4_H

#include "Arduino.h"
#include "BLEDevice.h"
#include "esp_gap_bt_api.h"

typedef uint16_t ar4_err_t;

// ar4_err_t Status/Error codes
#define AR4_OK      0       // ar4_err_t value indicating success (no error)
#define AR4_FAIL    -1      // Generic ar4_err_t code indicating failure 

#define AR4_ERR_NO_GATT_SERVICE    0x01
#define AR4_ERR_NO_GATT_CHAR       0x02
#define AR4_ERR_NO_CLIENT          0x03
#define AR4_ERR_NOT_CONNECTED      0x04
#define AR4_ERR_UNAUTHORIZED       0x05

// Aranet4 specific codes
#define AR4_PARAM_TEMPERATURE   1
#define AR4_PARAM_HUMIDITY      2
#define AR4_PARAM_PRESSURE      3
#define AR4_PARAM_CO2           4

#define AR4_NO_DATA_FOR_PARAM   -1

//Subscribe / Aranet service
#define AR4_SUBSCRIBE_HISTORY   0x0032
#define AR4_NOTIFY_HISTORY      0x0031

// Service UUIDs
static BLEUUID UUID_Aranet4  ("f0cd1400-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Generic  ("00001800-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common   ("0000180a-0000-1000-8000-00805f9b34fb");

// Read / Aranet service
static BLEUUID UUID_Aranet4_CurrentReadings     ("f0cd1503-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Aranet4_CurrentReadingsDet  ("f0cd3001-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Aranet4_Interval            ("f0cd2002-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Aranet4_SecondsSinceUpdate  ("f0cd2004-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Aranet4_TotalReadings       ("f0cd2001-95da-4f4b-9ac8-aa55d312af0c");
static BLEUUID UUID_Aranet4_Cmd                 ("f0cd1402-95da-4f4b-9ac8-aa55d312af0c");

// Read / Generic servce
static BLEUUID UUID_Generic_DeviceName ("00002a00-0000-1000-8000-00805f9b34fb");

//Read / Common servce
static BLEUUID UUID_Common_Manufacturer ("00002a29-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common_Model        ("00002a24-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common_Serial       ("00002a25-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common_HwRev        ("00002a27-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common_SwRev        ("00002a28-0000-1000-8000-00805f9b34fb");
static BLEUUID UUID_Common_Battery      ("00002a19-0000-1000-8000-00805f9b34fb");

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


class Aranet4Callbacks : public BLESecurityCallbacks {
private:
    bool conencted = false;
    bool authenticated = false;

    uint32_t onPassKeyRequest() {
        return onPinRequested();
    }

    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
        conencted = true;
        authenticated = auth_cmpl.success;
    }

    // Not required for what we are doing
    void onPassKeyNotify(uint32_t pass_key) { }
    bool onConfirmPIN(uint32_t pass_key) { return false; }
    bool onSecurityRequest() { return true; }

    // Callback when Araner4 requires PIN confirm
    virtual uint32_t onPinRequested() = 0;

public:
    bool    isConnected() { return conencted; }
    bool    isAuthenticated() { return authenticated; }

    virtual void onConnected() = 0;
    virtual void onFailed(uint8_t code) = 0;
    virtual void onDisconnected() = 0;
};

class Aranet4 {
public:
    void      init(Aranet4Callbacks* callbacks);
    ar4_err_t connect(esp_bd_addr_t addr);
    ar4_err_t connect(String addr);
    void      disconnect();

    AranetData  getCurrentReadings();
    uint16_t    getSecondsSinceUpdate();
    uint16_t    getTotalReadings();
    uint16_t    getInterval();
    String      getName();
    String      getVersion();

    ar4_err_t    getStatus();

    bool         isPaired(esp_bd_addr_t addr);
private:
    Aranet4Callbacks* aranetCallbacks = nullptr;
    BLEClient* pClient = nullptr;
    ar4_err_t status = AR4_OK;

    ar4_err_t getValue(BLEUUID serviceUuid, BLEUUID charUuid, uint8_t* data, uint16_t* len);
    String    getStringValue(BLEUUID serviceUuid, BLEUUID charUuid);
    uint16_t  getU16Value(BLEUUID serviceUuid, BLEUUID charUuid);
};

#endif
