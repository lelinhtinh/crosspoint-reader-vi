#!/usr/bin/env bash
set -euo pipefail

echo "[pre-commit] Running cppcheck (PlatformIO)..."
if pio check --fail-on-defect low --fail-on-defect medium --fail-on-defect high; then
  echo "[pre-commit] cppcheck OK"
else
  echo "[pre-commit] 'pio' failed; trying 'python -m platformio'..."
  if python -m platformio check --fail-on-defect low --fail-on-defect medium --fail-on-defect high; then
    echo "[pre-commit] cppcheck OK (via python -m platformio)"
  else
    echo "[pre-commit] cppcheck step failed. Please ensure PlatformIO is installed (pip install -U platformio)."
    exit 1
  fi
fi