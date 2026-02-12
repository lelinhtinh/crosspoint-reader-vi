# Font Sizing for Vietnamese Display

## Overview

Vietnamese text uses combining diacritical marks (dấu thanh) that stack above and below base characters. Proper font sizing is critical for legibility on e-paper displays.

## Current Font Configuration

### Font Family: Andada Pro

Andada Pro is a serif typeface with excellent Vietnamese support:

- **Designer**: Carolina Giovagnoli, Andrés Torresi
- **License**: SIL Open Font License 1.1
- **Vietnamese Coverage**: Full support for precomposed and combining marks
- **Variants**: Regular, Bold (no Italic variants needed for Vietnamese)

### Available Sizes

The system provides four reader font sizes optimized for Vietnamese text:

| Size ID | Point Size | Use Case | Line Height Factor |
|---------|-----------|----------|-------------------|
| `SYSTEM_FONT_12_ID` | 12pt | Small - Maximum text density | 1.4x |
| `SYSTEM_FONT_14_ID` | 14pt | Medium - Default reading size | 1.45x |
| `SYSTEM_FONT_16_ID` | 16pt | Large - Comfortable reading | 1.5x |
| `SYSTEM_FONT_18_ID` | 18pt | Extra Large - Maximum legibility | 1.55x |

Additional UI fonts:

- **10pt**: UI elements (menus, buttons)
- **12pt**: UI text (dialogs, settings)
- **8pt**: Version info, metadata

## Vietnamese Typography Considerations

### Diacritical Mark Stacking

Vietnamese uses up to 3 combining marks on a single character:

- **Tone marks**: 5 tones (sắc, huyền, hỏi, ngã, nặng)
- **Vowel modifications**: â, ă, ê, ô, ơ, ư, đ
- **Combined**: e.g., "ế" = e + circumflex + acute

**Impact on sizing:**

- Vertical spacing must accommodate stacked marks
- Ascender/descender height affects diacritical mark placement
- Line height should be 1.4-1.6x font size to prevent mark overlap

### E-Paper Display Constraints

**Xteink X4 Display:**

- Resolution: 1024x758 pixels
- PPI: ~212 (estimated 4.7" display)
- Bit depth: 2-bit grayscale (4 levels)

**Rendering quality:**

- Font sizes 12-18pt render at ~25-38 pixels in height
- 2-bit antialiasing enabled for sizes ≥12pt (via `--2bit` flag)
- 1-bit rendering for UI fonts ≤10pt (smaller file size)

## Font Size Calculation Methodology

### Base Size Selection

The baseline font sizes (12, 14, 16, 18pt) were selected through:

1. **Readability Testing**: Vietnamese text samples tested at multiple sizes
2. **X-height Analysis**: Ensuring lowercase letters maintain clarity
3. **Diacritical Mark Clearance**: Verifying marks don't touch adjacent lines
4. **Display DPI Matching**: Optimizing for ~212 PPI e-paper display

### Conversion Formula

```
Pixel Height ≈ (Point Size × Display PPI) / 72
```

For Xteink X4 (~212 PPI):

- 12pt → ~35 pixels
- 14pt → ~41 pixels
- 16pt → ~47 pixels
- 18pt → ~53 pixels

### Line Height Calculation

Vietnamese requires generous line spacing:

```
Line Height = Font Size × Line Height Factor
```

Example for 14pt font:

```
Line Height = 14pt × 1.45 = 20.3pt ≈ 58 pixels
```

This provides:

- Clear separation between lines
- No overlap of descenders (g, y, p) with ascenders on next line
- Sufficient space for diacritical marks

## Testing and Validation

### Test Text Samples

Use these Vietnamese text samples for font testing:

```
Đây là văn bản tiếng Việt với đầy đủ dấu thanh.
ẴẲẴẲẴẲ - Diacritical mark stress test
Một chương trình đọc sách điện tử mã nguồn mở.
```

### Validation Checklist

- [x] All diacritical marks render completely
- [x] No overlap between lines
- [x] Sufficient contrast at all font sizes
- [x] Bold variant maintains readability
- [x] Long paragraphs remain comfortable to read
- [x] Proper rendering in both NFC and NFD Unicode normalization

### Test Results (Andada Pro)

**Run test**: `python3 lib/EpdFont/scripts/test_vietnamese_font.py`

**Character Coverage**: ✅ 100% (All 134 Vietnamese characters supported)

**Actual Rendering Metrics** (measured at 150 DPI):

| Size | Height | Ascender | Descender | Status |
|------|--------|----------|-----------|--------|
| 8pt | 20px | 16px | -4px | ✅ PASS |
| 10pt | 25px | 20px | -5px | ✅ PASS |
| 12pt | 29px | 24px | -6px | ✅ PASS |
| **14pt** | **34px** | **28px** | **-7px** | ✅ PASS |
| 16pt | 39px | 32px | -8px | ✅ PASS |
| 18pt | 44px | 36px | -9px | ✅ PASS |
| 20pt | 49px | 40px | -10px | ✅ PASS |

**Tested Characters**:

- Basic Vietnamese vowels with tone marks (À, Á, Ả, etc.)
- Combined characters (Ấ, Ầ, Ẩ, Ẫ, Ậ, etc.)
- Special Vietnamese letters (Đ, Ơ, Ư)
- Uppercase & lowercase variants
- All diacritical mark combinations

**Diacritical Mark Quality**: All marks render completely without clipping or overlap.

### Performance Metrics

Font file sizes (with 2-bit antialiasing):

- 12pt Regular: ~85KB, Bold: ~87KB
- 14pt Regular: ~197KB, Bold: ~206KB
- 16pt Regular: ~250KB, Bold: ~262KB
- 18pt Regular: ~307KB, Bold: ~324KB

Total font data: ~1.7MB (all sizes, Regular + Bold)

## Customizing Font Sizes

To add or modify font sizes:

1. **Edit build script**: `lib/EpdFont/scripts/convert-builtin-fonts.sh`
   ```bash
   READER_FONT_SIZES=(12 14 16 18 20)  # Add 20pt
   ```

2. **Rebuild fonts**:
   ```bash
   cd lib/EpdFont/scripts
   bash convert-builtin-fonts.sh
   ```

3. **Update font IDs**:
   ```bash
   bash build-font-ids.sh > ../../../src/fontIds.h
   ```

4. **Register in code**: Update `src/main.cpp` to load new font

5. **Test thoroughly**: Verify Vietnamese rendering at new size

## Font Switching Guide

To change the system font family (e.g., from Andada Pro to another):

1. **Obtain font files**: Place `.ttf` files in `lib/EpdFont/builtinFonts/source/NewFont/`

2. **Verify Vietnamese support**: Test that all Vietnamese characters render correctly

3. **Update build script**: Modify font paths in `convert-builtin-fonts.sh`

4. **Consider metrics**: Adjust sizes if new font has different x-height or ascender/descender proportions

5. **Rebuild and test**: Generate new font files and test thoroughly

## References

- [Unicode Vietnamese Encoding](http://www.unicode.org/charts/PDF/U1EA0.pdf) - Vietnamese block specification
- [Andada Pro on Google Fonts](https://fonts.google.com/specimen/Andada+Pro)
- [SIL OFL License](https://scripts.sil.org/OFL)
- [Vietnamese Typography Guidelines](https://vietnamesetypography.com/) (community resource)

## Contributing

When proposing font size changes or new fonts:

1. Provide test images showing Vietnamese text rendering
2. Include performance metrics (file sizes, build times)
3. Document any rendering issues or limitations
4. Test with both NFC and NFD normalized text
