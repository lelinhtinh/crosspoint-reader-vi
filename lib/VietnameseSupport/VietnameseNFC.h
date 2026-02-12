#pragma once

#include <cstdint>

/**
 * Vietnamese Unicode Normalization
 *
 * This module provides Vietnamese-specific UTF-8 handling with NFC (Composed) normalization.
 * It transforms NFD (Decomposed) Vietnamese text into NFC (Precomposed) form.
 *
 * Example: 'u' + COMBINING_TILDE (U+0303) → 'ũ' (U+0169)
 *
 * This is a fork-specific extension. Upstream lib/Utf8 only provides basic UTF-8 codepoint
 * extraction without Vietnamese composition. Place Vietnamese-specific code here to avoid
 * future merge conflicts with upstream.
 */

/**
 * Get the next Unicode codepoint from a UTF-8 string with Vietnamese NFC normalization.
 * This handles NFD (decomposed) Vietnamese text by combining base characters with
 * combining diacritical marks into precomposed characters.
 *
 * @param string Pointer to pointer to UTF-8 string (will be advanced past consumed bytes)
 * @return The next codepoint (NFC normalized for Vietnamese)
 */
uint32_t utf8NextCodepointNFC(const unsigned char **string);
