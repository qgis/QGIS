/*
===============================================================================

  FILE:  decoder.hpp

  CONTENTS:
    Decoder stuff

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
//                       ****************************                        -
//                        ARITHMETIC CODING EXAMPLES                         -
//                       ****************************                        -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// Fast arithmetic coding implementation                                     -
// -> 32-bit variables, 32-bit product, periodic updates, table decoding     -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// Version 1.00  -  April 25, 2004                                           -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
//                                  WARNING                                  -
//                                 =========                                 -
//                                                                           -
// The only purpose of this program is to demonstrate the basic principles   -
// of arithmetic coding. The original version of this code can be found in   -
// Digital Signal Compression: Principles and Practice                       -
// (Cambridge University Press, 2011, ISBN: 9780511984655)                   -
//                                                                           -
// Copyright (c) 2019 by Amir Said (said@ieee.org) &                         -
//                       William A. Pearlman (pearlw@ecse.rpi.edu)           -
//                                                                           -
// Redistribution and use in source and binary forms, with or without        -
// modification, are permitted provided that the following conditions are    -
// met:                                                                      -
//                                                                           -
// 1. Redistributions of source code must retain the above copyright notice, -
// this list of conditions and the following disclaimer.                     -
//                                                                           -
// 2. Redistributions in binary form must reproduce the above copyright      -
// notice, this list of conditions and the following disclaimer in the       -
// documentation and/or other materials provided with the distribution.      -
//                                                                           -
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       -
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED -
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           -
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER -
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  -
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       -
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        -
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    -
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      -
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        -
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//                                                                           -
// A description of the arithmetic coding method used here is available in   -
//                                                                           -
// Lossless Compression Handbook, ed. K. Sayood                              -
// Chapter 5: Arithmetic Coding (A. Said), pp. 101-152, Academic Press, 2003 -
//                                                                           -
// A. Said, Introduction to Arithetic Coding Theory and Practice             -
// HP Labs report HPL-2004-76  -  http://www.hpl.hp.com/techreports/         -
//                                                                           -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


#ifndef __decoder_hpp__
#define __decoder_hpp__

#include <cassert>
#include <memory>

#include "coderbase.hpp"

namespace lazperf
{
namespace decoders
{

template<typename TInputStream>
struct arithmetic
{
public:
    arithmetic(TInputStream& in) : instream(in)
    {
        init();
    }

    arithmetic() : pIn(new TInputStream()), instream(*pIn)
    {
        init();
    }

    arithmetic(const arithmetic<TInputStream>& src) :
        pIn(new TInputStream(*src.pIn)), instream(*pIn)
    {
        value = src.value;
        length = src.length;
    }

    ~arithmetic() {
    }

    template <typename TSrcStream>
    void initStream(TSrcStream& src, uint32_t cnt)
    {
        if (cnt)
        {
            instream.copy(src, cnt);
            readInitBytes();
        }
    }

    void readInitBytes()
    {
        value =
            (instream.getByte() << 24) |
            (instream.getByte() << 16) |
            (instream.getByte() << 8) |
            instream.getByte();
    }

    template<typename TEntropyModel>
    uint32_t decodeBit(TEntropyModel& m)
    {
        uint32_t x = m.bit_0_prob * (length >> BM__LengthShift);       // product l x p0
        uint32_t sym = (value >= x);                                   // decision

        // update & shift interval
        if (sym == 0)
        {
            length  = x;
            ++m.bit_0_count;
        }
        else
        {
            value  -= x;                                  // shifted interval base = 0
            length -= x;
        }

        if (length < AC__MinLength)
            renorm_dec_interval();        // renormalization
        if (--m.bits_until_update == 0)
            m.update();       // periodic model update
        return sym;                                         // return data bit value
    }

    template<typename TEntropyModel>
    uint32_t decodeSymbol(TEntropyModel& m)
    {
        uint32_t n, sym, x, y = length;

        if (m.decoder_table)               // use table look-up for faster decoding
        {
            unsigned dv = value / (length >>= DM__LengthShift);
            unsigned t = dv >> m.table_shift;

            sym = m.decoder_table[t];      // initial decision based on table look-up
            n = m.decoder_table[t+1] + 1;

            while (n > sym + 1)                        // finish with bisection search
            {
                uint32_t k = (sym + n) >> 1;
                if (m.distribution[k] > dv)
                    n = k;
                else
                    sym = k;
            }

            // compute products
            x = m.distribution[sym] * length;
            if (sym != m.last_symbol)
                y = m.distribution[sym+1] * length;
        }
        else                                    // decode using only multiplications
        {
            x = sym = 0;
            length >>= DM__LengthShift;
            uint32_t k = (n = m.symbols) >> 1;
            // decode via bisection search
            do {
                uint32_t z = length * m.distribution[k];
                if (z > value)
                {
                    n = k;
                    y = z;                                             // value is smaller
                }
                else
                {
                    sym = k;
                    x = z;                                     // value is larger or equal
                }
            } while ((k = (sym + n) >> 1) != sym);
        }

        value -= x;                                               // update interval
        length = y - x;

        // renormalization
        if (length < AC__MinLength)
            renorm_dec_interval();

        ++m.symbol_count[sym];
        // periodic model update
        if (--m.symbols_until_update == 0)
            m.update();
        return sym;
    }

    uint32_t readBit()
    {
        uint32_t sym = value / (length >>= 1);            // decode symbol, change length
        value -= length * sym;                                    // update interval

        if (length < AC__MinLength)
            renorm_dec_interval();        // renormalization

        return sym;
    }

    uint32_t readBits(uint32_t bits)
    {
        assert(bits && (bits <= 32));

        if (bits > 19)
        {
            uint32_t tmp = readShort();
            bits = bits - 16;
            uint32_t tmp1 = readBits(bits) << 16;
            return (tmp1 | tmp);
        }

        uint32_t sym = value / (length >>= bits);// decode symbol, change length
        value -= length * sym;                                    // update interval

        if (length < AC__MinLength)
            renorm_dec_interval();        // renormalization
        return sym;
    }

    uint8_t readByte()
    {
        uint32_t sym = value / (length >>= 8);            // decode symbol, change length
        value -= length * sym;                                    // update interval

        if (length < AC__MinLength)
            renorm_dec_interval();        // renormalization

        assert(sym < (1 << 8));
        return (uint8_t)sym;
    }

    uint16_t readShort()
    {
        uint32_t sym = value / (length >>= 16);           // decode symbol, change length
        value -= length * sym;                                    // update interval

        if (length < AC__MinLength)
            renorm_dec_interval();        // renormalization

        assert(sym < (1<<16));
        return (uint16_t)sym;
    }

    uint32_t readInt()
    {
        uint32_t lowerInt = readShort();
        uint32_t upperInt = readShort();
        return (upperInt<<16)|lowerInt;
    }

    /* danger in float reinterpretation */
    float readFloat()
    {
        U32I32F32 u32i32f32;
        u32i32f32.u32 = readInt();
        return u32i32f32.f32;
    }

    uint64_t readInt64()
    {
        uint64_t lowerInt = readInt();
        uint64_t upperInt = readInt();
        return (upperInt<<32)|lowerInt;
    }

    /* danger in float reinterpretation */
    double readDouble()
    {
        U64I64F64 u64i64f64;
        u64i64f64.u64 = readInt64();
        return u64i64f64.f64;
    }

    TInputStream& getInStream()
    {
        return instream;
    }

    arithmetic<TInputStream>& operator = (const arithmetic<TInputStream>&) = delete;

private:
    void init()
    {
        value = 0;
        length = AC__MaxLength;
    }

    void renorm_dec_interval()
    {
        do {                                          // read least-significant byte
            value = (value << 8) | instream.getByte();
        } while ((length <<= 8) < AC__MinLength);        // length multiplied by 256
    }

    uint32_t value;
    uint32_t length;

    std::unique_ptr<TInputStream> pIn;
    TInputStream& instream;
};

} // namespace decoders
} // namespace lazperf

#endif // __decoder_hpp__
