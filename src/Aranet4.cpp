/*
 *  Name:       Aranet4.cpp
 *  Created:    2021-04-17
 *  Author:     Anrijs Jargans <anrijs@anrijs.lv>
 *  Url:        https://github.com/Anrijs/Aranet4-ESP32
 */

#include "Aranet4.h"
#include "Arduino.h"

// Queue to store history data
QueueHandle_t Aranet4::historyQueue = xQueueCreate(120, sizeof(uint16_t));

Aranet4::Aranet4(Aranet4Callbacks* callbacks) {
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
void Aranet4::init(uint16_t mtu) {
    // Set up bluetooth device and security
    NimBLEDevice::init("");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_KEYBOARD_ONLY);
    NimBLEDevice::setMTU(mtu);
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
 * @param [in] type Address type
 * @return status code
 */
ar4_err_t Aranet4::connect(uint8_t* addr, bool secure, uint8_t type) {
    return connect(NimBLEAddress(addr, type), secure);
}

/**
 * @brief Connect to Aranet4 device
 * @param [in] addr Address of bluetooth device
 * @param [in] secure Start in secure mode (bonded)
 * @param [in] type Address type
 * @return status code
 */
ar4_err_t Aranet4::connect(String addr, bool secure, uint8_t type) {
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
 * @brief Set the timeout to wait for connection attempt to complete
 */
void Aranet4::setConnectTimeout(uint8_t time) {
    if (pClient != nullptr) {
      pClient->setConnectTimeout(time);
    }
}

/**
 * @brief Are we connected to a server?
 */
bool Aranet4::isConnected() {
    return pClient != nullptr && pClient->isConnected();
}

/**
 * @brief Current readings from Aranet4
 */
AranetData Aranet4::getCurrentReadings() {
    AranetData data;
    AranetType type = getType();
    uint8_t raw[100];
    uint16_t len = 100;

    switch (type) {
    case ARANET4:
        status = getValue(getAranetService(), UUID_Aranet4_CurrentReadingsDet, raw, &len);
        break;
    case ARANET2:
    case ARANET_RADIATION:
    case ARANET_RADON:
        status = getValue(getAranetService(), UUID_Aranet2_CurrentReadings, raw, &len);
        break;
    default:
        status = AR4_FAIL;
        break;
    }

    if (status == AR4_OK)
        status = data.parseFromGATT(raw, len, type);

    return data;
}

/**
 * @brief Seconds since last Aranet4 measurement
 */
uint16_t Aranet4::getSecondsSinceUpdate() {
    return getU16Value(getAranetService(), UUID_Aranet4_SecondsSinceUpdate);
}

/**
 * @brief Total readings stored in Aranet4 memory
 */
uint16_t Aranet4::getTotalReadings() {
    return getU16Value(getAranetService(), UUID_Aranet4_TotalReadings);
}

/**
 * @brief Aranet4 measurement intervals
 */
uint16_t Aranet4::getInterval() {
    return getU16Value(getAranetService(), UUID_Aranet4_Interval);
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

AranetType Aranet4::getType() {
    String name = getName();
    char c0 = name.charAt(6);
    char c1 = name.charAt(7);
    char c2 = name.charAt(8);

    if (c0 == '4') return ARANET4;
    if (c0 == '2') return ARANET2;
    if (c0 == (char) 0xE2 && c1 == (char) 0x98 && c2 == (char) 0xA2) return ARANET_RADIATION;
    if (c0 == 'R' && c1 == 'n') return ARANET_RADON;

    return UNKNOWN;
}

/**
 * @brief Check is is Aranet2
 */
bool Aranet4::isAranet2() {
    return getType() == ARANET2;
}

/**
 * @brief Check is is Aranet4
 */
bool Aranet4::isAranet4() {
    return getType() == ARANET4;
}

/**
 * @brief Check is is Aranet Radiation
 */
bool Aranet4::isAranetRadiation() {
    return getType() == ARANET_RADIATION;
}
/**
 * @brief Check is is Aranet Radon
 */
bool Aranet4::isAranetRadon() {
    getType() == ARANET_RADON;
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
    return getValue(pClient->getService(serviceUuid), charUuid, data, len);
}

/**
 * @brief Reads raw data from Aranet4
 * @param [in] service GATT Service to read
 * @param [in] charUuid GATT Char UUID to read
 * @param [out] data Pointer to where received data will be stored
 * @param [in|out] Size of data on input, received data size on output (truncated if larger than input)
 * @return Read status code (AR4_READ_*)
 */
ar4_err_t Aranet4::getValue(NimBLERemoteService* service, NimBLEUUID charUuid, uint8_t* data, uint16_t* len) {
    if (pClient == nullptr) return AR4_ERR_NO_CLIENT;
    if (!pClient->isConnected())  return AR4_ERR_NOT_CONNECTED;
    if (service == nullptr) return AR4_ERR_NO_GATT_SERVICE;

    NimBLERemoteCharacteristic* pRemoteCharacteristic = service->getCharacteristic(charUuid);
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
    return getStringValue(pClient->getService(serviceUuid), charUuid);
}

/**
 * @brief Reads string value from Aranet4
 * @param [in] service GATT Service to read
 * @param [in] charUuid GATT Char UUID to read
 * @return String value
 */
String Aranet4::getStringValue(NimBLERemoteService* service, NimBLEUUID charUuid) {
    uint8_t buf[33];
    uint16_t len = 32;
    status = getValue(service, charUuid, buf, &len);
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
    return getU16Value(pClient->getService(serviceUuid), charUuid);
}

/**
 * @brief Reads u16 value from Aranet4
 * @param [in] service GATT Service to read
 * @param [in] charUuid GATT Char UUID to read
 * @return u16 value
 */
uint16_t Aranet4::getU16Value(NimBLERemoteService* service, NimBLEUUID charUuid) {
    uint16_t val = 0;
    uint16_t len = 2;
    status = getValue(service, charUuid, (uint8_t *) &val, &len);

    if (len == 2) {
        return val;
    }

    status = AR4_FAIL;
    return 0;
}

/**
 * @brief Writes command to Aranet
 * @param [in] data Command data
 * @param [in] len Command data length
 * @return write status
 */
ar4_err_t Aranet4::writeCmd(uint8_t* data, uint16_t len) {
    if (pClient == nullptr) return AR4_ERR_NO_CLIENT;
    if (!pClient->isConnected())  return AR4_ERR_NOT_CONNECTED;

    NimBLERemoteService* pRemoteService = getAranetService();
    if (pRemoteService == nullptr) {
        return AR4_ERR_NO_GATT_SERVICE;
    }

    NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(UUID_Aranet4_Cmd);
    if (pRemoteCharacteristic == nullptr) {
        return AR4_ERR_NO_GATT_CHAR;
    }

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canWrite()) {
        if (pRemoteCharacteristic->writeValue(data, len, true)) return AR4_OK;
    }
    return AR4_FAIL;
}

/**
 * @brief Callback for history subscriptions. This will store received data in historyQueue
 */
void Aranet4::historyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    uint8_t param = pData[0];
    uint16_t idx = pData[1] + (pData[2] << 8) - 1;
    uint8_t count = pData[3];
    uint16_t pos = 4;

    while (count > 0 && pos < length) {
        uint16_t val = pData[pos++];

        if (param != AR4_PARAM_HUMIDITY) {
            val += (pData[pos++] << 8);
        }

        xQueueSend(historyQueue, &val, portMAX_DELAY);
        count -= 1;
    }
}

NimBLERemoteService* Aranet4::getAranetService() {
    NimBLERemoteService* pRemoteService = pClient->getService(UUID_Aranet4);
    if (pRemoteService == nullptr) {
        pRemoteService = pClient->getService(UUID_Aranet4_Old);
    }
    return pRemoteService;
}

/**
 * @brief Subscribe and request history
 * @param [in] cmd Command data. Must be 8 bytes.
 * @return subscription status
 */
ar4_err_t Aranet4::subscribeHistory(uint8_t* cmd) {
    if (pClient == nullptr) return AR4_ERR_NO_CLIENT;
    if (!pClient->isConnected())  return AR4_ERR_NOT_CONNECTED;

    NimBLERemoteService* pRemoteService = getAranetService();
    if (pRemoteService == nullptr) {
         Serial.println("NO SERVC");
        return AR4_ERR_NO_GATT_SERVICE;
    }

    NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(UUID_Aranet4_Notify_History);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("NO CHAR");
        return AR4_ERR_NO_GATT_CHAR;
    }

    pRemoteCharacteristic->unsubscribe(false);

    if (writeCmd(cmd, 8) != AR4_OK) {
        return AR4_FAIL;
    }

    if (pRemoteCharacteristic->subscribe(true, historyCallback)) {
        return AR4_OK;
    }

    return AR4_FAIL;
}

