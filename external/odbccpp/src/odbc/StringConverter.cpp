#include <odbc/Exception.h>
#include <odbc/StringConverter.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/charset/Utf16.h>
#include <odbc/internal/charset/Utf8.h>
#include <cassert>
#include <cstring>
#include <sstream>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
// StringConverter class
//------------------------------------------------------------------------------
u16string StringConverter::utf8ToUtf16(const char* src)
{
    ODBC_CHECK(src != nullptr, "Input string must not be nullptr.");
    return utf8ToUtf16(src, nullptr);
}
//------------------------------------------------------------------------------
u16string StringConverter::utf8ToUtf16(const char* src, size_t srcLength)
{
    ODBC_CHECK(src != nullptr, "Input string must not be nullptr.");
    return utf8ToUtf16(src, src + srcLength);
}
//------------------------------------------------------------------------------
u16string StringConverter::utf8ToUtf16(const char* begin, const char* end)
{
    assert(begin != nullptr);

    if (end == nullptr)
        end = begin + strlen(begin);

    size_t len = utf8ToUtf16Length(begin, end);
    u16string str;
    str.reserve(len);

    const char* curr = begin;

    while (curr < end)
    {
        pair<int, char32_t> cp = utf8ToCodePoint(begin, curr, end);
        curr += cp.first;

        assert(utf16::isRepresentable(cp.second));

        if (utf16::needsSurrogatePair(cp.second))
        {
            pair<char16_t, char16_t> sp = utf16::encodeSurrogatePair(cp.second);
            str.push_back(sp.first);
            str.push_back(sp.second);
        }
        else
        {
            str.push_back(static_cast<char16_t>(cp.second));
        }
    }

    return str;
}
//------------------------------------------------------------------------------
pair<int, char32_t> StringConverter::utf8ToCodePoint(
    const char* begin, const char* curr, const char* end)
{
    assert(begin != nullptr && end != nullptr);
    assert(begin <= curr && curr < end);

    int len = utf8::getSequenceLength(*curr);
    if (len == 1)
    {
        // Short-cut for the easy and common case.
        return pair<int, char32_t>(len, *curr);
    }
    if (len == -1)
    {
        ODBC_FAIL("The string contains an invalid UTF-8 byte sequence at "
                  "position " << (curr - begin) << ".");
    }

    // We have to make sure that we don't exceed the end of the string.
    if ((curr + len) > end)
    {
        ODBC_FAIL("The string contains an incomplete UTF-8 byte sequence at "
                  "position " << (curr - begin) << ".");
    }

    if (!utf8::isValidSequence(len, curr))
    {
        ODBC_FAIL("The string contains an invalid UTF-8 byte sequence at "
                  "position " << (curr - begin) << ".");
    }

    return pair<int, char32_t>(len, utf8::decode(len, curr));
}
//------------------------------------------------------------------------------
size_t StringConverter::utf8ToUtf16Length(const char* begin, const char* end)
{
    assert(begin != nullptr && end != nullptr);

    size_t len = 0;

    const char* curr = begin;
    while (curr < end)
    {
        pair<int, char32_t> cp = utf8ToCodePoint(begin, curr, end);
        curr += cp.first;

        ODBC_CHECK(utf16::isRepresentable(cp.second),
                   "The UTF-8 string contains codepoint U+" <<
                   std::hex << (uint32_t)cp.second <<
                   ", which cannot be represented in UTF-16.");

        len += utf16::needsSurrogatePair(cp.second) ? 2 : 1;
    }

    return len;
}
//------------------------------------------------------------------------------
NS_ODBC_END
