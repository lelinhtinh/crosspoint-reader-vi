#!/usr/bin/env bash
set -euo pipefail

echo "[pre-commit] Running clang-format..."
if ./bin/clang-format-fix; then
  if ! git diff --quiet; then
    echo "[pre-commit] clang-format changed files. Please stage the changes and commit again."
    git --no-pager diff --name-only
    exit 1
  fi
  echo "[pre-commit] clang-format OK"
else
  echo "[pre-commit] clang-format-fix failed. Attempting per-file run to show errors..."
  git ls-files --exclude-standard | grep -E '\.(c|cpp|h|hpp)$' | grep -v -E '^lib/EpdFont/builtinFonts/' | while read -r file; do
    echo "Formatting: $file"
    if ! clang-format -style=file -i "$file"; then
      echo "  ERROR: clang-format failed on $file"
    fi
  done
  echo "[pre-commit] Please ensure clang-format is installed and the script can run in your environment."
  exit 1
fi