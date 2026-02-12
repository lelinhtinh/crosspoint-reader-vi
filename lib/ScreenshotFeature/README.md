# Screenshot Feature

PBM (Portable Bitmap) screenshot capture feature for CrossPoint Reader Vietnamese fork.

## Overview

This feature allows users to capture screenshots of the e-paper display by holding the CONFIRM button for 2 seconds. Screenshots are saved in PBM format to the `/screenshots/` directory on the SD card.

## Features

- **Button Trigger**: Hold CONFIRM button for 2+ seconds to capture
- **PBM Format**: Portable Bitmap (P4/binary) format - widely compatible
- **Auto-rotation**: Rotates display 90° counterclockwise (landscape → portrait)
- **Popup Feedback**: Shows "Screenshot saved" or "Failed to save screenshot" message
- **User-toggleable**: Can be enabled/disabled in System Settings
- **Memory-safe**: Checks heap before allocating ~48KB rotation buffer

## Usage

### Enable/Disable

1. Navigate to Settings → System Settings
2. Toggle "Enable Screenshot"
3. Setting persists across reboots

### Taking Screenshots

1. Ensure screenshot feature is enabled in settings
2. Navigate to the screen you want to capture
3. Hold the CONFIRM button for 2+ seconds
4. Release when popup appears
5. Screenshot saved to `/screenshots/screenshot_<timestamp>.pbm`

### Viewing Screenshots

Screenshots are saved in PBM format. You can view them on your computer using:

- **Linux**: `feh`, `gimp`, ImageMagick
- **macOS**: Preview (may need conversion)
- **Windows**: GIMP, IrfanView, Paint.NET

**Convert to PNG:**
```bash
convert screenshot_12345.pbm screenshot.png
```

## Build Configuration

This feature is controlled by the `ENABLE_SCREENSHOT_FEATURE` preprocessor flag:

```ini
# In platformio.ini
build_flags = -DENABLE_SCREENSHOT_FEATURE=1  # Enabled (default)
build_flags = -DENABLE_SCREENSHOT_FEATURE=0  # Disabled
```

### When Disabled

- Binary size: -7KB
- Heap savings: -48KB (rotation buffer never allocated)
- Settings toggle hidden from UI
- Button trigger removed from main loop
- Settings file remains backward compatible (writes dummy byte)

## Technical Details

### Screenshot Format

- **Format**: PBM (Portable Bitmap) - P4 binary format
- **Dimensions**: 480x800 pixels (portrait, after rotation)
- **Color depth**: 1-bit (black & white)
- **File size**: ~48KB per screenshot
- **Header**: `P4\n<height> <width>\n`

### Display Rotation

The e-paper display runs in landscape mode (800x480), but screenshots are rotated to portrait (480x800) for easier viewing:

```
Original (landscape):     Saved (portrait):
┌──────────────┐          ┌─────┐
│   800x480    │    →     │     │
└──────────────┘          │ 480 │
                          │  x  │
                          │ 800 │
                          └─────┘
```

### Memory Usage

**Rotation buffer**: ~48KB allocated temporarily during capture

```cpp
const size_t outSize = (DISPLAY_HEIGHT / 8) * DISPLAY_WIDTH;
// = (480 / 8) * 800 = 60 * 800 = 48,000 bytes
```

**Heap check before allocation:**
```cpp
if (ESP.getFreeHeap() < static_cast<int>(outSize + 8192)) {
  // Insufficient memory - fail gracefully
}
```

### Pixel Format Conversion

E-ink format (white=1, black=0) is inverted to PBM format (black=1, white=0):

```cpp
const bool isWhite = (buffer[inByteIndex] >> inBitPosition) & 1;
if (!isWhite) {
  rotated[outByteIndex] |= (1 << outBitPosition);
}
```

## Implementation Files

**Core implementation** (inline with `#ifdef` guards):
- `lib/GfxRenderer/GfxRenderer.h` - Method declaration
- `lib/GfxRenderer/GfxRenderer.cpp` - Screenshot capture logic
- `src/main.cpp` - Button trigger handling
- `src/CrossPointSettings.h` - Settings field
- `src/CrossPointSettings.cpp` - Settings persistence
- `src/activities/settings/SettingsActivity.cpp` - UI toggle

**Code markers:**
```cpp
// FORK-FEATURE-BEGIN: SCREENSHOT
// ... screenshot-specific code ...
// FORK-FEATURE-END: SCREENSHOT
```

## Button Behavior

### Trigger Logic

```cpp
// In src/main.cpp
if (SETTINGS.screenshotEnabled && millis() - lastScreenshotAt > 1000) {
  static bool screenshotTaken = false;
  if (gpio.isPressed(HalGPIO::BTN_CONFIRM) && gpio.getHeldTime() > 2000) {
    if (!screenshotTaken) {
      screenshotTaken = true;
      gpio.consumeButtonUntilRelease(HalGPIO::BTN_CONFIRM);
      renderer.saveScreenshot("/screenshots");
      ScreenComponents::drawPopup(renderer, "Screenshot saved");
    }
  } else if (!gpio.isPressed(HalGPIO::BTN_CONFIRM)) {
    screenshotTaken = false;
  }
}
```

### Debouncing

- **Minimum interval**: 1 second between screenshots
- **Hold time**: 2 seconds required
- **Button consumption**: Prevents interference with other button handlers

## Error Handling

Screenshot capture can fail due to:

1. **Insufficient heap memory** (< 56KB free)
2. **SD card write failure**
3. **Directory creation failure**

All failures show popup: "Failed to save screenshot"

## Settings Persistence

The `screenshotEnabled` setting is stored in the settings file:

```cpp
// When enabled
out.write(&screenshotEnabled, sizeof(screenshotEnabled));

// When disabled (maintains file format compatibility)
uint8_t dummy = 0;
out.write(&dummy, sizeof(dummy));
```

This ensures settings files remain compatible when switching between builds with the feature enabled/disabled.

## Testing

### Build testing
```bash
# With feature enabled (default)
pio run -e default

# With feature disabled
pio run -e minimal
```

### Runtime testing
1. Enable screenshot in settings
2. Navigate to any screen (e.g., home, reader)
3. Hold CONFIRM button for 2+ seconds
4. Verify popup appears
5. Check SD card `/screenshots/` directory
6. Verify PBM file exists and is ~48KB

### Memory monitoring
```cpp
Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
```

Expected: ~200KB free after boot with all features enabled

## Troubleshooting

**Screenshot not saving?**
- Check if feature is enabled in settings
- Verify SD card is inserted and writable
- Check free heap (Serial monitor): `ESP.getFreeHeap()`
- Ensure `/screenshots/` directory is not protected

**Out of memory error?**
- Close other apps/activities first
- Disable other memory-intensive features temporarily
- Try again after device reboot

## License

Part of CrossPoint Reader Vietnamese Fork. See main project LICENSE.

## Repository

https://github.com/baivong/crosspoint-reader-vi
