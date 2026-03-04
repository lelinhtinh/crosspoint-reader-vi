#pragma once

#include <builtinFonts/bookerly_12_bold.h>
#include <builtinFonts/bookerly_12_bolditalic.h>
#include <builtinFonts/bookerly_12_italic.h>
#include <builtinFonts/bookerly_12_regular.h>
#include <builtinFonts/bookerly_14_bold.h>
#include <builtinFonts/bookerly_14_bolditalic.h>
#include <builtinFonts/bookerly_14_italic.h>
#include <builtinFonts/bookerly_14_regular.h>
#include <builtinFonts/bookerly_16_bold.h>
#include <builtinFonts/bookerly_16_bolditalic.h>
#include <builtinFonts/bookerly_16_italic.h>
#include <builtinFonts/bookerly_16_regular.h>
#include <builtinFonts/bookerly_18_bold.h>
#include <builtinFonts/bookerly_18_bolditalic.h>
#include <builtinFonts/bookerly_18_italic.h>
#include <builtinFonts/bookerly_18_regular.h>
#include <builtinFonts/notosans_8_regular.h>
// FORK-FEATURE-BEGIN: VIETNAMESE
// NotoSans reader fonts (12–18pt) are hidden from font choices when VI is enabled.
// The .h files are deleted by generate_fonts_vi.sh to reclaim ~7MB of flash.
#if !ENABLE_VIETNAMESE_SUPPORT
#include <builtinFonts/notosans_12_bold.h>
#include <builtinFonts/notosans_12_bolditalic.h>
#include <builtinFonts/notosans_12_italic.h>
#include <builtinFonts/notosans_12_regular.h>
#include <builtinFonts/notosans_14_bold.h>
#include <builtinFonts/notosans_14_bolditalic.h>
#include <builtinFonts/notosans_14_italic.h>
#include <builtinFonts/notosans_14_regular.h>
#include <builtinFonts/notosans_16_bold.h>
#include <builtinFonts/notosans_16_bolditalic.h>
#include <builtinFonts/notosans_16_italic.h>
#include <builtinFonts/notosans_16_regular.h>
#include <builtinFonts/notosans_18_bold.h>
#include <builtinFonts/notosans_18_bolditalic.h>
#include <builtinFonts/notosans_18_italic.h>
#include <builtinFonts/notosans_18_regular.h>
#endif  // !ENABLE_VIETNAMESE_SUPPORT
// FORK-FEATURE-END: VIETNAMESE
#include <builtinFonts/ubuntu_10_bold.h>
#include <builtinFonts/ubuntu_10_regular.h>
#include <builtinFonts/ubuntu_12_bold.h>
#include <builtinFonts/ubuntu_12_regular.h>
