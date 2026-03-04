#!/usr/bin/env bash
# generate_fonts_vi.sh
#
# Regenerates all built-in font .h files.
#
# fontconvert.py already includes Vietnamese glyph ranges by default:
#   - Latin Extended-B subset: Ơ/ơ (U+01A0-01A1), Ư/ư (U+01AF-01B0)
#   - Vietnamese Extended: U+1EA0–U+1EF9 (all precomposed NFC chars)
#
# This script's primary purpose (post-upstream-sync):
#   1. Replace Ubuntu UI fonts with NotoSans — Ubuntu lacks U+1EXX entirely,
#      causing garbled Vietnamese in filenames and UI labels.
#   2. Delete NotoSans reader fonts (12–18pt, 4 styles = 16 files) to
#      reclaim ~7MB flash that would otherwise overflow the app partition.
#
# Font coverage:
#   NotoSans      - Full Vietnamese support ✓ (used for UI fonts: ubuntu_10, ubuntu_12)
#   Bookerly      - Full Vietnamese support ✓ (reader font)
#   Ubuntu        - No 0x1EXX range — replaced by NotoSans for UI
#
# Usage:
#   cd lib/EpdFont
#   bash scripts/generate_fonts_vi.sh
#
# Requirements:
#   pip install freetype-py fonttools
#
# FORK NOTE: Run this script after pulling font updates from upstream,
# or when adding new font sizes. It ensures Vietnamese text renders
# correctly with utf8NextCodepointNFC() in GfxRenderer and EpdFont.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FONT_DIR="$SCRIPT_DIR/../builtinFonts"
SOURCE_DIR="$FONT_DIR/source"
PYTHON="python3"
CONVERT="$SCRIPT_DIR/fontconvert.py"

# All Vietnamese glyph ranges are now built into fontconvert.py defaults.
# No additional intervals needed.

generate() {
  local name="$1"
  local size="$2"
  local mode="$3"  # "--2bit" or ""
  shift 3
  local fonts=("$@")  # font files in priority order

  # Add --compress for 2-bit fonts to reduce flash usage (matches upstream convention)
  local compress_flag=""
  if [[ "$mode" == "--2bit" ]]; then
    compress_flag="--compress"
  fi

  echo "Generating ${name} (${size}pt)..."
  $PYTHON "$CONVERT" "${name}" "${size}" "${fonts[@]}" $mode $compress_flag \
    > "$FONT_DIR/${name}.h"
}

# ─── NotoSans ────────────────────────────────────────────────────────────────
# Sizes 14, 16, 18 are full reader fonts (all 4 styles).
# Size 8 is the SMALL_FONT_ID (regular only).
# Size 10 and 12 are generated as ubuntu_10_* and ubuntu_12_* (UI font replacement).
# NotoSans reader fonts 12pt are NOT regenerated — NotoSans is hidden
#   from reader font choices when ENABLE_VIETNAMESE_SUPPORT=1.
NS="$SOURCE_DIR/NotoSans"
for size in 8 14 16 18; do
  mode=""; [ "$size" -ge 12 ] && mode="--2bit"
  generate "notosans_${size}_regular"    $size $mode "$NS/NotoSans-Regular.ttf"
  [ "$size" -eq 8 ] && continue  # only regular at 8pt
  generate "notosans_${size}_bold"       $size $mode "$NS/NotoSans-Bold.ttf"
  generate "notosans_${size}_italic"     $size $mode "$NS/NotoSans-Italic.ttf"
  generate "notosans_${size}_bolditalic" $size $mode "$NS/NotoSans-BoldItalic.ttf"
done

# ─── Ubuntu UI fonts → replaced by NotoSans VI for Vietnamese UI ─────────────
# ubuntu_10_* is the UI_10_FONT_ID — menus, labels, file names.
# ubuntu_12_* is the UI_12_FONT_ID — screen headers (page titles, etc.).
# Both are regenerated as NotoSans 1-bit with Vietnamese glyph coverage so that
# Vietnamese file names and UI labels render correctly.
for size in 10 12; do
  for style in regular bold; do
    TTF="$NS/NotoSans-$(echo $style | sed 's/bold/Bold/;s/regular/Regular/').ttf"
    generate "ubuntu_${size}_${style}" $size "" "$TTF"
  done
