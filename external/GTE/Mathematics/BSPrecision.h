// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.08.22

#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

// Support for determining the number of bits of precision required to compute
// an expression using BSNumber or BSRational.

namespace gte
{
    class BSPrecision
    {
    public:
        enum Type
        {
            IS_FLOAT,
            IS_DOUBLE,
            IS_INT32,
            IS_INT64,
            IS_UINT32,
            IS_UINT64
        };

        struct Parameters
        {
            Parameters()
                :
                minExponent(0),
                maxExponent(0),
                maxBits(0),
                maxWords(0)
            {
            }

            Parameters(int inMinExponent, int inMaxExponent, int inMaxBits)
                :
                minExponent(inMinExponent),
                maxExponent(inMaxExponent),
                maxBits(inMaxBits),
                maxWords(GetMaxWords())
            {
            }

            inline int GetMaxWords() const
            {
                return maxBits / 32 + ((maxBits % 32) > 0 ? 1 : 0);
            }

            int minExponent, maxExponent, maxBits, maxWords;
        };

        Parameters bsn, bsr;

        BSPrecision() = default;

        BSPrecision(Type type)
        {
            switch (type)
            {
            case IS_FLOAT:
                bsn = Parameters(-149, 127, 24);
                break;
            case IS_DOUBLE:
                bsn = Parameters(-1074, 1023, 53);
                break;
            case IS_INT32:
                bsn = Parameters(0, 30, 31);
                break;
            case IS_INT64:
                bsn = Parameters(0, 62, 63);
                break;
            case IS_UINT32:
                bsn = Parameters(0, 31, 32);
                break;
            case IS_UINT64:
                bsn = Parameters(0, 63, 64);
                break;
            }
            bsr = bsn;
        }

        BSPrecision(int minExponent, int maxExponent, int maxBits)
            :
            bsn(minExponent, maxExponent, maxBits),
            bsr(minExponent, maxExponent, maxBits)
        {
        }
    };

    inline BSPrecision operator+(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result;

        result.bsn.minExponent = std::min(bsp0.bsn.minExponent, bsp1.bsn.minExponent);
        if (bsp0.bsn.maxExponent >= bsp1.bsn.maxExponent)
        {
            result.bsn.maxExponent = bsp0.bsn.maxExponent;
            if (bsp0.bsn.maxExponent - bsp0.bsn.maxBits + 1 <= bsp1.bsn.maxExponent)
            {
                ++result.bsn.maxExponent;
            }

            result.bsn.maxBits = bsp0.bsn.maxExponent - bsp1.bsn.minExponent + 1;
            if (result.bsn.maxBits <= bsp0.bsn.maxBits + bsp1.bsn.maxBits - 1)
            {
                ++result.bsn.maxBits;
            }
        }
        else
        {
            result.bsn.maxExponent = bsp1.bsn.maxExponent;
            if (bsp1.bsn.maxExponent - bsp1.bsn.maxBits + 1 <= bsp0.bsn.maxExponent)
            {
                ++result.bsn.maxExponent;
            }

            result.bsn.maxBits = bsp1.bsn.maxExponent - bsp0.bsn.minExponent + 1;
            if (result.bsn.maxBits <= bsp0.bsn.maxBits + bsp1.bsn.maxBits - 1)
            {
                ++result.bsn.maxBits;
            }
        }
        result.bsn.maxWords = result.bsn.GetMaxWords();

        // Addition is n0/d0 + n1/d1 = (n0*d1 + n1*d0)/(d0*d1). The numerator
        // and denominator of a number are assumed to have the same
        // parameters, so for the addition, the numerator is used for the
        // parameter computations.

        // Compute the parameters for the multiplication.
        int mulMinExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        int mulMaxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        int mulMaxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;

        // Compute the parameters for the addition. The number n0*d1 and n1*d0
        // are in the same arbitrary-precision set.
        result.bsr.minExponent = mulMinExponent;
        result.bsr.maxExponent = mulMaxExponent + 1; // Always a carry-out.
        result.bsr.maxBits = mulMaxExponent - mulMinExponent + 1;
        if (result.bsr.maxBits <= 2 * mulMaxBits - 1)
        {
            ++result.bsr.maxBits;
        }
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator-(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return bsp0 + bsp1;
    }

    inline BSPrecision operator*(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result;

        result.bsn.minExponent = bsp0.bsn.minExponent + bsp1.bsn.minExponent;
        result.bsn.maxExponent = bsp0.bsn.maxExponent + bsp1.bsn.maxExponent + 1;
        result.bsn.maxBits = bsp0.bsn.maxBits + bsp1.bsn.maxBits;
        result.bsn.maxWords = result.bsn.GetMaxWords();

        // Multiplication is (n0/d0) * (n1/d1) = (n0 * n1) / (d0 * d1). The
        // parameters are the same as for numerator/denominator.
        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator/(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result;

        // BSNumber does not support division, so result.bsr has all members
        // set to zero.

        // Division is (n0/d0) / (n1/d1) = (n0 * d1) / (n1 * d0). The
        // parameters are the same as for multiplication.
        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    // Comparisons for BSNumber do not involve dynamic allocations, so
    // the results are the extremes of the inputs. Comparisons for BSRational
    // involve multiplications of numerators and denominators.
    inline BSPrecision operator==(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        BSPrecision result;

        result.bsn.minExponent = std::min(bsp0.bsn.minExponent, bsp1.bsn.minExponent);
        result.bsn.maxExponent = std::max(bsp0.bsn.maxExponent, bsp1.bsn.maxExponent);
        result.bsn.maxBits = std::max(bsp0.bsn.maxBits, bsp1.bsn.maxBits);
        result.bsn.maxWords = result.bsn.GetMaxWords();

        result.bsr.minExponent = bsp0.bsr.minExponent + bsp1.bsr.minExponent;
        result.bsr.maxExponent = bsp0.bsr.maxExponent + bsp1.bsr.maxExponent + 1;
        result.bsr.maxBits = bsp0.bsr.maxBits + bsp1.bsr.maxBits;
        result.bsr.maxWords = result.bsr.GetMaxWords();

        return result;
    }

    inline BSPrecision operator!=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator<(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator<=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator>(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }

    inline BSPrecision operator>=(BSPrecision const& bsp0, BSPrecision const& bsp1)
    {
        return operator==(bsp0, bsp1);
    }
}
