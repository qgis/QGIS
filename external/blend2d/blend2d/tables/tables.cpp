// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/tables/tables_p.h>

namespace bl {

// bl::BitCountOfByteTable
// =======================

struct BitCountOfByteTableGen {
  static constexpr uint8_t value(size_t i) noexcept {
    return uint8_t( ((i >> 0) & 1) + ((i >> 1) & 1) + ((i >> 2) & 1) + ((i >> 3) & 1) +
                    ((i >> 4) & 1) + ((i >> 5) & 1) + ((i >> 6) & 1) + ((i >> 7) & 1) );
  }
};

static constexpr const LookupTable<uint8_t, 256> bit_count_byte_table_
  = make_lookup_table<uint8_t, 256, BitCountOfByteTableGen>();

const LookupTable<uint8_t, 256> bit_count_byte_table = bit_count_byte_table_;

// bl::ModuloTable
// ===============

#define INV()  {{ 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0   , 0   , 0   , 0   , 0   , 0   , 0    }}
#define DEF(N) {{ 1%N, 2%N, 3%N, 4%N, 5%N, 6%N, 7%N, 8%N, 9%N, 10%N, 11%N, 12%N, 13%N, 14%N, 15%N, 16%N }}

const ModuloTable modulo_table[18] = {
  INV(  ), DEF( 1), DEF( 2), DEF( 3),
  DEF( 4), DEF( 5), DEF( 6), DEF( 7),
  DEF( 8), DEF( 9), DEF(10), DEF(11),
  DEF(12), DEF(13), DEF(14), DEF(15),
  DEF(16), DEF(17)
};

#undef DEF
#undef INV

// bl::CommonTable
// ===============

// NOTE: We must go through `common_table` as it's the only way to convince MSVC to emit constexpr. If this step
// is missing it will emit initialization code for this const data, which is exactly what we don't want. Also, we
// cannot just add `constexpr` to the real `common_table` as MSVC would complain about different storage type.
static constexpr const CommonTable common_table_;
const CommonTable common_table = common_table_;

} // {bl}
