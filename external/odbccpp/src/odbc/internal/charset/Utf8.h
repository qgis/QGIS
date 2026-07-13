#ifndef ODBC_INTERNAL_CHARSET_UTF8_H_INCLUDED
#define ODBC_INTERNAL_CHARSET_UTF8_H_INCLUDED
//------------------------------------------------------------------------------
#include <cassert>
#include <odbc/Config.h>
#include <odbc/internal/Macros.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
namespace utf8 {
//------------------------------------------------------------------------------
/**
 * Determine the UTF-8 sequence length given the first byte of a UTF-8 sequence.
 *
 * @param c  First byte of a UTF-8 sequence.
 * @return   The length of the sequence or -1 if the passed byte cannot be the
 *           first byte of a sequence.
 */
inline int getSequenceLength(char c)
{
    if ((c & 0x80) == 0x00)
        return 1;
    if ((c & 0xE0) == 0xC0)
        return 2;
    if ((c & 0xF0) == 0xE0)
        return 3;
    if ((c & 0xF8) == 0xF0)
        return 4;
    return -1;
}
//------------------------------------------------------------------------------
/**
 * Checks if an UTF-8 sequence is valid.
 *
 * @param len  Length of the sequence. Must be in [1,4].
 * @param c    Pointer to the sequence start.
 * @return     Returns true if the sequence is a valid UTF-8 sequence of the
 *             given length, false otherwise.
 */
inline bool isValidSequence(int len, const char* c)
{
    switch (len)
    {
    case 1:
        return ((c[0] & 0x80) == 0x00);
    case 2:
        return ((c[0] & 0xE0) == 0xC0) && ((c[1] & 0xC0) == 0x80);
    case 3:
        return ((c[0] & 0xF0) == 0xE0) && ((c[1] & 0xC0) == 0x80)
               && ((c[2] & 0xC0) == 0x80);
    case 4:
        return ((c[0] & 0xF8) == 0xF0) && ((c[1] & 0xC0) == 0x80)
               && ((c[2] & 0xC0) == 0x80) && ((c[3] & 0xC0) == 0x80);
    }
    ODBC_UNREACHABLE;
}
//------------------------------------------------------------------------------
/**
 * Decodes a UTF-8 sequence of length 2.
 *
 * @param c  Pointer to the sequence start.
 * @return   Returns the decoded character.
 */
inline char32_t decode2(const char* c)
{
    assert(isValidSequence(2, c));
    char32_t b1 = (char32_t)(unsigned char)(c[0]);
    char32_t b2 = (char32_t)(unsigned char)(c[1]);
    return ((b1 & 0x1F) << 6) | (b2 & 0x3F);
}
//------------------------------------------------------------------------------
/**
 * Decodes a UTF-8 sequence of length 3.
 *
 * @param c  Pointer to the sequence start.
 * @return   Returns the decoded character.
 */
inline char32_t decode3(const char* c)
{
    assert(isValidSequence(3, c));
    char32_t b1 = (char32_t)(unsigned char)(c[0]);
    char32_t b2 = (char32_t)(unsigned char)(c[1]);
    char32_t b3 = (char32_t)(unsigned char)(c[2]);
    return ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
}
//------------------------------------------------------------------------------
/**
 * Decodes a UTF-8 sequence of length 4.
 *
 * @param c  Pointer to the sequence start.
 * @return   Returns the decoded character.
 */
inline char32_t decode4(const char* c)
{
    assert(isValidSequence(4, c));
    char32_t b1 = (char32_t)(unsigned char)(c[0]);
    char32_t b2 = (char32_t)(unsigned char)(c[1]);
    char32_t b3 = (char32_t)(unsigned char)(c[2]);
    char32_t b4 = (char32_t)(unsigned char)(c[3]);
    return ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6)
           | (b4 & 0x3F);
}
//------------------------------------------------------------------------------
/**
 * Decodes a UTF-8 sequence of the given length, which must be in [1, 4].
 *
 * @param len  The length of the sequence. Must be in [1, 4].
 * @param c    Pointer to the sequence start.
 * @return     Returns the decoded character.
 */
inline char32_t decode(int len, const char* c)
{
    switch (len)
    {
    case 1:
        return *c;
    case 2:
        return decode2(c);
    case 3:
        return decode3(c);
    case 4:
        return decode4(c);
    }
    ODBC_UNREACHABLE;
}
//------------------------------------------------------------------------------
} // namespace utf8
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
