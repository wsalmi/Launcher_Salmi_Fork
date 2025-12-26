#!/bin/bash
# Simple wrapper to run build with progress monitoring

cd /Users/wesleysalmi/source-me/Launcher_Salmi_Fork

echo "=================================="
echo "  Starting Full Build Process"
echo "=================================="
echo ""
echo "This will compile all 70+ firmware variants."
echo "Estimated time: 1-2 hours"
echo ""
echo "Progress will be shown in real-time."
echo "Results will be in: releases/v2.6.6/"
echo ""
echo "Press Ctrl+C to abort"
echo ""
sleep 3

./build_all_releases.sh
