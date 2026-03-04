# Vietnamese Support Library

Vietnamese NFC (Canonical Composition) normalization library for CrossPoint Reader Vietnamese fork.

## Overview

Provides Unicode NFC normalization for Vietnamese text rendering on e-paper displays.
Converts NFD (base character + combining diacritical marks) to NFC (precomposed characters),
ensuring correct display of all Vietnamese tonal marks.

## Features

- **NFC Normalization**: NFD → NFC conversion for Vietnamese
- **8 combining marks supported**:
  - `U+0300` COMBINING GRAVE ACCENT (huyền: à)
  - `U+0301` COMBINING ACUTE ACCENT (sắc: á)
  - `U+0302` COMBINING CIRCUMFLEX ACCENT (mũ: â)
  - `U+0303` COMBINING TILDE (ngã: ã)
  - `U+0306` COMBINING BREVE (trăng: ă)
  - `U+0309` COMBINING HOOK ABOVE (hỏi: ả)
  - `U+031B` COMBINING HORN (móc: ơ, ư)
  - `U+0323` COMBINING DOT BELOW (nặng: ạ)
- **134 precomposed Vietnamese characters** (full alphabet + all tone combinations)
- **Zero-overhead fallback**: when disabled, maps directly to `utf8NextCodepoint()`

## Build Configuration

Controlled by `-DENABLE_VIETNAMESE_SUPPORT=1` in `platformio.ini` (enabled by default in this fork).

When disabled (`=0`):
- Falls back to standard `utf8NextCodepoint()` — no composition performed
- Binary size reduced by ~5 KB
- Vietnamese NFD text may render with decomposed marks instead of precomposed glyphs

## Integration

Automatically used by:
- `lib/GfxRenderer/GfxRenderer.cpp` — text rendering (`drawText`, `getTextAdvanceX`, `drawTextRotated90CW`)
- `lib/EpdFont/EpdFont.cpp` — font dimension calculation (`getTextBounds`)

Both files use the `utf8NextCodepointNFC` macro, which resolves to the real function when
`ENABLE_VIETNAMESE_SUPPORT=1`, or to `utf8NextCodepoint` otherwise.

## Font Support

Vietnamese rendering requires font `.h` files that cover:
- **Latin Extended-B** (`U+0180–U+024F`): base characters ơ (U+01A1), ư (U+01B0), đ (U+0111)
- **Latin Extended Additional** (`U+1E00–U+1EFF`): 134 precomposed Vietnamese NFC characters

| Font family | Role | Styles | Sizes | Vietnamese |
|---|---|---|---|---|
| **Bookerly** | Reader | Regular, Bold, Italic, BoldItalic | 12, 14, 16, 18pt | ✅ Full |
| **Ubuntu** | UI (menus, headers, filenames) | Regular, Bold | 10, 12pt | ✅ Full |
| **NotoSans** | UI (status bar, small text) | Regular | 8pt | ✅ Full |

> **NotoSans 12–18pt reader fonts are excluded** when `ENABLE_VIETNAMESE_SUPPORT=1`
> (guarded by `#if !ENABLE_VIETNAMESE_SUPPORT` in `main.cpp`), reclaiming ~7 MB flash
> for Vietnamese Bookerly font data.

### Regenerating fonts

```bash
cd lib/EpdFont
bash scripts/generate_fonts_vi.sh
```

Run after pulling new font versions from upstream, adding font families, or updating `fontconvert.py`.

## Implementation

### Composition algorithm

1. Read base codepoint from UTF-8 stream
2. Peek at next codepoint(s) to detect combining marks
3. Look up `base + mark → precomposed` in the NFC table
4. Handle multiple combining marks on a single base (e.g., ố = o + circumflex + acute)
5. Return composed NFC codepoint

### Examples

```
NFD:  u (U+0075) + U+0303        →  NFC: ũ (U+0169)
NFD:  o (U+006F) + U+0302 + U+0301  →  NFC: ố (U+1ED1)
```

## Testing

Load an EPUB with Vietnamese content containing NFD-encoded text and verify correct rendering:

```
Vowels with tones:  à ả ã á ạ  ă ằ ẳ ẵ ắ ặ  â ầ ẩ ẫ ấ ậ
Horn vowels:        ơ ờ ở ỡ ớ ợ  ư ừ ử ữ ứ ự
Special:            đ Đ
```

## Dependencies

- `lib/Utf8` — core UTF-8 iteration (`utf8NextCodepoint`, `utf8IsCombiningMark`)

## License

Part of CrossPoint Reader Vietnamese Fork. See main project [LICENSE](../../LICENSE).
