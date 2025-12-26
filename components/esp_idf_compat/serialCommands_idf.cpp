/**
 * Serial Commands - ESP-IDF Native Implementation
 */

#include "serialCommands_idf.h"
#include "serial_compat.h"
#include "wifi_compat.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sstream>
#include <algorithm>

static const char* TAG = "SerialCmd";

static std::string commandBuffer = "";
static uint32_t lastCharMillis = 0;

// Extern references to Arduino components (temporary during migration)
extern int bright;  // From globals.h
extern void setBrightness(int value);  // From display.h

// Helper: Convert string to lowercase
static void toLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

// Helper: Trim whitespace
static void trim(std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        str.clear();
        return;
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    str = str.substr(start, end - start + 1);
}

// Helper: Split command and arguments
static void splitCommand(const std::string& line, std::string& cmd, std::string& args) {
    size_t spacePos = line.find(' ');
    if (spacePos != std::string::npos) {
        cmd = line.substr(0, spacePos);
        args = line.substr(spacePos + 1);
        trim(args);
    } else {
        cmd = line;
        args.clear();
    }
}

static void handleSerialLine(std::string line) {
    trim(line);
    if (line.empty()) return;

    // Echo the command
    Serial.print("\n> ");
    Serial.println(line);

    // Parse command and arguments
    std::string cmd, args;
    splitCommand(line, cmd, args);
    toLower(cmd);

    if (cmd == "help" || cmd == "?") {
        cmd_help_IDF();
    } else if (cmd == "wifi") {
        cmd_wifi_IDF(args);
    } else if (cmd == "webui") {
        cmd_webui_IDF();
    } else if (cmd == "brightness" || cmd == "bright") {
        cmd_brightness_IDF(args);
    } else if (cmd == "restart") {
        cmd_restart_IDF();
    } else if (cmd == "status") {
        cmd_status_IDF();
    } else {
        Serial.print("Unknown: '");
        Serial.print(cmd);
        Serial.println("'");
    }
}

void processSerialCommand_IDF() {
    while (Serial.available() > 0) {
        int c = Serial.read();
        if (c < 0) break;

        char ch = (char)c;
        
        // Ignore control characters except \n, \r, backspace
        if (ch < 32 && ch != '\n' && ch != '\r' && ch != 8) {
            continue;
        }
        
        lastCharMillis = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (ch == '\n' || ch == '\r') {
            if (!commandBuffer.empty()) {
                handleSerialLine(commandBuffer);
                commandBuffer.clear();
            }
        } else if (ch == 8 || ch == 127) {  // Backspace or DEL
            if (!commandBuffer.empty()) {
                commandBuffer.pop_back();
            }
        } else if ((uint8_t)ch >= 32 && (uint8_t)ch <= 126) {
            // Append printable ASCII only
            commandBuffer += ch;
        }
    }

    // Process after idle timeout (300ms without newline)
    if (!commandBuffer.empty()) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (now - lastCharMillis > 300) {
            handleSerialLine(commandBuffer);
            commandBuffer.clear();
        }
    }
}

void cmd_help_IDF() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║  WiFi & WebUI Control via Serial      ║");
    Serial.println("╠════════════════════════════════════════╣");
    Serial.println("║ wifi scan                              ║");
    Serial.println("║   List available networks              ║");
    Serial.println("║                                        ║");
    Serial.println("║ wifi <ssid> <password>                 ║");
    Serial.println("║   Connect to WiFi network              ║");
    Serial.println("║                                        ║");
    Serial.println("║ wifi status                            ║");
    Serial.println("║   Show WiFi connection status          ║");
    Serial.println("║                                        ║");
    Serial.println("║ webui                                  ║");
    Serial.println("║   Show WebUI access address            ║");
    Serial.println("║                                        ║");
    Serial.println("║ brightness [1-100]                     ║");
    Serial.println("║   Get or set display brightness        ║");
    Serial.println("║                                        ║");
    Serial.println("║ status                                 ║");
    Serial.println("║   Device status and info               ║");
    Serial.println("║                                        ║");
    Serial.println("║ restart                                ║");
    Serial.println("║   Reboot the device                    ║");
    Serial.println("╚════════════════════════════════════════╝\n");
}

void cmd_wifi_IDF(const std::string& args) {
    if (args.empty()) {
        Serial.println("Usage: wifi <ssid> <password>");
        Serial.println("       wifi scan");
        Serial.println("       wifi status");
        return;
    }

    if (args == "scan") {
        Serial.println("Scanning WiFi networks...");
        int n = WiFi.scanNetworks();
        if (n == 0) {
            Serial.println("No networks found");
        } else {
            Serial.print("Found ");
            Serial.print(n);
            Serial.println(" networks:");
            for (int i = 0; i < n; i++) {
                Serial.print(i + 1);
                Serial.print(". ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.println(" dBm)");
            }
        }
        return;
    }

    if (args == "status") {
        if (WiFi.isConnected()) {
            Serial.print("Connected to: ");
            Serial.println(WiFi.SSID());
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("WiFi: Not connected");
        }
        return;
    }

    // Parse SSID and password
    size_t spacePos = args.find(' ');
    if (spacePos == std::string::npos) {
        Serial.println("Error: SSID and password required");
        return;
    }

    std::string ssid = args.substr(0, spacePos);
    std::string password = args.substr(spacePos + 1);

    Serial.print("Connecting to: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (!WiFi.isConnected() && attempts < 20) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.isConnected()) {
        Serial.println("WiFi connected successfully!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Access WebUI at: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

void cmd_webui_IDF() {
    if (WiFi.isConnected()) {
        Serial.println("WebUI is available at:");
        Serial.print("http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi not connected");
        Serial.println("First use: wifi <ssid> <password>");
    }
}

void cmd_brightness_IDF(const std::string& args) {
    if (!args.empty()) {
        int val = std::stoi(args);
        if (val >= 1 && val <= 100) {
            setBrightness(val);
            Serial.print("Brightness: ");
            Serial.print(val);
            Serial.println("%");
        } else {
            Serial.println("Error: 1-100");
        }
    } else {
        Serial.print("Brightness: ");
        Serial.print(bright);
        Serial.println("%");
    }
}

void cmd_restart_IDF() {
    Serial.println("Restarting...");
    Serial.flush();
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}

void cmd_status_IDF() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    Serial.println("\n═════════════════════════════════════");
    
    // Chip model
    Serial.print("Chip: ");
    if (chip_info.model == CHIP_ESP32) {
        Serial.print("ESP32");
    } else if (chip_info.model == CHIP_ESP32S2) {
        Serial.print("ESP32-S2");
    } else if (chip_info.model == CHIP_ESP32S3) {
        Serial.print("ESP32-S3");
    } else if (chip_info.model == CHIP_ESP32C3) {
        Serial.print("ESP32-C3");
    } else {
        Serial.print("Unknown");
    }
    
    Serial.print(" (");
    Serial.print(esp_clk_cpu_freq() / 1000000);
    Serial.println(" MHz)");
    
    // Memory info
    Serial.print("Heap: ");
    Serial.print(esp_get_free_heap_size() / 1024);
    Serial.print(" KB | Flash: ");
    Serial.print(chip_info.flash_size_mbytes);
    Serial.println(" MB");
    
    // Brightness
    Serial.print("Brightness: ");
    Serial.println(bright);

    // WiFi status
    if (WiFi.isConnected()) {
        Serial.print("WiFi: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("WebUI: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi: Not connected");
    }
    
    Serial.println("═════════════════════════════════════\n");
}
