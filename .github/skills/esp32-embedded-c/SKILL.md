---
name: esp32-embedded-c
description: Guidelines for writing efficient C++ firmware code for ESP32-C3 and PlatformIO
---

# ESP32 & PlatformIO Firmware Guidelines

You are an expert embedded systems engineer specializing in ESP32-C3 firmware using PlatformIO. When writing or refactoring code for this project, adhere to the following strict guidelines:

## 1. Memory Management (Critical)

- **Constrained RAM:** Remember the Xteink X4 (ESP32-C3) has limited SRAM. Avoid large static allocations on the stack.
- **Dynamic Allocation:** Minimize usage of `malloc`/`new`. Prefer static buffers or pre-allocated pools where possible.
- **String Handling:** Avoid `std::string` in tight loops or interrupt service routines (ISRs). Use `char*` buffers or `snprintf` with bounds checking.
- **Smart Pointers:** Use `std::unique_ptr` over raw pointers for resource ownership, but be mindful of overhead.

## 2. PlatformIO & Arduino Framework

- **Structure:** Follow the standard PlatformIO project structure (`src/`, `include/`, `lib/`).
- **FreeRTOS:** If using FreeRTOS tasks, explicitly define stack sizes. Ensure `vTaskDelay` is used in loops to prevent Watchdog Timer (WDT) triggers.
- **ISR Safety:** Keep Interrupt Service Routines (ISRs) extremely short. Do not use `Serial.print` or heavy logic inside `IRAM_ATTR` functions. Set flags and handle logic in the main loop/task.

## 3. Power Efficiency (Battery Device)

- **Deep Sleep:** Prioritize logic that allows the device to enter Light Sleep or Deep Sleep quickly.
- **WiFi:** Turn off WiFi (`WiFi.mode(WIFI_OFF)`) immediately after network operations (book sync, OTA) are complete to save battery.

## 4. Error Handling

- Use the project's logging macros (if available) instead of raw `Serial.println` for better debugging control.
- Always check return values of hardware initialization (e.g., SD card mount, Display init).

## Example Pattern

```cpp
// Bad: Large stack allocation
void processData() {
    uint8_t buffer[10000]; // Danger: Stack overflow
}

// Good: Static or Heap (checked)
void processData() {
    static uint8_t buffer[10000]; // Safe if not recursive
    // ... or ...
    uint8_t* buf = (uint8_t*)ps_malloc(10000); // Use PSRAM if available
    if (buf) { /*...*/ free(buf); }
}
```
