// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.03.08

#pragma once

#include <Mathematics/BitHacks.h>
#include <Mathematics/Math.h>
#include <Mathematics/IEEEBinary.h>
#include <istream>
#include <ostream>

// The class BSNumber (binary scientific number) is designed to provide exact
// arithmetic for robust algorithms, typically those for which we need to know
// the exact sign of determinants. The template parameter UInteger must
// have support for at least the following public interface. The fstream
// objects for Write/Read must be created using std::ios::binary. The return
// value of Write/Read is 'true' iff the operation was successful.
//
//      class UInteger
//      {
//      public:
//          UInteger();
//          UInteger(UInteger const& number);
//          UInteger(uint32_t number);
//          UInteger(uint64_t number);
//          UInteger& operator=(UInteger const& number);
//          UInteger(UInteger&& number);
//          UInteger& operator=(UInteger&& number);
//          void SetNumBits(int numBits);
//          int32_t GetNumBits() const;
//          bool operator==(UInteger const& number) const;
//          bool operator< (UInteger const& number) const;
//          void Add(UInteger const& n0, UInteger const& n1);
//          void Sub(UInteger const& n0, UInteger const& n1);
//          void Mul(UInteger const& n0, UInteger const& n1);
//          void ShiftLeft(UInteger const& number, int shift);
//          int32_t ShiftRightToOdd(UInteger const& number);
//          int32_t RoundUp();
//          uint64_t GetPrefix(int numRequested) const;
//          bool Write(std::ofstream& output) const;
//          bool Read(std::ifstream& input);
//      };
//
// GTEngine currently has 32-bits-per-word storage for UInteger. See the
// classes UIntegerAP32 (arbitrary precision), UIntegerFP32<N> (fixed
// precision), and UIntegerALU32 (arithmetic logic unit shared by the previous
// two classes). The document at the following link describes the design,
// implementation, and use of BSNumber and BSRational.
//   https://www.geometrictools.com/Documentation/ArbitraryPrecision.pdf

// Support for unit testing algorithm correctness. The invariant for a
// nonzero BSNumber is that the UInteger part is a positive odd number.
// Expose this define to allow validation of the invariant.
#define GTE_VALIDATE_BSNUMBER

// Enable this to throw exceptions in ConvertFrom when infinities or NaNs
// are the floating-point inputs. BSNumber does not have representations
// for these numbers.
#define GTE_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN

// Support for debugging algorithms that use exact rational arithmetic. Each
// BSNumber and BSRational has a double-precision member that is exposed when
// the conditional define is enabled. Be aware that this can be very slow
// because of the conversion to double-precision whenever new objects are
// created by arithmetic operations. As a faster alternative, you can add
// temporary code in your algorithms that explicitly convert specific rational
// numbers to double precision.
//#define GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE

namespace gte
{
    template <typename UInteger> class BSRational;

    template <typename UInteger>
    class BSNumber
    {
    public:
        // Construction and destruction. The default constructor generates the
        // zero BSNumber.
        BSNumber()
            :
            mSign(0),
            mBiasedExponent(0)
        {
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = 0.0;
#endif
        }

        BSNumber(float number)
        {
            ConvertFrom<IEEEBinary32>(number);
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        BSNumber(double number)
        {
            ConvertFrom<IEEEBinary64>(number);
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = number;
#endif
        }

        BSNumber(int32_t number)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                if (number < 0)
                {
                    mSign = -1;
                    number = -number;
                }
                else
                {
                    mSign = 1;
                }

                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = (uint32_t)number;
            }
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
        }

