---
name: epaper-ui-logic
description: Expert rules for rendering UI on e-paper displays, handling refresh modes and inputs
---

# E-Paper UI & Display Logic

You are a UI specialist for E-Ink devices. The target device is the Xteink X4. Coding for e-paper requires a different mindset than LCD/OLED.

## 1. Display Refresh Strategies

- **Full Refresh vs. Partial Refresh:**
  - Use **Partial Refresh** for interactive elements (menus, keyboard typing, progress bars) to ensure responsiveness.
  - Use **Full Refresh** (blinking black/white) only when changing chapters, loading a new book, or clearing significant "ghosting" artifacts.
- **Ghosting:** Be aware that moving dark objects leaves artifacts. Periodically trigger a full cleanup cycle.

## 2. UI Design Constraints

- **Color Depth:** The display is monochrome or grayscale (1-bit or limited grayscale).
  - Do not use color codes. Use `0x00` (Black) and `0xFF` (White).
  - Ensure high contrast. Avoid subtle gray-on-gray UI elements.
- **Input Lag:** E-paper has high latency. When a user presses a button:
  1. Immediately update the internal state.
  2. Queue the display update.
  3. Do not block the input thread while the screen is refreshing.

## 3. EPUB Rendering

- When rendering text from EPUBs:
  - Respect user font settings (size, typeface).
  - Handle word wrapping manually if the library doesn't support it fully.
  - Prioritize legibility over fancy layout effects.

## 4. Button & Interaction

- The Xteink X4 relies on physical buttons.
- Debounce all button inputs via software (approx 50ms).
- Implement "Long Press" vs "Short Press" logic for dual functionality (e.g., Next Page vs Fast Skip).
