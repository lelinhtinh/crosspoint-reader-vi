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

Vietnamese text rendering requires font `.h` files that include both:
- **Latin Extended-B** (`U+0180–U+024F`): base characters ơ (U+01A1), ư (U+01B0), đ (U+0111)
- **Latin Extended Additional** (`U+1E00–U+1EFF`): 134 precomposed Vietnamese NFC characters

> `fontconvert.py` already includes Latin Extended-A (`U+0100–U+017F`) by default.
> Cyrillic, Combining Diacritical Marks, Math, and Arrows are commented out in this
> fork (not used in Vietnamese ebooks) to reduce flash usage.

### Supported Fonts (Vietnamese-ready)

| Font family | Role | Styles | Sizes | Coverage |
|---|---|---|---|---|
| **NotoSans** | UI only (status bar, menus) | Regular | 8pt | ✅ Full |
| **Ubuntu** (NotoSans VI) | UI (menus, headers, file names) | Regular, Bold | 10, 12pt | ✅ Full |
| **Bookerly** | Reader font | Regular, Bold, Italic, BoldItalic | 12–18pt | ✅ Full |
| **OpenDyslexic** | Reader font | Regular, Bold, Italic, BoldItalic | 8–14pt | ✅ Full |

> NotoSans 12–18pt reader fonts are **excluded** when `ENABLE_VIETNAMESE_SUPPORT=1`:
> they are removed by `generate_fonts_vi.sh` and guarded by `#if !ENABLE_VIETNAMESE_SUPPORT`
> in `all.h` and `main.cpp`, reclaiming ~7 MB of flash for Vietnamese font data.

### Regenerating Fonts

```bash
cd lib/EpdFont
bash scripts/generate_fonts_vi.sh
```

Run this script after:
- Pulling new font versions from upstream
- Adding new font families
- Updating `fontconvert.py` from upstream

The script regenerates all font families with Vietnamese glyph coverage, then **deletes**
`notosans_12–18_*.h` (unused when VI is enabled). Script source: [`lib/EpdFont/scripts/generate_fonts_vi.sh`](../EpdFont/scripts/generate_fonts_vi.sh)

## ⚠️ Firmware Update Warning — Partition Table Change

This fork modifies `partitions.csv` compared to upstream CrossPoint Reader:

| Partition | Upstream | This fork |
|---|---|---|
| `app0` | 6.25 MB (0x640000) | **7.75 MB (0x7C0000)** |
| `app1` | 6.25 MB (0x640000) | **7.75 MB (0x7C0000)** |
| `spiffs` | 3.375 MB | 0.375 MB |

### Who is affected

Anyone whose device currently has **either** of these:
- Upstream CrossPoint Reader firmware (6.25 MB partitions)
- An older version of this fork before the partition change

### Why OTA update will crash

The ESP32 stores the active partition layout in the `otadata` partition in flash.
If new firmware is flashed with a **different** `partitions.csv` without erasing first,
the chip will boot using the old partition offsets — pointing into garbage — and crash
repeatedly (`rst:0x3 RTC_SW_SYS_RST` in serial log).

### Fix: full erase + reflash via USB

Connect the device via USB and run:

```bash
# 1. Erase entire flash (wipes otadata + old partition layout)
python3 ~/.platformio/packages/tool-esptoolpy/esptool.py \
  --chip esp32c3 --port /dev/ttyACM0 erase_flash

# 2. Reflash with the new partition table
pio run --target upload --upload-port /dev/ttyACM0
```

Or using `pio` directly:

```bash
pio run --target erase --upload-port /dev/ttyACM0
pio run --target upload --upload-port /dev/ttyACM0
```

> **Note:** Erasing flash resets all settings (WiFi credentials, reading progress, etc.).
> Back up your settings from the web UI before erasing if needed.

### OTA update is not supported across partition table changes

OTA (over-the-air) update **cannot** be used when `partitions.csv` changes. You must
flash via USB cable. There is no workaround.

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
- Font regeneration with Vietnamese coverage: see `generate_fonts_vi.sh`

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

Vietnamese text rendering requires font `.h` files that include the **Latin Extended Additional** Unicode block (`U+1E00–U+1EFF`), which contains all 256 precomposed Vietnamese characters (NFC form).

### Supported Fonts (Vietnamese-ready)

| Font family | Styles | Sizes | Vietnamese coverage |
|---|---|---|---|
| **NotoSans** | Regular, Bold, Italic, BoldItalic | 8, 12, 14, 16, 18pt | ✅ Full (0x1E00–0x1EFF) |
| **Bookerly** | Regular, Bold, Italic, BoldItalic | 12, 14, 16, 18pt | ✅ Full (0x1E00–0x1EFF) |
| **OpenDyslexic** | Regular, Bold, Italic, BoldItalic | 8, 10, 12, 14pt | ✅ Full (0x1E00–0x1EFF) |
| **Ubuntu** | Regular, Bold | 10, 12pt | ⚠️ Partial (no 0x1EXX glyphs) |

> **Note:** Ubuntu TTF only includes basic Vietnamese (`đ`/`Đ`) but lacks the full precomposed character set needed for proper rendering. Avoid using Ubuntu for Vietnamese content.

### Regenerating Fonts

All font `.h` files in `lib/EpdFont/builtinFonts/` that support Vietnamese have been regenerated with the `--additional-intervals 0x1E00,0x1EFF` flag:

```bash
cd lib/EpdFont
bash scripts/generate_fonts_vi.sh
```

Run this script after:
- Pulling new font versions from upstream
- Adding new font families
- Updating `fontconvert.py` from upstream

The script regenerates all font families with Vietnamese glyph coverage (skips Ubuntu). Script source: [`lib/EpdFont/scripts/generate_fonts_vi.sh`](../EpdFont/scripts/generate_fonts_vi.sh)

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
- Font regeneration with Vietnamese coverage: see `generate_fonts_vi.sh`