        BSNumber(uint32_t number)
        {
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                mSign = 1;
                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = number;
            }
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        BSNumber(int64_t number)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                if (number < 0)
                {
                    mSign = -1;
                    number = -number;
                }
                else
                {
                    mSign = 1;
                }

                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = (uint64_t)number;
            }
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
        }

        BSNumber(uint64_t number)
        {
            if (number == 0)
            {
                mSign = 0;
                mBiasedExponent = 0;
            }
            else
            {
                mSign = 1;
                mBiasedExponent = BitHacks::GetTrailingBit(number);
                mUInteger = number;
            }
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(number);
#endif
        }

        // The number must be of the form "x" or "+x" or "-x", where x is a
        // positive integer with nonzero leading digit.
        BSNumber(std::string const& number)
        {
            LogAssert(number.size() > 0, "A number must be specified.");

            // Get the leading '+' or '-' if it exists.
            std::string intNumber;
            int sign;
            if (number[0] == '+')
            {
                intNumber = number.substr(1);
                sign = +1;
                LogAssert(intNumber.size() > 1, "Invalid number format.");
            }
            else if (number[0] == '-')
            {
                intNumber = number.substr(1);
                sign = -1;
                LogAssert(intNumber.size() > 1, "Invalid number format.");
            }
            else
            {
                intNumber = number;
                sign = +1;
            }

            *this = ConvertToInteger(intNumber);
            mSign = sign;
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = static_cast<double>(*this);
#endif
        }

        BSNumber(char const* number)
            :
            BSNumber(std::string(number))
        {
        }

        ~BSNumber() = default;

        // Copy semantics.
        BSNumber(BSNumber const& number)
        {
            *this = number;
        }

        BSNumber& operator=(BSNumber const& number)
        {
            mSign = number.mSign;
            mBiasedExponent = number.mBiasedExponent;
            mUInteger = number.mUInteger;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = number.mValue;
#endif
            return *this;
        }

        // Move semantics.
        BSNumber(BSNumber&& number) noexcept
        {
            *this = std::move(number);
        }

        BSNumber& operator=(BSNumber&& number) noexcept
        {
            mSign = number.mSign;
            mBiasedExponent = number.mBiasedExponent;
            mUInteger = std::move(number.mUInteger);
            number.mSign = 0;
            number.mBiasedExponent = 0;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = number.mValue;
            number.mValue = 0.0;
#endif
            return *this;
        }

        // Implicit conversions. These always use the default rounding mode,
        // round-to-nearest-ties-to-even.
        inline operator float() const
        {
            return ConvertTo<IEEEBinary32>();
        }

        inline operator double() const
        {
            return ConvertTo<IEEEBinary64>();
        }

        // Member access.
        inline void SetSign(int32_t sign)
        {
            mSign = sign;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        inline int32_t GetSign() const
        {
            return mSign;
        }

        inline void SetBiasedExponent(int32_t biasedExponent)
        {
            mBiasedExponent = biasedExponent;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        inline int32_t GetBiasedExponent() const
        {
            return mBiasedExponent;
        }

        inline void SetExponent(int32_t exponent)
        {
            mBiasedExponent = exponent - mUInteger.GetNumBits() + 1;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        inline int32_t GetExponent() const
        {
            return mBiasedExponent + mUInteger.GetNumBits() - 1;
        }

        inline UInteger const& GetUInteger() const
        {
            return mUInteger;
        }

        inline UInteger& GetUInteger()
        {
            return mUInteger;
        }

        // Comparisons.
        bool operator==(BSNumber const& number) const
        {
            return (mSign == number.mSign ? EqualIgnoreSign(*this, number) : false);
        }

        bool operator!=(BSNumber const& number) const
        {
            return !operator==(number);
        }

        bool operator< (BSNumber const& number) const
        {
            if (mSign > 0)
            {
                if (number.mSign <= 0)
                {
                    return false;
                }

                // Both numbers are positive.
                return LessThanIgnoreSign(*this, number);
            }
            else if (mSign < 0)
            {
                if (number.mSign >= 0)
                {
                    return true;
                }

                // Both numbers are negative.
                return LessThanIgnoreSign(number, *this);
            }
            else
            {
                return number.mSign > 0;
            }
        }

        bool operator<=(BSNumber const& number) const
        {
            return !number.operator<(*this);
        }

        bool operator> (BSNumber const& number) const
        {
            return number.operator<(*this);
        }

        bool operator>=(BSNumber const& number) const
        {
            return !operator<(number);
        }

        // Unary operations.
        BSNumber operator+() const
        {
            return *this;
        }

        BSNumber operator-() const
        {
            BSNumber result = *this;
            result.mSign = -result.mSign;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = (double)result;
#endif
            return result;
        }

        // Arithmetic.
        BSNumber operator+(BSNumber const& n1) const
        {
            BSNumber const& n0 = *this;

            if (n0.mSign == 0)
            {
                return n1;
            }

            if (n1.mSign == 0)
            {
                return n0;
            }

            if (n0.mSign > 0)
            {
                if (n1.mSign > 0)
                {
                    // n0 + n1 = |n0| + |n1|
                    return AddIgnoreSign(n0, n1, +1);
                }
                else // n1.mSign < 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 + n1 = |n0| - |n1| > 0
                            return SubIgnoreSign(n0, n1, +1);
                        }
                        else
                        {
                            // n0 + n1 = -(|n1| - |n0|) < 0
                            return SubIgnoreSign(n1, n0, -1);
                        }
                    }
                    // else n0 + n1 = 0
                }
            }
            else // n0.mSign < 0
            {
                if (n1.mSign < 0)
                {
                    // n0 + n1 = -(|n0| + |n1|)
                    return AddIgnoreSign(n0, n1, -1);
                }
                else // n1.mSign > 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 + n1 = -(|n0| - |n1|) < 0
                            return SubIgnoreSign(n0, n1, -1);
                        }
                        else
                        {
                            // n0 + n1 = |n1| - |n0| > 0
                            return SubIgnoreSign(n1, n0, +1);
                        }
                    }
                    // else n0 + n1 = 0
                }
            }

            return BSNumber();  // = 0
        }

        BSNumber operator-(BSNumber const& n1) const
        {
            BSNumber const& n0 = *this;

            if (n0.mSign == 0)
            {
                return -n1;
            }

            if (n1.mSign == 0)
            {
                return n0;
            }

            if (n0.mSign > 0)
            {
                if (n1.mSign < 0)
                {
                    // n0 - n1 = |n0| + |n1|
                    return AddIgnoreSign(n0, n1, +1);
                }
                else // n1.mSign > 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 - n1 = |n0| - |n1| > 0
                            return SubIgnoreSign(n0, n1, +1);
                        }
                        else
                        {
                            // n0 - n1 = -(|n1| - |n0|) < 0
                            return SubIgnoreSign(n1, n0, -1);
                        }
                    }
                    // else n0 - n1 = 0
                }
            }
            else // n0.mSign < 0
            {
                if (n1.mSign > 0)
                {
                    // n0 - n1 = -(|n0| + |n1|)
                    return AddIgnoreSign(n0, n1, -1);
                }
                else // n1.mSign < 0
                {
                    if (!EqualIgnoreSign(n0, n1))
                    {
                        if (LessThanIgnoreSign(n1, n0))
                        {
                            // n0 - n1 = -(|n0| - |n1|) < 0
                            return SubIgnoreSign(n0, n1, -1);
                        }
                        else
                        {
                            // n0 - n1 = |n1| - |n0| > 0
                            return SubIgnoreSign(n1, n0, +1);
                        }
                    }
                    // else n0 - n1 = 0
                }
            }

            return BSNumber();  // = 0
        }

        BSNumber operator*(BSNumber const& number) const
        {
            BSNumber result;  // = 0
            int sign = mSign * number.mSign;
            if (sign != 0)
            {
                result.mSign = sign;
                result.mBiasedExponent = mBiasedExponent + number.mBiasedExponent;
                result.mUInteger.Mul(mUInteger, number.mUInteger);
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = (double)result;
#endif
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(result.IsValid(), "Invalid BSNumber.");
#endif
            return result;
        }

        BSNumber& operator+=(BSNumber const& number)
        {
            *this = operator+(number);
            return *this;
        }

        BSNumber& operator-=(BSNumber const& number)
        {
            *this = operator-(number);
            return *this;
        }

        BSNumber& operator*=(BSNumber const& number)
        {
            *this = operator*(number);
            return *this;
        }

        // Disk input/output. The fstream objects should be created using
        // std::ios::binary. The return value is 'true' iff the operation
        // was successful.
        bool Write(std::ostream& output) const
        {
            if (output.write((char const*)&mSign, sizeof(mSign)).bad())
            {
                return false;
            }

            if (output.write((char const*)&mBiasedExponent,
                sizeof(mBiasedExponent)).bad())
            {
                return false;
            }

            return mUInteger.Write(output);
        }

        bool Read(std::istream& input)
        {
            if (input.read((char*)&mSign, sizeof(mSign)).bad())
            {
                return false;
            }

            if (input.read((char*)&mBiasedExponent, sizeof(mBiasedExponent)).bad())
            {
                return false;
            }

            return mUInteger.Read(input);
        }

#if defined(GTE_VALIDATE_BSNUMBER)
        bool IsValid() const
        {
            if (mSign != 0)
            {
                return
                    mUInteger.GetNumBits() > 0 &&
                    mUInteger.GetSize() > 0 &&
                    (mUInteger.GetBits()[0] & 0x00000001u) == 1u;
            }
            else
            {
                return
                    mBiasedExponent == 0 &&
                    mUInteger.GetNumBits() == 0 &&
                    mUInteger.GetSize() == 0;
            }
        }
#endif

    private:
        // Helper for converting a string to a BSNumber. The string must be
        // valid for a nonnegative integer without a leading '+' sign.
        static BSNumber ConvertToInteger(std::string const& number)
        {
            int digit = static_cast<int>(number.back()) - static_cast<int>('0');
            BSNumber x(digit);
            if (number.size() > 1)
            {
                LogAssert(number.find_first_of("123456789") == 0, "Invalid number format.");
                LogAssert(number.find_first_not_of("0123456789") == std::string::npos, "Invalid number format.");
                BSNumber ten(10), pow10(10);
                for (size_t i = 1, j = number.size() - 2; i < number.size(); ++i, --j)
                {
                    digit = static_cast<int>(number[j]) - static_cast<int>('0');
                    if (digit > 0)
                    {
                        x += BSNumber(digit) * pow10;
                    }
                    pow10 *= ten;
                }
            }
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(x.IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            x.mValue = (double)x;
#endif
            return x;
        }

        // Helpers for operator==, operator<, operator+, operator-.
        static bool EqualIgnoreSign(BSNumber const& n0, BSNumber const& n1)
        {
            return n0.mBiasedExponent == n1.mBiasedExponent && n0.mUInteger == n1.mUInteger;
        }

        static bool LessThanIgnoreSign(BSNumber const& n0, BSNumber const& n1)
        {
            int32_t e0 = n0.GetExponent(), e1 = n1.GetExponent();
            if (e0 < e1)
            {
                return true;
            }
            if (e0 > e1)
            {
                return false;
            }
            return n0.mUInteger < n1.mUInteger;
        }

        // Add two positive numbers.
        static BSNumber AddIgnoreSign(BSNumber const& n0, BSNumber const& n1, int32_t resultSign)
        {
            BSNumber result, temp;

            int32_t diff = n0.mBiasedExponent - n1.mBiasedExponent;
            if (diff > 0)
            {
                temp.mUInteger.ShiftLeft(n0.mUInteger, diff);
                result.mUInteger.Add(temp.mUInteger, n1.mUInteger);
                result.mBiasedExponent = n1.mBiasedExponent;
            }
            else if (diff < 0)
            {
                temp.mUInteger.ShiftLeft(n1.mUInteger, -diff);
                result.mUInteger.Add(n0.mUInteger, temp.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent;
            }
            else
            {
                temp.mUInteger.Add(n0.mUInteger, n1.mUInteger);
                int32_t shift = result.mUInteger.ShiftRightToOdd(temp.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent + shift;
            }

            result.mSign = resultSign;
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(result.IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = (double)result;
#endif
            return result;
        }

        // Subtract two positive numbers where n0 > n1.
        static BSNumber SubIgnoreSign(BSNumber const& n0, BSNumber const& n1, int32_t resultSign)
        {
            BSNumber result, temp;

            int32_t diff = n0.mBiasedExponent - n1.mBiasedExponent;
            if (diff > 0)
            {
                temp.mUInteger.ShiftLeft(n0.mUInteger, diff);
                result.mUInteger.Sub(temp.mUInteger, n1.mUInteger);
                result.mBiasedExponent = n1.mBiasedExponent;
            }
            else if (diff < 0)
            {
                temp.mUInteger.ShiftLeft(n1.mUInteger, -diff);
                result.mUInteger.Sub(n0.mUInteger, temp.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent;
            }
            else
            {
                temp.mUInteger.Sub(n0.mUInteger, n1.mUInteger);
                int32_t shift = result.mUInteger.ShiftRightToOdd(temp.mUInteger);
                result.mBiasedExponent = n0.mBiasedExponent + shift;
            }

            result.mSign = resultSign;
#if defined (GTE_VALIDATE_BSNUMBER)
            LogAssert(result.IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            result.mValue = (double)result;
#endif
            return result;
        }

        // Support for conversions from floating-point numbers to BSNumber.
        template <typename IEEE>
        void ConvertFrom(typename IEEE::FloatType number)
        {
            IEEE x(number);

            // Extract sign s, biased exponent e, and trailing significand t.
            typename IEEE::UIntType s = x.GetSign();
            typename IEEE::UIntType e = x.GetBiased();
            typename IEEE::UIntType t = x.GetTrailing();

            if (e == 0)
            {
                if (t == 0)  // zeros
                {
                    // x = (-1)^s * 0
                    mSign = 0;
                    mBiasedExponent = 0;
                }
                else  // subnormal numbers
                {
                    // x = (-1)^s * 0.t * 2^{1-EXPONENT_BIAS}
                    int32_t last = BitHacks::GetTrailingBit(t);
                    int32_t diff = IEEE::NUM_TRAILING_BITS - last;
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = IEEE::MIN_SUB_EXPONENT - diff;
                    mUInteger = (t >> last);
                }
            }
            else if (e < IEEE::MAX_BIASED_EXPONENT)  // normal numbers
            {
                // x = (-1)^s * 1.t * 2^{e-EXPONENT_BIAS}
                if (t > 0)
                {
                    int32_t last = BitHacks::GetTrailingBit(t);
                    int32_t diff = IEEE::NUM_TRAILING_BITS - last;
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent =
                        static_cast<int32_t>(e) - IEEE::EXPONENT_BIAS - diff;
                    mUInteger = ((t | IEEE::SUP_TRAILING) >> last);
                }
                else
                {
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = static_cast<int32_t>(e) - IEEE::EXPONENT_BIAS;
                    mUInteger = (typename IEEE::UIntType)1;
                }
            }
            else  // e == MAX_BIASED_EXPONENT, special numbers
            {
                if (t == 0)  // infinities
                {
                    // x = (-1)^s * infinity
#if defined(GTE_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN)
                    LogError("BSNumber does not have a representation for infinities.");
#else
                    // Return (-1)^s * 2^{1+EXPONENT_BIAS} for a graceful
                    // exit.
                    mSign = (s > 0 ? -1 : 1);
                    mBiasedExponent = 1 + IEEE::EXPONENT_BIAS;
                    mUInteger = (typename IEEE::UIntType)1;
#endif
                }
                else  // not-a-number (NaN)
                {
#if defined(GTE_THROW_ON_CONVERT_FROM_INFINITY_OR_NAN)
                    LogError("BSNumber does not have a representation for NaNs.");
#else
                    // Return 0 for a graceful exit.
                    mSign = 0;
                    mBiasedExponent = 0;
#endif
                }
            }
        }

        // Support for conversions from BSNumber to floating-point numbers.
        template <typename IEEE>
        typename IEEE::FloatType ConvertTo() const
        {
            typename IEEE::UIntType s = (mSign < 0 ? 1 : 0);
            typename IEEE::UIntType e, t;

            if (mSign != 0)
            {
                // The conversions use round-to-nearest-ties-to-even
                // semantics.
                int32_t exponent = GetExponent();
                if (exponent < IEEE::MIN_EXPONENT)
                {
                    if (exponent < IEEE::MIN_EXPONENT - 1
                        || mUInteger.GetNumBits() == 1)
                    {
                        // x = 1.0*2^{MIN_EXPONENT-1}, round to zero.
                        e = 0;
                        t = 0;
                    }
                    else
                    {
                        // Round to min subnormal.
                        e = 0;
                        t = 1;
                    }
                }
                else if (exponent < IEEE::MIN_SUB_EXPONENT)
                {
                    // The second input is in {0, ..., NUM_TRAILING_BITS-1}.
                    t = GetTrailing<IEEE>(0, IEEE::MIN_SUB_EXPONENT - exponent - 1);
                    if (t & IEEE::SUP_TRAILING)
                    {
                        // Leading NUM_SIGNIFICAND_BITS bits were all 1, so
                        // round to min normal.
                        e = 1;
                        t = 0;
                    }
                    else
                    {
                        e = 0;
                    }
                }
                else if (exponent <= IEEE::EXPONENT_BIAS)
                {
                    e = static_cast<uint32_t>(exponent + IEEE::EXPONENT_BIAS);
                    t = GetTrailing<IEEE>(1, 0);
                    if (t & (IEEE::SUP_TRAILING << 1))
                    {
                        // Carry-out occurred, so increase exponent by 1 and
                        // shift right to compensate.
                        ++e;
                        t >>= 1;
                    }
                    // Eliminate the leading 1 (implied for normals).
                    t &= ~IEEE::SUP_TRAILING;
                }
                else
                {
                    // Set to infinity.
                    e = IEEE::MAX_BIASED_EXPONENT;
                    t = 0;
                }
            }
            else
            {
                // The input is zero.
                e = 0;
                t = 0;
            }

            IEEE x(s, e, t);
            return x.number;
        }

        template <typename IEEE>
        typename IEEE::UIntType GetTrailing(int32_t normal, int32_t sigma) const
        {
            int32_t const numRequested = IEEE::NUM_SIGNIFICAND_BITS + normal;

            // We need numRequested bits to determine rounding direction.
            // These are stored in the high-order bits of 'prefix'.
            uint64_t prefix = mUInteger.GetPrefix(numRequested);

            // The first bit index after the implied binary point for
            // rounding.
            int32_t diff = numRequested - sigma;
            int32_t roundBitIndex = 64 - diff;

            // Determine value based on round-to-nearest-ties-to-even.
            uint64_t mask = (1ull << roundBitIndex);
            uint64_t round;
            if (prefix & mask)
            {
                // The first bit of the remainder is 1.
                if (mUInteger.GetNumBits() == diff)
                {
                    // The first bit of the remainder is the lowest-order bit
                    // of mBits[0]. Apply the ties-to-even rule.
                    if (prefix & (mask << 1))
                    {
                        // The last bit of the trailing significand is odd,
                        // so round up.
                        round = 1;
                    }
                    else
                    {
                        // The last bit of the trailing significand is even,
                        // so round down.
                        round = 0;
                    }
                }
                else
                {
                    // The first bit of the remainder is not the lowest-order
                    // bit of mBits[0].  The remainder as a fraction is larger
                    // than 1/2, so round up.
                    round = 1;
                }
            }
            else
            {
                // The first bit of the remainder is 0, so round down.
                round = 0;
            }

            // Get the unrounded trailing significand.
            uint64_t trailing = prefix >> (roundBitIndex + 1);

            // Apply the rounding.
            trailing += round;
            return static_cast<typename IEEE::UIntType>(trailing);
        }

#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
    public:
        // List this first so that it shows up first in the debugger watch
        // window.
        double mValue;
    private:
#endif

        // The number 0 is represented by: mSign = 0, mBiasedExponent = 0 and
        // mUInteger = 0. For nonzero numbers, mSign != 0 and mUInteger > 0.
        int32_t mSign;
        int32_t mBiasedExponent;
        UInteger mUInteger;

        // BSRational depends on the design of BSNumber, so allow it to have
        // full access to the implementation.
        friend class BSRational<UInteger>;
    };


    // Explicit conversion to a user-specified precision. The rounding
    // mode is one of the flags provided in <cfenv>. The modes are
    //   FE_TONEAREST:  round to nearest ties to even
    //   FE_DOWNWARD:   round towards negative infinity
    //   FE_TOWARDZERO: round towards zero
    //   FE_UPWARD:     round towards positive infinity
    template <typename UInteger>
    void Convert(BSNumber<UInteger> const& input, int32_t precision,
        int32_t roundingMode, BSNumber<UInteger>& output)
    {
        if (precision <= 0)
        {
            LogError("Precision must be positive.");
        }

        int64_t const maxSize = static_cast<int64_t>(UInteger::GetMaxSize());
        int64_t const excess = 32LL * maxSize - static_cast<int64_t>(precision);
        if (excess <= 0)
        {
            LogError("The maximum precision has been exceeded.");
        }

        if (input.GetSign() == 0)
        {
            output = BSNumber<UInteger>(0);
            return;
        }

        // Let p = precision and n+1 be the number of bits of the input.
        // Compute n+1-p. If it is nonpositive, then the requested precision
        // is already satisfied by the input.
        int32_t np1mp = input.GetUInteger().GetNumBits() - precision;
        if (np1mp <= 0)
        {
            output = input;
            return;
        }

        // At this point, the requested number of bits is smaller than the
        // number of bits in the input. Round the input to the smaller number
        // of bits using the specified rounding mode.
        UInteger& outW = output.GetUInteger();
        outW.SetNumBits(precision);
        outW.SetAllBitsToZero();
        int32_t const outSize = outW.GetSize();
        int32_t const precisionM1 = precision - 1;
        int32_t const outLeading = precisionM1 % 32;
        uint32_t outMask = (1 << outLeading);
        auto& outBits = outW.GetBits();
        int32_t outCurrent = outSize - 1;

        UInteger const& inW = input.GetUInteger();
        int32_t const inSize = inW.GetSize();
        int32_t const inLeading = (inW.GetNumBits() - 1) % 32;
        uint32_t inMask = (1 << inLeading);
        auto const& inBits = inW.GetBits();
        int32_t inCurrent = inSize - 1;

        int32_t lastBit = -1;
        for (int i = precisionM1; i >= 0; --i)
        {
            if (inBits[inCurrent] & inMask)
            {
                outBits[outCurrent] |= outMask;
                lastBit = 1;
            }
            else
            {
                lastBit = 0;
            }

            if (inMask == 0x00000001u)
            {
                --inCurrent;
                inMask = 0x80000000u;
            }
            else
            {
                inMask >>= 1;
            }

            if (outMask == 0x00000001u)
            {
                --outCurrent;
                outMask = 0x80000000u;
            }
            else
            {
                outMask >>= 1;
            }
        }

        // At this point as a sequence of bits, r = u_{n-p} ... u_0.
        int32_t sign = input.GetSign();
        int32_t outExponent = input.GetExponent();
        if (roundingMode == FE_TONEAREST)
        {
            // Determine whether u_{n-p} is positive.
            uint32_t positive = (inBits[inCurrent] & inMask) != 0u;
            if (positive && (np1mp > 1 || lastBit == 1))
            {
                // round up
                outExponent += outW.RoundUp();
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == FE_UPWARD)
        {
            // The remainder r must be positive because n-p >= 0 and u_0 = 1.
            if (sign > 0)
            {
                // round up
                outExponent += outW.RoundUp();
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == FE_DOWNWARD)
        {
            // The remainder r must be positive because n-p >= 0 and u_0 = 1.
            if (sign < 0)
            {
                // Round down. This is the round-up operation applied to
                // w, but the final sign is negative which amounts to
                // rounding down.
                outExponent += outW.RoundUp();
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode != FE_TOWARDZERO)
        {
            // Currently, no additional implementation-dependent modes
            // are supported for rounding.
            LogError("Implementation-dependent rounding mode not supported.");
        }
        // else roundingMode == FE_TOWARDZERO. Truncate the r bits, which
        // requires no additional work.

        // Shift the bits if necessary to obtain the invariant that BSNumber
        // objects have bit patterns that are odd integers.
        if (outW.GetNumBits() > 0 && (outW.GetBits()[0] & 1u) == 0)
        {
            UInteger temp = outW;
            outExponent += outW.ShiftRightToOdd(temp);
        }

        // Do not use SetExponent(outExponent) at this step. The number of
        // requested bits is 'precision' but outW.GetNumBits() will be
        // different when round-up occurs, and SetExponent accesses
        // outW.GetNumBits().
        output.SetSign(sign);
        output.SetBiasedExponent(outExponent - static_cast<int32_t>(precisionM1));
#if defined (GTE_VALIDATE_BSNUMBER)
        LogAssert(output.IsValid(), "Invalid BSNumber.");
#endif
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
        output.mValue = static_cast<double>(output);
#endif
    }
}

namespace std
{
    // TODO: Allow for implementations of the math functions in which a
    // specified precision is used when computing the result.

    template <typename UInteger>
    inline gte::BSNumber<UInteger> acos(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::acos((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> acosh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::acosh((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> asin(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::asin((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> asinh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::asinh((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> atan(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::atan((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> atanh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::atanh((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> atan2(gte::BSNumber<UInteger> const& y, gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::atan2((double)y, (double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> ceil(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::ceil((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> cos(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::cos((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> cosh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::cosh((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> exp(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::exp((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> exp2(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::exp2((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> fabs(gte::BSNumber<UInteger> const& x)
    {
        return (x.GetSign() >= 0 ? x : -x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> floor(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::floor((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> fmod(gte::BSNumber<UInteger> const& x, gte::BSNumber<UInteger> const& y)
    {
        return (gte::BSNumber<UInteger>)std::fmod((double)x, (double)y);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> frexp(gte::BSNumber<UInteger> const& x, int* exponent)
    {
        if (x.GetSign() != 0)
        {
            gte::BSNumber<UInteger> result = x;
            *exponent = result.GetExponent() + 1;
            result.SetExponent(-1);
            return result;
        }
        else
        {
            *exponent = 0;
            return gte::BSNumber<UInteger>(0);
        }
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> ldexp(gte::BSNumber<UInteger> const& x, int exponent)
    {
        gte::BSNumber<UInteger> result = x;
        result.SetBiasedExponent(result.GetBiasedExponent() + exponent);
        return result;
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> log(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::log((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> log2(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::log2((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> log10(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::log10((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> pow(gte::BSNumber<UInteger> const& x, gte::BSNumber<UInteger> const& y)
    {
        return (gte::BSNumber<UInteger>)std::pow((double)x, (double)y);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> sin(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::sin((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> sinh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::sinh((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> sqrt(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::sqrt((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> tan(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::tan((double)x);
    }

    template <typename UInteger>
    inline gte::BSNumber<UInteger> tanh(gte::BSNumber<UInteger> const& x)
    {
        return (gte::BSNumber<UInteger>)std::tanh((double)x);
    }

    // Type trait that says BSNumber is a signed type.
    template <typename UInteger>
    struct is_signed<gte::BSNumber<UInteger>> : true_type {};
}

namespace gte
{
    template <typename UInteger>
    inline BSNumber<UInteger> atandivpi(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)atandivpi((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> atan2divpi(BSNumber<UInteger> const& y, BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)atan2divpi((double)y, (double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> clamp(BSNumber<UInteger> const& x, BSNumber<UInteger> const& xmin, BSNumber<UInteger> const& xmax)
    {
        return (BSNumber<UInteger>)clamp((double)x, (double)xmin, (double)xmax);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> cospi(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)cospi((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> exp10(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)exp10((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> invsqrt(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)invsqrt((double)x);
    }

    template <typename UInteger>
    inline int isign(BSNumber<UInteger> const& x)
    {
        return isign((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> saturate(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)saturate((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sign(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)sign((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sinpi(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)sinpi((double)x);
    }

    template <typename UInteger>
    inline BSNumber<UInteger> sqr(BSNumber<UInteger> const& x)
    {
        return (BSNumber<UInteger>)sqr((double)x);
    }

    // See the comments in GteMath.h about trait is_arbitrary_precision.
    template <typename UInteger>
    struct is_arbitrary_precision_internal<BSNumber<UInteger>> : std::true_type {};
}
