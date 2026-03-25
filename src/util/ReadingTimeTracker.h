#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// FORK-FEATURE-BEGIN: READING_TIME
namespace ReadingTimeTracker {

// Load total accumulated reading seconds from <cachePath>/readingtime.bin.
// Returns 0 if the file does not exist or is corrupt.
uint32_t load(const std::string& cachePath);

// Save total accumulated reading seconds to <cachePath>/readingtime.bin.
void save(const std::string& cachePath, uint32_t totalSeconds);

// Format reading time into buf.
// Produces "2h 30m", "45m", "30s", or "" (empty string) when seconds == 0.
// Uses the tr() i18n macro for "h"/"m"/"s" abbreviations.
void format(uint32_t seconds, char* buf, size_t bufSize);

}  // namespace ReadingTimeTracker
// FORK-FEATURE-END: READING_TIME
