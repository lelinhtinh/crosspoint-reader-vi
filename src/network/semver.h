#pragma once

#include <string>

// Parse a semantic version string into major/minor/patch integers.
// Tolerant to leading 'v' and non-digit separators; missing fields default to 0.
void parseSemver(const std::string &ver, int &major, int &minor, int &patch);
