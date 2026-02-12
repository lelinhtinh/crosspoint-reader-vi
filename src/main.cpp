#include <Arduino.h>
#include <Epub.h>
#include <GfxRenderer.h>
#include <HalDisplay.h>
#include <HalGPIO.h>
#include <SDCardManager.h>
#include <SPI.h>
#include <builtinFonts/all.h>

#include <cstring>

#include "Battery.h"
#include "CrossPointSettings.h"
#include "CrossPointState.h"
#include "KOReaderCredentialStore.h"
#include "MappedInputManager.h"
#include "RecentBooksStore.h"
#include "ScreenComponents.h"
#include "activities/boot_sleep/BootActivity.h"
#include "activities/boot_sleep/SleepActivity.h"
#include "activities/browser/OpdsBookBrowserActivity.h"
#include "activities/home/HomeActivity.h"
#include "activities/home/MyLibraryActivity.h"
#include "activities/network/CrossPointWebServerActivity.h"
#include "activities/reader/ReaderActivity.h"
#include "activities/settings/SettingsActivity.h"
#include "activities/util/FullScreenMessageActivity.h"
#include "fontIds.h"

HalDisplay display;
HalGPIO gpio;
MappedInputManager mappedInputManager(gpio);
GfxRenderer renderer(display);
Activity *currentActivity;

// Fonts - Andada Pro for Vietnamese support (Regular and Bold only)
// Italic will fallback to Regular, BoldItalic will fallback to Bold
EpdFont systemFont14RegularFont(&system_font_14_regular);
EpdFont systemFont14BoldFont(&system_font_14_bold);
EpdFontFamily systemFont14FontFamily(&systemFont14RegularFont, &systemFont14BoldFont);
#ifndef OMIT_FONTS
EpdFont systemFont12RegularFont(&system_font_12_regular);
EpdFont systemFont12BoldFont(&system_font_12_bold);
EpdFontFamily systemFont12FontFamily(&systemFont12RegularFont, &systemFont12BoldFont);
EpdFont systemFont16RegularFont(&system_font_16_regular);
EpdFont systemFont16BoldFont(&system_font_16_bold);
EpdFontFamily systemFont16FontFamily(&systemFont16RegularFont, &systemFont16BoldFont);
EpdFont systemFont18RegularFont(&system_font_18_regular);
EpdFont systemFont18BoldFont(&system_font_18_bold);
EpdFontFamily systemFont18FontFamily(&systemFont18RegularFont, &systemFont18BoldFont);
#endif // OMIT_FONTS

EpdFont smallFont(&system_font_8_regular);
EpdFontFamily smallFontFamily(&smallFont);

EpdFont ui10RegularFont(&system_font_10_regular);
EpdFont ui10BoldFont(&system_font_10_bold);
EpdFontFamily ui10FontFamily(&ui10RegularFont, &ui10BoldFont);

EpdFont ui12RegularFont(&system_font_12_regular);
EpdFont ui12BoldFont(&system_font_12_bold);
EpdFontFamily ui12FontFamily(&ui12RegularFont, &ui12BoldFont);

// measurement of power button press duration calibration value
unsigned long t1 = 0;
unsigned long t2 = 0;

void exitActivity() {
  if (currentActivity) {
    currentActivity->onExit();
    delete currentActivity;
    currentActivity = nullptr;
  }
}

void enterNewActivity(Activity *activity) {
  currentActivity = activity;
  currentActivity->onEnter();
}

// Verify power button press duration on wake-up from deep sleep
// Pre-condition: isWakeupByPowerButton() == true
void verifyPowerButtonDuration() {
  if (SETTINGS.shortPwrBtn == CrossPointSettings::SHORT_PWRBTN::SLEEP) {
    // Fast path for short press
    // Needed because inputManager.isPressed() may take up to ~500ms to return the correct state
    return;
  }

  // Give the user up to 1000ms to start holding the power button, and must hold for SETTINGS.getPowerButtonDuration()
  const auto start = millis();
  bool abort = false;
  // Subtract the current time, because inputManager only starts counting the HeldTime from the first update()
  // This way, we remove the time we already took to reach here from the duration,
  // assuming the button was held until now from millis()==0 (i.e. device start time).
  const uint16_t calibration = start;
  const uint16_t calibratedPressDuration =
      (calibration < SETTINGS.getPowerButtonDuration()) ? SETTINGS.getPowerButtonDuration() - calibration : 1;

  gpio.update();
  // Needed because inputManager.isPressed() may take up to ~500ms to return the correct state
  while (!gpio.isPressed(HalGPIO::BTN_POWER) && millis() - start < 1000) {
    delay(10); // only wait 10ms each iteration to not delay too much in case of short configured duration.
    gpio.update();
  }

  t2 = millis();
  if (gpio.isPressed(HalGPIO::BTN_POWER)) {
    do {
      delay(10);
      gpio.update();
    } while (gpio.isPressed(HalGPIO::BTN_POWER) && gpio.getHeldTime() < calibratedPressDuration);
    abort = gpio.getHeldTime() < calibratedPressDuration;
  } else {
    abort = true;
  }

  if (abort) {
    // Button released too early. Returning to sleep.
    // IMPORTANT: Re-arm the wakeup trigger before sleeping again
    gpio.startDeepSleep();
  }
}

