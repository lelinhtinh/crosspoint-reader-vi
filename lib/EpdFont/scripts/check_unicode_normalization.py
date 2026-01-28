#!/usr/bin/env python3
"""
Script to check Unicode normalization form (NFC vs NFD) in text files or epub.
Vietnamese text can be encoded in two ways:
  - NFC (Precomposed): ũ = U+0169 (single codepoint)
  - NFD (Decomposed): ũ = u (U+0075) + combining tilde (U+0303)

If epub uses NFD but font only supports NFC, characters will be missing.
"""

import unicodedata
import sys
import os


def analyze_text(text: str, label: str = "Text"):
    """Analyze a text string for Unicode normalization."""
    print(f"\n{'='*60}")
    print(f"Analyzing: {label}")
    print(f"{'='*60}")

    # Check overall normalization
    is_nfc = unicodedata.is_normalized('NFC', text)
    is_nfd = unicodedata.is_normalized('NFD', text)

    print(f"Length: {len(text)} characters")
    print(f"Is NFC normalized: {is_nfc}")
    print(f"Is NFD normalized: {is_nfd}")

    # Find combining characters
    combining_chars = []
    for i, char in enumerate(text):
        if unicodedata.category(char) == 'Mn':  # Mark, Nonspacing
            combining_chars.append((i, char, ord(char)))

    if combining_chars:
        print(f"\n⚠️  Found {len(combining_chars)} combining characters (NFD form):")
        for pos, char, code in combining_chars[:20]:  # Show first 20
            name = unicodedata.name(char, 'UNKNOWN')
            # Show context
            start = max(0, pos - 2)
            end = min(len(text), pos + 2)
            context = text[start:end]
            print(f"  Position {pos}: U+{code:04X} ({name})")
            print(f"    Context: '{context}'")
    else:
        print("\n✓ No combining characters found (text is in NFC form)")

    # Find Vietnamese-specific characters
    vietnamese_chars = []
    for i, char in enumerate(text):
        code = ord(char)
        # Check various Vietnamese ranges
        if (0x0102 <= code <= 0x0103 or  # Ă ă
            0x0110 <= code <= 0x0111 or  # Đ đ
            0x0128 <= code <= 0x0129 or  # Ĩ ĩ
            0x0168 <= code <= 0x0169 or  # Ũ ũ
            0x01A0 <= code <= 0x01A1 or  # Ơ ơ
            0x01AF <= code <= 0x01B0 or  # Ư ư
            0x1EA0 <= code <= 0x1EF9):   # Vietnamese vowels with diacritics
            vietnamese_chars.append((char, code))

    if vietnamese_chars:
        unique_chars = sorted(set(vietnamese_chars), key=lambda x: x[1])
        print(f"\n✓ Found {len(unique_chars)} unique Vietnamese characters (NFC form):")
        chars_str = ' '.join(c for c, _ in unique_chars)
        print(f"  {chars_str}")

    return is_nfc, is_nfd, len(combining_chars)


def check_file(filepath: str):
    """Check a text file for Unicode normalization."""
    if not os.path.exists(filepath):
        print(f"Error: File not found: {filepath}")
        return

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    analyze_text(content, filepath)


def check_epub(epub_path: str):
    """Check an epub file for Unicode normalization."""
    import zipfile

    if not os.path.exists(epub_path):
        print(f"Error: File not found: {epub_path}")
        return

    print(f"\n{'='*60}")
    print(f"Checking EPUB: {epub_path}")
    print(f"{'='*60}")

    total_combining = 0
    files_with_nfd = []

    with zipfile.ZipFile(epub_path, 'r') as zf:
        for name in zf.namelist():
            if name.endswith(('.html', '.xhtml', '.htm', '.xml')):
                try:
                    content = zf.read(name).decode('utf-8')
                    is_nfc, is_nfd, combining_count = analyze_text(content, name)
                    if combining_count > 0:
                        total_combining += combining_count
                        files_with_nfd.append((name, combining_count))
                except Exception as e:
                    print(f"Error reading {name}: {e}")

    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")

    if total_combining > 0:
        print(f"⚠️  EPUB contains NFD (decomposed) text!")
        print(f"Total combining characters: {total_combining}")
        print(f"Files with NFD text:")
        for name, count in files_with_nfd:
            print(f"  - {name}: {count} combining chars")
        print("\nSolution options:")
        print("  1. Convert epub to NFC: Run normalize_epub_nfc.py")
        print("  2. Add NFC normalization to firmware (complex)")
    else:
        print("✓ EPUB uses NFC (precomposed) text - should work correctly")


def demo():
    """Demo showing NFC vs NFD difference."""
    print("\n" + "="*60)
    print("DEMO: NFC vs NFD for Vietnamese text")
    print("="*60)

    test_words = ['cũng', 'Nhĩ', 'Căn', 'Việt Nam', 'đẹp']

    for word in test_words:
        print(f"\n--- {word} ---")

        nfc = unicodedata.normalize('NFC', word)
        nfd = unicodedata.normalize('NFD', word)

        print(f"NFC ({len(nfc)} chars): ", end='')
        for c in nfc:
            print(f"U+{ord(c):04X}", end=' ')
        print()

        print(f"NFD ({len(nfd)} chars): ", end='')
        for c in nfd:
            cat = unicodedata.category(c)
            if cat == 'Mn':
                print(f"U+{ord(c):04X}(Mn)", end=' ')
            else:
                print(f"U+{ord(c):04X}", end=' ')
        print()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python check_unicode_normalization.py <file.epub>")
        print("  python check_unicode_normalization.py <file.txt>")
        print("  python check_unicode_normalization.py --demo")
        print("\nThis script checks if Vietnamese text uses NFC or NFD Unicode form.")
        demo()
        sys.exit(0)

    arg = sys.argv[1]

    if arg == '--demo':
        demo()
    elif arg.endswith('.epub'):
        check_epub(arg)
    else:
        check_file(arg)
