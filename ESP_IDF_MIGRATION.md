# ESP-IDF Migration Guide

## Overview

This document describes the gradual migration of Launcher from Arduino Framework to native ESP-IDF, targeting the v2.7.x release series. The migration follows a **hybrid approach** that allows both Arduino and ESP-IDF code to coexist during the transition.

## Why ESP-IDF?

### Current Challenges
- **Firmware size**: At 95-100% partition capacity on 8MB devices (m5stack-cardputer)
- **Memory pressure**: Limited heap on devices with complex graphics/networking
- **Framework overhead**: Arduino abstraction adds ~10-30% size/memory cost
- **Advanced features**: ESP-IDF provides finer control over WiFi, power, and hardware

### Expected Benefits
- **Firmware size reduction**: 10-30% smaller binaries
- **Memory efficiency**: Lower heap usage, better PSRAM management
- **Performance**: Direct hardware access, optimized networking stack
- **Future-proofing**: Access to latest ESP-IDF features and optimizations

## Migration Strategy: 4-Phase Approach

### Phase 1: Isolated Components (v2.7.0-v2.7.1)
**Target**: Self-contained modules with minimal dependencies
- ✅ **serialCommands** (proof-of-concept completed)
- **partitioner** (flash partition management)
- **settings** (migrate EEPROM/NVS to pure ESP-IDF nvs_flash)
- **powerSave** (ESP-IDF power management APIs)

**Status**: serialCommands migration complete as proof-of-concept

### Phase 2: Moderate Complexity (v2.7.2-v2.7.3)
**Target**: Networking and web interface
- **onlineLauncher** (WiFi connection/scanning)
- **webInterface** (ESPAsyncWebServer ESP-IDF port)
- **sd_functions** (ESP-IDF SDMMC driver)

### Phase 3: Complex Subsystems (v2.7.4-v2.8.0)
**Target**: Core application logic
- **main** (FreeRTOS task management)
- **mykeyboard** (input handling)
- **massStorage** (USB MSC)

### Phase 4: Display System (v3.0.0)
**Target**: Graphics libraries (deferred to major version)
- **display** system abstraction
- Graphics library ports (ArduinoGFX, TFT_eSPI, LovyanGFX)
- Board-specific display configurations

## Implementation Architecture

### Component Structure

```
components/
└── esp_idf_compat/
    ├── CMakeLists.txt              # ESP-IDF component registration
    ├── include/
    │   ├── serial_compat.h         # Arduino Serial → uart_driver wrapper
    │   ├── wifi_compat.h           # Arduino WiFi → esp_wifi wrapper
    │   └── serialCommands_idf.h    # Pure ESP-IDF serial commands
    ├── serial_compat.cpp           # Serial compatibility layer
    ├── wifi_compat.cpp             # WiFi compatibility layer
    └── serialCommands_idf.cpp      # ESP-IDF serialCommands implementation
```

### Compatibility Layer

The `esp_idf_compat` component provides Arduino-like APIs using ESP-IDF:

**Serial API** (`serial_compat.h`):
```cpp
class SerialCompat {
    void begin(uint32_t baud);
    size_t print(const char* str);
    size_t println(const char* str);
    int available();
    int read();
    std::string readStringUntil(char terminator, uint32_t timeout_ms = 300);
};
extern SerialCompat Serial;  // Global instance
```

**WiFi API** (`wifi_compat.h`):
```cpp
class WiFiCompat {
    static void begin(const char* ssid, const char* password);
    static void disconnect();
    static wl_status_t status();
    static bool isConnected();
    static std::string localIP();
    static int scanNetworks();
    static std::string SSID(uint8_t networkItem);
    static int32_t RSSI(uint8_t networkItem);
};
extern WiFiCompat WiFi;  // Global instance
```

### Build System Integration

**PlatformIO Configuration** (`platformio.ini`):
```ini
[env:test-esp-idf-cardputer]
framework = espidf
platform = espressif32
board = esp32s3box
build_flags =
    -DTEST_ESP_IDF_MIGRATION
    -DLAUNCHER='"2.7.0-idf-test"'
build_src_filter =
    +<*>
    -<serialCommands.cpp>  # Exclude Arduino version
```

