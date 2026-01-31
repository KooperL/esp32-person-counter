// Partition scheme: No OTA, 2mb app, 2mb fatfs


#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME 5
#define WIFI_SCAN_INTERVAL 10000
#define REPORT_INTERVAL 5000
#define MAX_DEVICES 50
#define MAC_LEN 18

char wifiDevices[MAX_DEVICES][MAC_LEN];
int wifiCount = 0;

char bleDevices[MAX_DEVICES][MAC_LEN];
int bleCount = 0;

bool macExists(char devices[MAX_DEVICES][MAC_LEN], int count, const char* mac)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(devices[i], mac) == 0) return true;
    }
    return false;
}

void addDevice(char devices[MAX_DEVICES][MAC_LEN], int* count, const char* mac)
{
    if (*count >= MAX_DEVICES) return;
    if (!macExists(devices, *count, mac)) {
        strncpy(devices[*count], mac, MAC_LEN - 1);
        devices[*count][MAC_LEN - 1] = '\0';
        (*count)++;
    }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (bleCount >= MAX_DEVICES) return;

        char mac[MAC_LEN];
        strncpy(mac, advertisedDevice.getAddress().toString().c_str(), MAC_LEN - 1);
        mac[MAC_LEN - 1] = '\0';

        addDevice(bleDevices, &bleCount, mac);
    }
};

void scanWiFi()
{
    int n = WiFi.scanNetworks(false, false, false, 300);
    for (int i = 0; i < n && wifiCount < MAX_DEVICES; i++) {
        char bssid[MAC_LEN];
        strncpy(bssid, WiFi.BSSIDstr(i).c_str(), MAC_LEN - 1);
        bssid[MAC_LEN - 1] = '\0';
        addDevice(wifiDevices, &wifiCount, bssid);
    }
    WiFi.scanDelete();
}

void scanBluetooth()
{
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->start(SCAN_TIME, false);
    pBLEScan->clearResults();
}

void plotDeviceCounts()
{
    Serial.print("WiFi:");
    Serial.print(wifiCount);
    Serial.print(",Bluetooth:");
    Serial.println(bleCount);
}

unsigned long lastWiFiScan = 0;
unsigned long lastReport = 0;

void setup()
{
    Serial.begin(115200);
    delay(1000);

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

    scanBluetooth();

    if (currentMillis - lastReport >= REPORT_INTERVAL) {
        plotDeviceCounts();
        lastReport = currentMillis;
        memset(wifiDevices, 0, sizeof(wifiDevices));
        memset(bleDevices, 0, sizeof(bleDevices));
        wifiCount = 0;
        bleCount = 0;
    }

    delay(1000);
}
