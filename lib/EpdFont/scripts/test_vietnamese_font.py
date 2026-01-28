#!/usr/bin/env python3
"""
Test script to preview Vietnamese font rendering at different sizes.
This helps verify that Vietnamese diacritics are rendered correctly.
"""

import freetype
from PIL import Image, ImageDraw
import sys
import os

# Vietnamese test strings
VIETNAMESE_SAMPLES = [
    # Basic vowels with all tones
    "À Á Ả Ã Ạ  à á ả ã ạ",
    "Ă Ắ Ằ Ẳ Ẵ Ặ  ă ắ ằ ẳ ẵ ặ",
    "Â Ấ Ầ Ẩ Ẫ Ậ  â ấ ầ ẩ ẫ ậ",
    "È É Ẻ Ẽ Ẹ  è é ẻ ẽ ẹ",
    "Ê Ế Ề Ể Ễ Ệ  ê ế ề ể ễ ệ",
    "Ì Í Ỉ Ĩ Ị  ì í ỉ ĩ ị",
    "Ò Ó Ỏ Õ Ọ  ò ó ỏ õ ọ",
    "Ô Ố Ồ Ổ Ỗ Ộ  ô ố ồ ổ ỗ ộ",
    "Ơ Ớ Ờ Ở Ỡ Ợ  ơ ớ ờ ở ỡ ợ",
    "Ù Ú Ủ Ũ Ụ  ù ú ủ ũ ụ",
    "Ư Ứ Ừ Ử Ữ Ự  ư ứ ừ ử ữ ự",
    "Ỳ Ý Ỷ Ỹ Ỵ  ỳ ý ỷ ỹ ỵ",
    "Đ đ",
    "",
    # Real Vietnamese text samples
    "Việt Nam đất nước xinh đẹp",
    "Xin chào! Tôi là người Việt Nam.",
    "Ậy ệch, Ộp ệp, Ự ỹ - dấu kép",
    "0123456789 !@#$%^&*()",
]

# Font sizes to test
FONT_SIZES = [8, 10, 12, 14, 16, 18, 20]

def render_text_to_image(face, text, size):
    """Render text using freetype and return as PIL Image."""
    face.set_char_size(size * 64, size * 64, 150, 150)

    # Calculate text dimensions
    width = 0
    max_ascender = 0
    max_descender = 0

    for char in text:
        face.load_char(char, freetype.FT_LOAD_RENDER)
        width += face.glyph.advance.x >> 6
        bitmap_top = face.glyph.bitmap_top
        bitmap_height = face.glyph.bitmap.rows

        if bitmap_top > max_ascender:
            max_ascender = bitmap_top
        descender = bitmap_height - bitmap_top
        if descender > max_descender:
            max_descender = descender

    height = max_ascender + max_descender + 4
    if height < size + 10:
        height = size + 10

    # Create image
    img = Image.new('L', (max(width + 10, 100), height), 255)

    # Render each character
    x = 5
    baseline = max_ascender + 2

    for char in text:
        face.load_char(char, freetype.FT_LOAD_RENDER)
        bitmap = face.glyph.bitmap

        # Draw bitmap
        for row in range(bitmap.rows):
            for col in range(bitmap.width):
                pixel = bitmap.buffer[row * bitmap.width + col]
                if pixel > 0:
                    px = x + face.glyph.bitmap_left + col
                    py = baseline - face.glyph.bitmap_top + row
                    if 0 <= px < img.width and 0 <= py < img.height:
                        # Blend with existing pixel
                        existing = img.getpixel((px, py))
                        new_val = max(0, existing - pixel)
                        img.putpixel((px, py), new_val)

        x += face.glyph.advance.x >> 6

    return img

def test_font(font_path, output_dir="test_output"):
    """Test a font file with Vietnamese characters."""

    if not os.path.exists(font_path):
        print(f"Error: Font file not found: {font_path}")
        return False

    os.makedirs(output_dir, exist_ok=True)

    face = freetype.Face(font_path)
    font_name = os.path.basename(font_path).replace('.ttf', '').replace('.otf', '')

    print(f"\n{'='*60}")
    print(f"Testing font: {font_name}")
    print(f"{'='*60}")

    # Check Vietnamese character support
    print("\nChecking Vietnamese character support...")
    missing_chars = []
    test_chars = "ẠẢẤẦẨẪẬẮẰẲẴẶẸẺẼẾỀỂỄỆỈỊỌỎỐỒỔỖỘỚỜỞỠỢỤỦỨỪỬỮỰỲỴỶỸĐƠƯ"
    test_chars += test_chars.lower()

    for char in test_chars:
        glyph_index = face.get_char_index(ord(char))
        if glyph_index == 0:
            missing_chars.append(char)

    if missing_chars:
        print(f"⚠️  Missing characters: {''.join(sorted(set(missing_chars)))}")
    else:
        print("✓ All Vietnamese characters are supported!")

    # Generate test images for each size
    for size in FONT_SIZES:
        print(f"\nSize {size}pt:")

        # Combine all samples into one image
        images = []
        for sample in VIETNAMESE_SAMPLES:
            if sample:
                img = render_text_to_image(face, sample, size)
                images.append(img)
            else:
                # Empty line
                images.append(Image.new('L', (100, 5), 255))

        # Calculate total height
        total_height = sum(img.height for img in images) + 20
        max_width = max(img.width for img in images) + 20

        # Create combined image
        combined = Image.new('L', (max_width, total_height), 255)
        y = 10
        for img in images:
            combined.paste(img, (10, y))
            y += img.height

        # Save image
        output_path = os.path.join(output_dir, f"{font_name}_{size}pt.png")
        combined.save(output_path)
        print(f"  Saved: {output_path}")

        # Print metrics
        face.set_char_size(size * 64, size * 64, 150, 150)
        ascender = face.size.ascender >> 6
        descender = face.size.descender >> 6
        height = face.size.height >> 6
        print(f"  Metrics: height={height}, ascender={ascender}, descender={descender}")

    print(f"\n✓ Test images saved to: {output_dir}/")
    return True

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Default font path
    default_font = os.path.join(script_dir, "../builtinFonts/source/Pridi/Pridi-Regular.ttf")

    if len(sys.argv) > 1:
        font_path = sys.argv[1]
    else:
        font_path = default_font

    output_dir = os.path.join(script_dir, "test_output")

    print("Vietnamese Font Test Tool")
    print("=" * 60)

    # Test Regular
    test_font(font_path, output_dir)

    # Also test Bold if available
    bold_path = font_path.replace("-Regular", "-Bold").replace("_regular", "_bold")
    if os.path.exists(bold_path) and bold_path != font_path:
        test_font(bold_path, output_dir)

    print("\n" + "=" * 60)
    print("Test complete!")
    print(f"Open the PNG files in {output_dir}/ to verify rendering.")
    print("\nThings to check:")
    print("  1. Diacritics are not cut off (especially Ậ, Ệ, Ộ, Ự)")
    print("  2. Characters are readable at each size")
    print("  3. Spacing between characters looks correct")
    print("  4. Đ and đ render correctly")

if __name__ == "__main__":
    main()
