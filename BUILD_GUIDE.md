# Build Scripts Guide

This directory contains scripts to build all Launcher firmware releases for distribution.

## Quick Start

### Option 1: Bash Script (Recommended)
```bash
./build_all_releases.sh
```

### Option 2: Python Script
```bash
python3 build_all_releases.py
```

## What the Scripts Do

1. **Compile** all 70+ device environments from `platformio.ini`
2. **Create individual ZIPs** for each device: `Launcher-2.6.6-{device}.zip`
3. **Create All.zip** containing all firmwares: `Launcher-2.6.6-All.zip`

## Output Location

All releases are saved to:
```
releases/v2.6.6/
├── Launcher-2.6.6-m5stack-cplus2.zip
├── Launcher-2.6.6-m5stack-cardputer.zip
├── Launcher-2.6.6-CYD-2432S028.zip
├── ...
└── Launcher-2.6.6-All.zip
```

## Compilation Time

- **Single device**: ~30-60 seconds
- **All devices**: ~1-2 hours (depending on your machine)

The scripts show progress for each environment being built.

## Requirements

- PlatformIO installed
- Python 3 (for Python script)
- Bash (for shell script)
- `zip` command available

## Customization

### Build Specific Devices Only

Edit the `ENVIRONMENTS` array in either script to include only the devices you want:

```bash
# Example: Build only M5Stack devices
ENVIRONMENTS=(
    "m5stack-cardputer"
    "m5stack-c"
    "m5stack-cplus1_1"
    "m5stack-cplus2"
    "m5stack-core"
    "m5stack-core2"
    "m5stack-cores3"
)
```

### Change Version Number

Edit the `VERSION` variable at the top of the script:
```bash
VERSION="2.6.7"  # or whatever version you want
```

## Troubleshooting

### Script Not Executable
```bash
chmod +x build_all_releases.sh
```

### PlatformIO Path Wrong
Edit the `PIO_PATH` variable in the script to match your PlatformIO installation:
```bash
# Find your PlatformIO path
which pio

# Update in script
PIO_PATH="/your/path/to/pio"
```

### Build Failures

Some environments may fail to build due to:
- Missing dependencies
- Platform-specific issues
- Memory constraints

The script will continue building other environments and report failed builds at the end.

## For GitHub Release

After running the script:

1. **Test a few builds** to ensure they work
2. **Upload to GitHub Release**:
   ```bash
   # Go to: https://github.com/wsalmi/Launcher_Salmi_Fork/releases
   # Create new release with tag v2.6.6
   # Upload files from releases/v2.6.6/
   ```

3. **Include Release Notes**:
   - Use `RELEASE_NOTES.md` as the description
   - Mention the serial commands feature
   - Credit @WSalmi

## Files Generated

Each ZIP contains:
- `Launcher-2.6.6-{device}.bin` - Ready to flash firmware

## Build Log

The Python script saves a build log to `build.log` for debugging.

## Support

For issues or questions:
- Check the main [README.md](README.md)
- Join [Discord community](https://discord.gg/BE9by2a2FF)
- Open an issue on GitHub

## Version History

- **2.6.6** - Initial release with serial commands by @WSalmi
