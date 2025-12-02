// ble.h

#ifndef BLE_H
#define BLE_H

#include <time.h>

#include "WCHBLElib/WCHBLElib.h"

#define BLESCAN_TIMEOUT 3
#define BLE_SUCCESS     0
#define BLE_FAILED      1

typedef struct {
    const char *addr;
    const char *name;
    int8_t rssi;
} Device;

void BleAdvertisingDeviceInfo(const char* addr, const char* name, int8_t rssi);
void ConnectionState(WCHBLEHANDLE *connection, int state);
void DisconnectStateCallBack(void *arg);
void versionOfDevice();
void scanDevices();
void listDevices();
void connectToDevice(const char* mac_addr);
void disconnectFromDevice();
void discoverServices();
void discoverCharacteristics(int);
void writeToDevice();

#endif // BLE_H
