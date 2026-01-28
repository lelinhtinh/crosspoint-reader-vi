#!/bin/bash

set -e

cd "$(dirname "$0")"

# Only Pridi font for Vietnamese support
# Pridi doesn't have Italic variants - only Regular and Bold are generated
# Italic text in epub will render as Regular, BoldItalic will render as Bold

# Reader font sizes (with --2bit for better quality)
PRIDI_FONT_SIZES=(12 14 16 18)

for size in ${PRIDI_FONT_SIZES[@]}; do
  # Regular
  font_name="pridi_${size}_regular"
  font_path="../builtinFonts/source/Pridi/Pridi-Regular.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path --2bit > $output_path
  echo "Generated $output_path"

  # Bold
  font_name="pridi_${size}_bold"
  font_path="../builtinFonts/source/Pridi/Pridi-Bold.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path --2bit > $output_path
  echo "Generated $output_path"
done

# UI Font sizes (without --2bit, smaller file size)
UI_FONT_SIZES=(10 12)

for size in ${UI_FONT_SIZES[@]}; do
  # Regular
  font_name="pridi_${size}_regular"
  font_path="../builtinFonts/source/Pridi/Pridi-Regular.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path > $output_path
  echo "Generated $output_path"

  # Bold
  font_name="pridi_${size}_bold"
  font_path="../builtinFonts/source/Pridi/Pridi-Bold.ttf"
  output_path="../builtinFonts/${font_name}.h"
  python fontconvert.py $font_name $size $font_path > $output_path
  echo "Generated $output_path"
done

# Small font for version info etc. - only Regular
python fontconvert.py pridi_8_regular 8 ../builtinFonts/source/Pridi/Pridi-Regular.ttf > ../builtinFonts/pridi_8_regular.h
echo "Generated ../builtinFonts/pridi_8_regular.h"
