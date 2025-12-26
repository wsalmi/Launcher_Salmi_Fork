# Quick Start: ESP-IDF Migration

## TL;DR - What Was Done

Created **ESP-IDF compatibility layer** and migrated **serialCommands** as proof-of-concept for gradual ESP-IDF migration in Launcher v2.7.x.

## What You Need to Know

### 📁 New Files Created
```
components/esp_idf_compat/
├── CMakeLists.txt                      # ESP-IDF component
├── include/
│   ├── serial_compat.h                 # Arduino Serial wrapper
│   ├── wifi_compat.h                   # Arduino WiFi wrapper
│   └── serialCommands_idf.h            # ESP-IDF commands
├── serial_compat.cpp                   # UART implementation
├── wifi_compat.cpp                     # WiFi implementation
└── serialCommands_idf.cpp              # Commands implementation
```

### 📖 Documentation
- **[ESP_IDF_MIGRATION.md](ESP_IDF_MIGRATION.md)** - Complete migration guide (350+ lines)
- **[ESP_IDF_IMPLEMENTATION_SUMMARY.md](ESP_IDF_IMPLEMENTATION_SUMMARY.md)** - What was implemented
- **[ESP_IDF_ARCHITECTURE.md](ESP_IDF_ARCHITECTURE.md)** - Visual diagrams and architecture

### 🔧 Build Configuration
Added test environment to `platformio.ini`:
```ini
[env:test-esp-idf-cardputer]
framework = espidf
```

## Quick Test

### Build ESP-IDF Version
```bash
pio run -e test-esp-idf-cardputer
```

### Build Arduino Version (Reference)
```bash
pio run -e m5stack-cardputer
```

### Compare Sizes
```bash
ls -lh .pio/build/*/firmware.bin
```

## Serial Commands

Same commands work in both Arduino and ESP-IDF versions:

```bash
> help                          # Show commands
> wifi scan                     # List networks
> wifi MySSID MyPassword        # Connect to WiFi
> wifi status                   # Check connection
> webui                         # Get WebUI URL
> brightness 50                 # Set brightness
> status                        # System info
> restart                       # Reboot
```

## Expected Results

### Firmware Size
- **Arduino**: ~1.5MB (95.7% partition on m5stack-cardputer)
- **ESP-IDF**: ~1.2-1.35MB (10-20% reduction expected)
- **Benefit**: More headroom for features

### Memory Usage
- **Arduino**: Higher overhead from String class, abstractions
- **ESP-IDF**: Lower heap usage, direct API access
- **Benefit**: More stable, especially on constrained devices

## Migration Roadmap

### ✅ Phase 1 - Complete
- serialCommands migrated to ESP-IDF
- Compatibility layer created
- Build system configured
- Documentation complete

### ⏳ Phase 2 - Next (v2.7.1)
- partitioner.cpp
- settings.cpp (NVS migration)
- powerSave.cpp

### 🔜 Phase 3 - Later (v2.7.2+)
- webInterface.cpp
- onlineLauncher.cpp
- sd_functions.cpp

### 🔮 Phase 4 - Future (v3.0.0)
- display.cpp system
- Graphics libraries

## Key Differences: Arduino vs ESP-IDF

| Feature | Arduino | ESP-IDF |
|---------|---------|---------|
| **String** | `String` | `std::string` |
| **Serial** | `Serial` object | `uart_driver.h` |
| **WiFi** | `WiFi.h` | `esp_wifi.h` |
| **Time** | `millis()` | `xTaskGetTickCount()` |
| **Delay** | `delay()` | `vTaskDelay()` |
| **Restart** | `ESP.restart()` | `esp_restart()` |

## Compatibility Layer

The wrapper provides Arduino-like APIs using ESP-IDF:

```cpp
// Still works the same way!
Serial.begin(115200);
Serial.println("Hello");

WiFi.begin(ssid, password);
if (WiFi.isConnected()) {
    Serial.println(WiFi.localIP());
}
```

**Behind the scenes**: Using ESP-IDF `uart_driver` and `esp_wifi` APIs.

## Integration Points

### Arduino Code (src/)
```cpp
#include "serialCommands.h"  // Arduino version

void loop() {
    processSerialCommand();  // Arduino implementation
}
```

### ESP-IDF Code (components/)
```cpp
#include "serialCommands_idf.h"  // ESP-IDF version

void loop() {
    processSerialCommand_IDF();  // ESP-IDF implementation
}
```

**Note**: Currently both exist; build filter selects which to compile.

## Troubleshooting

### Build Fails
1. Check ESP-IDF framework installation: `pio platform install espressif32`
2. Verify component structure: `ls components/esp_idf_compat/`
3. Check CMakeLists.txt syntax

### Commands Don't Work
1. Verify Serial is initialized: `Serial.begin(115200)`
2. Check WiFi is configured: `WiFi.mode(WIFI_MODE_STA)`
3. Test with simple command: `help`

### Size Not Reduced
1. Build with `-Os` optimization
2. Enable LTO (Link Time Optimization)
3. Strip unused functions: `-Wl,--gc-sections`

## Next Actions

### For Testing
1. Flash ESP-IDF version to hardware
2. Connect Serial Monitor at 115200 baud
3. Test all commands
4. Monitor heap usage

### For Development
1. Port next module (partitioner recommended)
2. Follow same pattern as serialCommands
3. Update build configuration
4. Test thoroughly

### For Integration
1. Modify `main.cpp` to call `processSerialCommand_IDF()`
2. Link display/settings dependencies
3. Test hybrid build (ESP-IDF commands + Arduino display)
4. Validate on multiple boards

## Resources

- **Documentation**: [ESP_IDF_MIGRATION.md](ESP_IDF_MIGRATION.md)
- **Architecture**: [ESP_IDF_ARCHITECTURE.md](ESP_IDF_ARCHITECTURE.md)
- **ESP-IDF Docs**: https://docs.espressif.com/projects/esp-idf/
- **Discord**: https://discord.gg/BE9by2a2FF

## Questions?

See detailed documentation or ask in Discord #launcher channel.

---

**Status**: Proof-of-concept complete, ready for validation  
**Version**: v2.7.0-dev  
**Date**: December 26, 2025  
**Developer**: @WSalmi
