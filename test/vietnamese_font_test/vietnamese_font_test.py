#!/usr/bin/env python3
"""
Vietnamese Font Character Support Test

This script tests that the generated font headers contain all required
Vietnamese Unicode characters. It parses the font header files and
verifies coverage of Vietnamese alphabet.
"""

import re
import os
import sys
from pathlib import Path

# Vietnamese Unicode ranges
VIETNAMESE_CHARS = {
    # Latin Extended Additional (Vietnamese vowels with diacritics)
    'lowercase_vowels_with_diacritics': {
        'ạ': 0x1EA1, 'ả': 0x1EA3, 'ấ': 0x1EA5, 'ầ': 0x1EA7, 'ẩ': 0x1EA9,
        'ẫ': 0x1EAB, 'ậ': 0x1EAD, 'ắ': 0x1EAF, 'ằ': 0x1EB1, 'ẳ': 0x1EB3,
        'ẵ': 0x1EB5, 'ặ': 0x1EB7, 'ẹ': 0x1EB9, 'ẻ': 0x1EBB, 'ẽ': 0x1EBD,
        'ế': 0x1EBF, 'ề': 0x1EC1, 'ể': 0x1EC3, 'ễ': 0x1EC5, 'ệ': 0x1EC7,
        'ỉ': 0x1EC9, 'ị': 0x1ECB, 'ọ': 0x1ECD, 'ỏ': 0x1ECF, 'ố': 0x1ED1,
        'ồ': 0x1ED3, 'ổ': 0x1ED5, 'ỗ': 0x1ED7, 'ộ': 0x1ED9, 'ớ': 0x1EDB,
        'ờ': 0x1EDD, 'ở': 0x1EDF, 'ỡ': 0x1EE1, 'ợ': 0x1EE3, 'ụ': 0x1EE5,
        'ủ': 0x1EE7, 'ứ': 0x1EE9, 'ừ': 0x1EEB, 'ử': 0x1EED, 'ữ': 0x1EEF,
        'ự': 0x1EF1, 'ỳ': 0x1EF3, 'ỵ': 0x1EF5, 'ỷ': 0x1EF7, 'ỹ': 0x1EF9,
    },
    'uppercase_vowels_with_diacritics': {
        'Ạ': 0x1EA0, 'Ả': 0x1EA2, 'Ấ': 0x1EA4, 'Ầ': 0x1EA6, 'Ẩ': 0x1EA8,
        'Ẫ': 0x1EAA, 'Ậ': 0x1EAC, 'Ắ': 0x1EAE, 'Ằ': 0x1EB0, 'Ẳ': 0x1EB2,
        'Ẵ': 0x1EB4, 'Ặ': 0x1EB6, 'Ẹ': 0x1EB8, 'Ẻ': 0x1EBA, 'Ẽ': 0x1EBC,
        'Ế': 0x1EBE, 'Ề': 0x1EC0, 'Ể': 0x1EC2, 'Ễ': 0x1EC4, 'Ệ': 0x1EC6,
        'Ỉ': 0x1EC8, 'Ị': 0x1ECA, 'Ọ': 0x1ECC, 'Ỏ': 0x1ECE, 'Ố': 0x1ED0,
        'Ồ': 0x1ED2, 'Ổ': 0x1ED4, 'Ỗ': 0x1ED6, 'Ộ': 0x1ED8, 'Ớ': 0x1EDA,
        'Ờ': 0x1EDC, 'Ở': 0x1EDE, 'Ỡ': 0x1EE0, 'Ợ': 0x1EE2, 'Ụ': 0x1EE4,
        'Ủ': 0x1EE6, 'Ứ': 0x1EE8, 'Ừ': 0x1EEA, 'Ử': 0x1EEC, 'Ữ': 0x1EEE,
        'Ự': 0x1EF0, 'Ỳ': 0x1EF2, 'Ỵ': 0x1EF4, 'Ỷ': 0x1EF6, 'Ỹ': 0x1EF8,
    },
    # Latin Extended-A and B (special Vietnamese letters)
    'special_letters': {
        'Ă': 0x0102, 'ă': 0x0103,  # A with breve
        'Đ': 0x0110, 'đ': 0x0111,  # D with stroke
        'Ĩ': 0x0128, 'ĩ': 0x0129,  # I with tilde
        'Ũ': 0x0168, 'ũ': 0x0169,  # U with tilde
        'Ơ': 0x01A0, 'ơ': 0x01A1,  # O with horn
        'Ư': 0x01AF, 'ư': 0x01B0,  # U with horn
    },
    # Basic Latin with diacritics (also used in Vietnamese)
    'basic_with_diacritics': {
        'À': 0x00C0, 'Á': 0x00C1, 'Â': 0x00C2, 'Ã': 0x00C3,
        'È': 0x00C8, 'É': 0x00C9, 'Ê': 0x00CA,
        'Ì': 0x00CC, 'Í': 0x00CD,
        'Ò': 0x00D2, 'Ó': 0x00D3, 'Ô': 0x00D4, 'Õ': 0x00D5,
        'Ù': 0x00D9, 'Ú': 0x00DA,
        'Ý': 0x00DD,
        'à': 0x00E0, 'á': 0x00E1, 'â': 0x00E2, 'ã': 0x00E3,
        'è': 0x00E8, 'é': 0x00E9, 'ê': 0x00EA,
        'ì': 0x00EC, 'í': 0x00ED,
        'ò': 0x00F2, 'ó': 0x00F3, 'ô': 0x00F4, 'õ': 0x00F5,
        'ù': 0x00F9, 'ú': 0x00FA,
        'ý': 0x00FD,
    }
}

