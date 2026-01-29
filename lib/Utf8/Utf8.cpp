#include "Utf8.h"

int utf8CodepointLen(const unsigned char c) {
  if (c < 0x80)
    return 1; // 0xxxxxxx
  if ((c >> 5) == 0x6)
    return 2; // 110xxxxx
  if ((c >> 4) == 0xE)
    return 3; // 1110xxxx
  if ((c >> 3) == 0x1E)
    return 4; // 11110xxx
  return 1;   // fallback for invalid
}

uint32_t utf8NextCodepoint(const unsigned char **string) {
  if (**string == 0) {
    return 0;
  }

  const int bytes = utf8CodepointLen(**string);
  const uint8_t *chr = *string;
  *string += bytes;

  if (bytes == 1) {
    return chr[0];
  }

  uint32_t cp = chr[0] & ((1 << (7 - bytes)) - 1); // mask header bits

  for (int i = 1; i < bytes; i++) {
    cp = (cp << 6) | (chr[i] & 0x3F);
  }

  return cp;
}

// ============================================================================
// Vietnamese NFC Normalization
// ============================================================================
// Combining diacritical marks used in Vietnamese NFD:
//   0x0300 - COMBINING GRAVE ACCENT (huyền)
//   0x0301 - COMBINING ACUTE ACCENT (sắc)
//   0x0302 - COMBINING CIRCUMFLEX ACCENT (mũ)
//   0x0303 - COMBINING TILDE (ngã)
//   0x0306 - COMBINING BREVE (trăng)
//   0x0309 - COMBINING HOOK ABOVE (hỏi)
//   0x031B - COMBINING HORN (móc)
//   0x0323 - COMBINING DOT BELOW (nặng)

// Check if a codepoint is a Vietnamese combining mark
static bool isVietnameseCombiningMark(uint32_t cp) {
  return cp == 0x0300 || cp == 0x0301 || cp == 0x0302 || cp == 0x0303 || cp == 0x0306 || cp == 0x0309 || cp == 0x031B ||
         cp == 0x0323;
}

