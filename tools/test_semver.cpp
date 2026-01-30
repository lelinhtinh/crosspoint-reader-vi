#include <iostream>
#include <string>
#include <vector>

#include "network/semver.h"

struct Case {
  std::string ver;
  int ma, mi, pa;
};

int main() {
  std::vector<Case> cases = {
      {"v0.16.1", 0, 16, 1}, {"0.16.1", 0, 16, 1}, {"0.16.0-fork.vi.2", 0, 16, 0}, {"0.16.0-fork.vi.2-dev", 0, 16, 0},
      {"v1.2", 1, 2, 0},     {"2", 2, 0, 0},       {"version3.4.5", 3, 4, 5},      {"beta", 0, 0, 0},
  };

  bool ok = true;
  for (const auto &c : cases) {
    int a = 0, b = 0, d = 0;
    parseSemver(c.ver, a, b, d);
    if (a != c.ma || b != c.mi || d != c.pa) {
      std::cout << "FAIL: " << c.ver << " -> " << a << "." << b << "." << d << " expected " << c.ma << "." << c.mi
                << "." << c.pa << "\n";
      ok = false;
    } else {
      std::cout << "OK: " << c.ver << " -> " << a << "." << b << "." << d << "\n";
    }
  }

  return ok ? 0 : 1;
}
