#!/bin/bash

set -e

cd "$(dirname "$0")"

# Andada Pro font for Vietnamese support
# Andada Pro doesn't have Italic variants - only Regular and Bold are generated
# Italic text in epub will render as Regular, BoldItalic will render as Bold

# Reader font sizes (with --2bit for better quality)
READER_FONT_SIZES=(12 14 16 18)

for size in ${READER_FONT_SIZES[@]}; do
  # Regular
  font_name="system_font_${size}_regular"
  font_path="../builtinFonts/source/Andada_Pro/AndadaPro-Regular.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path --2bit > $output_path
  echo "Generated $output_path"

  # Bold
  font_name="system_font_${size}_bold"
  font_path="../builtinFonts/source/Andada_Pro/AndadaPro-Bold.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path --2bit > $output_path
  echo "Generated $output_path"
done

# UI Font sizes (without --2bit, smaller file size)
UI_FONT_SIZES=(10 12)

for size in ${UI_FONT_SIZES[@]}; do
  # Regular
  font_name="system_font_${size}_regular"
  font_path="../builtinFonts/source/Andada_Pro/AndadaPro-Regular.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path > $output_path
  echo "Generated $output_path"

  # Bold
  font_name="system_font_${size}_bold"
  font_path="../builtinFonts/source/Andada_Pro/AndadaPro-Bold.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path > $output_path
  echo "Generated $output_path"
done

# Small font for version info etc. - only Regular
python fontconvert.py system_font_8_regular 8 ../builtinFonts/source/Andada_Pro/AndadaPro-Regular.ttf > ../builtinFonts/system_font_8_regular.h
echo "Generated ../builtinFonts/system_font_8_regular.h"
