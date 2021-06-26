// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.07.21

#pragma once

#include <Mathematics/BSNumber.h>

// See the comments in BSNumber.h about the UInteger requirements. The
// denominator of a BSRational is chosen to be positive, which allows some
// simplification of comparisons. Also see the comments about exposing the
// GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE conditional define.

namespace gte
{
    template <typename UInteger>
    class BSRational
    {
    public:
        // Construction. The default constructor generates the zero
        // BSRational. The constructors that take only numerators set the
        // denominators to one.
        BSRational()
            :
            mNumerator(0),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(BSRational const& r)
        {
            *this = r;
        }

        BSRational(float numerator)
            :
            mNumerator(numerator),
            mDenominator(1.0f)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(double numerator)
            :
            mNumerator(numerator),
            mDenominator(1.0)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(int32_t numerator)
            :
            mNumerator(numerator),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(uint32_t numerator)
            :
            mNumerator(numerator),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(int64_t numerator)
            :
            mNumerator(numerator),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(uint64_t numerator)
            :
            mNumerator(numerator),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(BSNumber<UInteger> const& numerator)
            :
            mNumerator(numerator),
            mDenominator(1)
        {
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(float numerator, float denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(double numerator, double denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(int32_t numerator, int32_t denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(uint32_t numerator, uint32_t denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(int64_t numerator, int64_t denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(uint64_t numerator, uint64_t denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(BSNumber<UInteger> const& numerator, BSNumber<UInteger> const& denominator)
            :
            mNumerator(numerator),
            mDenominator(denominator)
        {
            LogAssert(mDenominator.mSign != 0, "Division by zero.");
            if (mDenominator.mSign < 0)
            {
                mNumerator.mSign = -mNumerator.mSign;
                mDenominator.mSign = 1;
            }

            // Set the exponent of the denominator to zero, but you can do so
            // only by modifying the biased exponent. Adjust the numerator
            // accordingly. This prevents large growth of the exponents in
            // both numerator and denominator simultaneously.
            mNumerator.mBiasedExponent -= mDenominator.GetExponent();
            mDenominator.mBiasedExponent = -(mDenominator.GetUInteger().GetNumBits() - 1);

#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(std::string const& number)
        {
            LogAssert(number.size() > 0, "A number must be specified.");

            // Get the leading '+' or '-' if it exists.
            std::string fpNumber;
            int sign;
            if (number[0] == '+')
            {
                fpNumber = number.substr(1);
                sign = +1;
                LogAssert(fpNumber.size() > 1, "Invalid number format.");
            }
            else if (number[0] == '-')
            {
                fpNumber = number.substr(1);
                sign = -1;
                LogAssert(fpNumber.size() > 1, "Invalid number format.");
            }
            else
            {
                fpNumber = number;
                sign = +1;
            }

            size_t decimal = fpNumber.find('.');
            if (decimal != std::string::npos)
            {
                if (decimal > 0)
                {
                    if (decimal < fpNumber.size())
                    {
                        // The number is "x.y".
                        BSNumber<UInteger> intPart = BSNumber<UInteger>::ConvertToInteger(fpNumber.substr(0, decimal));
                        BSRational frcPart = ConvertToFraction(fpNumber.substr(decimal + 1));
                        mNumerator = intPart * frcPart.mDenominator + frcPart.mNumerator;
                        mDenominator = frcPart.mDenominator;
                    }
                    else
                    {
                        // The number is "x.".
                        mNumerator = BSNumber<UInteger>::ConvertToInteger(fpNumber.substr(0,fpNumber.size()-1));
                        mDenominator = 1;
                    }
                }
                else
                {
                    // The number is ".y".
                    BSRational frcPart = ConvertToFraction(fpNumber.substr(1));
                    mNumerator = frcPart.mNumerator;
                    mDenominator = frcPart.mDenominator;
                }
            }
            else
            {
                // The number is "x".
                mNumerator = BSNumber<UInteger>::ConvertToInteger(fpNumber);
                mDenominator = 1;
            }
            mNumerator.SetSign(sign);

#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
        }

        BSRational(const char* number)
            :
            BSRational(std::string(number))
        {
        }

        // Implicit conversions. These always use the default rounding mode,
        // round-to-nearest-ties-to-even.
        operator float() const
        {
            float output;
            Convert(*this, FE_TONEAREST, output);
            return output;
        }

        operator double() const
        {
            double output;
            Convert(*this, FE_TONEAREST, output);
            return output;
        }

        // Assignment.
        BSRational& operator=(BSRational const& r)
        {
            mNumerator = r.mNumerator;
            mDenominator = r.mDenominator;
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
            return *this;
        }

        // Support for move semantics.
        BSRational(BSRational&& r) noexcept
        {
            *this = std::move(r);
        }

        BSRational& operator=(BSRational&& r) noexcept
        {
            mNumerator = std::move(r.mNumerator);
            mDenominator = std::move(r.mDenominator);
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = (double)*this;
#endif
            return *this;
        }

        // Member access.
        inline void SetSign(int sign)
        {
            mNumerator.SetSign(sign);
            mDenominator.SetSign(1);
#if defined(GTL_BINARY_SCIENTIFIC_SHOW_DOUBLE)
            mValue = sign * std::fabs(mValue);
#endif
        }

        inline int GetSign() const
        {
            return mNumerator.GetSign() * mDenominator.GetSign();
        }

        inline BSNumber<UInteger> const& GetNumerator() const
        {
            return mNumerator;
        }

        inline BSNumber<UInteger>& GetNumerator()
        {
            return mNumerator;
        }

        inline BSNumber<UInteger> const& GetDenominator() const
        {
            return mDenominator;
        }

        inline BSNumber<UInteger>& GetDenominator()
        {
            return mDenominator;
        }

        // Comparisons.
        bool operator==(BSRational const& r) const
        {
            // Do inexpensive sign tests first for optimum performance.
            if (mNumerator.mSign != r.mNumerator.mSign)
            {
                return false;
            }
            if (mNumerator.mSign == 0)
            {
                // The numbers are both zero.
                return true;
            }

            return mNumerator * r.mDenominator == mDenominator * r.mNumerator;
        }

        bool operator!=(BSRational const& r) const
        {
            return !operator==(r);
        }

        bool operator< (BSRational const& r) const
        {
            // Do inexpensive sign tests first for optimum performance.
            if (mNumerator.mSign > 0)
            {
                if (r.mNumerator.mSign <= 0)
                {
                    return false;
                }
            }
            else if (mNumerator.mSign == 0)
            {
                return r.mNumerator.mSign > 0;
            }
            else if (mNumerator.mSign < 0)
            {
                if (r.mNumerator.mSign >= 0)
                {
                    return true;
                }
            }

            return mNumerator * r.mDenominator < mDenominator * r.mNumerator;
        }

        bool operator<=(BSRational const& r) const
        {
            return !r.operator<(*this);
        }

        bool operator> (BSRational const& r) const
        {
            return r.operator<(*this);
        }

        bool operator>=(BSRational const& r) const
        {
            return !operator<(r);
        }

        // Unary operations.
        BSRational operator+() const
        {
            return *this;
        }

        BSRational operator-() const
        {
            return BSRational(-mNumerator, mDenominator);
        }

        // Arithmetic.
        BSRational operator+(BSRational const& r) const
        {
            BSNumber<UInteger> product0 = mNumerator * r.mDenominator;
            BSNumber<UInteger> product1 = mDenominator * r.mNumerator;
            BSNumber<UInteger> numerator = product0 + product1;

            // Complex expressions can lead to 0/denom, where denom is not 1.
            if (numerator.mSign != 0)
            {
                BSNumber<UInteger> denominator = mDenominator * r.mDenominator;
                return BSRational(numerator, denominator);
            }
            else
            {
                return BSRational(0);
            }
        }

        BSRational operator-(BSRational const& r) const
        {
            BSNumber<UInteger> product0 = mNumerator * r.mDenominator;
            BSNumber<UInteger> product1 = mDenominator * r.mNumerator;
            BSNumber<UInteger> numerator = product0 - product1;

            // Complex expressions can lead to 0/denom, where denom is not 1.
            if (numerator.mSign != 0)
            {
                BSNumber<UInteger> denominator = mDenominator * r.mDenominator;
                return BSRational(numerator, denominator);
            }
            else
            {
                return BSRational(0);
            }
        }

        BSRational operator*(BSRational const& r) const
        {
            BSNumber<UInteger> numerator = mNumerator * r.mNumerator;

            // Complex expressions can lead to 0/denom, where denom is not 1.
            if (numerator.mSign != 0)
            {
                BSNumber<UInteger> denominator = mDenominator * r.mDenominator;
                return BSRational(numerator, denominator);
            }
            else
            {
                return BSRational(0);
            }
        }

        BSRational operator/(BSRational const& r) const
        {
            LogAssert(r.mNumerator.mSign != 0, "Division by zero in BSRational::operator/.");

            BSNumber<UInteger> numerator = mNumerator * r.mDenominator;

            // Complex expressions can lead to 0/denom, where denom is not 1.
            if (numerator.mSign != 0)
            {
                BSNumber<UInteger> denominator = mDenominator * r.mNumerator;
                if (denominator.mSign < 0)
                {
                    numerator.mSign = -numerator.mSign;
                    denominator.mSign = 1;
                }
                return BSRational(numerator, denominator);
            }
            else
            {
                return BSRational(0);
            }
        }

        BSRational& operator+=(BSRational const& r)
        {
            *this = operator+(r);
            return *this;
        }

        BSRational& operator-=(BSRational const& r)
        {
            *this = operator-(r);
            return *this;
        }

        BSRational& operator*=(BSRational const& r)
        {
            *this = operator*(r);
            return *this;
        }

        BSRational& operator/=(BSRational const& r)
        {
            *this = operator/(r);
            return *this;
        }

        // Disk input/output. The fstream objects should be created using
        // std::ios::binary. The return value is 'true' iff the operation
        // was successful.
        bool Write(std::ostream& output) const
        {
            return mNumerator.Write(output) && mDenominator.Write(output);
        }

        bool Read(std::istream& input)
        {
            return mNumerator.Read(input) && mDenominator.Read(input);
        }

    private:
        // Helper for converting a string to a BSRational, where the string
        // is the fractional part "y" of the string "x.y".
        static BSRational ConvertToFraction(std::string const& number)
        {
            LogAssert(number.find_first_not_of("0123456789") == std::string::npos, "Invalid number format.");
            BSRational y(0), ten(10), pow10(10);
            for (size_t i = 0; i < number.size(); ++i)
            {
                int digit = static_cast<int>(number[i]) - static_cast<int>('0');
                if (digit > 0)
                {
                     y += BSRational(digit) / pow10;
                }
                pow10 *= ten;
            }
            return y;
        }

#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
    public:
        // List this first so that it shows up first in the debugger watch
        // window.
        double mValue;
    private:
#endif

        BSNumber<UInteger> mNumerator, mDenominator;
    };


    // Explicit conversion to a user-specified precision. The rounding
    // mode is one of the flags provided in <cfenv>. The modes are
    //   FE_TONEAREST:  round to nearest ties to even
    //   FE_DOWNWARD:   round towards negative infinity
    //   FE_TOWARDZERO: round towards zero
    //   FE_UPWARD:     round towards positive infinity
    template <typename UInteger>
    void Convert(BSRational<UInteger> const& input, int32_t precision,
        int32_t roundingMode, BSNumber<UInteger>& output)
    {
        if (precision <= 0)
        {
            LogError("Precision must be positive.");
        }

        size_t const maxNumBlocks = UInteger::GetMaxSize();
        size_t const numPrecBlocks = static_cast<size_t>((precision + 31) / 32);
        if (numPrecBlocks >= maxNumBlocks)
        {
            LogError("The maximum precision has been exceeded.");
        }

        if (input.GetSign() == 0)
        {
            output = BSNumber<UInteger>(0);
            return;
        }

        BSNumber<UInteger> n = input.GetNumerator();
        BSNumber<UInteger> d = input.GetDenominator();

        // The ratio is abstractly of the form n/d = (1.u*2^p)/(1.v*2^q).
        // Convert to the form
        //   (1.u/1.v)*2^{p-q}, if 1.u >= 1.v
        //   2*(1.u/1.v)*2^{p-q-1} if 1.u < 1.v
        // which are in the interval [1,2).
        int32_t sign = n.GetSign() * d.GetSign();
        n.SetSign(1);
        d.SetSign(1);
        int32_t pmq = n.GetExponent() - d.GetExponent();
        n.SetExponent(0);
        d.SetExponent(0);
        if (n < d)
        {
            n.SetExponent(n.GetExponent() + 1);
            --pmq;
        }

        // Let p = precision. At this time, n/d = 1.c in [1,2). Define the
        // sequence of bits w = 1c = w_{p-1} w_{p-2} ... w_0 r, where
        // w_{p-1} = 1. The bits r after w_0 are used for rounding based on
        // the user-specified rounding mode.

        // Compute p bits for w, the leading bit guaranteed to be 1 and
        // occurring at index (1 << (precision-1)).
        BSNumber<UInteger> one(1), two(2);
        UInteger& w = output.GetUInteger();
        w.SetNumBits(precision);
        w.SetAllBitsToZero();
        int32_t const size = w.GetSize();
        int32_t const precisionM1 = precision - 1;
        int32_t const leading = precisionM1 % 32;
        uint32_t mask = (1 << leading);
        auto& bits = w.GetBits();
        int32_t current = size - 1;
        int32_t lastBit = -1;
        for (int i = precisionM1; i >= 0; --i)
        {
            if (n < d)
            {
                n = two * n;
                lastBit = 0;
            }
            else
            {
                n = two * (n - d);
                bits[current] |= mask;
                lastBit = 1;
                if (n.GetSign() == 0)
                {
                    // The input rational has a finite number of bits in its
                    // representation, so it is exactly a BSNumber.
                    if (i > 0)
                    {
                        // The number n is zero for the remainder of the loop,
                        // so the last bit of the p-bit precision pattern is
                        // a zero. There is no need to continue looping.
                        lastBit = 0;
                    }
                    break;
                }
            }

            if (mask == 0x00000001u)
            {
                --current;
                mask = 0x80000000u;
            }
            else
            {
                mask >>= 1;
            }
        }

        // At this point as a sequence of bits, r = n/d = r0 r1 ...
        if (roundingMode == FE_TONEAREST)
        {
            n = n - d;
            if (n.GetSign() > 0 || (n.GetSign() == 0 && lastBit == 1))
            {
                // round up
                pmq += w.RoundUp();
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == FE_UPWARD)
        {
            if (n.GetSign() > 0 && sign > 0)
            {
                // round up
                pmq += w.RoundUp();
            }
            // else round down, equivalent to truncating the r bits
        }
        else if (roundingMode == FE_DOWNWARD)
        {
            if (n.GetSign() > 0 && sign < 0)
            {
                // Round down. This is the round-up operation applied to
                // w, but the final sign is negative which amounts to
                // rounding down.
                pmq += w.RoundUp();
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
        if ((w.GetBits()[0] & 1) == 0)
        {
            UInteger temp = w;
            auto shift = w.ShiftRightToOdd(temp);
            pmq += shift;
        }

        // Do not use SetExponent(pmq) at this step. The number of
        // requested bits is 'precision' but w.GetNumBits() will be
        // different when round-up occurs, and SetExponent accesses
        // w.GetNumBits().
        output.SetSign(sign);
        output.SetBiasedExponent(pmq - precisionM1);
#if defined(GTE_BINARY_SCIENTIFIC_SHOW_DOUBLE)
        output.mValue = (double)output;
#endif
    }

    // This conversion is used to avoid having to expose BSNumber in the
    // APConversion class as well as other places where BSRational<UInteger>
    // is passed via a template parameter named Rational.
    template <typename UInteger>
    void Convert(BSRational<UInteger> const& input, int32_t precision,
        int32_t roundingMode, BSRational<UInteger>& output)
    {
        BSNumber<UInteger> numerator;
        Convert(input, precision, roundingMode, numerator);
        output = BSRational<UInteger>(numerator);
    }

    // Convert to 'float' or 'double' using the specified rounding mode.
    template <typename UInteger, typename FPType>
    void Convert(BSRational<UInteger> const& input, int32_t roundingMode, FPType& output)
    {
        static_assert(std::is_floating_point<FPType>::value, "Invalid floating-point type.");
        BSNumber<UInteger> number;
        Convert(input, std::numeric_limits<FPType>::digits, roundingMode, number);
        output = static_cast<FPType>(number);
    }
}

namespace std
{
    // TODO: Allow for implementations of the math functions in which a
    // specified precision is used when computing the result.

    template <typename UInteger>
    inline gte::BSRational<UInteger> acos(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::acos((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> acosh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::acosh((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> asin(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::asin((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> asinh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::asinh((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> atan(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::atan((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> atanh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::atanh((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> atan2(gte::BSRational<UInteger> const& y, gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::atan2((double)y, (double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> ceil(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::ceil((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> cos(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::cos((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> cosh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::cosh((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> exp(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::exp((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> exp2(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::exp2((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> fabs(gte::BSRational<UInteger> const& x)
    {
        return (x.GetSign() >= 0 ? x : -x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> floor(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::floor((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> fmod(gte::BSRational<UInteger> const& x, gte::BSRational<UInteger> const& y)
    {
        return (gte::BSRational<UInteger>)std::fmod((double)x, (double)y);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> frexp(gte::BSRational<UInteger> const& x, int* exponent)
    {
        gte::BSRational<UInteger> result = x;
        auto& numer = result.GetNumerator();
        auto& denom = result.GetDenominator();
        int32_t e = numer.GetExponent() - denom.GetExponent();
        numer.SetExponent(0);
        denom.SetExponent(0);
        int32_t saveSign = numer.GetSign();
        numer.SetSign(1);
        if (numer >= denom)
        {
            ++e;
            numer.SetExponent(-1);
        }
        numer.SetSign(saveSign);
        *exponent = e;
        return result;
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> ldexp(gte::BSRational<UInteger> const& x, int exponent)
    {
        gte::BSRational<UInteger> result = x;
        int biasedExponent = result.GetNumerator().GetBiasedExponent();
        biasedExponent += exponent;
        result.GetNumerator().SetBiasedExponent(biasedExponent);
        return result;
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> log(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::log((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> log2(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::log2((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> log10(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::log10((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> pow(gte::BSRational<UInteger> const& x, gte::BSRational<UInteger> const& y)
    {
        return (gte::BSRational<UInteger>)std::pow((double)x, (double)y);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> sin(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::sin((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> sinh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::sinh((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> sqrt(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::sqrt((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> tan(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::tan((double)x);
    }

    template <typename UInteger>
    inline gte::BSRational<UInteger> tanh(gte::BSRational<UInteger> const& x)
    {
        return (gte::BSRational<UInteger>)std::tanh((double)x);
    }

    // Type trait that says BSRational is a signed type.
    template <typename UInteger>
    struct is_signed<gte::BSRational<UInteger>> : true_type {};
}

namespace gte
{
    template <typename UInteger>
    inline BSRational<UInteger> atandivpi(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)atandivpi((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> atan2divpi(BSRational<UInteger> const& y, BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)atan2divpi((double)y, (double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> clamp(BSRational<UInteger> const& x, BSRational<UInteger> const& xmin, BSRational<UInteger> const& xmax)
    {
        return (BSRational<UInteger>)clamp((double)x, (double)xmin, (double)xmax);
    }

    template <typename UInteger>
    inline BSRational<UInteger> cospi(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)cospi((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> exp10(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)exp10((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> invsqrt(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)invsqrt((double)x);
    }

    template <typename UInteger>
    inline int isign(BSRational<UInteger> const& x)
    {
        return isign((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> saturate(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)saturate((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> sign(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)sign((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> sinpi(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)sinpi((double)x);
    }

    template <typename UInteger>
    inline BSRational<UInteger> sqr(BSRational<UInteger> const& x)
    {
        return (BSRational<UInteger>)sqr((double)x);
    }

    // See the comments in Math.h about traits is_arbitrary_precision
    // and has_division_operator.
    template <typename UInteger>
    struct is_arbitrary_precision_internal<BSRational<UInteger>> : std::true_type {};

    template <typename UInteger>
    struct has_division_operator_internal<BSRational<UInteger>> : std::true_type {};
}
