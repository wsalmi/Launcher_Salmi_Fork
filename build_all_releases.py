#!/usr/bin/env python3
"""
Build all Launcher firmware releases and create ZIP packages
Version: 2.6.6
Author: WSalmi
"""

import os
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

# Configuration
VERSION = "2.6.6"
PIO_PATH = "/Users/wesleysalmi/source-me/serialPoc/.venv/bin/pio"
PROJECT_ROOT = Path(__file__).parent
BUILD_DIR = PROJECT_ROOT / ".pio" / "build"
RELEASE_DIR = PROJECT_ROOT / "releases" / f"v{VERSION}"
ALL_ZIP = RELEASE_DIR / f"Launcher-{VERSION}-All.zip"

# All available environments from platformio.ini
ENVIRONMENTS = [
    # Arduino
    "arduino-nesso-n1",

    # CYD (Cheap Yellow Display)
    "CYD-2432S028",
    "CYD-2-USB",
    "CYD-2432W328C",
    "CYD-2432W328C_2",
    "CYD-2432S024R",
    "CYD-2432W328R",
    "CYD-2432S022C",
    "CYD-2432S032C",
    "CYD-2432S032R",
    "CYD-3248S035C",
    "CYD-3248S035R",
    "CYD-8048S043C",
    "CYD-8048W550C",
    "CYD-3248W535C",
    "CYD-4827S043R",

    # Headless
    "headless-esp32-4mb",
    "headless-esp32-8mb",
    "headless-esp32s3-4mb",
    "headless-esp32s3-8mb",
    "headless-esp32s3-16mb",

    # Elecrow
    "elecrow-24B",
    "elecrow-28B",
    "elecrow-35B",
    "elecrow-35Bv2_2",

    # Lilygo
    "lilygo-t-deck-pro",
    "lilygo-t-deck",
    "lilygo-t-display-S3-amoled",
    "lilygo-t-display-S3-pro",
    "lilygo-t-display-S3-touch",
    "lilygo-t-dongle-s3-tft",
    "lilygo-t-embed",
    "lilygo-t-embed-cc1101",
    "lilygo-t-lora-pager",
    "lilygo-t5-epaper-s3-pro",
    "lilygo-t-hmi",

    # M5Stack
    "m5stack-cardputer",
    "m5stack-core",
    "m5stack-core-4Mb",
    "m5stack-core2",
    "m5stack-cores3",
    "m5stack-cplus1_1",
    "m5stack-c",
    "m5stack-cplus2",
    "m5stack-tab5",
    "m5stack-paper",
    "m5stack-paper-s3",

    # Marauder & Others
    "Marauder-Mini",
    "Marauder-v7",
    "Awok-Mini",
    "Marauder-v4-OG",
    "Marauder-v61",
    "Awok-Touch",
    "Phantom_S024R",
    "smoochiee-board",
    "WaveSentry-R1",
]


def print_header(text):
    """Print formatted header"""
    print("\n" + "=" * 80)
    print(f"  {text}")
    print("=" * 80 + "\n")


def build_environment(env_name):
    """Build a specific environment"""
    print(f"📦 Building {env_name}...")

    try:
        result = subprocess.run(
            [PIO_PATH, "run", "-e", env_name],
            cwd=PROJECT_ROOT,
            capture_output=True,
            text=True,
            check=True
        )

        # Check if firmware.bin exists
        firmware_path = BUILD_DIR / env_name / "firmware.bin"
        if firmware_path.exists():
            print(f"✅ {env_name} built successfully ({firmware_path.stat().st_size:,} bytes)")
            return True
        else:
            print(f"⚠️  {env_name} built but firmware.bin not found")
            return False

    except subprocess.CalledProcessError as e:
        print(f"❌ {env_name} failed to build")
        print(f"   Error: {e.stderr[:200] if e.stderr else 'Unknown error'}")
        return False
    except Exception as e:
        print(f"❌ {env_name} failed with exception: {e}")
        return False


def create_individual_zip(env_name):
    """Create a ZIP file for a specific environment"""
    firmware_path = BUILD_DIR / env_name / "firmware.bin"

    if not firmware_path.exists():
        return None

    # Create ZIP for this environment
    zip_name = f"Launcher-{VERSION}-{env_name}.zip"
    zip_path = RELEASE_DIR / zip_name

    try:
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            zipf.write(firmware_path, f"Launcher-{VERSION}-{env_name}.bin")

        print(f"📦 Created {zip_name} ({zip_path.stat().st_size:,} bytes)")
        return zip_path
    except Exception as e:
        print(f"❌ Failed to create ZIP for {env_name}: {e}")
        return None


def create_all_zip(individual_zips):
    """Create All.zip containing all individual firmware files"""
    print_header(f"Creating All.zip with {len(individual_zips)} firmwares")

    try:
        with zipfile.ZipFile(ALL_ZIP, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for env_name in individual_zips:
                firmware_path = BUILD_DIR / env_name / "firmware.bin"
                if firmware_path.exists():
                    zipf.write(firmware_path, f"Launcher-{VERSION}-{env_name}.bin")

        print(f"✅ Created All.zip ({ALL_ZIP.stat().st_size:,} bytes)")
        print(f"   Location: {ALL_ZIP}")
        return True
    except Exception as e:
        print(f"❌ Failed to create All.zip: {e}")
        return False


def main():
    """Main build process"""
    print_header(f"Launcher v{VERSION} - Build All Releases")

    # Create release directory
    RELEASE_DIR.mkdir(parents=True, exist_ok=True)
    print(f"📁 Release directory: {RELEASE_DIR}\n")

    # Track results
    successful_builds = []
    failed_builds = []
    individual_zips = []

    # Build all environments
    print_header(f"Building {len(ENVIRONMENTS)} environments")

    for i, env in enumerate(ENVIRONMENTS, 1):
        print(f"\n[{i}/{len(ENVIRONMENTS)}] ", end="")

        if build_environment(env):
            successful_builds.append(env)

            # Create individual ZIP
            zip_path = create_individual_zip(env)
            if zip_path:
                individual_zips.append(env)
        else:
            failed_builds.append(env)

    # Create All.zip
    if individual_zips:
        create_all_zip(individual_zips)

    # Summary
    print_header("Build Summary")
    print(f"✅ Successful: {len(successful_builds)}/{len(ENVIRONMENTS)}")
    print(f"📦 ZIPs created: {len(individual_zips)}")
    print(f"❌ Failed: {len(failed_builds)}")

    if failed_builds:
        print("\nFailed builds:")
        for env in failed_builds:
            print(f"  - {env}")

    print(f"\n📁 All releases available at: {RELEASE_DIR}")

    # Return exit code
    return 0 if not failed_builds else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n\n⚠️  Build process interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n\n❌ Unexpected error: {e}")
        sys.exit(1)
