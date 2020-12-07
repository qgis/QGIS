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
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  CHANGE HISTORY:
  
===============================================================================
*/


#ifndef __compressor_hpp__
#define __compressor_hpp__

#include "model.hpp"

#include <vector>
#include <memory>
#include <cassert>

namespace laszip {
	namespace compressors {
		struct integer {
			integer(U32 bits = 16, U32 contexts = 1, U32 bits_high = 8, U32 range = 0):
				bits(bits), contexts(contexts), bits_high(bits_high), range(range) {
					
				if (range) { // the corrector's significant bits and range
					corr_bits = 0;
					corr_range = range;
					while (range)
					{
						range = range >> 1;
						corr_bits++;
					}
					if (corr_range == (1u << (corr_bits-1))) {
						corr_bits--;
					}

					// the corrector must fall into this interval
					corr_min = -((I32)(corr_range/2));
					corr_max = corr_min + corr_range - 1;
				}
				else if (bits && bits < 32) {
					corr_bits = bits;
					corr_range = 1u << bits;

					// the corrector must fall into this interval
					corr_min = -((I32)(corr_range/2));
					corr_max = corr_min + corr_range - 1;
				}
				else {
					corr_bits = 32;
					corr_range = 0;
					// the corrector must fall into this interval
					corr_min = I32_MIN;
					corr_max = I32_MAX;
				}

				k = 0;
			}

			~integer() {
				mBits.clear();
				mCorrector.clear();
			}

			void init() {
				using laszip::models::arithmetic;
				using laszip::models::arithmetic_bit;

				U32 i;

				// maybe create the models
				if (mBits.empty()) {
					for (i = 0; i < contexts; i++)
						mBits.push_back(arithmetic(corr_bits+1));

#ifndef COMPRESS_ONLY_K
					// mcorrector0 is already in init state
					for (i = 1; i <= corr_bits; i++) {
						U32 v = i <= bits_high ? 1 << i : 1 << bits_high;
						mCorrector.push_back(arithmetic(v));
					}
#endif
				}
			}

			unsigned int getK() const { return k; }

			template<
				typename TEncoder
			>
			void compress(TEncoder& enc, I32 pred, I32 real, U32 context) {
				// the corrector will be within the interval [ - (corr_range - 1)  ...  + (corr_range - 1) ]
				I32 corr = real - pred;
				// we fold the corrector into the interval [ corr_min  ...  corr_max ]
				if (corr < corr_min) corr += corr_range;
				else if (corr > corr_max) corr -= corr_range;

				writeCorrector(enc, corr, mBits[context]);
			}

			template<
				typename TEncoder,
				typename TEntropyModel
			>
			void writeCorrector(TEncoder& enc, int c, TEntropyModel& mBits) {
				U32 c1;

				// find the tighest interval [ - (2^k - 1)  ...  + (2^k) ] that contains c

				k = 0;

				// do this by checking the absolute value of c (adjusted for the case that c is 2^k)

				c1 = (c <= 0 ? -c : c-1);

				// this loop could be replaced with more efficient code

				while (c1)
				{
					c1 = c1 >> 1;
					k = k + 1;
				}

				// the number k is between 0 and corr_bits and describes the interval the corrector falls into
				// we can compress the exact location of c within this interval using k bits

				enc.encodeSymbol(mBits, k);

#ifdef COMPRESS_ONLY_K
				if (k) // then c is either smaller than 0 or bigger than 1
				{
					assert((c != 0) && (c != 1));
					if (k < 32)
					{
						// translate the corrector c into the k-bit interval [ 0 ... 2^k - 1 ]
						if (c < 0) // then c is in the interval [ - (2^k - 1)  ...  - (2^(k-1)) ]
						{
							// so we translate c into the interval [ 0 ...  + 2^(k-1) - 1 ] by adding (2^k - 1)
							enc.writeBits(k, c + ((1<<k) - 1));
						}
						else // then c is in the interval [ 2^(k-1) + 1  ...  2^k ]
						{
							// so we translate c into the interval [ 2^(k-1) ...  + 2^k - 1 ] by subtracting 1
							enc.writeBits(k, c - 1);
						}
					}
				}
				else // then c is 0 or 1
				{
					assert((c == 0) || (c == 1));
					enc.writeBit(c);
				}
#else // COMPRESS_ONLY_K
				if (k) // then c is either smaller than 0 or bigger than 1
				{
					assert((c != 0) && (c != 1));
					if (k < 32)
					{
						// translate the corrector c into the k-bit interval [ 0 ... 2^k - 1 ]
						if (c < 0) // then c is in the interval [ - (2^k - 1)  ...  - (2^(k-1)) ]
						{
							// so we translate c into the interval [ 0 ...  + 2^(k-1) - 1 ] by adding (2^k - 1)
							c += ((1<<k) - 1);
						}
						else // then c is in the interval [ 2^(k-1) + 1  ...  2^k ]
						{
							// so we translate c into the interval [ 2^(k-1) ...  + 2^k - 1 ] by subtracting 1
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
							int k1 = k-bits_high;
							// c1 represents the lowest k-bits_high+1 bits
							c1 = c & ((1<<k1) - 1);
							// c represents the highest bits_high bits
							c = c >> k1;
							// compress the higher bits using a context table
							enc.encodeSymbol(mCorrector[k-1], c);
							// store the lower k1 bits raw
							enc.writeBits(k1, c1);
						}
					}
				}
				else // then c is 0 or 1
				{
					assert((c == 0) || (c == 1));
					enc.encodeBit(mCorrector0,c);
				}
#endif // COMPRESS_ONLY_K
			}

			U32 k;

			U32 bits;

			U32 contexts;
			U32 bits_high;
			U32 range;

			U32 corr_bits;
			U32 corr_range;
			I32 corr_min;
			I32 corr_max;


			std::vector<laszip::models::arithmetic> mBits;

			laszip::models::arithmetic_bit mCorrector0;
			std::vector<laszip::models::arithmetic> mCorrector;
		};
	}
}

#endif // __compressor_hpp__
