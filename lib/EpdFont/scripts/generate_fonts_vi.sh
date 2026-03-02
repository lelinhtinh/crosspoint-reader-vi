#!/usr/bin/env bash
# generate_fonts_vi.sh
#
# Regenerates all built-in font .h files with Vietnamese glyph support.
#
# fontconvert.py already includes Latin Extended-A (U+0100–U+017F) by default,
# so we only need to add:
#   - Latin Extended-B (U+0180–U+024F): base chars ơ (U+01A1), ư (U+01B0), đ (U+0111)
#   - Latin Extended Additional (U+1E00–U+1EFF): all 134 precomposed Vietnamese NFC chars
#
# fontconvert.py also has Cyrillic, Combining Diacritical Marks, Math symbols
# and Arrows commented out in this fork (not used in Vietnamese ebooks).
#
# Font coverage:
#   NotoSans      - Full Vietnamese support ✓ (used for UI fonts: ubuntu_10, ubuntu_12)
#   Bookerly      - Full Vietnamese support ✓ (reader font)
#   OpenDyslexic  - Full Vietnamese support ✓ (reader font)
#   Ubuntu        - Partial (lacks full 0x1EXX range) — skipped
#
# Usage:
#   cd lib/EpdFont
#   bash scripts/generate_fonts_vi.sh
#
# Requirements:
#   pip install freetype-py
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

# Latin Extended-A (U+0100–U+017F) is already in fontconvert.py defaults.
# Only Extended-B and Latin Extended Additional are needed as additions.
VI_INTERVALS="--additional-intervals 0x0180,0x024F --additional-intervals 0x1E00,0x1EFF"

generate() {
  local name="$1"
  local size="$2"
  local mode="$3"  # "--2bit" or ""
  shift 3
  local fonts=("$@")  # font files in priority order

  echo "Generating ${name} (${size}pt)..."
  $PYTHON "$CONVERT" "${name}" "${size}" "${fonts[@]}" $VI_INTERVALS $mode \
    > "$FONT_DIR/${name}.h"
}

# ─── NotoSans ────────────────────────────────────────────────────────────────
# Sizes 14, 16, 18 are full reader fonts (all 4 styles).
# Size 8 is the SMALL_FONT_ID (regular only).
# Size 10 and 12 are generated as ubuntu_10_* and ubuntu_12_* (UI font replacement).
# Size 12 as reader font (notosans_12) is NOT regenerated — NotoSans is hidden
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

# ─── OpenDyslexic ────────────────────────────────────────────────────────────
OD="$SOURCE_DIR/OpenDyslexic"
for size in 8 10 12 14; do
  mode=""; [ "$size" -ge 12 ] && mode="--2bit"
  generate "opendyslexic_${size}_regular"    $size $mode "$OD/OpenDyslexic-Regular.otf"
  generate "opendyslexic_${size}_bold"       $size $mode "$OD/OpenDyslexic-Bold.otf"
  generate "opendyslexic_${size}_italic"     $size $mode "$OD/OpenDyslexic-Italic.otf"
  generate "opendyslexic_${size}_bolditalic" $size $mode "$OD/OpenDyslexic-BoldItalic.otf"
done

echo ""
echo "Done! All fonts regenerated with Vietnamese glyph support."
echo "Verify with: grep '0x1E' lib/EpdFont/builtinFonts/notosans_16_regular.h"

# ─── Remove NotoSans reader fonts (12–18pt) ────────────────────────────────
# When ENABLE_VIETNAMESE_SUPPORT=1, NotoSans is excluded from reader font
# choices (and guarded by #if !ENABLE_VIETNAMESE_SUPPORT in all.h / main.cpp).
# Deleting these 13 files reclaims ~7 MB of flash, making room for Vietnamese
# font data in Bookerly/OpenDyslexic/ubuntu.
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
