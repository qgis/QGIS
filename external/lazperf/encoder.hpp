/*
===============================================================================

  FILE:  encoder.hpp

  CONTENTS:
    Encoder stuff

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

#ifndef __encoder_hpp__
#define __encoder_hpp__

#include <memory>

#include "coderbase.hpp"

namespace lazperf {
namespace encoders {

template<typename TOutStream>
struct arithmetic
{
public:
    arithmetic(TOutStream& out, bool valid = true) : outstream(out)
    {
        init(valid);
    }

    arithmetic(bool valid) : pOut(new TOutStream), outstream(*pOut)
    {
        init(valid);
    }

    arithmetic(const arithmetic<TOutStream>& src) :
        pOut(new TOutStream(*src.pOut)), outstream(*pOut)
    {
        init(src);
    }

    ~arithmetic()
    {
        delete [] outbuffer;
    }

    void makeValid()
    { valid = true; }

    void done()
    {
        uint32_t init_base = base;                 // done encoding: set final data bytes
        bool another_byte = true;

        if (length > 2 * AC__MinLength)
        {
            base  += AC__MinLength;                                     // base offset
            length = AC__MinLength >> 1;             // set new length for 1 more byte
        }
        else
        {
            base  += AC__MinLength >> 1;                                // base offset
            length = AC__MinLength >> 9;            // set new length for 2 more bytes
            another_byte = false;
        }

        if (init_base > base)                         // overflow = carry
            propagate_carry();
        renorm_enc_interval();                        // renormalization = output last bytes

        if (endbyte != endbuffer)
        {
            assert(outbyte < outbuffer + AC_BUFFER_SIZE);
            outstream.putBytes(outbuffer + AC_BUFFER_SIZE, AC_BUFFER_SIZE);
        }

        //ABELL - We control the buffer size. This calculation should never be
        //  negative and we shouldn't need 64 bits.
        int64_t buffer_size = outbyte - outbuffer;
        if (buffer_size)
            outstream.putBytes(outbuffer, (uint32_t)buffer_size);

        // write two or three zero bytes to be in sync with the decoder's byte reads
        outstream.putByte(0);
        outstream.putByte(0);
        if (another_byte)
            outstream.putByte(0);
    }

    /* Encode a bit with modelling                               */
    template<typename EntropyModel>
    void encodeBit(EntropyModel& m, uint32_t sym)
    {
        assert(sym <= 1);

        uint32_t x = m.bit_0_prob * (length >> BM__LengthShift);       // product l x p0
        // update interval
        if (sym == 0) {
            length = x;
            ++m.bit_0_count;
        }
        else {
            uint32_t init_base = base;
            base += x;
            length -= x;
            if (init_base > base)                                 // overflow = carry
                propagate_carry();
        }

        if (length < AC__MinLength)
            renorm_enc_interval();              // renormalization
        if (--m.bits_until_update == 0)
            m.update();                         // periodic model update
    }

    /* Encode a symbol with modelling                            */
    template <typename EntropyModel>
    void encodeSymbol(EntropyModel& m, uint32_t sym)
    {
        assert(sym <= m.last_symbol);

        uint32_t x, init_base = base;
        // compute products
        if (sym == m.last_symbol)
        {
            x = m.distribution[sym] * (length >> DM__LengthShift);
            base   += x;                                            // update interval
            length -= x;                                          // no product needed
        }
        else
        {
            x = m.distribution[sym] * (length >>= DM__LengthShift);
            base   += x;                                            // update interval
            length  = m.distribution[sym+1] * length - x;
        }

        if (init_base > base)
            propagate_carry();                 // overflow = carry
        if (length < AC__MinLength)
            renorm_enc_interval();        // renormalization

        ++m.symbol_count[sym];
        if (--m.symbols_until_update == 0)
            m.update();    // periodic model update
    }

    /* Encode a bit without modelling                            */
    void writeBit(uint32_t sym)
    {
        assert(sym < 2);

        uint32_t init_base = base;
        base += sym * (length >>= 1);                // new interval base and length

        if (init_base > base)
            propagate_carry();                 // overflow = carry
        if (length < AC__MinLength)
            renorm_enc_interval();        // renormalization
    }

    void writeBits(uint32_t bits, uint32_t sym)
    {
        assert(bits && (bits <= 32) && (sym < (1u<<bits)));

        if (bits > 19)
        {
            writeShort(sym);
            sym = sym >> 16;
            bits = bits - 16;
        }

        uint32_t init_base = base;
        base += sym * (length >>= bits);             // new interval base and length

        if (init_base > base)
            propagate_carry();                 // overflow = carry
        if (length < AC__MinLength)
            renorm_enc_interval();        // renormalization
    }

    void writeByte(uint8_t sym)
    {
        uint32_t init_base = base;
        base += (uint32_t)(sym) * (length >>= 8);           // new interval base and length

        if (init_base > base)
            propagate_carry();                 // overflow = carry
        if (length < AC__MinLength)
            renorm_enc_interval();        // renormalization
    }

    void writeShort(uint16_t sym)
    {
        uint32_t init_base = base;
        base += (uint32_t)(sym) * (length >>= 16);          // new interval base and length

        if (init_base > base)
            propagate_carry();                 // overflow = carry
        if (length < AC__MinLength)
            renorm_enc_interval();        // renormalization
    }

    void writeInt(uint32_t sym)
    {
        writeShort((uint16_t)(sym & 0xFFFF)); // lower 16 bits
        writeShort((uint16_t)(sym >> 16));    // UPPER 16 bits
    }

    void writeFloat(float sym) /* danger in float reinterpretation */
    {
        U32I32F32 u32i32f32;
        u32i32f32.f32 = sym;

        writeInt(u32i32f32.u32);
    }

    void writeInt64(uint64_t sym)
    {
        writeInt((uint32_t)(sym & 0xFFFFFFFF)); // lower 32 bits
        writeInt((uint32_t)(sym >> 32));        // UPPER 32 bits
    }

    void writeDouble(double sym) /* danger in float reinterpretation */
    {
        U64I64F64 u64i64f64;
        u64i64f64.f64 = sym;

        writeInt64(u64i64f64.u64);
    }

    TOutStream& getOutStream()
    {
        return outstream;
    }

    uint32_t num_encoded()
    {
        return valid ? outstream.numBytesPut() : 0;
    }

    const uint8_t *encoded_bytes()
    {
        return valid ? outstream.data() : nullptr;
    }


