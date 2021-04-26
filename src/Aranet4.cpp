/*
 *  Name:       Aranet4.cpp
 *  Created:    2021-04-17
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "Aranet4.h"
#include "Arduino.h"

Aranet4::Aranet4(Aranet4Callbacks* callbacks) {
    // nothing to do
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(callbacks, false);
}

Aranet4::~Aranet4() {
    disconnect();
    NimBLEDevice::deleteClient(pClient);
}

/**
 * @brief Initialize ESP32 bluetooth device and security profile
 * @param [in] cllbacks Pointer to Aranet4Callbacks class callback
 */
void Aranet4::init() {
    // Set up bluetooth device and security
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY);
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] adv Advertised bluetooth device
 * @param [in] secure Start in secure mode (bonded)
 * @return status code
 */
ar4_err_t Aranet4::connect(NimBLEAdvertisedDevice* adv, bool secure) {
    if (pClient != nullptr && pClient->isConnected()) {
        Serial.println("WARNING: Previous connection was not closed and will be disconnected.");
        pClient->disconnect();
    }

    if(pClient->connect(adv)) {
        if (secure) return secureConnection();
        return AR4_OK;
    } else {
        return AR4_ERR_NOT_CONNECTED;
    }

    return AR4_FAIL;
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @param [in] secure Start in secure mode (bonded)
 * @return status code
 */
ar4_err_t Aranet4::connect(NimBLEAddress addr, bool secure) {
    if (pClient != nullptr && pClient->isConnected()) {
        Serial.println("WARNING: Previous connection was not closed and will be disconnected.");
        pClient->disconnect();
    }

    if(pClient->connect(addr)) {
        if (secure) return secureConnection();
        return AR4_OK;
    } else {
        return AR4_ERR_NOT_CONNECTED;
    }

    return AR4_FAIL;
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @param [in] secure Start in secure mode (bonded)
 * @return status code
 */
ar4_err_t Aranet4::connect(uint8_t* addr, uint8_t type, bool secure) {
    return connect(NimBLEAddress(addr, type), secure);
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @param [in] secure Start in secure mode (bonded)
 * @return status code
 */
ar4_err_t Aranet4::connect(String addr, uint8_t type, bool secure) {
    std::string addrstr(addr.c_str());
    return connect(NimBLEAddress(addrstr, type), secure);
}

/**
 * @brief Start secure mode / bond device
 * @return status code
 */
ar4_err_t Aranet4::secureConnection() {
    if (pClient->secureConnection()) {
        return AR4_OK;
    }
    return AR4_FAIL;
}

/**
 * @brief Disconnects from bluetooth device
 */
void Aranet4::disconnect() {
    if (pClient != nullptr && pClient->isConnected()) {
      pClient->disconnect();
    }
}

/**
 * @brief Current readings from Aranet4
 */
AranetData Aranet4::getCurrentReadings() {
    AranetData data;
    uint16_t len = sizeof(AranetData);
    status = getValue(UUID_Aranet4, UUID_Aranet4_CurrentReadingsDet, (uint8_t*) &data, &len);

    return data;
}

/**
 * @brief Seconds since last Aranet4 measurement
 */
uint16_t Aranet4::getSecondsSinceUpdate() {
    return getU16Value(UUID_Aranet4, UUID_Aranet4_SecondsSinceUpdate);
}

/**
 * @brief Total readings stored in Aranet4 memory
 */
uint16_t Aranet4::getTotalReadings() {
    return getU16Value(UUID_Aranet4, UUID_Aranet4_TotalReadings);
}

/**
 * @brief Aranet4 measurement intervals
 */
uint16_t Aranet4::getInterval() {
    return getU16Value(UUID_Aranet4, UUID_Aranet4_Interval);
}

/**
 * @brief Aranet4 device name
 */
String Aranet4::getName() {
    return getStringValue(UUID_Generic, UUID_Generic_DeviceName);
}

/**
 * @brief Aranet4 software version
 */
String Aranet4::getSwVersion() {
    return getStringValue(UUID_Common, UUID_Common_SwRev);
}

/**
 * @brief Aranet4 firmware version
 */
String Aranet4::getFwVersion() {
    return getStringValue(UUID_Common, UUID_Common_FwRev);
}

/**
 * @brief Aranet4 hardware version
 */
String Aranet4::getHwVersion() {
    return getStringValue(UUID_Common, UUID_Common_HwRev);
}

/**
 * @brief Status code of last action
 */
ar4_err_t Aranet4::getStatus() {
    return status;
}

/**
 * @brief Reads raw data from Aranet4
 * @param [in] serviceUuid GATT Service UUID to read
 * @param [in] charUuid GATT Char UUID to read
 * @param [out] data Pointer to where received data will be stored
 * @param [in|out] Size of data on input, received data size on output (truncated if larger than input)
 * @return Read status code (AR4_READ_*)
 */
ar4_err_t Aranet4::getValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid, uint8_t* data, uint16_t* len) {
    if (pClient == nullptr) return AR4_ERR_NO_CLIENT;

    if (!pClient->isConnected())  return AR4_ERR_NOT_CONNECTED;

    NimBLERemoteService* pRemoteService = pClient->getService(serviceUuid);
    if (pRemoteService == nullptr) {
        return AR4_ERR_NO_GATT_SERVICE;
    }

    NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(charUuid);
    if (pRemoteCharacteristic == nullptr) {
        return AR4_ERR_NO_GATT_CHAR;
    }

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
        std::string str = pRemoteCharacteristic->readValue();
        if (str.length() < *len) *len = str.length();
        memcpy(data, str.c_str(), *len);
        return AR4_OK;
    }

    return AR4_FAIL;
}

/**
 * @brief Reads string value from Aranet4
 * @param [in] serviceUuid GATT Service UUID to read
 * @param [in] charUuid GATT Char UUID to read
 * @return String value
 */
String Aranet4::getStringValue(NimBLEUUID serviceUuid, NimBLEUUID charUuid) {
    uint8_t buf[33];
    uint16_t len = 32;
    status = getValue(serviceUuid, charUuid, buf, &len);
    buf[len] = 0; // trerminate string
    return String((char *) buf);
}

/**
 * @brief Reads u16 value from Aranet4
 * @param [in] serviceUuid GATT Service UUID to read
 * @param [in] charUuid GATT Char UUID to read
 * @return u16 value
 */
uint16_t Aranet4::getU16Value(NimBLEUUID serviceUuid, NimBLEUUID charUuid) {
    uint16_t val = 0;
    uint16_t len = 2;
    status = getValue(serviceUuid, charUuid, (uint8_t *) &val, &len);

    if (len == 2) {
        return val;
    }

    status = AR4_FAIL;
    return 0;
}
