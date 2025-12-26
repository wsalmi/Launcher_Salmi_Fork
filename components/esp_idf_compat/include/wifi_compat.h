/**
 * ESP-IDF WiFi Compatibility Layer
 *
 * Provides simplified Arduino-like WiFi interface using ESP-IDF esp_wifi
 * Designed for serialCommands.cpp migration
 */

#pragma once

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include <cstdint>
#include <string>
#include <vector>

// WiFi status enumeration matching Arduino WiFi
enum wl_status_t {
    WL_NO_SHIELD = 255,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6
};

struct WiFiNetwork {
    std::string ssid;
    int32_t rssi;
    uint8_t channel;
    wifi_auth_mode_t authmode;
};

class WiFiCompat {
private:
    static bool initialized;
    static bool sta_started;
    static wl_status_t current_status;
    static std::vector<WiFiNetwork> scan_results;

    static void
    wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

    static void init_if_needed();

public:
    // Station mode
    static void mode(wifi_mode_t mode);
    static void begin(const char *ssid, const char *password);
    static void disconnect();

    // Status
    static wl_status_t status();
    static bool isConnected();
    static std::string localIP();
    static std::string SSID();

    // Scanning
    static int scanNetworks();
    static std::string SSID(uint8_t networkItem);
    static int32_t RSSI(uint8_t networkItem);
    static uint8_t channel(uint8_t networkItem);
    static wifi_auth_mode_t encryptionType(uint8_t networkItem);

    // Utility
    static std::string macAddress();
};

// Global instance alias for Arduino compatibility
extern WiFiCompat WiFi;
