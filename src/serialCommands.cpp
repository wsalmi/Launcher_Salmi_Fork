#include "serialCommands.h"
#include "display.h"
#include "settings.h"
#include "webInterface.h"
#include <globals.h>

String serialCommandBuffer = "";
static unsigned long lastSerialCharMillis = 0;

static void handleSerialLine(String line) {
    line.trim();
    if (line.length() == 0) return;

    // Echo the command
    Serial.printf("\n> %s\n", line.c_str());

    // Parse command and arguments
    int spaceIndex = line.indexOf(' ');
    String cmd = "";
    String args = "";

    if (spaceIndex > 0) {
        cmd = line.substring(0, spaceIndex);
        args = line.substring(spaceIndex + 1);
        args.trim();
    } else {
        cmd = line;
    }

    cmd.toLowerCase();

    if (cmd == "help" || cmd == "?") {
        cmd_help();
    } else if (cmd == "wifi") {
        cmd_wifi(args);
    } else if (cmd == "webui") {
        cmd_webui();
    } else if (cmd == "brightness" || cmd == "bright") {
        if (args.length() > 0) {
            int val = args.toInt();
            if (val >= 1 && val <= 100) {
                setBrightness(val);
                Serial.printf("Brightness: %d%%\n", val);
            } else {
                Serial.println("Error: 1-100");
            }
        } else {
            Serial.printf("Brightness: %d%%\n", bright);
        }
    } else if (cmd == "restart") {
        Serial.println(F("Restarting..."));
        delay(1000);
#ifndef HEADLESS
        FREE_TFT
#endif
        ESP.restart();
    } else if (cmd == "status") {
        cmd_status();
    } else {
        Serial.printf("Unknown: '%s'\n", cmd.c_str());
    }
}

void processSerialCommand() {
    while (Serial.available() > 0) {
        char c = Serial.read();

        // Ignore control characters except \n, \r, backspace, delete
        if (c < 32 && c != '\n' && c != '\r' && c != 8) { continue; }
        lastSerialCharMillis = millis();

        if (c == '\n' || c == '\r') {
            if (serialCommandBuffer.length() > 0) {
                handleSerialLine(serialCommandBuffer);
                serialCommandBuffer = "";
            }
        } else if (c == 8 || c == 127) {
            if (serialCommandBuffer.length() > 0) {
                serialCommandBuffer.remove(serialCommandBuffer.length() - 1);
            }
        } else if ((uint8_t)c >= 32 && (uint8_t)c <= 126) {
            // Append printable ASCII characters only
            serialCommandBuffer += c;
        } else {
            // Ignore non-printable/control characters (e.g., 0x03 ETX)
        }
    }

    // If buffer has content but no newline received, process after idle timeout
    if (serialCommandBuffer.length() > 0) {
        if (millis() - lastSerialCharMillis > 300) {
            handleSerialLine(serialCommandBuffer);
            serialCommandBuffer = "";
        }
    }
}

void cmd_help() {
    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║  WiFi & WebUI Control via Serial      ║"));
    Serial.println(F("╠════════════════════════════════════════╣"));
    Serial.println(F("║ wifi scan                              ║"));
    Serial.println(F("║   List available networks              ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ wifi <ssid> <password>                 ║"));
    Serial.println(F("║   Connect to WiFi network              ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ wifi status                            ║"));
    Serial.println(F("║   Show WiFi connection status          ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ webui                                  ║"));
    Serial.println(F("║   Show WebUI access address            ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ brightness [0-255]                     ║"));
    Serial.println(F("║   Get or set display brightness        ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ status                                 ║"));
    Serial.println(F("║   Device status and info               ║"));
    Serial.println(F("║                                        ║"));
    Serial.println(F("║ restart                                ║"));
    Serial.println(F("║   Reboot the device                    ║"));
    Serial.println(F("╚════════════════════════════════════════╝\n"));
}

void cmd_wifi(String args) {
    if (args.length() == 0) {
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
            Serial.printf("Found %d networks:\n", n);
            for (int i = 0; i < n; i++) {
                Serial.printf("%d. %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            }
        }
        return;
    }

    if (args == "status") {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("Connected to: %s\n", WiFi.SSID().c_str());
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
        } else {
            Serial.println("WiFi: Not connected");
        }
        return;
    }

    int spaceIndex = args.indexOf(' ');
    if (spaceIndex < 0) {
        Serial.println("Error: SSID and password required");
        return;
    }

    String ssid = args.substring(0, spaceIndex);
    String password = args.substring(spaceIndex + 1);

    Serial.printf("Connecting to: %s\n", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected successfully!");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.println("Access WebUI at: http://" + WiFi.localIP().toString());
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

void cmd_webui() {
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WebUI is available at:");
        Serial.printf("http://%s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi not connected");
        Serial.println("First use: wifi <ssid> <password>");
    }
}

void cmd_status() {
    Serial.println(F("\n═════════════════════════════════════"));
    Serial.printf("Chip: %s (%d MHz)\n", ESP.getChipModel(), ESP.getCpuFreqMHz());
    Serial.printf("Heap: %d KB | Flash: %d MB\n", ESP.getFreeHeap() / 1024, ESP.getFlashChipSize() / 1048576);
    Serial.printf("Brightness: %d\n", bright);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("WebUI: http://%s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("WiFi: Not connected");
    }
    Serial.println(F("═════════════════════════════════════\n"));
}