**ESP-IDF Component** (`components/esp_idf_compat/CMakeLists.txt`):
```cmake
idf_component_register(
    SRCS "serial_compat.cpp" "wifi_compat.cpp" "serialCommands_idf.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_wifi nvs_flash esp_netif
)
```

## Proof-of-Concept: serialCommands

### What Was Migrated

The `serialCommands` module provides CLI interface for WiFi configuration and device control via Serial at 115200 baud. It's ideal for migration because:
- **Self-contained**: Clear boundaries with minimal external dependencies
- **Simple APIs**: String parsing, Serial I/O, WiFi functions
- **No graphics**: Doesn't touch complex display system
- **High value**: Essential for debugging during migration

### Key Changes

| Aspect | Arduino Version | ESP-IDF Version |
|--------|----------------|-----------------|
| **Serial** | `Arduino.h` + `Serial` object | `serial_compat.h` (uart_driver wrapper) |
| **WiFi** | `WiFi.h` + Arduino WiFi API | `wifi_compat.h` (esp_wifi wrapper) |
| **String** | Arduino `String` class | C++ `std::string` |
| **Timing** | `millis()` | `xTaskGetTickCount() * portTICK_PERIOD_MS` |
| **Delay** | `delay()` | `vTaskDelay(pdMS_TO_TICKS())` |
| **Restart** | `ESP.restart()` | `esp_restart()` |
| **System Info** | `ESP.getChipModel()` | `esp_chip_info()` |

### File Comparison

**Original**: `src/serialCommands.cpp` (214 lines, Arduino)
- Uses `String`, `Serial`, `WiFi` Arduino classes
- Calls `millis()`, `delay()`, `ESP.restart()`

**Migrated**: `components/esp_idf_compat/serialCommands_idf.cpp` (251 lines, ESP-IDF)
- Uses `std::string`, `SerialCompat`, `WiFiCompat`
- Calls FreeRTOS timing functions
- Pure ESP-IDF system APIs

### Testing Strategy

1. **Compile test environment**:
   ```bash
   pio run -e test-esp-idf-cardputer
   ```

2. **Verify Serial Commands work**:
   - `help` - Show command list
   - `wifi scan` - List networks
   - `wifi <ssid> <password>` - Connect to WiFi
   - `status` - Display system info
   - `restart` - Reboot device

3. **Compare behavior**:
   - Same commands should work identically
   - Memory usage should be lower
   - Serial timing should match (300ms idle timeout)

## Migration Workflow

### Step-by-Step Process

1. **Identify target module** (use Phase 1-4 priorities)
2. **Analyze dependencies** (which Arduino APIs are used?)
3. **Create ESP-IDF version** in `components/esp_idf_compat/`
4. **Replace Arduino APIs** with ESP-IDF equivalents
5. **Add compatibility wrappers** if needed for other modules
6. **Update build configuration** (exclude Arduino version)
7. **Test functionality** (ensure identical behavior)
8. **Measure improvements** (firmware size, memory usage)

### Arduino to ESP-IDF API Mapping

| Arduino API | ESP-IDF Equivalent | Notes |
|-------------|-------------------|-------|
| `Serial.begin()` | `uart_driver_install()` + `uart_param_config()` | Use wrapper or direct |
| `Serial.print()` | `uart_write_bytes()` | Wrapper provides convenience |
| `String` | `std::string` | Standard C++, portable |
| `WiFi.begin()` | `esp_wifi_connect()` | More configuration needed |
| `WiFi.scanNetworks()` | `esp_wifi_scan_start()` | Async, need event handler |
| `millis()` | `esp_timer_get_time() / 1000` | Or FreeRTOS ticks |
| `delay()` | `vTaskDelay(pdMS_TO_TICKS())` | Must convert to ticks |
| `ESP.restart()` | `esp_restart()` | Direct equivalent |
| `ESP.getChipModel()` | `esp_chip_info()` | More detailed info |
| `EEPROM` | `nvs_flash` | NVS more robust, wear-leveling |

### Compatibility Layer Guidelines

**When to create a wrapper**:
- API is used across multiple modules
- Arduino API significantly simpler than ESP-IDF
- Temporary bridge during migration

