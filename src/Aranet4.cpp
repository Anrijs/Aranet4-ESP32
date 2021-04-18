/*
 *  Name:       Aranet4.cpp
 *  Created:    2021-04-17
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "aranet4.h"
#include "Arduino.h"

/**
 * @brief Initialize ESP32 bluetooth device and security profile
 * @param [in] cllbacks Pointer to Aranet4Callbacks class callback
 */
void Aranet4::init(Aranet4Callbacks* callbacks) {
    aranetCallbacks = callbacks;

    // Set up bluetooth device and security
    BLEDevice::init("");
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
    BLEDevice::setSecurityCallbacks(aranetCallbacks);

    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setKeySize();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setCapability(ESP_IO_CAP_IN);
    pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

/**
 * @brief Check if device is paired
 * @param [in] addr Address of bluetooth device
 * @return True if device is paired
 */
bool Aranet4::isPaired(esp_bd_addr_t addr) {
    int count = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t* devList = (esp_ble_bond_dev_t*) malloc(count * sizeof(esp_ble_bond_dev_t));
    esp_err_t status = esp_ble_get_bond_device_list(&count, devList);

    if (status != ESP_OK) {
        Serial.println("Aranet4: Failed to get bonded device list");
        return false;
    }

    for (int devId=0; devId<count; devId++) {
        esp_ble_bond_dev_t paired = devList[devId];
        if (memcmp(addr, paired.bd_addr, ESP_BD_ADDR_LEN) == 0) return true;
    }

    return false;
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @return Connection status code (AR4_CONN_*)
 */
ar4_err_t Aranet4::connect(esp_bd_addr_t addr) {
    pClient = BLEDevice::createClient();
    aranetClientCallbacks = new Aranet4ClientCallbacks();
    pClient->setClientCallbacks(aranetClientCallbacks);

    bool stat = pClient->connect(addr, BLE_ADDR_TYPE_RANDOM);

    if (!stat) {
        aranetCallbacks->onFailed(AR4_ERR_NOT_CONNECTED);
        return AR4_ERR_NOT_CONNECTED;
    }

    long timeout = millis() + 5000;

    while (!aranetClientCallbacks->isConnected()) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (millis() > timeout) {
            return AR4_ERR_TIMEOUT;
        }
    }

    aranetCallbacks->onConnected();
    return AR4_OK;
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @return Connection status code (AR4_CONN_*)
 */
ar4_err_t Aranet4::connect(String addr) {
    BLEAddress bleAddr = BLEAddress(addr.c_str());
    esp_bd_addr_t* native = bleAddr.getNative();

    return connect(*bleAddr.getNative());
}

/**
 * @brief Disconnects from bluetooth device
 */
void Aranet4::disconnect() {
    if (pClient != nullptr) {
        // Without delay, there could be crash
        vTaskDelay(500 / portTICK_PERIOD_MS);
        pClient->disconnect();
    }

    aranetCallbacks->onDisconnected();
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
String Aranet4::getVersion() {
    return getStringValue(UUID_Common, UUID_Common_SwRev);
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
ar4_err_t Aranet4::getValue(BLEUUID serviceUuid, BLEUUID charUuid, uint8_t* data, uint16_t* len) {
    if (pClient == nullptr) return AR4_ERR_NO_CLIENT;

    if (!aranetClientCallbacks->isConnected())  return AR4_ERR_NOT_CONNECTED;

    BLERemoteService* pRemoteService = pClient->getService(serviceUuid);
    if (pRemoteService == nullptr) {
        return AR4_ERR_NO_GATT_SERVICE;
    }

    BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(charUuid);
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
String Aranet4::getStringValue(BLEUUID serviceUuid, BLEUUID charUuid) {
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
uint16_t Aranet4::getU16Value(BLEUUID serviceUuid, BLEUUID charUuid) {
    uint16_t val = 0;
    uint16_t len = 2;
    status = getValue(serviceUuid, charUuid, (uint8_t *) &val, &len);

    if (len == 2) {
        return val;
    }

    status = AR4_FAIL;
    return 0;
}