/**
 * @brief Reads history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @param [in] param PArameter to fetch
 * @return Received point count
 */
int Aranet4::getHistoryByParamV1(int start, uint16_t count, uint16_t* data, uint8_t param) {
    if (start < 1) start = 1;

    // id 1 is oldest
    uint16_t end = start + (count - 1);
    uint8_t cmd[] = {0x82,param,0x00,0x00,0x01,0x00,0x01,0x00};

    memcpy(&cmd[4], (unsigned char*) &start, 2);
    memcpy(&cmd[6], (unsigned char*) &end,   2);

    status = subscribeHistory(cmd);
    if (status != AR4_OK) return 0;

    // wait for queue
    uint16_t recvd = 0;
    while (recvd < count) {
        if (!xQueueReceive(historyQueue, &data[recvd], 500 / portTICK_PERIOD_MS)) {
            Serial.printf("History queue timeout. Received %i, Expected: %i\n",recvd, count);
            break;
        }
        recvd++;
    }
    uint16_t tmp = 0;
    while (xQueueReceive(historyQueue, &tmp, 100 / portTICK_PERIOD_MS)) {
        // wait till done
    }
    xQueueReset(historyQueue);
    return recvd;
}

/**
 * @brief Reads CO2 history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count
 */
int Aranet4::getHistoryCO2(int start, uint16_t count, uint16_t* data) {
    return getHistoryByParamV1(start, count, data, AR4_PARAM_CO2);
}

