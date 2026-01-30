#include "util/ProgressUtils.h"

#include <Epub.h>
#include <SDCardManager.h>
#include <Xtc.h>
#include <Txt.h>

#include <cstring>
#include <string>

namespace ProgressUtils {

static bool endsWithLower(const std::string &s, const char *suff) {
  const size_t sl = s.size();
  const size_t nl = strlen(suff);
  if (sl < nl) return false;
  for (size_t i = 0; i < nl; ++i) {
    char a = tolower(s[sl - nl + i]);
    char b = tolower(suff[i]);
    if (a != b) return false;
  }
  return true;
}

bool getBookProgress(const std::string &path, int &currentOut, int &totalOut) {
  currentOut = 0;
  totalOut = 0;

  // EPUB
  if (endsWithLower(path, ".epub")) {
    Epub epub(path, "/.crosspoint");
    if (!epub.load(false)) {
      return false;
    }
    // Try to prefer TOC-based chapter numbers (user-visible chapters) when available
    const int spineTotal = epub.getSpineItemsCount();
    if (spineTotal <= 0) {
      return false;
    }

    // Try to read progress.bin
    FsFile f;
    const std::string progPath = epub.getCachePath() + "/progress.bin";
    if (SdMan.openFileForRead("PGR", progPath, f)) {
      uint8_t data[6] = {0};
      const int sz = f.read(data, 6);
      if (sz >= 4) {
        const int spineIdx = data[0] + (data[1] << 8);
        // If the EPUB has a TOC, map spine index to TOC index so we show chapter numbers that match the TOC.
        const int tocIndex = epub.getTocIndexForSpineIndex(spineIdx);
        if (tocIndex != -1) {
          // TOC exists: show 1-based TOC index and total TOC entries
          currentOut = tocIndex + 1;
          totalOut = epub.getTocItemsCount();
        } else {
          // Fallback: show 1-based spine index and total spine items
          currentOut = spineIdx + 1;
          totalOut = spineTotal;
        }
      } else {
        currentOut = 0;
        totalOut = epub.getTocItemsCount() > 0 ? epub.getTocItemsCount() : spineTotal;
      }
      f.close();
    } else {
      // No progress saved yet
      currentOut = 0;
      totalOut = epub.getTocItemsCount() > 0 ? epub.getTocItemsCount() : spineTotal;
    }

    return true;
  }

  // XTC / XTCH
  if (endsWithLower(path, ".xtc") || endsWithLower(path, ".xtch")) {
    Xtc xtc(path, "/.crosspoint");
    if (!xtc.load()) {
      return false;
    }
    const uint32_t total = xtc.getPageCount();
    if (total == 0) return false;
    totalOut = static_cast<int>(total);

    // progress file is 4 bytes (little-endian page)
    FsFile f;
    const std::string progPath = xtc.getCachePath() + "/progress.bin";
    if (SdMan.openFileForRead("XPG", progPath, f)) {
      uint8_t data[4] = {0};
      if (f.read(data, 4) == 4) {
        uint32_t page = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        currentOut = static_cast<int>(page + 1);
      } else {
        currentOut = 0;
      }
      f.close();
    } else {
      currentOut = 0;
    }
    return true;
  }

  // TXT (use page index cache + progress.bin)
  if (endsWithLower(path, ".txt") || endsWithLower(path, ".md")) {
    Txt txt(path, "/.crosspoint");
    if (!txt.load()) {
      return false;
    }
    // Try to read page index cache to get total pages
    const std::string indexPath = txt.getCachePath() + "/index.bin";
    FsFile idx;
    int pages = 0;
    if (SdMan.openFileForRead("TXI", indexPath, idx)) {
      // Seek to position of total pages in the index header format used by Txt
      // The format stores total pages as a uint32_t after several headers; to avoid parsing, attempt
      // a lightweight approach: read the file and look for the cached total pages near the end.
      // Fallback: do not show total if parsing fails (avoid heavy code here).
      // For safety, we won't parse index.bin here — instead just try progress.bin and omit total.
      idx.close();
    }

    // Read progress.bin used by TxtReaderActivity: 4 bytes with current page low 2 bytes
    FsFile f;
    const std::string progPath = txt.getCachePath() + "/progress.bin";
    if (SdMan.openFileForRead("TPG", progPath, f)) {
      uint8_t data[4] = {0};
      if (f.read(data, 4) == 4) {
        int page = data[0] + (data[1] << 8);
        currentOut = page + 1;
      } else {
        currentOut = 0;
      }
      f.close();
      // Total unknown here (not parsed) — indicate presence without total
      totalOut = 0;
      return true;
    }

    return false;
  }

  return false;
}

} // namespace ProgressUtils