private:
    void init(bool v)
    {
        valid = v;
        outbuffer = new uint8_t[2*AC_BUFFER_SIZE];
        endbuffer = outbuffer + 2 * AC_BUFFER_SIZE;

        base   = 0;
        length = AC__MaxLength;
        outbyte = outbuffer;
        endbyte = endbuffer;
    }

    void init(const arithmetic<TOutStream>& src)
    {
        valid = src.valid;
        outbuffer = new uint8_t[2*AC_BUFFER_SIZE];
        endbuffer = outbuffer + 2 * AC_BUFFER_SIZE;

        base   = src.base;
        length = src.length;
        outbyte = outbuffer + (src.outbyte - src.outbuffer);
        endbyte = outbuffer + (src.endbyte - src.outbuffer);
    }

    void propagate_carry()
    {
        uint8_t *b;

        if (outbyte == outbuffer)
            b = endbuffer - 1;
        else
            b = outbyte - 1;
        while (*b == 0xFFU)
        {
            *b = 0;
            if (b == outbuffer)
                b = endbuffer - 1;
            else
                b--;
            assert(outbuffer <= b);
            assert(b < endbuffer);
            assert(outbyte < endbuffer);
        }
    ++*b;
}

void renorm_enc_interval()
{
    do
    {                                          // output and discard top byte
        assert(outbuffer <= outbyte);
        assert(outbyte < endbuffer);
        assert(outbyte < endbyte);

        *outbyte++ = (uint8_t)(base >> 24);
        if (outbyte == endbyte)
            manage_outbuffer();
        base <<= 8;
    } while ((length <<= 8) < AC__MinLength);        // length multiplied by 256
}

void manage_outbuffer()
{
    if (outbyte == endbuffer)
        outbyte = outbuffer;
    outstream.putBytes(outbyte, AC_BUFFER_SIZE);
    endbyte = outbyte + AC_BUFFER_SIZE;
    assert(endbyte > outbyte);
    assert(outbyte < endbuffer);
}

arithmetic<TOutStream>& operator = (const arithmetic<TOutStream>&) = delete;

private:
    uint8_t* outbuffer;
    uint8_t* endbuffer;
    uint8_t* outbyte;
    uint8_t* endbyte;
    uint32_t base, value, length;
    bool valid;

    std::unique_ptr<TOutStream> pOut;
    TOutStream& outstream;
};

} // namespace encoders
} // namespace lazperf

#endif // __encoder_hpp__
