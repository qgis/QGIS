#ifndef ODBC_INTERNAL_CHARSET_UTF16_H_INCLUDED
#define ODBC_INTERNAL_CHARSET_UTF16_H_INCLUDED
//------------------------------------------------------------------------------
#include <cassert>
#include <utility>
#include <odbc/Config.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
namespace utf16 {
//------------------------------------------------------------------------------
/**
 * Checks if a code point is representable in UTF-16, i.e. it is a code-point
 * less or equal to U+10FFFF and not a surrogate part.
 *
 * @param c  The code point to check.
 * @return   True if the code point is representable in UTF-16, false otherwise.
 */
inline bool isRepresentable(char32_t c)
{
    return c <= 0x10FFFF && !(c >= 0xD800 && c <= 0xDFFF);
}
//------------------------------------------------------------------------------
/**
 * Checks if a code point must be represented by a surrogate pair.
 *
 * @param c  The code point to check.
 * @return   True if a surrogate pair is needed, false otherwise.
 */
inline bool needsSurrogatePair(char32_t c)
{
    return c >= 0x10000;
}
//------------------------------------------------------------------------------
/**
 * Encodes a code point from the supplementary planes as a surrogate pair.
 *
 * @param c  Character from the supplementary planes, i.e. from U+10000 to
 *           U+10FFFF.
 * @return   A pair containing the high surrogate as first and the low surrogate
 *           as second.
 */
inline std::pair<char16_t, char16_t> encodeSurrogatePair(char32_t c)
{
    assert((c >= 0x10000) && (c <= 0x10FFFF));
    c -= 0x10000;
    return std::pair<char16_t, char16_t>{
        (char16_t)(0xD800 | (c >> 10)), (char16_t)(0xDC00 | (c & 0x3FF))};
}
//------------------------------------------------------------------------------
} // namespace utf16
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