// Compose base + single combining mark into precomposed character
// Returns 0 if no composition exists
static uint32_t composeVietnamese(uint32_t base, uint32_t combining) {
  // Handle horn (U+031B) first - it modifies O and U
  if (combining == 0x031B) {
    switch (base) {
    case 'O':
      return 0x01A0; // Ơ
    case 'o':
      return 0x01A1; // ơ
    case 'U':
      return 0x01AF; // Ư
    case 'u':
      return 0x01B0; // ư
    // Horn on already-horned vowels (rare but possible)
    case 0x01A0:
      return 0x01A0; // Ơ
    case 0x01A1:
      return 0x01A1; // ơ
    case 0x01AF:
      return 0x01AF; // Ư
    case 0x01B0:
      return 0x01B0; // ư
    }
    return 0;
  }

  // Handle breve (U+0306) - for Ă
  if (combining == 0x0306) {
    switch (base) {
    case 'A':
      return 0x0102; // Ă
    case 'a':
      return 0x0103; // ă
    }
    return 0;
  }

  // Handle circumflex (U+0302) - for Â, Ê, Ô
  if (combining == 0x0302) {
    switch (base) {
    case 'A':
      return 0x00C2; // Â
    case 'a':
      return 0x00E2; // â
    case 'E':
      return 0x00CA; // Ê
    case 'e':
      return 0x00EA; // ê
    case 'O':
      return 0x00D4; // Ô
    case 'o':
      return 0x00F4; // ô
    }
    return 0;
  }

  // Handle tilde (U+0303)
  if (combining == 0x0303) {
    switch (base) {
    case 'A':
      return 0x00C3; // Ã
    case 'a':
      return 0x00E3; // ã
    case 'E':
      return 0x1EBC; // Ẽ
    case 'e':
      return 0x1EBD; // ẽ
    case 'I':
      return 0x0128; // Ĩ
    case 'i':
      return 0x0129; // ĩ
    case 'O':
      return 0x00D5; // Õ
    case 'o':
      return 0x00F5; // õ
    case 'U':
      return 0x0168; // Ũ
    case 'u':
      return 0x0169; // ũ
    case 'Y':
      return 0x1EF8; // Ỹ
    case 'y':
      return 0x1EF9; // ỹ
    // Tilde on horned vowels
    case 0x01A0:
      return 0x1EE0; // Ỡ
    case 0x01A1:
      return 0x1EE1; // ỡ
    case 0x01AF:
      return 0x1EEE; // Ữ
    case 0x01B0:
      return 0x1EEF; // ữ
    // Tilde on circumflex vowels
    case 0x00C2:
      return 0x1EAA; // Ẫ
    case 0x00E2:
      return 0x1EAB; // ẫ
    case 0x00CA:
      return 0x1EC4; // Ễ
    case 0x00EA:
      return 0x1EC5; // ễ
    case 0x00D4:
      return 0x1ED6; // Ỗ
    case 0x00F4:
      return 0x1ED7; // ỗ
    // Tilde on breve vowels
    case 0x0102:
      return 0x1EB4; // Ẵ
    case 0x0103:
      return 0x1EB5; // ẵ
    }
    return 0;
  }

  // Handle grave (U+0300)
  if (combining == 0x0300) {
    switch (base) {
    case 'A':
      return 0x00C0; // À
    case 'a':
      return 0x00E0; // à
    case 'E':
      return 0x00C8; // È
    case 'e':
      return 0x00E8; // è
    case 'I':
      return 0x00CC; // Ì
    case 'i':
      return 0x00EC; // ì
    case 'O':
      return 0x00D2; // Ò
    case 'o':
      return 0x00F2; // ò
    case 'U':
      return 0x00D9; // Ù
    case 'u':
      return 0x00F9; // ù
    case 'Y':
      return 0x1EF2; // Ỳ
    case 'y':
      return 0x1EF3; // ỳ
    // Grave on horned vowels
    case 0x01A0:
      return 0x1EDC; // Ờ
    case 0x01A1:
      return 0x1EDD; // ờ
    case 0x01AF:
      return 0x1EEA; // Ừ
    case 0x01B0:
      return 0x1EEB; // ừ
    // Grave on circumflex vowels
    case 0x00C2:
      return 0x1EA6; // Ầ
    case 0x00E2:
      return 0x1EA7; // ầ
    case 0x00CA:
      return 0x1EC0; // Ề
    case 0x00EA:
      return 0x1EC1; // ề
    case 0x00D4:
      return 0x1ED2; // Ồ
    case 0x00F4:
      return 0x1ED3; // ồ
    // Grave on breve vowels
    case 0x0102:
      return 0x1EB0; // Ằ
    case 0x0103:
      return 0x1EB1; // ằ
    }
    return 0;
  }

  // Handle acute (U+0301)
  if (combining == 0x0301) {
    switch (base) {
    case 'A':
      return 0x00C1; // Á
    case 'a':
      return 0x00E1; // á
    case 'E':
      return 0x00C9; // É
    case 'e':
      return 0x00E9; // é
    case 'I':
      return 0x00CD; // Í
    case 'i':
      return 0x00ED; // í
    case 'O':
      return 0x00D3; // Ó
    case 'o':
      return 0x00F3; // ó
    case 'U':
      return 0x00DA; // Ú
    case 'u':
      return 0x00FA; // ú
    case 'Y':
      return 0x00DD; // Ý
    case 'y':
      return 0x00FD; // ý
    // Acute on horned vowels
    case 0x01A0:
      return 0x1EDA; // Ớ
    case 0x01A1:
      return 0x1EDB; // ớ
    case 0x01AF:
      return 0x01AF; // Ứ - wait, need correct mapping
    case 0x01B0:
      return 0x1EE9; // ứ
    // Acute on circumflex vowels
    case 0x00C2:
      return 0x1EA4; // Ấ
    case 0x00E2:
      return 0x1EA5; // ấ
    case 0x00CA:
      return 0x1EBE; // Ế
    case 0x00EA:
      return 0x1EBF; // ế
    case 0x00D4:
      return 0x1ED0; // Ố
    case 0x00F4:
      return 0x1ED1; // ố
    // Acute on breve vowels
    case 0x0102:
      return 0x1EAE; // Ắ
    case 0x0103:
      return 0x1EAF; // ắ
    }
    // Fix: Ứ
    if (base == 0x01AF)
      return 0x1EE8; // Ứ
    return 0;
  }

  // Handle hook above (U+0309)
  if (combining == 0x0309) {
    switch (base) {
    case 'A':
      return 0x1EA2; // Ả
    case 'a':
      return 0x1EA3; // ả
    case 'E':
      return 0x1EBA; // Ẻ
    case 'e':
      return 0x1EBB; // ẻ
    case 'I':
      return 0x1EC8; // Ỉ
    case 'i':
      return 0x1EC9; // ỉ
    case 'O':
      return 0x1ECE; // Ỏ
    case 'o':
      return 0x1ECF; // ỏ
    case 'U':
      return 0x1EE6; // Ủ
    case 'u':
      return 0x1EE7; // ủ
    case 'Y':
      return 0x1EF6; // Ỷ
    case 'y':
      return 0x1EF7; // ỷ
    // Hook on horned vowels
    case 0x01A0:
      return 0x1EDE; // Ở
    case 0x01A1:
      return 0x1EDF; // ở
    case 0x01AF:
      return 0x1EEC; // Ử
    case 0x01B0:
      return 0x1EED; // ử
    // Hook on circumflex vowels
    case 0x00C2:
      return 0x1EA8; // Ẩ
    case 0x00E2:
      return 0x1EA9; // ẩ
    case 0x00CA:
      return 0x1EC2; // Ể
    case 0x00EA:
      return 0x1EC3; // ể
    case 0x00D4:
      return 0x1ED4; // Ổ
    case 0x00F4:
      return 0x1ED5; // ổ
    // Hook on breve vowels
    case 0x0102:
      return 0x1EB2; // Ẳ
    case 0x0103:
      return 0x1EB3; // ẳ
    }
    return 0;
  }

  // Handle dot below (U+0323)
  if (combining == 0x0323) {
    switch (base) {
    case 'A':
      return 0x1EA0; // Ạ
    case 'a':
      return 0x1EA1; // ạ
    case 'E':
      return 0x1EB8; // Ẹ
    case 'e':
      return 0x1EB9; // ẹ
    case 'I':
      return 0x1ECA; // Ị
    case 'i':
      return 0x1ECB; // ị
    case 'O':
      return 0x1ECC; // Ọ
    case 'o':
      return 0x1ECD; // ọ
    case 'U':
      return 0x1EE4; // Ụ
    case 'u':
      return 0x1EE5; // ụ
    case 'Y':
      return 0x1EF4; // Ỵ
    case 'y':
      return 0x1EF5; // ỵ
    // Dot below on horned vowels
    case 0x01A0:
      return 0x1EE2; // Ợ
    case 0x01A1:
      return 0x1EE3; // ợ
    case 0x01AF:
      return 0x1EF0; // Ự
    case 0x01B0:
      return 0x1EF1; // ự
    // Dot below on circumflex vowels
    case 0x00C2:
      return 0x1EAC; // Ậ
    case 0x00E2:
      return 0x1EAD; // ậ
    case 0x00CA:
      return 0x1EC6; // Ệ
    case 0x00EA:
      return 0x1EC7; // ệ
    case 0x00D4:
      return 0x1ED8; // Ộ
    case 0x00F4:
      return 0x1ED9; // ộ
    // Dot below on breve vowels
    case 0x0102:
      return 0x1EB6; // Ặ
    case 0x0103:
      return 0x1EB7; // ặ
    }
    return 0;
  }

  return 0;
}

// Peek at next codepoint without advancing the pointer
static uint32_t peekNextCodepoint(const unsigned char *string) { return utf8NextCodepoint(&string); }

uint32_t utf8NextCodepointNFC(const unsigned char **string) {
  if (**string == 0) {
    return 0;
  }

  // Get the first codepoint
  const unsigned char *savedPos = *string;
  uint32_t cp = utf8NextCodepoint(string);

  // Check if there's a combining mark following
  const unsigned char *peekPos = *string;
  uint32_t nextCp = peekNextCodepoint(peekPos);

  // Keep composing while we have combining marks
  while (nextCp != 0 && isVietnameseCombiningMark(nextCp)) {
    uint32_t composed = composeVietnamese(cp, nextCp);
    if (composed != 0) {
      // Successfully composed - consume the combining mark
      cp = composed;
      utf8NextCodepoint(string); // Advance past the combining mark
      // Check for more combining marks
      peekPos = *string;
      nextCp = peekNextCodepoint(peekPos);
    } else {
      // Can't compose - stop trying
      break;
    }
  }

  return cp;
}
