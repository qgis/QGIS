/*
===============================================================================

  FILE:  compressor.hpp
  
  CONTENTS:
    Integer compressor

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
  
  COPYRIGHT:
  
    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the Apache Public License 2.0 published by the Apache Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
===============================================================================
*/


#ifndef __compressor_hpp__
#define __compressor_hpp__

#include "model.hpp"

#include <cstdint>
#include <limits>
#include <vector>
#include <cassert>

namespace lazperf
{
namespace compressors
{

struct integer
{
public:
    integer(uint32_t bits = 16, uint32_t contexts = 1) : bits(bits), contexts(contexts)
    {
        if (bits && bits < 32)
        {
            corr_bits = bits;
            corr_range = 1u << bits;

            // the corrector must fall into this interval
            corr_min = -((int32_t)(corr_range/2));
            corr_max = corr_min + corr_range - 1;
        }
        else
        {
            corr_bits = 32;
            corr_range = 0;
            // the corrector must fall into this interval
            corr_min = (std::numeric_limits<int32_t>::min)();
            corr_max = (std::numeric_limits<int32_t>::max)();
        }

        k = 0;
    }

    ~integer()
    {
        mBits.clear();
        mCorrector.clear();
    }

    // ABELL - Maybe this is separate so that the compressor can be reused?
    // If so, why not called from the ctor?
    void init()
    {
        using models::arithmetic;
        using models::arithmetic_bit;

        // maybe create the models
        if (mBits.empty()) {
            for (uint32_t i = 0; i < contexts; i++)
                mBits.push_back(arithmetic(corr_bits+1));

            // mcorrector0 is already in init state
            for (uint32_t i = 1; i <= corr_bits; i++) {
                uint32_t v = i <= bits_high ? 1 << i : 1 << bits_high;
                mCorrector.push_back(arithmetic(v));
            }
        }
    }

    unsigned int getK() const
    { return k; }

    template<typename TEncoder>
    void compress(TEncoder& enc, int32_t pred, int32_t real, uint32_t context)
    {
        // the corrector will be within the interval [ - (corr_range - 1)  ...  + (corr_range - 1) ]
        int32_t corr = real - pred;
        // we fold the corrector into the interval [ corr_min  ...  corr_max ]
        if (corr < corr_min)
            corr += corr_range;
        else if (corr > corr_max)
            corr -= corr_range;
        writeCorrector(enc, corr, mBits[context]);
    }

    template<typename TEncoder, typename TEntropyModel>
    void writeCorrector(TEncoder& enc, int c, TEntropyModel& mBits)
    {
        // find the tighest interval [ - (2^k - 1)  ...  + (2^k) ] that contains c
        // do this by checking the absolute value of c (adjusted for the case that c is 2^k)

        uint32_t c1 = (c <= 0 ? -c : c - 1);
        // Find the number of bits containing information (32 - # leading 0 bits)
        // Tried an intrinsic for this with worse outcome.
        for (k = 0; c1; k++)
            c1 = c1 >> 1;

        // the number k is between 0 and corr_bits and describes the interval
        // the corrector falls into we can compress the exact location of c
        // within this interval using k bits

        enc.encodeSymbol(mBits, k);

        if (k) // then c is either smaller than 0 or bigger than 1
        {
            assert((c != 0) && (c != 1));
            // If k == 32, then the high bit is set, which only happens when the
            // value we want to encode is INT_MIN and all the information is in k,
            // which has already been encoded above.
            if (k == 32)
                return;
            // translate the corrector c into the k-bit interval [ 0 ... 2^k - 1 ]
            if (c < 0) // then c is in the interval [ - (2^k - 1)  ...  - (2^(k-1)) ]
            {
                // so we translate c into the interval [ 0 ...  + 2^(k-1) - 1 ]
                // by adding (2^k - 1)
                c += ((1<<k) - 1);
            }
            else // then c is in the interval [ 2^(k-1) + 1  ...  2^k ]
            {
                // so we translate c into the interval [ 2^(k-1) ...  + 2^k - 1 ]
                // by subtracting 1
                c -= 1;
            }

            if (k <= bits_high) // for small k we code the interval in one step
            {
                // compress c with the range coder
                enc.encodeSymbol(mCorrector[k-1], c);
            }
            else // for larger k we need to code the interval in two steps
            {
                // figure out how many lower bits there are
                int k1 = k - bits_high;
                // c1 represents the lowest k-bits_high+1 bits
                c1 = c & ((1 << k1) - 1);
                // c represents the highest bits_high bits
                c = c >> k1;
                // compress the higher bits using a context table
                enc.encodeSymbol(mCorrector[k-1], c);
                // store the lower k1 bits raw
                enc.writeBits(k1, c1);
            }
        }
        else // then c is 0 or 1
        {
            assert((c == 0) || (c == 1));
            enc.encodeBit(mCorrector0,c);
        }
    }

    uint32_t k;
    uint32_t bits;
    uint32_t contexts;
    const uint32_t bits_high {8};

    uint32_t corr_bits;
    uint32_t corr_range;
    int32_t corr_min;
    int32_t corr_max;

    std::vector<models::arithmetic> mBits;

    models::arithmetic_bit mCorrector0;
    std::vector<models::arithmetic> mCorrector;
};

} // namespace compressors
} // namespace lazperf

#endif // __compressor_hpp__
