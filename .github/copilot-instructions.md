# CrossPoint Reader AI Coding Agent Instructions

You are an embedded systems expert working on **CrossPoint Reader**, a firmware for the **Xteink X4** e-paper device (ESP32-C3).

## Project Overview

- **Target**: ESP32-C3 microcontroller with **~320KB usable RAM** and SD card storage
- **Build System**: PlatformIO with Arduino framework (`platformio run`, `pio run --target upload`)
- **Language**: C++17 with aggressive caching to SD card to minimize RAM
- **Fork-Specific**: Vietnamese language support (NFC/NFD Unicode normalization in `lib/Utf8/`)
- **Upstream**: [crosspoint-reader/crosspoint-reader](https://github.com/crosspoint-reader/crosspoint-reader)

## Architecture & Key Components

### Activity System (UI Framework)

```
Activity (base class) → Activity::onEnter() → Activity::loop() → Activity::onExit()
                                 ↓
        ActivityWithSubactivity (container for modal subactivities)
                                 ↓
        Specific Activities (HomeActivity, ReaderActivity, SettingsActivity, etc.)
```

- **Every screen is an Activity** in `src/activities/`. Activities are managed in main loop.
- **onEnter()**: Called once when screen becomes active (render initial state)
- **loop()**: Called repeatedly in main loop (handle input, update state)
- **preventAutoSleep()**: Return `true` to keep device awake (WiFi sync, parsing, etc.)

### Data Persistence Patterns

- **SD Card Caching**: First-time parsing of chapters → cached to `.crosspoint/epub_<hash>/sections/` → subsequent reads from cache
- **Singleton Settings**: `CrossPointSettings::instance()` — all app settings (font, layout, orientation)
- **State Management**: `CrossPointState` — tracks current activity, recent books, etc.
- **File Formats**: See [docs/file-formats.md](../docs/file-formats.md) for binary cache structure

### Button & Input Handling

```cpp
// Via HalGPIO abstraction in lib/hal/
gpio.isPressed(HalGPIO::BTN_CONFIRM)     // Current state
gpio.wasPressed(HalGPIO::BTN_CONFIRM)    // Just pressed (edge)
gpio.wasReleased(HalGPIO::BTN_CONFIRM)   // Just released (edge)
gpio.getHeldTime()                        // Duration in ms
gpio.consumeButtonUntilRelease(idx)       // Block further events (for screenshot)
```

- **Button mapping**: `MappedInputManager` translates physical buttons to logical actions via `CrossPointSettings`
- **Debouncing**: Handled in HalGPIO (~50ms)
- **Long press pattern**: Check `getHeldTime() > THRESHOLD` in main loop

### Display & Rendering

```cpp
renderer.drawText(FONT_ID, x, y, text)
renderer.fillRect(x, y, w, h, fill_with_black)
renderer.displayBuffer()                         // Partial refresh (fast)
renderer.displayBuffer(HalDisplay::FULL_REFRESH) // Full refresh (clear ghosting)
```

- **Refresh Modes**: Partial (interactive) vs. Full (chapter changes, cleanup)
- **Color**: Monochrome only — use `0x00` (black) and `0xFF` (white)
- **Ghosting**: Moving dark objects leaves artifacts — use full refresh periodically
- **Single Buffer Mode**: `DEINK_DISPLAY_SINGLE_BUFFER_MODE=1` in platformio.ini (memory constraint)

## Critical Developer Patterns

### 1. Memory-Conscious Code

```cpp
// Bad: Large stack allocation
void process() {
    uint8_t buffer[10000]; // Stack overflow risk
}

// Good: Static or heap with checking
void process() {
    static uint8_t buffer[10000]; // Reused between calls
    // or use PSRAM if available
    uint8_t* buf = (uint8_t*)ps_malloc(10000);
    if (!buf) { /* handle OOM */ }
    free(buf);
}
```

- Avoid large automatic arrays. Use `static` or heap allocation.
- Check all heap allocations for failure.
- Clean up resources in Activity destructor, not just in `onExit()`.

### 2. Activity Lifecycle (Most Common Bug)

```cpp
void MyActivity::onEnter() {
    renderer.fillRect(0, 0, 480, 800, false); // Clear screen
    renderer.drawText(...);
    renderer.displayBuffer();
    pagesUntilFullRefresh = 0; // Force next full refresh
}

void MyActivity::loop() {
    gpio.update();
    if (gpio.wasPressed(HalGPIO::BTN_BACK)) {
        // Exit activity
        exitActivity();
        enterNewActivity(new HomeActivity(...));
        return;
    }
    // ... handle other input ...
}
```

- Always clear screen and redraw in `onEnter()`.
- Use `pagesUntilFullRefresh` (in `renderer`) to trigger full refresh after next partial updates.
- Call `exitActivity()` + `enterNewActivity()` to transition screens.

### 3. Async Work & Power Management

```cpp
// Long-running tasks (parsing, network):
if (currentActivity->preventAutoSleep()) return; // Keep awake

// After network work, disable WiFi immediately:
WiFi.mode(WIFI_OFF);

// For Epub parsing, use callback to show progress popup:
const auto popupFn = [this]() { ScreenComponents::drawPopup(renderer, "Indexing..."); };
section->createSectionFile(..., popupFn);
```

- Return `true` from `preventAutoSleep()` while parsing or syncing.
- Disable WiFi after OTA/sync to save battery.
- Use simple popup callbacks for long operations (no progress bar — it's too CPU-intensive).

### 4. Vietnamese Text (Fork-Specific)

```cpp
#include "VietnameseNFC.h"
uint32_t cp = utf8NextCodepointNFC(&text); // Composes NFD → NFC for Vietnamese
```

- **src/util/VietnameseNFC.h**: Fork-specific Vietnamese combining mark composition
- For rendering Vietnamese text, `renderer.drawText()` automatically uses `utf8NextCodepointNFC()`
- Avoid `std::string` in ISRs; use `char*` with bounds checking

### 5. Popup & Dialog Patterns

```cpp
// Simple blocking popup (auto-dismisses):
ScreenComponents::drawPopup(renderer, "Screenshot saved");
popupDismissAt = millis() + 1500;
if (popupDismissAt > 0 && millis() >= popupDismissAt) {
    popupDismissAt = 0;
    if (currentActivity) currentActivity->onEnter(); // Redraw underneath
}

// For chapter progress during indexing:
const auto popupFn = [this]() {
    ScreenComponents::drawPopup(renderer, "Indexing...");
};
```

- Upstream simplified popup logic (commit f4df513) — no progress bar.
- Use lambda for callback to show popup only for large files (50KB+).

## Common Files & Their Roles

| Path | Purpose |
|------|---------|
| `src/main.cpp` | Setup, main loop, activity transitions, power management |
| `src/activities/Activity.h` | Base class for all screens |
| `src/CrossPointSettings.h` | Singleton for all user settings |
| `src/activities/reader/EpubReaderActivity.cpp` | Main book reader screen |
| `lib/Epub/Epub/Section.cpp` | Chapter parsing & caching logic |
| `lib/hal/HalGPIO.h/cpp` | Button input abstraction |
| `lib/GfxRenderer/GfxRenderer.h` | Display drawing API |
| `lib/Utf8/` | Core UTF-8 utilities |
| `src/util/VietnameseNFC.h/cpp` | Fork-specific Vietnamese NFC composition |

## Build & Verification

```bash
platformio run               # Build firmware
pio run --target upload      # Flash device
platformio check             # Static analysis (cppcheck)
./bin/clang-format-fix       # Auto-format code
python3 scripts/debugging_monitor.py  # Serial logs with timestamps
```

- **Check for linting errors** before committing: `platformio check` must pass.
- **Format code**: Run clang-format-fix to maintain consistency.
- **Memory usage**: Monitor `Free: X bytes` in logs — < 50KB is critical.

## Syncing with Upstream

This is a fork. Key commits to track:
- **b1dcb77**: UTF-8 truncation functions moved to `lib/Utf8/`
- **f4df513**: Popup logic simplified (removed progress bar)
- **11b2a59**: Button hints hidden in landscape CW mode
- **0d82b03**: USB wake-up fix using `getWakeupReason()`

When syncing from upstream, check [git log](https://github.com/crosspoint-reader/crosspoint-reader/commits/master) for new commits affecting `src/` or `lib/`.

## Debugging Tips

- **Serial output**: Prefix format is `[millis] [TAG]` — search logs by TAG (e.g., `[ERS]` for reader, `[ACT]` for activities).
- **Memory spikes**: Use `ESP.getFreeHeap()` printed periodically; crashes often follow from heap exhaustion.
- **Display ghosting**: If text is hard to read, trigger `pagesUntilFullRefresh = 0` to force full refresh.
- **Button debouncing**: Ensure `gpio.update()` is called before checking button state in `loop()`.
