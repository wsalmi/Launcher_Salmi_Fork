#!/bin/bash
#
# Build all Launcher firmware releases and create ZIP packages
# Version: 2.6.6
# Author: WSalmi
#

VERSION="2.6.6"
PIO_PATH="/Users/wesleysalmi/source-me/serialPoc/.venv/bin/pio"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/.pio/build"
RELEASE_DIR="${PROJECT_ROOT}/releases/v${VERSION}"
ALL_ZIP="${RELEASE_DIR}/Launcher-${VERSION}-All.zip"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# All available environments
ENVIRONMENTS=(
    # Arduino
    "arduino-nesso-n1"

    # CYD (Cheap Yellow Display)
    "CYD-2432S028"
    "CYD-2-USB"
    "CYD-2432W328C"
    "CYD-2432W328C_2"
    "CYD-2432S024R"
    "CYD-2432W328R"
    "CYD-2432S022C"
    "CYD-2432S032C"
    "CYD-2432S032R"
    "CYD-3248S035C"
    "CYD-3248S035R"
    "CYD-8048S043C"
    "CYD-8048W550C"
    "CYD-3248W535C"
    "CYD-4827S043R"

    # Headless
    "headless-esp32-4mb"
    "headless-esp32-8mb"
    "headless-esp32s3-4mb"
    "headless-esp32s3-8mb"
    "headless-esp32s3-16mb"

    # Elecrow
    "elecrow-24B"
    "elecrow-28B"
    "elecrow-35B"
    "elecrow-35Bv2_2"

    # Lilygo
    "lilygo-t-deck-pro"
    "lilygo-t-deck"
    "lilygo-t-display-S3-amoled"
    "lilygo-t-display-S3-pro"
    "lilygo-t-display-S3-touch"
    "lilygo-t-dongle-s3-tft"
    "lilygo-t-embed"
    "lilygo-t-embed-cc1101"
    "lilygo-t-lora-pager"
    "lilygo-t5-epaper-s3-pro"
    "lilygo-t-hmi"

    # M5Stack
    "m5stack-cardputer"
    "m5stack-core"
    "m5stack-core-4Mb"
    "m5stack-core2"
    "m5stack-cores3"
    "m5stack-cplus1_1"
    "m5stack-c"
    "m5stack-cplus2"
    "m5stack-tab5"
    "m5stack-paper"
    "m5stack-paper-s3"

    # Marauder & Others
    "Marauder-Mini"
    "Marauder-v7"
    "Awok-Mini"
    "Marauder-v4-OG"
    "Marauder-v61"
    "Awok-Touch"
    "Phantom_S024R"
    "smoochiee-board"
    "WaveSentry-R1"
)

# Print header
print_header() {
    echo -e "\n${BLUE}================================================================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}================================================================================${NC}\n"
}

# Build environment
build_env() {
    local env=$1
    local num=$2
    local total=$3

    printf "\n[%d/%d] Building %s...\n" "$num" "$total" "$env"

    if "${PIO_PATH}" run -e "${env}"; then
        if [ -f "${BUILD_DIR}/${env}/firmware.bin" ]; then
            local size=$(du -h "${BUILD_DIR}/${env}/firmware.bin" | cut -f1)
            printf "✅ %s built successfully (%s)\n" "$env" "$size"
            return 0
        else
            printf "⚠️  %s built but firmware.bin not found\n" "$env"
            return 1
        fi
    else
        printf "❌ %s failed to build\n" "$env"
        return 1
    fi
}

# Create individual ZIP
create_zip() {
    local env=$1
    local firmware="${BUILD_DIR}/${env}/firmware.bin"
    local zip_name="Launcher-${VERSION}-${env}.zip"
    local zip_path="${RELEASE_DIR}/${zip_name}"

    if [ ! -f "${firmware}" ]; then
        return 1
    fi

    # Copy firmware with proper name
    local firmware_name="Launcher-${VERSION}-${env}.bin"
    cp "${firmware}" "${RELEASE_DIR}/${firmware_name}"

    # Create ZIP
    (cd "${RELEASE_DIR}" && zip -q "${zip_name}" "${firmware_name}" && rm "${firmware_name}")

    if [ -f "${zip_path}" ]; then
        local size=$(du -h "${zip_path}" | cut -f1)
        printf "📦 Created %s (%s)\n" "$zip_name" "$size"
        return 0
    else
        printf "❌ Failed to create ZIP for %s\n" "$env"
        return 1
    fi
}

# Main
main() {
    print_header "Launcher v${VERSION} - Build All Releases"

    # Create release directory
    mkdir -p "${RELEASE_DIR}"
    printf "📁 Release directory: %s\n\n" "$RELEASE_DIR"

    # Track results
    local successful=0
    local failed=0
    local total=${#ENVIRONMENTS[@]}
    declare -a failed_envs

    print_header "Building ${total} environments"

    # Build all environments
    for i in "${!ENVIRONMENTS[@]}"; do
        local env="${ENVIRONMENTS[$i]}"
        local num=$((i + 1))

        if build_env "${env}" "${num}" "${total}"; then
            create_zip "${env}"
            ((successful++))
        else
            failed_envs+=("${env}")
            ((failed++))
        fi
        echo ""
    done

    # Create All.zip
    if [ ${successful} -gt 0 ]; then
        print_header "Creating All.zip with ${successful} firmwares"

        rm -f "${ALL_ZIP}"

        # Add all firmware files to All.zip
        for env in "${ENVIRONMENTS[@]}"; do
            local firmware="${BUILD_DIR}/${env}/firmware.bin"
            if [ -f "${firmware}" ]; then
                local firmware_name="Launcher-${VERSION}-${env}.bin"
                cp "${firmware}" "${RELEASE_DIR}/${firmware_name}"
                (cd "${RELEASE_DIR}" && zip -q "$(basename "${ALL_ZIP}")" "${firmware_name}" && rm "${firmware_name}")
            fi
        done

        if [ -f "${ALL_ZIP}" ]; then
            local size=$(du -h "${ALL_ZIP}" | cut -f1)
            printf "✅ Created All.zip (%s)\n" "$size"
            printf "   Location: %s\n" "$ALL_ZIP"
        fi
    fi

    # Summary
    print_header "Build Summary"
    printf "✅ Successful: %d/%d\n" "$successful" "$total"
    printf "❌ Failed: %d\n" "$failed"

    if [ ${#failed_envs[@]} -gt 0 ]; then
        printf "\nFailed builds:\n"
        for env in "${failed_envs[@]}"; do
            printf "  - %s\n" "$env"
        done
    fi

    printf "\n📁 All releases available at: %s\n\n" "$RELEASE_DIR"

    return $failed
}

# Run
main
exit $?
