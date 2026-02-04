#pragma once

#include <cstdint>
#include <string>

#define REPLACEMENT_GLYPH 0xFFFD

uint32_t utf8NextCodepoint(const unsigned char **string);

/**
 * Get the next Unicode codepoint from a UTF-8 string with Vietnamese NFC normalization.
 * This handles NFD (decomposed) Vietnamese text by combining base characters with
 * combining diacritical marks into precomposed characters.
 *
 * For example: 'u' + COMBINING_TILDE (U+0303) → 'ũ' (U+0169)
 *
 * @param string Pointer to pointer to UTF-8 string (will be advanced)
 * @return The next codepoint (NFC normalized for Vietnamese)
 */
uint32_t utf8NextCodepointNFC(const unsigned char **string);

// Remove the last UTF-8 codepoint from a std::string and return the new size.
size_t utf8RemoveLastChar(std::string &str);
// Truncate string by removing N UTF-8 codepoints from the end.
void utf8TruncateChars(std::string &str, size_t numChars);
