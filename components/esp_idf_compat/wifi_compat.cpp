/**
 * ESP-IDF WiFi Compatibility Layer Implementation
 */

#include "wifi_compat.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "lwip/inet.h"
#include "nvs_flash.h"
#include <cstring>

static const char *TAG = "WiFiCompat";

// Global instance
WiFiCompat WiFi;

// Static member initialization
bool WiFiCompat::initialized = false;
bool WiFiCompat::sta_started = false;
wl_status_t WiFiCompat::current_status = WL_DISCONNECTED;
std::vector<WiFiNetwork> WiFiCompat::scan_results;

void WiFiCompat::init_if_needed() {
    if (initialized) return;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    initialized = true;
    ESP_LOGI(TAG, "WiFi initialized");
}

void WiFiCompat::wifi_event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data
) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                current_status = WL_IDLE_STATUS;
                ESP_LOGI(TAG, "WiFi station started");
                break;

            case WIFI_EVENT_STA_CONNECTED: ESP_LOGI(TAG, "Connected to AP"); break;

            case WIFI_EVENT_STA_DISCONNECTED:
                current_status = WL_DISCONNECTED;
                ESP_LOGI(TAG, "Disconnected from AP");
                break;

            case WIFI_EVENT_SCAN_DONE:
                current_status = WL_SCAN_COMPLETED;
                ESP_LOGI(TAG, "WiFi scan completed");
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        current_status = WL_CONNECTED;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void WiFiCompat::mode(wifi_mode_t wifi_mode) {
    init_if_needed();
    ESP_ERROR_CHECK(esp_wifi_set_mode(wifi_mode));
}

void WiFiCompat::begin(const char *ssid, const char *password) {
    init_if_needed();

    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password) {
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    sta_started = true;
    current_status = WL_IDLE_STATUS;

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
}

void WiFiCompat::disconnect() {
    if (sta_started) {
        esp_wifi_disconnect();
        current_status = WL_DISCONNECTED;
    }
}

wl_status_t WiFiCompat::status() { return current_status; }

bool WiFiCompat::isConnected() { return current_status == WL_CONNECTED; }

std::string WiFiCompat::localIP() {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) return "0.0.0.0";

    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) { return "0.0.0.0"; }

    char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
    return std::string(ip_str);
}

std::string WiFiCompat::SSID() {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) { return std::string((char *)ap_info.ssid); }
    return "";
}

int WiFiCompat::scanNetworks() {
    init_if_needed();

    // Start scan
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
                      .active = {
                .min = 100,
                .max = 300,
            }, },
    };

    if (!sta_started) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        sta_started = true;
    }

    esp_err_t err = esp_wifi_scan_start(&scan_config, true);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Scan start failed: %s", esp_err_to_name(err));
        return 0;
    }

    // Get scan results
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);

    scan_results.clear();
    if (ap_count > 0) {
        wifi_ap_record_t *ap_records = new wifi_ap_record_t[ap_count];
        esp_wifi_scan_get_ap_records(&ap_count, ap_records);

        for (int i = 0; i < ap_count; i++) {
            WiFiNetwork net;
            net.ssid = std::string((char *)ap_records[i].ssid);
            net.rssi = ap_records[i].rssi;
            net.channel = ap_records[i].primary;
            net.authmode = ap_records[i].authmode;
            scan_results.push_back(net);
        }

        delete[] ap_records;
    }

    ESP_LOGI(TAG, "Found %d networks", ap_count);
    return ap_count;
}

std::string WiFiCompat::SSID(uint8_t networkItem) {
    if (networkItem >= scan_results.size()) return "";
    return scan_results[networkItem].ssid;
}

int32_t WiFiCompat::RSSI(uint8_t networkItem) {
    if (networkItem >= scan_results.size()) return 0;
    return scan_results[networkItem].rssi;
}

uint8_t WiFiCompat::channel(uint8_t networkItem) {
    if (networkItem >= scan_results.size()) return 0;
    return scan_results[networkItem].channel;
}

wifi_auth_mode_t WiFiCompat::encryptionType(uint8_t networkItem) {
    if (networkItem >= scan_results.size()) return WIFI_AUTH_OPEN;
    return scan_results[networkItem].authmode;
}

std::string WiFiCompat::macAddress() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    char mac_str[18];
    snprintf(
        mac_str,
        sizeof(mac_str),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );
    return std::string(mac_str);
}