done

# ─── Bookerly ────────────────────────────────────────────────────────────────
BK="$SOURCE_DIR/Bookerly"
for size in 12 14 16 18; do
  generate "bookerly_${size}_regular"    $size --2bit "$BK/Bookerly-Regular.ttf"
  generate "bookerly_${size}_bold"       $size --2bit "$BK/Bookerly-Bold.ttf"
  generate "bookerly_${size}_italic"     $size --2bit "$BK/Bookerly-Italic.ttf"
  generate "bookerly_${size}_bolditalic" $size --2bit "$BK/Bookerly-BoldItalic.ttf"
done


echo ""
echo "Done! All fonts regenerated with Vietnamese glyph support."
echo "Verify with: grep '1EA0' lib/EpdFont/builtinFonts/notosans_16_regular.h"

# ─── Remove NotoSans reader fonts (12–18pt) ────────────────────────────────
# When ENABLE_VIETNAMESE_SUPPORT=1, NotoSans is excluded from reader font
# choices (and guarded by #if !ENABLE_VIETNAMESE_SUPPORT in all.h / main.cpp).
# Deleting these 16 files reclaims ~7 MB of flash, making room for Vietnamese
# font data in Bookerly/ubuntu.
echo "Removing unused NotoSans reader fonts (12–18pt)..."
rm -f \
  "$FONT_DIR/notosans_12_regular.h" \
  "$FONT_DIR/notosans_12_bold.h" \
  "$FONT_DIR/notosans_12_italic.h" \
  "$FONT_DIR/notosans_12_bolditalic.h" \
  "$FONT_DIR/notosans_14_regular.h" \
  "$FONT_DIR/notosans_14_bold.h" \
  "$FONT_DIR/notosans_14_italic.h" \
  "$FONT_DIR/notosans_14_bolditalic.h" \
  "$FONT_DIR/notosans_16_regular.h" \
  "$FONT_DIR/notosans_16_bold.h" \
  "$FONT_DIR/notosans_16_italic.h" \
  "$FONT_DIR/notosans_16_bolditalic.h" \
  "$FONT_DIR/notosans_18_regular.h" \
  "$FONT_DIR/notosans_18_bold.h" \
  "$FONT_DIR/notosans_18_italic.h" \
  "$FONT_DIR/notosans_18_bolditalic.h"
echo "Done. notosans_8_regular.h (SMALL_FONT) kept."

# ─── Remove OpenDyslexic fonts (all sizes) ────────────────────────────────
# OpenDyslexic is not used in this Vietnamese fork. Delete any leftover files
# from previous builds or upstream checkouts to keep the build clean.
echo "Removing OpenDyslexic fonts (not used in VI fork)..."
rm -f \
  "$FONT_DIR/opendyslexic_8_regular.h" \
  "$FONT_DIR/opendyslexic_8_bold.h" \
  "$FONT_DIR/opendyslexic_8_italic.h" \
  "$FONT_DIR/opendyslexic_8_bolditalic.h" \
  "$FONT_DIR/opendyslexic_10_regular.h" \
  "$FONT_DIR/opendyslexic_10_bold.h" \
  "$FONT_DIR/opendyslexic_10_italic.h" \
  "$FONT_DIR/opendyslexic_10_bolditalic.h" \
  "$FONT_DIR/opendyslexic_12_regular.h" \
  "$FONT_DIR/opendyslexic_12_bold.h" \
  "$FONT_DIR/opendyslexic_12_italic.h" \
  "$FONT_DIR/opendyslexic_12_bolditalic.h" \
  "$FONT_DIR/opendyslexic_14_regular.h" \
  "$FONT_DIR/opendyslexic_14_bold.h" \
  "$FONT_DIR/opendyslexic_14_italic.h" \
  "$FONT_DIR/opendyslexic_14_bolditalic.h"
echo "Done."
