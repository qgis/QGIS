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

#include "common/types.hpp"

namespace laszip {
	namespace encoders {
		template<
			typename TOutStream
		>
		struct arithmetic {
			arithmetic(TOutStream& out) :
				outstream(out) {
				outbuffer = new U8[2*AC_BUFFER_SIZE];
				endbuffer = outbuffer + 2 * AC_BUFFER_SIZE;

				base   = 0;
				length = AC__MaxLength;
				outbyte = outbuffer;
				endbyte = endbuffer;
			}

			~arithmetic() {
				delete [] outbuffer;
			}

			void done() {
				U32 init_base = base;                 // done encoding: set final data bytes
				BOOL another_byte = TRUE;

				if (length > 2 * AC__MinLength) {
					base  += AC__MinLength;                                     // base offset
					length = AC__MinLength >> 1;             // set new length for 1 more byte
				}
				else {
					base  += AC__MinLength >> 1;                                // base offset
					length = AC__MinLength >> 9;            // set new length for 2 more bytes
					another_byte = FALSE;
				}

				if (init_base > base) propagate_carry();                 // overflow = carry
				renorm_enc_interval();                // renormalization = output last bytes

				if (endbyte != endbuffer)
				{
					assert(outbyte < outbuffer + AC_BUFFER_SIZE);
					outstream.putBytes(outbuffer + AC_BUFFER_SIZE, AC_BUFFER_SIZE);
				}

				I64 buffer_size = outbyte - outbuffer;
				if (buffer_size) outstream.putBytes(outbuffer, (U32)buffer_size);

				// write two or three zero bytes to be in sync with the decoder's byte reads
				outstream.putByte(0);
				outstream.putByte(0);

				if (another_byte) outstream.putByte(0);
			}

			/* Encode a bit with modelling                               */
			template<typename EntropyModel>
			void encodeBit(EntropyModel& m, U32 sym) {
				assert(sym <= 1);

				U32 x = m.bit_0_prob * (length >> BM__LengthShift);       // product l x p0
				// update interval
				if (sym == 0) {
					length = x;
					++m.bit_0_count;
				}
				else {
					U32 init_base = base;
					base += x;
					length -= x;
					if (init_base > base) propagate_carry();               // overflow = carry
				}

				if (length < AC__MinLength) renorm_enc_interval();        // renormalization
				if (--m.bits_until_update == 0) m.update();       // periodic model update
			}

			/* Encode a symbol with modelling                            */
			template <typename EntropyModel>
			void encodeSymbol(EntropyModel& m, U32 sym) {
				assert(sym <= m.last_symbol);

				U32 x, init_base = base;
				// compute products
				if (sym == m.last_symbol) {
					x = m.distribution[sym] * (length >> DM__LengthShift);
					base   += x;                                            // update interval
					length -= x;                                          // no product needed
				}
				else {
					x = m.distribution[sym] * (length >>= DM__LengthShift);
					base   += x;                                            // update interval
					length  = m.distribution[sym+1] * length - x;
				}

				if (init_base > base) propagate_carry();                 // overflow = carry
				if (length < AC__MinLength) renorm_enc_interval();        // renormalization

				++m.symbol_count[sym];
				if (--m.symbols_until_update == 0) m.update();    // periodic model update
			}

			/* Encode a bit without modelling                            */
			void writeBit(U32 sym) {
				assert(sym < 2);

				U32 init_base = base;
				base += sym * (length >>= 1);                // new interval base and length

				if (init_base > base) propagate_carry();                 // overflow = carry
				if (length < AC__MinLength) renorm_enc_interval();        // renormalization
			}

			void writeBits(U32 bits, U32 sym) {
				assert(bits && (bits <= 32) && (sym < (1u<<bits)));

				if (bits > 19)
				{
					writeShort(sym&U16_MAX);
					sym = sym >> 16;
					bits = bits - 16;
				}

				U32 init_base = base;
				base += sym * (length >>= bits);             // new interval base and length

				if (init_base > base) propagate_carry();                 // overflow = carry
				if (length < AC__MinLength) renorm_enc_interval();        // renormalization
			}

			void writeByte(U8 sym) {
				U32 init_base = base;
				base += (U32)(sym) * (length >>= 8);           // new interval base and length

				if (init_base > base) propagate_carry();                 // overflow = carry
				if (length < AC__MinLength) renorm_enc_interval();        // renormalization
			}

			void writeShort(U16 sym) {
				U32 init_base = base;
				base += (U32)(sym) * (length >>= 16);          // new interval base and length

				if (init_base > base) propagate_carry();                 // overflow = carry
				if (length < AC__MinLength) renorm_enc_interval();        // renormalization
			}

			void writeInt(U32 sym) {
				writeShort((U16)(sym & 0xFFFF)); // lower 16 bits
				writeShort((U16)(sym >> 16));    // UPPER 16 bits
			}

			void writeFloat(F32 sym) /* danger in float reinterpretation */ {
				U32I32F32 u32i32f32;
				u32i32f32.f32 = sym;

				writeInt(u32i32f32.u32);
			}

			void writeInt64(U64 sym) {
				writeInt((U32)(sym & 0xFFFFFFFF)); // lower 32 bits
				writeInt((U32)(sym >> 32));        // UPPER 32 bits
			}

			void writeDouble(F64 sym) /* danger in float reinterpretation */ {
				U64I64F64 u64i64f64;
				u64i64f64.f64 = sym;

				writeInt64(u64i64f64.u64);
			}

			TOutStream& getOutStream() {
				return outstream;
			}


		private:
			void propagate_carry() {
				U8 * b;
				if (outbyte == outbuffer)
					b = endbuffer - 1;
				else
					b = outbyte - 1;
				while (*b== 0xFFU)
				{
					*b = 0;
					if (b == outbuffer)
						b= endbuffer - 1;
					else
						b--;
					assert(outbuffer <= b);
					assert(b < endbuffer);
					assert(outbyte < endbuffer);
				}
				++*b;
			}

			void renorm_enc_interval() {
				do {                                          // output and discard top byte
					assert(outbuffer <= outbyte);
					assert(outbyte < endbuffer);
					assert(outbyte < endbyte);
					*outbyte++ = (U8)(base >> 24);
					if (outbyte == endbyte) manage_outbuffer();
					base <<= 8;
				} while ((length <<= 8) < AC__MinLength);        // length multiplied by 256
			}

			void manage_outbuffer() {
				if (outbyte == endbuffer) outbyte = outbuffer;
				outstream.putBytes(outbyte, AC_BUFFER_SIZE);
				endbyte = outbyte + AC_BUFFER_SIZE;
				assert(endbyte > outbyte);
				assert(outbyte < endbuffer);
			}

			arithmetic<TOutStream>(const arithmetic<TOutStream>&) = delete;
			arithmetic<TOutStream>& operator = (const arithmetic<TOutStream>&) = delete;

			private:
				U8* outbuffer;
				U8* endbuffer;
				U8* outbyte;
				U8* endbyte;
				U32 base, value, length;

				TOutStream& outstream;
		};
	}
}

#endif // __encoder_hpp__