/**
 * @brief Reads Temperature history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count
 */
int Aranet4::getHistoryTemperature(int start, uint16_t count, uint16_t* data) {
    return getHistoryByParamV1(start, count, data, AR4_PARAM_TEMPERATURE);
}

/**
 * @brief Reads Pressure history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count
 */
int Aranet4::getHistoryPressure(int start, uint16_t count, uint16_t* data) {
    return getHistoryByParamV1(start, count, data, AR4_PARAM_PRESSURE);
}

/**
 * @brief Reads Humidity history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count
 */
int Aranet4::getHistoryHumidity(int start, uint16_t count, uint16_t* data) {
    return getHistoryByParamV1(start, count, data, AR4_PARAM_HUMIDITY);
}

/**
 * @brief Reads precise Humidity (.1 step) history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count
 */
int Aranet4::getHistoryHumidity2(int start, uint16_t count, uint16_t* data) {
    return getHistoryByParamV1(start, count, data, AR4_PARAM_HUMIDITY2);
}

/**
 * @brief Reads all history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count (smallest)
 */
int Aranet4::getHistoryV1(int start, uint16_t count, AranetDataCompact* data, uint8_t flags) {
    uint16_t* temp = (uint16_t*) malloc(count * sizeof(uint16_t));
    int ret = count;
    int result = 0;

    if (flags & AR4_PARAM_CO2_FLAG) {
        result = getHistoryCO2(start, count, temp);
        for (int i = 0; i < result; i++) data[i].aranet4.co2 = temp[i];
        if (result < ret) ret = result;
    }

    if (flags & AR4_PARAM_TEMPERATURE_FLAG) {
        result = getHistoryTemperature(start, count, temp);
        for (int i = 0; i < result; i++) data[i].aranet4.temperature = temp[i];
        if (result < ret) ret = result;
    }

    if (flags & AR4_PARAM_PRESSURE_FLAG) {
        result = getHistoryPressure(start, count, temp);
        for (int i = 0; i < result; i++) data[i].aranet4.pressure = temp[i];
        if (result < ret) ret = result;
    }

    if (flags & AR4_PARAM_HUMIDITY2_FLAG) {
        result = getHistoryHumidity2(start, count, temp);
        for (int i = 0; i < result; i++) data[i].aranet4.humidity = temp[i];
        if (result < ret) ret = result;
    } else if (flags & AR4_PARAM_HUMIDITY_FLAG) {
        result = getHistoryHumidity(start, count, temp);
        for (int i = 0; i < result; i++) data[i].aranet4.humidity = temp[i];
        if (result < ret) ret = result;
    }

    free(temp);

    return ret;
}

