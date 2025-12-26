/**
 * Serial Commands - ESP-IDF Native Implementation
 *
 * Pure ESP-IDF version using:
 * - uart_driver.h instead of Arduino Serial
 * - esp_wifi.h instead of Arduino WiFi
 * - std::string instead of Arduino String
 * - esp_log.h for logging
 *
 * This is a proof-of-concept for gradual ESP-IDF migration.
 */

#pragma once

#include <string>

// Process incoming serial commands (call from main loop)
void processSerialCommand_IDF();

// Individual command handlers
void cmd_help_IDF();
void cmd_wifi_IDF(const std::string &args);
void cmd_webui_IDF();
void cmd_status_IDF();
void cmd_brightness_IDF(const std::string &args);
void cmd_restart_IDF();
