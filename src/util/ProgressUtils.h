#pragma once

#include <string>

namespace ProgressUtils {

// Get book progress for a given file path.
// For EPUB: returns chapter index (1-based) and total spine entries.
// For XTC: returns page index (1-based) and total pages.
// For TXT: returns page index (1-based) and total pages when available.
// Returns true if total is known (current may be 0 if not started).
bool getBookProgress(const std::string &path, int &currentOut, int &totalOut);

} // namespace ProgressUtils
