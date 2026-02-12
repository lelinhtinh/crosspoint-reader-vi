# Vietnamese Support Library

Vietnamese NFC (Canonical Composition) normalization library for CrossPoint Reader Vietnamese fork.

## Overview

This library provides Unicode normalization specifically for Vietnamese text rendering on e-paper displays. It handles conversion from NFD (Decomposed) to NFC (Precomposed) form, ensuring proper display of Vietnamese diacritical marks.

## Features

- **NFC Normalization**: Converts Vietnamese text from NFD (base + combining marks) to NFC (precomposed characters)
- **8 Vietnamese Combining Marks Support**:
  - `U+0300` - COMBINING GRAVE ACCENT (huyền: à)
  - `U+0301` - COMBINING ACUTE ACCENT (sắc: á)
  - `U+0302` - COMBINING CIRCUMFLEX ACCENT (mũ: â)
  - `U+0303` - COMBINING TILDE (ngã: ã)
  - `U+0306` - COMBINING BREVE (trăng: ă)
  - `U+0309` - COMBINING HOOK ABOVE (hỏi: ả)
  - `U+031B` - COMBINING HORN (móc: ơ, ư)
  - `U+0323` - COMBINING DOT BELOW (nặng: ạ)

- **134 Vietnamese Characters**: Full coverage of Vietnamese alphabet including all tone combinations
- **Zero-overhead fallback**: When disabled, falls back to standard UTF-8 processing

## Usage

### Include Header

```cpp
#ifdef ENABLE_VIETNAMESE_SUPPORT
  #include "VietnameseNFC.h"
#else
  #define utf8NextCodepointNFC utf8NextCodepoint
#endif
```

### Processing Text

```cpp
const char* text = "Tiếng Việt"; // Can be NFD or NFC
uint32_t codepoint = utf8NextCodepointNFC(&text);
// Returns properly composed Vietnamese codepoints
```

### Integration Points

This library is automatically used by:
- **GfxRenderer** (`lib/GfxRenderer/GfxRenderer.cpp`) - Text rendering
- **EpdFont** (`lib/EpdFont/EpdFont.cpp`) - Font dimension calculations

## Build Configuration

This library is controlled by the `ENABLE_VIETNAMESE_SUPPORT` preprocessor flag:

```ini
# In platformio.ini
build_flags = -DENABLE_VIETNAMESE_SUPPORT=1  # Enabled (default)
build_flags = -DENABLE_VIETNAMESE_SUPPORT=0  # Disabled (fallback to UTF-8)
```

### When Disabled

- Binary size: -5KB
- Falls back to standard `utf8NextCodepoint()` from `lib/Utf8`
- Vietnamese text still renders but may show decomposed characters (NFD form)

## Font Support

Vietnamese text rendering requires fonts with proper glyph coverage. This fork uses:

- **Andada Pro** - Serif font with excellent Vietnamese support
- **Sizes**: 8pt, 10pt, 12pt, 14pt (default), 16pt, 18pt, 20pt
- **Line heights**: 1.4-1.55x for Vietnamese diacritical mark clearance
- **Antialiasing**: 2-bit for sizes ≥12pt

See `docs/vietnamese-typography.md` for detailed typography guidelines.

## Implementation Details

### Composition Algorithm

1. Reads base codepoint from UTF-8 stream
2. Peeks at next codepoint to detect combining marks
3. Uses lookup table to compose base + mark → precomposed character
4. Handles multiple combining marks on single base (e.g., ố = o + circumflex + acute)
5. Returns composed NFC codepoint

### Example Composition

```
Input (NFD):  u (U+0075) + COMBINING TILDE (U+0303)
Output (NFC): ũ (U+0169)

Input (NFD):  o (U+006F) + COMBINING CIRCUMFLEX (U+0302) + COMBINING ACUTE (U+0301)
Output (NFC): ố (U+1ED1)
```

## Testing

Vietnamese character coverage can be tested using:

```bash
# Build with Vietnamese support
pio run -e default

# Test with sample Vietnamese text
# Load an EPUB with Vietnamese content (NFD or NFC)
# Verify proper rendering of all diacritical marks
```

Test samples:
- Vowels with tones: à, ả, ã, á, ạ, ă, ằ, ẳ, ẵ, ắ, ặ, â, ầ, ẩ, ẫ, ấ, ậ
- Horn vowels: ơ, ờ, ở, ỡ, ớ, ợ, ư, ừ, ử, ữ, ứ, ự
- Special: đ (d with stroke)

## Dependencies

- **Utf8** (`lib/Utf8`) - Core UTF-8 utilities

## License

Part of CrossPoint Reader Vietnamese Fork. See main project LICENSE.

## Repository

https://github.com/baivong/crosspoint-reader-vi

## Contributors

- Vietnamese NFC implementation: CrossPoint Reader Vietnamese Fork team
- Based on Unicode NFC specification
- Font selection and typography: Andada Pro integration
