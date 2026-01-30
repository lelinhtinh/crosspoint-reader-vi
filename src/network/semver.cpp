#include "network/semver.h"

#include <cctype>

void parseSemver(const std::string &ver, int &major, int &minor, int &patch) {
  major = minor = patch = 0;
  size_t i = 0;

  while (i < ver.size() && !std::isdigit(static_cast<unsigned char>(ver[i])))
    ++i;

  auto parseInt = [&](int &out) {
    out = 0;
    bool found = false;
    while (i < ver.size() && std::isdigit(static_cast<unsigned char>(ver[i]))) {
      found = true;
      out = out * 10 + (ver[i] - '0');
      ++i;
    }
    return found;
  };

  parseInt(major);
  while (i < ver.size() && !std::isdigit(static_cast<unsigned char>(ver[i])))
    ++i;
  parseInt(minor);
  while (i < ver.size() && !std::isdigit(static_cast<unsigned char>(ver[i])))
    ++i;
  parseInt(patch);
}
