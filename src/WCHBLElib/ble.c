// ble.c

#include "ble.h"
#include "common.h"

WCHBLEHANDLE *connection = NULL;
Device *devices = NULL;
GattPrimaryService services[10] = { 0 };
GattCharacteristic characteristics[200] = { 0 };
int devices_count = 0;
int services_count = 0;
int characteristics_count = 0;

void scanDevices() {
    printf("==> List devices:\n");
    printf("::> Only show devices with OUI: 50:54:7B\n");

    int saved_stdout = dup(STDOUT_FILENO);
    suppress_stdout();
    int ret = WCHBle_BLE_Scan(BLESCAN_TIMEOUT, BleAdvertisingDeviceInfo);
    restore_stdout(saved_stdout);

    if (ret == BLE_FAILED) {
        printf("  > Fail to discover primary services, abort.\n");
        exit(1);
    }
}

void add_device(const char *addr, const char *name, int8_t rssi) {
    devices = (Device *)realloc(devices, (devices_count + 1) * sizeof(Device));
    
    if (devices == NULL) {
        printf("  > Error al redimensionar memoria, abort.\n");
        exit(1);
    }

    devices[devices_count].addr = strdup(addr);
    devices[devices_count].name = strdup(name);
    devices[devices_count].rssi = rssi;

    devices_count++;
}

void listDevices() {
    for (int i = 0; i < devices_count; i++) {
        printf("--> MAC: %s Name: %s RSSI: %d dBm\n", devices[i].addr, devices[i].name, devices[i].rssi);
    }
}

void connectToDevice(const char* mac_addr) {
    printf("==> Connecting to device: %s\n", mac_addr);
    connection = WCHBle_Connect(mac_addr, ConnectionState);

    if (connection) {
        WCHBle_register_on_disconnect(connection,DisconnectStateCallBack);
    }
}

void disconnectFromDevice() {
    if (connection) {
        WCHBle_Disconnect(connection);
        printf("==> Disconnect.\n");
    }
}

void BleAdvertisingDeviceInfo(const char *addr, const char *name, int8_t rssi) {
    char OUI[8];

    strncpy(OUI,addr,8);

    if (strcmp("50:54:7B",OUI) == 0) {
        add_device(addr, name, rssi);
    }
}

void ConnectionState(WCHBLEHANDLE *connection, int state) {
    if (connection)
        printf("==> Connect OK!\n");
    else {
        printf("  > Connect failed!\n");
        exit(1);
    }
}

void DisconnectStateCallBack(void *arg) {
    disconnectFromDevice();
}

void versionOfDevice() {
    const char *VER_ARRAY[] = {"V1.0", "V1.1", "V1.2", "V2.0", "V2.1", "V3.0", "V4.0", "V4.1", "V4.2", "V5.0", "V5.1", "V5.2"};
    int saved_stdout = dup(STDOUT_FILENO);
    suppress_stdout();
    int ver = WCHBLEGetBluetoothVer();
    restore_stdout(saved_stdout);

    if(ver >= 0 && ver < sizeof(VER_ARRAY)/sizeof(VER_ARRAY[0]))
        printf("==> Device version: %s\n", VER_ARRAY[ver]);
    else
        printf("==> Device version code: %d\n", ver);
}


void discoverServices() {
    int ret = WCHBle_Discover_Primary(connection, services, &services_count);
    printf("==> Services: %d\n", services_count);
    
    if (services_count == 0) {
        printf("  > Services is empty, abort.\n");
        return;
    }
    
    if (ret != BLE_SUCCESS) {
        printf("  > Fail to discover primary services, abort.\n");
        return;
    }

    for (int i = 0; i < services_count; i++) {
        discoverCharacteristics(i);
    }
}

void discoverCharacteristics(int service_index) {
    char uuid_str[MAX_LEN_UUID_STR+1] = {0};
    char *handle = format_to_char(services[service_index].attr_handle_start);

    Gatt_UUID_to_Str(&services[service_index].uuid, uuid_str, sizeof(uuid_str));

    int ret = WCHBle_Discover_Characteristics(connection, handle, characteristics, &characteristics_count);
    if (ret != BLE_SUCCESS) {
        printf("  > Fail to discover characteristics.\n");
        return;
    }
}

void writeToDevice() {
    if (characteristics_count == 0)
        return;

    printf("==> Send UART message:\n");

    for (int i = 0; i < 10; i++) {
        char time_str[100] = { 0 };
        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        snprintf(time_str, sizeof(time_str),
            "Host date & times: %04d-%02d-%02d %02d:%02d:%02d\n",
            local->tm_year + 1900,
            local->tm_mon + 1,
            local->tm_mday,
            local->tm_hour,
            local->tm_min,
            local->tm_sec);

        size_t buf_len = sizeof(time_str);
        printf("--> %s", time_str);
        WCHBle_Write_Characteristic(connection, "0xfff2", true, time_str, buf_len);
    
        sleep(1);
    }
}
