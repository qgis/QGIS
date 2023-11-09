#include <algorithm>
#include <cstring>
#include <odbc/internal/Odbc.h>
#include <odbc/internal/UtilInternal.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
static uint32_t fromLittleEndianArray(const SQLCHAR* number)
{
    uint32_t ret =
          (static_cast<unsigned char>(number[0]) << 0)
        | (static_cast<unsigned char>(number[1]) << 8)
        | (static_cast<unsigned char>(number[2]) << 16)
        | (static_cast<unsigned char>(number[3]) << 24);
    return ret;
}
//------------------------------------------------------------------------------
static void toLittleEndianArray(uint32_t val, SQLCHAR* target)
{
    target[0] = val & 0xFF;
    target[1] = (val >> 8) & 0xFF;
    target[2] = (val >> 16) & 0xFF;
    target[3] = (val >> 24) & 0xFF;
}
//------------------------------------------------------------------------------
template<int N>
static bool allZero(const uint32_t(&number)[N])
{
    for (int i = 0; i < N; ++i) {
        if (number[i] != 0)
            return false;
    }
    return true;
}
//------------------------------------------------------------------------------
void UtilInternal::numericToString(const SQL_NUMERIC_STRUCT& num, char* str)
{
    static_assert((SQL_MAX_NUMERIC_LEN % 4) == 0,
        "SQL_MAX_NUMERIC_LEN is not a multiple of 4");

    // Translate to base 2^32
    constexpr int NUM_DIGITS = SQL_MAX_NUMERIC_LEN / 4;
    uint32_t digits[NUM_DIGITS];
    for (int i = 0; i < NUM_DIGITS; ++i) {
        digits[i] = fromLittleEndianArray(num.val + 4 * i);
    }

    // Perform an Euclidean division by 10
    char* pos = str;
    while (!allZero(digits))
    {
        uint64_t carry = 0;
        for (int i = NUM_DIGITS - 1; i >= 0; --i)
        {
            uint64_t with_carry = digits[i] + (carry << 32);
            digits[i] = static_cast<uint32_t>(with_carry / 10);
            carry = with_carry % 10;
        }
        *pos = '0' + static_cast<char>(carry);
        ++pos;
    }
    // Treat the special case in which the number is 0
    if (pos == str) {
        *pos = '0';
        ++pos;
    } else if (num.sign == 0) {
        // Append the sign (only if not 0)
        *pos = '-';
        ++pos;
    }

    // Reverse and terminate the string
    reverse(str, pos);
    *pos = '\0';
}
//------------------------------------------------------------------------------
void UtilInternal::decimalToNumeric(const decimal& dec, SQL_NUMERIC_STRUCT& num)
{
    static_assert((SQL_MAX_NUMERIC_LEN % 4) == 0,
        "SQL_MAX_NUMERIC_LEN is not a multiple of 4");

    num.scale = dec.scale();
    num.precision = dec.precision();
    num.sign = (dec.signum() == -1) ? 0 : 1;

    constexpr int NUM_DIGITS = SQL_MAX_NUMERIC_LEN / 4;
    uint32_t digits[NUM_DIGITS];
    memset(digits, 0, NUM_DIGITS * sizeof(uint32_t));

    const char* pos = dec.unscaledValue();
    // Skip the sign
    if (dec.signum() == -1)
        ++pos;
    while (*pos)
    {
        uint64_t value = *pos - '0';
        for (int i = 0; i < NUM_DIGITS; i++)
        {
            value = value + static_cast<uint64_t>(digits[i]) * 10;
            digits[i] = static_cast<uint32_t>(value);
            value = value >> 32;
        }
        ++pos;
    }

    for (int i = 0; i < NUM_DIGITS; ++i) {
        toLittleEndianArray(digits[i], num.val + 4 * i);
    }
}
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