**When to use direct ESP-IDF**:
- Module is self-contained
- ESP-IDF API is straightforward
- Maximum performance/size critical

**Example**: Serial wrapper simplifies 50+ lines of UART setup to `Serial.begin(115200)`.

## Testing & Validation

### Build Testing
```bash
# Test ESP-IDF environment
pio run -e test-esp-idf-cardputer

# Test Arduino environment (reference)
pio run -e m5stack-cardputer

# Compare firmware sizes
ls -lh .pio/build/*/firmware.bin
```

### Functional Testing
- **Serial Commands**: Test all commands via Serial Monitor
- **WiFi**: Verify scanning and connection
- **Memory**: Monitor heap usage during operation
- **Stability**: Run for 30+ minutes without crashes

### Metrics to Track
- **Firmware size**: Before/after migration (target: -10 to -30%)
- **Free heap**: Runtime memory available (target: +15-25%)
- **Boot time**: Startup performance
- **Binary compatibility**: Must still work with existing apps

## Known Issues & Limitations

### Current Limitations

1. **Display system remains Arduino**: Graphics libraries not yet ported
2. **ArduinoJson dependency**: Works on both, but Arduino-style API
3. **ESPAsyncWebServer**: Need ESP-IDF port (exists, needs integration)
4. **Board configs**: 50+ board definitions tied to Arduino framework
5. **Custom libraries**: CYD-touch, Custom_Update need porting

### Compatibility Gaps

- `Serial.peek()` not implemented (not used in serialCommands)
- Arduino `String` methods not fully compatible with `std::string`
- Partition table format same, but tooling different

### Workarounds

- Use `#ifdef TEST_ESP_IDF_MIGRATION` to conditionally compile
- Keep Arduino versions in `src/` for reference
- Compatibility layer bridges critical APIs

## Next Steps

### Immediate (v2.7.0)
- ✅ Complete serialCommands proof-of-concept
- ⏳ Validate test-esp-idf-cardputer environment
- ⏳ Measure firmware size reduction
- ⏳ Document memory improvements

### Short-term (v2.7.1)
- Port `partitioner.cpp` to ESP-IDF
- Port `settings.cpp` to pure NVS (remove EEPROM)
- Port `powerSave.cpp` to ESP-IDF power APIs
- Create more board test environments

### Medium-term (v2.7.2-v2.8.0)
- Port WiFi and web interface modules
- Port SD card functions to SDMMC driver
- Gradually migrate main application logic
- Expand compatibility layer as needed

### Long-term (v3.0.0)
- Port or replace graphics libraries
- Full ESP-IDF build for all devices
- Remove Arduino framework dependency
- Optimize for size and performance

## Resources

### ESP-IDF Documentation
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/index.html)
- [WiFi Driver](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/network/esp_wifi.html)
- [UART Driver](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/peripherals/uart.html)
- [NVS Flash](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/storage/nvs_flash.html)

### Migration Examples
- [Arduino to ESP-IDF Migration](https://github.com/espressif/arduino-esp32/tree/master/docs/en/migration_guides)
- [ESP-IDF Components](https://components.espressif.com/)

### Community
- [Discord: Launcher Community](https://discord.gg/BE9by2a2FF)
- [GitHub Issues](https://github.com/bmorcelli/Launcher/issues)

## Contributing

Contributions to the ESP-IDF migration are welcome! Please:
1. Follow the phase priorities (Phase 1 → Phase 4)
2. Maintain compatibility layer for shared APIs
3. Test on multiple devices (ESP32, ESP32-S3)
4. Document API changes and rationale
5. Measure size/memory improvements

## Conclusion

The gradual ESP-IDF migration approach allows:
- **Incremental progress**: Migrate modules one at a time
- **Risk mitigation**: Arduino versions remain available
- **Continuous validation**: Test after each module migration
- **Flexible timeline**: Release improvements in minor versions (v2.7.x)

The serialCommands proof-of-concept demonstrates feasibility and provides a template for future migrations. By v3.0.0, Launcher will be fully ESP-IDF native with significant size and performance improvements.

---

**Version**: 1.0
**Date**: December 26, 2025
**Status**: Phase 1 - serialCommands proof-of-concept complete
**Next Review**: After v2.7.0 release
