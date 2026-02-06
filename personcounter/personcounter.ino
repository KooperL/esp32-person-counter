// Partition scheme: No OTA, 2mb app, 2mb fatfs
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BLE_SCAN_TIME 5
#define BLE_SCAN_INTERVAL 6000
#define WIFI_SCAN_INTERVAL 10000
#define REPORT_INTERVAL 5000
#define MAX_DEVICES 50
#define MAC_LEN 18

char wifiDevices[MAX_DEVICES][MAC_LEN];
char bleDevices[MAX_DEVICES][MAC_LEN];

bool macExists(char devices[MAX_DEVICES][MAC_LEN], int count, const char* mac)
{
    for (int i = 0; i < count; i++) {
        if (devices[i][0] == '\0') break;
        if (strcmp(devices[i], mac) == 0) return true;
    }
    return false;
}

int getDeviceCount(char devices[MAX_DEVICES][MAC_LEN])
{
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (devices[i][0] == '\0') return i;
    }
    return MAX_DEVICES;
}

void addDevice(char devices[MAX_DEVICES][MAC_LEN], const char* mac)
{
    int count = getDeviceCount(devices);
    if (count >= MAX_DEVICES) return;
    if (!macExists(devices, count, mac)) {
        strncpy(devices[count], mac, MAC_LEN - 1);
        devices[count][MAC_LEN - 1] = '\0';
    }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (getDeviceCount(bleDevices) >= MAX_DEVICES) return;
        char mac[MAC_LEN];
        strncpy(mac, advertisedDevice.getAddress().toString().c_str(), MAC_LEN - 1);
        mac[MAC_LEN - 1] = '\0';
        addDevice(bleDevices, mac);
    }
};

void scanWiFi()
{
    int n = WiFi.scanNetworks(false, true, false, 300);
    int currentCount = getDeviceCount(wifiDevices);
    for (int i = 0; i < n && currentCount < MAX_DEVICES; i++) {
        char bssid[MAC_LEN];
        strncpy(bssid, WiFi.BSSIDstr(i).c_str(), MAC_LEN - 1);
        bssid[MAC_LEN - 1] = '\0';
        addDevice(wifiDevices, bssid);
        currentCount = getDeviceCount(wifiDevices);
    }
    WiFi.scanDelete();
}

void scanBluetooth()
{
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->start(BLE_SCAN_TIME, false);
    pBLEScan->clearResults();
}

void reportDevices()
{
    Serial.print("WiFi:");
    Serial.print(getDeviceCount(wifiDevices));
    Serial.print(",Bluetooth:");
    Serial.println(getDeviceCount(bleDevices));
}

unsigned long lastWiFiScan = 0;
unsigned long lastBLEScan = 0;
unsigned long lastReport = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    memset(wifiDevices, 0, sizeof(wifiDevices));
    memset(bleDevices, 0, sizeof(bleDevices));
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    delay(500);
}

void loop()
{
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastWiFiScan >= WIFI_SCAN_INTERVAL) {
        scanWiFi();
        lastWiFiScan = currentMillis;
    }
    
    if (currentMillis - lastBLEScan >= BLE_SCAN_INTERVAL) {
        scanBluetooth();
        lastBLEScan = currentMillis;
    }
    
    if (currentMillis - lastReport >= REPORT_INTERVAL) {
        lastReport = currentMillis;
        reportDevices();

        // clean up
        memset(wifiDevices, 0, sizeof(wifiDevices));
        memset(bleDevices, 0, sizeof(bleDevices));
    }
    
    delay(100);
}