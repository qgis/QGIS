#ifndef ODBC_STRING_CONVERTER_H_INCLUDED
#define ODBC_STRING_CONVERTER_H_INCLUDED
//------------------------------------------------------------------------------
#include <odbc/Config.h>
#include <cstddef>
#include <utility>
#include <string>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
class ODBC_EXPORT StringConverter
{
public:
    StringConverter() = delete;

    /**
     * Converts a null-terminated UTF-8 string to a UTF-16 string.
     *
     * @param src  The null-terminated UTF-8 string to be converted.
     * @return     The resulting UTF-16 string.
     */
    static std::u16string utf8ToUtf16(const char* src);

    /**
     * Converts a UTF-8 string to a UTF-16 string.
     *
     * @param src        The UTF-8 string to be converted.
     * @param srcLength  The length of the input string.
     * @return           The resulting UTF-16 string.
     */
    static std::u16string utf8ToUtf16(const char* src, std::size_t srcLength);

private:
    static std::u16string utf8ToUtf16(const char* begin, const char* end);

    static std::pair<int, char32_t> utf8ToCodePoint(
        const char* begin,
        const char* curr,
        const char* end);

    static std::size_t utf8ToUtf16Length(const char* begin, const char* end);
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
