// FORK-FEATURE-BEGIN: READING_TIME
#include "ReadingTimeTracker.h"

#include <HalStorage.h>
#include <I18n.h>
#include <Logging.h>

namespace ReadingTimeTracker {

uint32_t load(const std::string& cachePath) {
  FsFile f;
  if (!Storage.openFileForRead("RTT", cachePath + "/readingtime.bin", f)) {
    return 0;
  }
  uint8_t data[4];
  const int n = f.read(data, 4);
  f.close();
  if (n != 4) {
    return 0;
  }
  return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
}

void save(const std::string& cachePath, uint32_t totalSeconds) {
  if (cachePath.empty()) {
    return;
  }
  FsFile f;
  if (!Storage.openFileForWrite("RTT", cachePath + "/readingtime.bin", f)) {
    LOG_ERR("RTT", "Failed to open readingtime.bin for write");
    return;
  }
  uint8_t data[4];
  data[0] = totalSeconds & 0xFF;
  data[1] = (totalSeconds >> 8) & 0xFF;
  data[2] = (totalSeconds >> 16) & 0xFF;
  data[3] = (totalSeconds >> 24) & 0xFF;
  f.write(data, 4);
  f.close();
}

void format(uint32_t seconds, char* buf, size_t bufSize) {
  if (bufSize == 0) {
    return;
  }
  // Zero seconds — caller displays fallback text (e.g. "Continue Reading")
  if (seconds == 0) {
    buf[0] = '\0';
    return;
  }
  if (seconds < 60) {
    snprintf(buf, bufSize, "%u%s", (unsigned)seconds, tr(STR_SECONDS_ABBR));
    return;
  }
  const uint32_t h = seconds / 3600;
  const uint32_t m = (seconds % 3600) / 60;
  if (h > 0 && m > 0) {
    snprintf(buf, bufSize, "%u%s %u%s", (unsigned)h, tr(STR_HOURS_ABBR), (unsigned)m, tr(STR_MINUTES_ABBR));
  } else if (h > 0) {
    snprintf(buf, bufSize, "%u%s", (unsigned)h, tr(STR_HOURS_ABBR));
  } else {
    snprintf(buf, bufSize, "%u%s", (unsigned)m, tr(STR_MINUTES_ABBR));
  }
}

}  // namespace ReadingTimeTracker
// FORK-FEATURE-END: READING_TIME
