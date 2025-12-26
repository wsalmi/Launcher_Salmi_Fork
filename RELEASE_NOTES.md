# Launcher v2.6.6 - Release Notes

## 🎉 What's New

### Serial Commands (Contribution by @WSalmi)
- **NEW!** Essential serial commands for WiFi configuration and WebUI access
- Perfect for headless devices (no keyboard/SD card required)
- Commands available at 115200 baud

**Available Commands:**
- `help` or `?` - Show all available commands
- `wifi scan` - List available networks with signal strength
- `wifi <ssid> <password>` - Connect to WiFi network
- `wifi status` - Show current WiFi connection status
- `webui` - Display WebUI access URL (requires WiFi)
- `brightness [1-100]` - Get or set screen brightness
- `status` - Show chip model, frequency, memory, flash info
- `restart` - Reboot device

**Features:**
- Commands work without newline character (300ms idle timeout)
- Control character filtering for terminal compatibility
- Optimized firmware size focusing on WiFi/WebUI essentials

## 📦 Installation

### Method 1: Web Flasher (Easiest)
Visit [Launcher Flasher](https://bmorcelli.github.io/Launcher/) and follow the instructions.

### Method 2: esptool.py
1. Download the `.bin` file for your device from the releases
2. Connect your device via USB
3. Flash using esptool.py:
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 \
  write_flash -z 0x10000 Launcher-2.6.6-{YourDevice}.bin
```

### Method 3: PlatformIO Web Tool
1. Go to https://web.esphome.io/
2. Connect your device
3. Select the `.bin` file and flash

## 🔧 Supported Devices

### M5Stack Family
- M5StickC, M5StickC Plus, M5StickC Plus2
- M5Stack Cardputer (including ADV version)
- M5Stack Core, Core2, CoreS3
- M5Stack Tab5, Paper, Paper S3

### Lilygo
- T-Deck, T-Deck Pro
- T-Display S3 (Amoled, Pro, Touch)
- T-Dongle S3 TFT
- T-Embed, T-Embed CC1101
- T-LoRa Pager
- T5 E-Paper S3 Pro
- T-HMI

### CYD (Cheap Yellow Display)
- 2432S028 (2.8" resistive)
- 2-USB, 2432S024R, 2432S022C
- 2432S032C, 2432S032R
- 2432W328C, 2432W328R
- 3248S035C, 3248S035R
- 8048S043C, 8048W550C
- 3248W535C, 4827S043R

### Others
- Marauder (Mini, v4-OG, v6.1, v7)
- AWOK (Mini, Touch)
- Elecrow (24B, 28B, 35B)
- Phantom S024R
- Smoochiee Board
- WaveSentry R1
- Headless ESP32/ESP32-S3 (4MB, 8MB, 16MB)

## 📋 Files in This Release

- `Launcher-2.6.6-{device}.zip` - Individual firmware for each device
- `Launcher-2.6.6-All.zip` - All firmwares in one package

Each ZIP contains the compiled `.bin` file ready to flash.

## 🐛 Known Issues

- UiFlow 1 doesn't work with Launcher (uses old MicroPython/ESP-IDF)

## 💡 Tips

- SD Card recommended: SDHC (not SDXC), max 32GB, formatted FAT32
- Use [Rufus](https://rufus.ie/) to format SD Card
- More info at [Launcher Wiki](https://github.com/bmorcelli/Launcher/wiki)

## 🤝 Contributing

This is a fork with serial commands contribution by @WSalmi.

Original project: [bmorcelli/Launcher](https://github.com/bmorcelli/Launcher)

## 📝 Changelog

See [README.md](https://github.com/wsalmi/Launcher_Salmi_Fork#latest-changelog) for detailed changelog.

## ⚖️ License

See [LICENSE](LICENSE) file for details.

---

**Need help?** Join the [Discord community](https://discord.gg/BE9by2a2FF)
