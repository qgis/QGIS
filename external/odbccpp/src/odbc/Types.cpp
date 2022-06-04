#include <odbc/Exception.h>
#include <odbc/Types.h>
#include <odbc/internal/Macros.h>
#include <algorithm>
#include <cstdio>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
decimal::decimal() : value_("0"), precision_(1), scale_(1)
{
}
//------------------------------------------------------------------------------
decimal::decimal(std::int64_t value, std::uint8_t precision, std::uint8_t scale)
    : decimal(to_string(value), precision, scale)
{
}
//------------------------------------------------------------------------------
decimal::decimal(
    std::uint64_t value, std::uint8_t precision, std::uint8_t scale)
    : decimal(to_string(value), precision, scale)
{
}
//------------------------------------------------------------------------------
decimal::decimal(
    std::string const& value, std::uint8_t precision, std::uint8_t scale)
    : decimal(value.c_str(), precision, scale)
{
}
//------------------------------------------------------------------------------
decimal::decimal(const char* value, std::uint8_t precision, std::uint8_t scale)
    : precision_(precision), scale_(scale)
{
    ODBC_CHECK(precision >= 1 && precision <= 38,
        "precision value must lie within [1,38]");
    ODBC_CHECK(scale <= precision, "scale value must lie within [0,precision]");

    // decimal = [sign] digit+
    // sign = "+" | "-"
    // digit = "0" - "9"

    const char* pos = value;

    // Process the sign
    bool isNegative = false;
    if (*pos == '+') {
        ++pos;
    } else if (*pos == '-') {
        isNegative = true;
        ++pos;
    }

    // Process the digits
    const char* digitsStart = pos;

    // Skip leading zeros
    while (*pos == '0') {
        ++pos;
    }

    // Process remaining digits
    const char* nonZeroDigitsStart = pos;
    while (*pos) {
        ODBC_CHECK(('0' <= *pos) && (*pos <= '9'),
            "Decimal contains an invalid digit at position " << pos - value);
        ++pos;
    }

    const char* digitsEnd = pos;
    ODBC_CHECK(digitsStart != digitsEnd, "Decimal does not contain any digits");

    // Special case: Only zeros
    if (nonZeroDigitsStart == digitsEnd) {
        value_ = "0";
        return;
    }

    // Usual case: some non-zero digits
    ptrdiff_t numDigits = digitsEnd - nonZeroDigitsStart;
    ODBC_CHECK(numDigits <= precision, "Decimal cannot have more than "
        << precision << " digits, but has " << numDigits);

    if (isNegative)
        value_.push_back('-');
    value_.append(nonZeroDigitsStart, digitsEnd);
}
//------------------------------------------------------------------------------
int8_t decimal::signum() const
{
    switch (value_[0]) {
    case '-':
        return -1;
    case '0':
        return 0;
    default:
        return 1;
    }
}
//------------------------------------------------------------------------------
string decimal::toString() const
{
    if (scale_ == 0)
        return value_;

    bool isSigned = (value_[0] == '-');
    size_t numDigits = value_.length() - (isSigned ? 1 : 0);

    string ret;
    if (scale_ < numDigits)
    {
        ret.reserve(value_.length() + 1);
        size_t numCharsBeforeDecimalPoint = value_.length() - scale_;
        ret.append(value_.begin(), value_.begin() + numCharsBeforeDecimalPoint);
        ret.push_back('.');
        ret.append(value_.begin() + numCharsBeforeDecimalPoint, value_.end());
    }
    else
    {
        size_t len = (isSigned ? 1 : 0) + 2 + scale_;
        ret.reserve(len);
        if (isSigned)
            ret.push_back('-');
        ret.append("0.");
        size_t numZeros = scale_ - numDigits;
        ret.append(numZeros, '0');
        ret.append(value_.begin() + (isSigned ? 1 : 0), value_.end());
    }
    return ret;
}
//------------------------------------------------------------------------------
bool decimal::operator==(const decimal& other) const
{
    return cmp(other) == 0;
}
//------------------------------------------------------------------------------
bool decimal::operator!=(const decimal& other) const
{
    return cmp(other) != 0;
}
//------------------------------------------------------------------------------
bool decimal::operator<(const decimal& other) const
{
    return cmp(other) < 0;
}
//------------------------------------------------------------------------------
bool decimal::operator>(const decimal& other) const
{
    return cmp(other) > 0;
}
//------------------------------------------------------------------------------
bool decimal::operator<=(const decimal& other) const
{
    return cmp(other) <= 0;
}
//------------------------------------------------------------------------------
bool decimal::operator>=(const decimal& other) const
{
    return cmp(other) >= 0;
}
//------------------------------------------------------------------------------
int decimal::cmp(const decimal& other) const
{
    // If signums are different, we can decide the relation of the numbers to
    // each other.
    int signumThis = signum();
    int signumOther = other.signum();
    if (signumThis != signumOther)
        return signumThis - signumOther;

    // Signums are equals. Let's take a shortcut for the special case that both
    // numbers are 0.
    if (signumThis == 0)
        return 0;

    // When comparing the numbers, we have to consider the scale properly.
    // We assign positions to each digit. The digit immediately before the the
    // decimal point is assigned position 0, the digit before that position 1
    // and so on. The first digit after the decimal point is assigned position
    // -1 and so on. After that we can compare digit by digit starting with
    // the digit having the highest position.
    bool isSigned = (value_[0] == '-');
    int numDigitsThis = (int)value_.length() - (isSigned ? 1 : 0);
    int numDigitsOther = (int)other.value_.length() - (isSigned ? 1 : 0);

    int maxDigitThis = numDigitsThis - scale_ - 1;
    int minDigitThis = -(int)scale_;
    int maxDigitOther = numDigitsOther - other.scale_ - 1;
    int minDigitOther = -(int)other.scale_;

    // We iterate over the whole position range of both numbers.
    int maxDigit = max(maxDigitThis, maxDigitOther);
    int minDigit = min(minDigitThis, minDigitOther);
    for (int i = maxDigit; i > minDigit; --i)
    {
        // Get the digit at position i of this number. Substitute '0' if
        // position i is not available.
        char digitThis;
        if ((i > maxDigitThis) || (i < minDigitThis))
        {
            digitThis = '0';
        }
        else
        {
            int rpos = -minDigitThis + i;
            digitThis = value_[value_.length() - 1 - rpos];
        }
        // Get the digit at position i of the other number. Substitute '0' if
        // position i is not available.
        char digitOther;
        if ((i > maxDigitOther) || (i < minDigitOther))
        {
            digitOther = '0';
        }
        else
        {
            int rpos = -minDigitOther + i;
            digitOther = other.value_[other.value_.length() - 1 - rpos];
        }
        // If the digits are different, we are done.
        if (digitThis != digitOther)
            return (int)digitThis - (int)digitOther;
    }
    return 0;
}
//------------------------------------------------------------------------------
ostream& operator<<(ostream& out, const decimal& d)
{
    out << d.toString();
    return out;
}
//------------------------------------------------------------------------------
date::date() : date(0, 1, 1) {}
//------------------------------------------------------------------------------
date::date(int year, int month, int day)
{
    ODBC_CHECK((year >= 0) && (year <= 9999), "Invalid year (" << year << ")");
    ODBC_CHECK((month >= 1) && (month <= 12),
        "Invalid month (" << month << ")");
    ODBC_CHECK((day >= 1) && (day <= daysInMonth(year, month)),
        "Invalid day (" << day << ")");
    year_ = year;
    month_ = month;
    day_ = day;
}
//------------------------------------------------------------------------------
string date::toString() const
{
    char buffer[32];
    snprintf(buffer, 32, "%04d-%02d-%02d", year_, month_, day_);
    return string(buffer);
}
//------------------------------------------------------------------------------
bool date::operator==(const date& other) const
{
    return (year_ == other.year_)
        && (month_ == other.month_)
        && (day_ == other.day_);
}
//------------------------------------------------------------------------------
bool date::operator!=(const date& other) const
{
    return !(*this == other);
}
//------------------------------------------------------------------------------
bool date::operator<(const date& other) const
{
    if (year_ != other.year_)
        return year_ < other.year_;
    if (month_ != other.month_)
        return month_ < other.month_;
    return day_ < other.day_;
}
//------------------------------------------------------------------------------
bool date::operator>(const date& other) const
{
    if (year_ != other.year_)
        return year_ > other.year_;
    if (month_ != other.month_)
        return month_ > other.month_;
    return day_ > other.day_;
}
//------------------------------------------------------------------------------
bool date::operator<=(const date& other) const
{
    return !(*this > other);
}
//------------------------------------------------------------------------------
bool date::operator>=(const date& other) const
{
    return !(*this < other);
}
//------------------------------------------------------------------------------
int date::daysInMonth(int year, int month)
{
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return daysInFebruary(year);
    default:
        ODBC_FAIL("Invalid month (" << month << ")");
    }
}
//------------------------------------------------------------------------------
int date::daysInFebruary(int year)
{
    if ((year % 400) == 0)
        return 29;
    if ((year % 100) == 0)
        return 28;
    if ((year % 4) == 0)
        return 29;
    return 28;
}
//------------------------------------------------------------------------------
ostream& operator<<(ostream& out, const date& d)
{
    out << d.toString();
    return out;
}
//------------------------------------------------------------------------------
time::time() : time(0, 0, 0) {}
//------------------------------------------------------------------------------
time::time(int hour, int minute, int second)
{
    ODBC_CHECK((hour >= 0) && (hour <= 23), "Invalid hour (" << hour << ")");
    ODBC_CHECK((minute >= 0) && (minute <= 59),
        "Invalid minute (" << minute << ")");
    ODBC_CHECK((second >= 0) && (second <= 59),
        "Invalid second (" << second << ")");
    hour_ = hour;
    minute_ = minute;
    second_ = second;
}
//------------------------------------------------------------------------------
string time::toString() const
{
    char buffer[32];
    snprintf(buffer, 32, "%02d:%02d:%02d", hour_, minute_, second_);
    return string(buffer);
}
//------------------------------------------------------------------------------
bool time::operator==(const time& other) const
{
    return (hour_ == other.hour_)
        && (minute_ == other.minute_)
        && (second_ == other.second_);
}
//------------------------------------------------------------------------------
bool time::operator!=(const time& other) const
{
    return !(*this == other);
}
//------------------------------------------------------------------------------
bool time::operator<(const time& other) const
{
    if (hour_ != other.hour_)
        return hour_ < other.hour_;
    if (minute_ != other.minute_)
        return minute_ < other.minute_;
    return second_ < other.second_;
}
//------------------------------------------------------------------------------
bool time::operator>(const time& other) const
{
    if (hour_ != other.hour_)
        return hour_ > other.hour_;
    if (minute_ != other.minute_)
        return minute_ > other.minute_;
    return second_ > other.second_;
}
//------------------------------------------------------------------------------
bool time::operator<=(const time& other) const
{
    return !(*this > other);
}
//------------------------------------------------------------------------------
bool time::operator>=(const time& other) const
{
    return !(*this < other);
}
//------------------------------------------------------------------------------
ostream& operator<<(ostream& out, const time& t)
{
    out << t.toString();
    return out;
}
//------------------------------------------------------------------------------
timestamp::timestamp() : timestamp(0, 1, 1, 0, 0, 0, 0) {}
//------------------------------------------------------------------------------
timestamp::timestamp(int year, int month, int day, int hour, int minute,
    int second, int milliseconds) : date(year, month, day),
    time(hour, minute, second)
{
    ODBC_CHECK((milliseconds >= 0) && (milliseconds <= 999),
        "Invalid milliseconds (" << milliseconds << ")");
    milliseconds_ = milliseconds;
}
//------------------------------------------------------------------------------
string timestamp::toString() const
{
    char buffer[40];
    snprintf(buffer, 40, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        year_, month_, day_, hour_, minute_, second_, milliseconds_);
    return string(buffer);
}
//------------------------------------------------------------------------------
bool timestamp::operator==(const timestamp& other) const
{
    return date::operator==(other)
        && time::operator==(other)
        && (milliseconds_ == other.milliseconds_);
}
//------------------------------------------------------------------------------
bool timestamp::operator!=(const timestamp& other) const
{
    return !(*this == other);
}
//------------------------------------------------------------------------------
bool timestamp::operator<(const timestamp& other) const
{
    if (date::operator!=(other))
        return date::operator<(other);
    if (time::operator!=(other))
        return time::operator<(other);
    return milliseconds_ < other.milliseconds_;
}
//------------------------------------------------------------------------------
bool timestamp::operator>(const timestamp& other) const
{
    if (date::operator!=(other))
        return date::operator>(other);
    if (time::operator!=(other))
        return time::operator>(other);
    return milliseconds_ > other.milliseconds_;
}
//------------------------------------------------------------------------------
bool timestamp::operator<=(const timestamp& other) const
{
    return !(*this > other);
}
//------------------------------------------------------------------------------
bool timestamp::operator>=(const timestamp& other) const
{
    return !(*this < other);
}
//------------------------------------------------------------------------------
ostream& operator<<(ostream& out, const timestamp& ts)
{
    out << ts.toString();
    return out;
}
//------------------------------------------------------------------------------
NS_ODBC_END