void waitForPowerRelease() {
  gpio.update();
  while (gpio.isPressed(HalGPIO::BTN_POWER)) {
    delay(50);
    gpio.update();
  }
}

// Enter deep sleep mode
void enterDeepSleep() {
  exitActivity();
  enterNewActivity(new SleepActivity(renderer, mappedInputManager));

  display.deepSleep();
  Serial.printf("[%lu] [   ] Power button press calibration value: %lu ms\n", millis(), t2 - t1);
  Serial.printf("[%lu] [   ] Entering deep sleep.\n", millis());

  gpio.startDeepSleep();
}

void onGoHome();
void onGoToMyLibraryWithTab(const std::string &path, MyLibraryActivity::Tab tab);
void onGoToReader(const std::string &initialEpubPath, MyLibraryActivity::Tab fromTab) {
  exitActivity();
  enterNewActivity(
      new ReaderActivity(renderer, mappedInputManager, initialEpubPath, fromTab, onGoHome, onGoToMyLibraryWithTab));
}
void onContinueReading() { onGoToReader(APP_STATE.openEpubPath, MyLibraryActivity::Tab::Recent); }

void onGoToFileTransfer() {
  exitActivity();
  enterNewActivity(new CrossPointWebServerActivity(renderer, mappedInputManager, onGoHome));
}

void onGoToSettings() {
  exitActivity();
  enterNewActivity(new SettingsActivity(renderer, mappedInputManager, onGoHome));
}

void onGoToMyLibrary() {
  exitActivity();
  enterNewActivity(new MyLibraryActivity(renderer, mappedInputManager, onGoHome, onGoToReader));
}

void onGoToMyLibraryWithTab(const std::string &path, MyLibraryActivity::Tab tab) {
  exitActivity();
  enterNewActivity(new MyLibraryActivity(renderer, mappedInputManager, onGoHome, onGoToReader, tab, path));
}

void onGoToBrowser() {
  exitActivity();
  enterNewActivity(new OpdsBookBrowserActivity(renderer, mappedInputManager, onGoHome));
}

void onGoHome() {
  exitActivity();
  enterNewActivity(new HomeActivity(renderer, mappedInputManager, onContinueReading, onGoToMyLibrary, onGoToSettings,
                                    onGoToFileTransfer, onGoToBrowser));
}

void setupDisplayAndFonts() {
  display.begin();
  Serial.printf("[%lu] [   ] Display initialized\n", millis());
  renderer.insertFont(SYSTEM_FONT_14_ID, systemFont14FontFamily);
#ifndef OMIT_FONTS
  renderer.insertFont(SYSTEM_FONT_12_ID, systemFont12FontFamily);
  renderer.insertFont(SYSTEM_FONT_16_ID, systemFont16FontFamily);
  renderer.insertFont(SYSTEM_FONT_18_ID, systemFont18FontFamily);
#endif // OMIT_FONTS
  renderer.insertFont(UI_10_FONT_ID, ui10FontFamily);
  renderer.insertFont(UI_12_FONT_ID, ui12FontFamily);
  renderer.insertFont(SMALL_FONT_ID, smallFontFamily);
  Serial.printf("[%lu] [   ] Fonts setup\n", millis());
}

void setup() {
  t1 = millis();

  gpio.begin();

  // Only start serial if USB connected
  if (gpio.isUsbConnected()) {
    Serial.begin(115200);
    // Wait up to 3 seconds for Serial to be ready to catch early logs
    unsigned long start = millis();
    while (!Serial && (millis() - start) < 3000) {
      delay(10);
    }
  }

  // SD Card Initialization
  // We need 6 open files concurrently when parsing a new chapter
  if (!SdMan.begin()) {
    Serial.printf("[%lu] [   ] SD card initialization failed\n", millis());
    setupDisplayAndFonts();
    exitActivity();
    enterNewActivity(new FullScreenMessageActivity(renderer, mappedInputManager, "SD card error", EpdFontFamily::BOLD));
    return;
  }

  SETTINGS.loadFromFile();
  KOREADER_STORE.loadFromFile();

  switch (gpio.getWakeupReason()) {
  case HalGPIO::WakeupReason::PowerButton:
    // For normal wakeups, verify power button press duration
    Serial.printf("[%lu] [   ] Verifying power button press duration\n", millis());
    verifyPowerButtonDuration();
    break;
  case HalGPIO::WakeupReason::AfterUSBPower:
    // If USB power caused a cold boot, go back to sleep
    Serial.printf("[%lu] [   ] Wakeup reason: After USB Power\n", millis());
    gpio.startDeepSleep();
    break;
  case HalGPIO::WakeupReason::AfterFlash:
    // After flashing, just proceed to boot
  case HalGPIO::WakeupReason::Other:
  default:
    break;
  }

  // First serial output only here to avoid timing inconsistencies for power button press duration verification
  Serial.printf("[%lu] [   ] Starting CrossPoint version " CROSSPOINT_VERSION "\n", millis());

  setupDisplayAndFonts();

  exitActivity();
  enterNewActivity(new BootActivity(renderer, mappedInputManager));

  APP_STATE.loadFromFile();
  RECENT_BOOKS.loadFromFile();

  if (APP_STATE.openEpubPath.empty()) {
    onGoHome();
  } else {
    // Clear app state to avoid getting into a boot loop if the epub doesn't load
    const auto path = APP_STATE.openEpubPath;
    APP_STATE.openEpubPath = "";
    APP_STATE.saveToFile();
    onGoToReader(path, MyLibraryActivity::Tab::Recent);
  }

  // Ensure we're not still holding the power button before leaving setup
  waitForPowerRelease();
}

