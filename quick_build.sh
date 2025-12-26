#!/bin/bash
#
# Quick Build Script - Compile specific environments for testing
# Version: 2.6.6
#

VERSION="2.6.6"
PIO="/Users/wesleysalmi/source-me/serialPoc/.venv/bin/pio"

# Test with a few environments first
ENVS=(
    "m5stack-cplus2"
    "m5stack-cardputer"
    "m5stack-c"
)

echo "Building test environments..."

for env in "${ENVS[@]}"; do
    echo ""
    echo "Building $env..."
    "$PIO" run -e "$env"

    if [ -f ".pio/build/$env/firmware.bin" ]; then
        echo "✅ $env successful"
    else
        echo "❌ $env failed"
    fi
done

echo ""
echo "Done!"
