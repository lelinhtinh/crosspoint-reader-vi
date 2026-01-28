#!/usr/bin/env bash
# Run Vietnamese font character support test
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

python3 vietnamese_font_test.py "$@"