void loop() {
  static unsigned long maxLoopDuration = 0;
  const unsigned long loopStartTime = millis();
  static unsigned long lastMemPrint = 0;

  gpio.update();

  if (Serial && millis() - lastMemPrint >= 10000) {
    Serial.printf("[%lu] [MEM] Free: %d bytes, Total: %d bytes, Min Free: %d bytes\n", millis(), ESP.getFreeHeap(),
                  ESP.getHeapSize(), ESP.getMinFreeHeap());
    lastMemPrint = millis();
  }

  // Screenshot shortcut: Hold BTN_CONFIRM for 2 seconds
  static unsigned long lastScreenshotAt = 0;
  static unsigned long popupDismissAt = 0;

  if (popupDismissAt > 0 && millis() >= popupDismissAt) {
    popupDismissAt = 0;
    if (currentActivity) {
      currentActivity->onEnter();
    }
  }

  // FORK-FEATURE-BEGIN: SCREENSHOT
  // Screenshot capture via long-press BTN_CONFIRM (2 seconds)
  // Saves PBM format to /screenshots/ directory
#ifdef ENABLE_SCREENSHOT_FEATURE
  if (SETTINGS.screenshotEnabled && millis() - lastScreenshotAt > 1000) {
    static bool screenshotTaken = false;
    if (gpio.isPressed(HalGPIO::BTN_CONFIRM) && gpio.getHeldTime() > 2000) {
      if (!screenshotTaken) {
        screenshotTaken = true;
        lastScreenshotAt = millis();
        Serial.printf("[%lu] [SCR] Hold BTN_CONFIRM detected\n", millis());
        gpio.consumeButtonUntilRelease(HalGPIO::BTN_CONFIRM);
        const bool success = renderer.saveScreenshot("/screenshots");
        const char *msg = success ? "Screenshot saved" : "Failed to save screenshot";
        ScreenComponents::drawPopup(renderer, msg);
        popupDismissAt = millis() + 1500;
      }
    } else if (!gpio.isPressed(HalGPIO::BTN_CONFIRM)) {
      screenshotTaken = false;
    }
  }
#endif
  // FORK-FEATURE-END: SCREENSHOT

  // Check for any user activity (button press or release) or active background work
  static unsigned long lastActivityTime = millis();
  if (gpio.wasAnyPressed() || gpio.wasAnyReleased() || (currentActivity && currentActivity->preventAutoSleep())) {
    lastActivityTime = millis(); // Reset inactivity timer
  }

  const unsigned long sleepTimeoutMs = SETTINGS.getSleepTimeoutMs();
  if (millis() - lastActivityTime >= sleepTimeoutMs) {
    Serial.printf("[%lu] [SLP] Auto-sleep triggered after %lu ms of inactivity\n", millis(), sleepTimeoutMs);
    enterDeepSleep();
    // This should never be hit as `enterDeepSleep` calls esp_deep_sleep_start
    return;
  }

  if (gpio.isPressed(HalGPIO::BTN_POWER) && gpio.getHeldTime() > SETTINGS.getPowerButtonDuration()) {
    enterDeepSleep();
    // This should never be hit as `enterDeepSleep` calls esp_deep_sleep_start
    return;
  }

  const unsigned long activityStartTime = millis();
  if (currentActivity) {
    currentActivity->loop();
  }
  const unsigned long activityDuration = millis() - activityStartTime;

  const unsigned long loopDuration = millis() - loopStartTime;
  if (loopDuration > maxLoopDuration) {
    maxLoopDuration = loopDuration;
    if (maxLoopDuration > 50) {
      Serial.printf("[%lu] [LOOP] New max loop duration: %lu ms (activity: %lu ms)\n", millis(), maxLoopDuration,
                    activityDuration);
    }
  }

  // Add delay at the end of the loop to prevent tight spinning
  // When an activity requests skip loop delay (e.g., webserver running), use yield() for faster response
  // Otherwise, use longer delay to save power
  if (currentActivity && currentActivity->skipLoopDelay()) {
    yield(); // Give FreeRTOS a chance to run tasks, but return immediately
  } else {
    delay(10); // Normal delay when no activity requires fast response
  }
}
