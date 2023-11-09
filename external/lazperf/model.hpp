/*
===============================================================================

  FILE:  model.hpp

  CONTENTS:


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

#ifndef __model_hpp__
#define __model_hpp__

#include "coderbase.hpp"
#include "utils.hpp"

#include <stdexcept>

namespace lazperf
{
namespace models
{
		struct arithmetic {
			arithmetic(uint32_t syms, bool com = false, uint32_t *initTable = nullptr) :
				symbols(syms), compress(com),
				distribution(nullptr), symbol_count(nullptr), decoder_table(nullptr) {
				if ( (symbols < 2) || (symbols > (1 << 11)) ) {
					throw std::runtime_error("Invalid number of symbols");
				}

				last_symbol = symbols - 1;
				if ((!compress) && (symbols > 16)) {
					uint32_t table_bits = 3;
					while (symbols > (1U << (table_bits + 2))) ++table_bits;
					table_size  = 1 << table_bits;
					table_shift = DM__LengthShift - table_bits;
					decoder_table = reinterpret_cast<uint32_t*>(utils::aligned_malloc(sizeof(uint32_t) * (table_size + 2)));
				}
				else { // small alphabet: no table needed
					decoder_table = 0;
					table_size = table_shift = 0;
				}

				distribution = reinterpret_cast<uint32_t*>(utils::aligned_malloc(symbols * sizeof(uint32_t)));
				symbol_count = reinterpret_cast<uint32_t*>(utils::aligned_malloc(symbols * sizeof(uint32_t)));

				total_count = 0;
				update_cycle = symbols;

				if (initTable)
					for (uint32_t k = 0; k < symbols; k++) symbol_count[k] = initTable[k];
				else
					for (uint32_t k = 0; k < symbols; k++) symbol_count[k] = 1;

				update();
				symbols_until_update = update_cycle = (symbols + 6) >> 1;
			}

			~arithmetic() {
				if (distribution) utils::aligned_free(distribution);
				if (symbol_count) utils::aligned_free(symbol_count);
				if (decoder_table) utils::aligned_free(decoder_table);
			}

			arithmetic(const arithmetic& other)
				: symbols(other.symbols), compress(other.compress),
				  total_count(other.total_count), update_cycle(other.update_cycle),
				  symbols_until_update(other.symbols_until_update), last_symbol(other.last_symbol),
				  table_size(other.table_size), table_shift(other.table_shift)
            {
                size_t size(symbols * sizeof(uint32_t));
                distribution = reinterpret_cast<uint32_t*>(utils::aligned_malloc(size));
                std::copy(other.distribution, other.distribution + symbols, distribution);

                symbol_count = reinterpret_cast<uint32_t*>(utils::aligned_malloc(size));
                std::copy(other.symbol_count, other.symbol_count + symbols, symbol_count);

                if (table_size)
                {
                    size = (table_size + 2) * sizeof(uint32_t);
                    decoder_table = reinterpret_cast<uint32_t*>(utils::aligned_malloc(size));
                    std::copy(other.decoder_table, other.decoder_table + (table_size + 2), decoder_table);
                }
                else
                    decoder_table = nullptr;
			}

			arithmetic(arithmetic&& other) : symbols(other.symbols), compress(other.compress),
                distribution(other.distribution), symbol_count(other.symbol_count),
				decoder_table(other.decoder_table),
				total_count(other.total_count), update_cycle(other.update_cycle),
				symbols_until_update(other.symbols_until_update), last_symbol(other.last_symbol),
				table_size(other.table_size), table_shift(other.table_shift)
            {
                other.distribution = other.decoder_table = other.symbol_count = NULL;
                other.symbol_count = 0;
			}

			arithmetic& operator = (arithmetic&& other) {
				if (this != &other) {
					if (distribution) utils::aligned_free(distribution);
					if (symbol_count) utils::aligned_free(symbol_count);
					if (decoder_table) utils::aligned_free(decoder_table);

					symbols = other.symbols;
					compress = other.compress;

					distribution = other.distribution;
					symbol_count = other.symbol_count;
					decoder_table = other.decoder_table;

					total_count = other.total_count;
					update_cycle = other.update_cycle;
					symbols_until_update = other.symbols_until_update;
					last_symbol = other.last_symbol;
					table_size = other.table_size;
					table_shift = other.table_shift;

					other.distribution = other.symbol_count = other.decoder_table = nullptr;
					other.total_count = other.update_cycle = other.symbols_until_update =
						other.last_symbol = other.table_size = other.table_shift = 0;
				}

				return *this;
			}

			inline void update() {
				// halve counts when a threshold is reached
				if ((total_count += update_cycle) > DM__MaxCount) {
					total_count = 0;
					for (uint32_t n = 0; n < symbols; n++)
					{
						total_count += (symbol_count[n] = (symbol_count[n] + 1) >> 1);
					}
				}

				// compute cumulative distribution, decoder table
				uint32_t k, sum = 0, s = 0;
				uint32_t scale = 0x80000000U / total_count;

				if (compress || (table_size == 0)) {
					for (k = 0; k < symbols; k++)
					{
						distribution[k] = (scale * sum) >> (31 - DM__LengthShift);
						sum += symbol_count[k];
					}
				}
				else {
					for (k = 0; k < symbols; k++)
					{
						distribution[k] = (scale * sum) >> (31 - DM__LengthShift);
						sum += symbol_count[k];
						uint32_t w = distribution[k] >> table_shift;
						while (s < w) decoder_table[++s] = k - 1;
					}
					decoder_table[0] = 0;
					while (s <= table_size) decoder_table[++s] = symbols - 1;
				}

				// set frequency of model updates
				update_cycle = (5 * update_cycle) >> 2;
				uint32_t max_cycle = (symbols + 6) << 3;

				if (update_cycle > max_cycle) update_cycle = max_cycle;
				symbols_until_update = update_cycle;
			}

			uint32_t symbols;
			bool compress;

			uint32_t * distribution, * symbol_count, * decoder_table;

			uint32_t total_count, update_cycle, symbols_until_update;
			uint32_t last_symbol, table_size, table_shift;
		};

		struct arithmetic_bit {
			arithmetic_bit() {
				// initialization to equiprobable model
				bit_0_count = 1;
				bit_count   = 2;
				bit_0_prob  = 1U << (BM__LengthShift - 1);
				// start with frequent updates
				update_cycle = bits_until_update = 4;
			}

			arithmetic_bit(arithmetic_bit&& other):
				update_cycle(other.update_cycle), bits_until_update(other.bits_until_update),
				bit_0_prob(other.bit_0_prob), bit_0_count(other.bit_0_count), bit_count(other.bit_count) {
			}

			arithmetic_bit& operator = (arithmetic_bit&& other) {
				if (this != &other) {
					update_cycle = other.update_cycle;
					bits_until_update = other.bits_until_update;
					bit_0_prob = other.bit_0_prob;
					bit_0_count = other.bit_0_count;
					bit_count = other.bit_count;

					other.update_cycle = other.bits_until_update =
						other.bit_0_prob = other.bit_0_count = other.bit_count = 0;
				}

				return *this;
			}

			void update() {
				// halve counts when a threshold is reached
				if ((bit_count += update_cycle) > BM__MaxCount)
				{
					bit_count = (bit_count + 1) >> 1;
					bit_0_count = (bit_0_count + 1) >> 1;
					if (bit_0_count == bit_count) ++bit_count;
				}

				// compute scaled bit 0 probability
				uint32_t scale = 0x80000000U / bit_count;
				bit_0_prob = (bit_0_count * scale) >> (31 - BM__LengthShift);

				// set frequency of model updates
				update_cycle = (5 * update_cycle) >> 2;
				if (update_cycle > 64) update_cycle = 64;
				bits_until_update = update_cycle;
			}

			uint32_t update_cycle, bits_until_update;
			uint32_t bit_0_prob, bit_0_count, bit_count;
		};
} // namespace models
} // namespace lazperf

#endif // __model_hpp__