/**
 * @brief Reads all history data in to array
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count (smallest)
 */
int Aranet4::getHistoryV2(uint16_t start, uint16_t count, AranetDataCompact* data, uint16_t params) {
    int ret = count;
    int result = 0;

    for (uint16_t param = 1; param < AR4_PARAM_MAX; param++) {
        uint16_t mask = 1 << (param - 1);
        if (params & mask) {
            result = getHistoryChunk(start, count, data, param);
            if (result < ret) ret = result;
        }
    }

    return ret;
}

int Aranet4::getHistoryChunk(uint16_t start, uint8_t count, AranetDataCompact* data, uint8_t param) {
    int end = start + count;
    int pos = 0;
    AranetHistoryHeader hdr;
    uint8_t buffer[256];
    uint16_t len = 256;

    uint8_t flen = 2;

    switch (param){
    case AR4_PARAM_HUMIDITY:
        flen = 1;
        break;
    case AR4_PARAM_RADIATION_DOSE:
    case AR4_PARAM_RADIATION_DOSE_RATE:
        flen = 3;
        break;
    case AR4_PARAM_RADON_CONCENTRATION:
        flen = 4;
        break;
    case AR4_PARAM_RADIATION_DOSE_INTEGRAL:
        flen = 8;
        break;
    }

    while (start < end) {
        buffer[0] = 0x61;              // command
        buffer[1] = param;             // parameter
        memcpy(buffer + 2, &start, 2); // start addr

        // write cmd
        status = writeCmd(buffer, 4);

        if (status != AR4_OK) {
            Serial.println("History CMD failed");
            return -1;
        }

        // read history data
        status = getValue(getAranetService(), UUID_Aranet4_History, buffer, &len);

        if (status != AR4_OK || len < sizeof(AranetHistoryHeader)) {
            Serial.println("History Read failed");
            return -1;
        }

        // process history data
        memcpy(&hdr, buffer, sizeof(AranetHistoryHeader));
        uint8_t* histptr = buffer + 10;
        uint8_t i = 0; // record id

        uint64_t val = 0;
        while (start < end && i < hdr.count) {
            memcpy(&val, histptr, flen);
            data[pos].set(param, val);
            histptr += flen;
            start++; i++; pos++;
        }
    }    

    return pos;
}

/**
 * @brief Reads all history data in to array (autodetect v1 or v2)
 * @param [in] start Start index
 * @param [in] count Data points to read
 * @param [out] data Pointer to data array, whre results will be stored
 * @return Received point count (smallest)
 */
int Aranet4::getHistory(uint16_t start, uint16_t count, AranetDataCompact* data, uint16_t params) {
    NimBLERemoteService* pRemoteService = getAranetService();
    if (pRemoteService == nullptr) {
        return 0;
    }

    NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(UUID_Aranet4_History);
    if (pRemoteCharacteristic) {
        return getHistoryV2(start, count, data, params);
    }
    return getHistoryV1(start, count, data, params);
}