def extract_codepoints_from_header(header_path):
    """Extract all codepoint values from a font header file."""
    codepoints = set()

    with open(header_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Look for glyph range definitions in EpdGlyph format
    # Format: { startCodepoint, endCodepoint, offset }
    # e.g., { 0x1EA0, 0x1EF9, 0xD9 }
    range_pattern = r'\{\s*0x([0-9A-Fa-f]+)\s*,\s*0x([0-9A-Fa-f]+)\s*,\s*0x[0-9A-Fa-f]+\s*\}'
    range_matches = re.findall(range_pattern, content)
    for start_hex, end_hex in range_matches:
        start = int(start_hex, 16)
        end = int(end_hex, 16)
        for cp in range(start, end + 1):
            codepoints.add(cp)

    # Also check for individual glyphs or decimal patterns
    # Pattern for decimal codepoint as first field: { 123, ...}
    decimal_pattern = r'\{\s*(\d+)\s*,'
    decimal_matches = re.findall(decimal_pattern, content)
    for m in decimal_matches:
        cp = int(m)
        if cp > 0:
            codepoints.add(cp)

    return codepoints

def test_font_header(header_path):
    """Test a single font header for Vietnamese character support."""
    print(f"\nTesting: {header_path}")
    print("=" * 60)

    if not os.path.exists(header_path):
        print(f"  ERROR: File not found!")
        return False, 0, 0

    codepoints = extract_codepoints_from_header(header_path)
    print(f"  Found {len(codepoints)} glyphs in font")

    total_required = 0
    total_found = 0
    missing_chars = []

    for category, chars in VIETNAMESE_CHARS.items():
        found = 0
        category_missing = []

        for char, cp in chars.items():
            total_required += 1
            if cp in codepoints:
                found += 1
                total_found += 1
            else:
                category_missing.append(f"{char} (U+{cp:04X})")

        status = "✓" if found == len(chars) else "✗"
        print(f"  {status} {category}: {found}/{len(chars)}")

        if category_missing:
            missing_chars.extend(category_missing)

    coverage = (total_found / total_required * 100) if total_required > 0 else 0
    print(f"\n  Total Vietnamese coverage: {total_found}/{total_required} ({coverage:.1f}%)")

    if missing_chars:
        print(f"\n  Missing characters ({len(missing_chars)}):")
        # Group by 10 for display
        for i in range(0, min(len(missing_chars), 30), 10):
            print(f"    {', '.join(missing_chars[i:i+10])}")
        if len(missing_chars) > 30:
            print(f"    ... and {len(missing_chars) - 30} more")

    return coverage == 100, total_found, total_required

def main():
    # Find the project root
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    fonts_dir = project_root / "lib" / "EpdFont" / "builtinFonts"

    print("Vietnamese Font Character Support Test")
    print("=" * 60)
    print(f"Fonts directory: {fonts_dir}")

    if not fonts_dir.exists():
        print(f"ERROR: Fonts directory not found: {fonts_dir}")
        return 1

    # Find all pridi font headers
    font_headers = sorted(fonts_dir.glob("pridi_*.h"))

    if not font_headers:
        print("ERROR: No Pridi font headers found!")
        return 1

    print(f"Found {len(font_headers)} font header files\n")

    all_passed = True
    results = []

    for header in font_headers:
        passed, found, required = test_font_header(header)
        results.append((header.name, passed, found, required))
        if not passed:
            all_passed = False

    # Summary
    print("\n")
    print("=" * 60)
    print("SUMMARY")
    print("=" * 60)

    for name, passed, found, required in results:
        status = "PASS ✓" if passed else "FAIL ✗"
        coverage = (found / required * 100) if required > 0 else 0
        print(f"  {name:30} {status}  ({coverage:.1f}%)")

    print("\n")
    if all_passed:
        print("✓ All fonts have complete Vietnamese character support!")
        return 0
    else:
        print("✗ Some fonts are missing Vietnamese characters")
        return 1

if __name__ == "__main__":
    sys.exit(main())
