/*
===============================================================================

  FILE:  decompressor.hpp
  
  CONTENTS:
    Integer decompressor

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


#ifndef __decompressor_hpp__
#define __decompressor_hpp__

#include "model.hpp"

#include <vector>
#include <memory>
#include <cassert>

namespace lazperf
{
namespace decompressors
{
		struct integer {
			integer(uint32_t bits = 16, uint32_t contexts = 1, uint32_t bits_high = 8, uint32_t range = 0):
				bits(bits), contexts(contexts), bits_high(bits_high), range(range) {
				if (range) { // the corrector's significant bits and range
					corr_bits = 0;
					corr_range = range;
					while (range)
					{
						range = range >> 1;
						corr_bits++;
					}
					if (corr_range == (1u << (corr_bits-1)))
					{
						corr_bits--;
					}
					// the corrector must fall into this interval
					corr_min = -((int32_t)(corr_range/2));
					corr_max = corr_min + corr_range - 1;
				}
				else if (bits && bits < 32) {
					corr_bits = bits;
					corr_range = 1u << bits;
					// the corrector must fall into this interval
					corr_min = -((int32_t)(corr_range/2));
					corr_max = corr_min + corr_range - 1;
				}
				else {
					corr_bits = 32;
					corr_range = 0;
					// the corrector must fall into this interval
					corr_min = (std::numeric_limits<int32_t>::min)();
					corr_max = (std::numeric_limits<int32_t>::max)();
				}

				k = 0;
			}

			void init() {
				using models::arithmetic;
				using models::arithmetic_bit;

				uint32_t i;

				// maybe create the models
				if (mBits.empty()) {
					for (i = 0; i < contexts; i++)
						mBits.push_back(arithmetic(corr_bits+1));

#ifndef COMPRESS_ONLY_K
					// mcorrector0 is already initialized
					for (i = 1; i <= corr_bits; i++) {
						uint32_t v = i <= bits_high ? 1 << i : 1 << bits_high;
						mCorrector.push_back(arithmetic(v));
					}
#endif
				}
			}

			template<
				typename TDecoder
			>
			int32_t decompress(TDecoder& dec, int32_t pred, uint32_t context) {
				int32_t real = pred + readCorrector(dec, mBits[context]);
				if (real < 0) real += corr_range;
				else if ((uint32_t)(real) >= corr_range) real -= corr_range;

				return real;
			}

			inline unsigned int getK() const { return k; }

			template<
				typename TDecoder,
				typename TEntroyModel
			>
			int32_t readCorrector(TDecoder& dec, TEntroyModel& mBits) {
				int32_t c;

				// decode within which interval the corrector is falling

				k = dec.decodeSymbol(mBits);

				// decode the exact location of the corrector within the interval

#ifdef COMPRESS_ONLY_K
				if (k) // then c is either smaller than 0 or bigger than 1
				{
					if (k < 32)
					{
						c = dec.readBits(k);

						if (c >= (1<<(k-1))) // if c is in the interval [ 2^(k-1)  ...  + 2^k - 1 ]
						{
							// so we translate c back into the interval [ 2^(k-1) + 1  ...  2^k ] by adding 1 
							c += 1;
						}
						else // otherwise c is in the interval [ 0 ...  + 2^(k-1) - 1 ]
						{
							// so we translate c back into the interval [ - (2^k - 1)  ...  - (2^(k-1)) ] by subtracting (2^k - 1)
							c -= ((1<<k) - 1);
						}
					}
					else
					{
						c = corr_min;
					}
				}
				else // then c is either 0 or 1
				{
					c = dec.readBit();
				}
#else // COMPRESS_ONLY_K
				if (k) // then c is either smaller than 0 or bigger than 1
				{
					if (k < 32)
					{
						if (k <= bits_high) // for small k we can do this in one step
						{
							// decompress c with the range coder
							c = dec.decodeSymbol(mCorrector[k-1]);
						}
						else
						{
							// for larger k we need to do this in two steps
							int k1 = k-bits_high;
							// decompress higher bits with table
							c = dec.decodeSymbol(mCorrector[k-1]);
							// read lower bits raw
							int c1 = dec.readBits(k1);
							// put the corrector back together
							c = (c << k1) | c1;
						}
						// translate c back into its correct interval
						if (c >= (1<<(k-1))) // if c is in the interval [ 2^(k-1)  ...  + 2^k - 1 ]
						{
							// so we translate c back into the interval [ 2^(k-1) + 1  ...  2^k ] by adding 1 
							c += 1;
						}
						else // otherwise c is in the interval [ 0 ...  + 2^(k-1) - 1 ]
						{
							// so we translate c back into the interval [ - (2^k - 1)  ...  - (2^(k-1)) ] by subtracting (2^k - 1)
							c -= ((1<<k) - 1);
						}
					}
					else
					{
						c = corr_min;
					}
				}
				else // then c is either 0 or 1
				{
					c = dec.decodeBit(mCorrector0);
				}
#endif // COMPRESS_ONLY_K

				return c;
			}

			uint32_t k;

			uint32_t bits;

			uint32_t contexts;
			uint32_t bits_high;
			uint32_t range;

			uint32_t corr_bits;
			uint32_t corr_range;
			int32_t corr_min;
			int32_t corr_max;


			std::vector<models::arithmetic> mBits;

			models::arithmetic_bit mCorrector0;
			std::vector<models::arithmetic> mCorrector;
		};
} // namespace decompressors
} // namespace lazperf

#endif // __decompressor_hpp__
