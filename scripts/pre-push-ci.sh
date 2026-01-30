#!/usr/bin/env bash
set -euo pipefail

echo "[pre-push] Running full CI: build & tests..."

# Build
echo "[pre-push] Building with PlatformIO..."
pio run

echo "[pre-push] Build OK"

# Semver tests
echo "[pre-push] Running semver tests..."
g++ -I src -std=c++17 test/semver/test_semver.cpp src/network/semver.cpp -o test/semver/test_semver
./test/semver/test_semver

echo "[pre-push] Semver tests OK"

# Hyphenation evaluation
echo "[pre-push] Running hyphenation evaluation tests..."
chmod +x test/hyphenation_eval/run_hyphenation_eval.sh
./test/hyphenation_eval/run_hyphenation_eval.sh

echo "[pre-push] Hyphenation tests OK"

# Vietnamese font tests
echo "[pre-push] Running Vietnamese font tests..."
chmod +x test/vietnamese_font_test/run_test.sh
./test/vietnamese_font_test/run_test.sh

echo "[pre-push] Vietnamese font tests OK"

echo "[pre-push] Full CI passed"