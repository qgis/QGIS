// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_SUPPORT_LOOKUPTABLE_P_H_INCLUDED
#define BLEND2D_SUPPORT_LOOKUPTABLE_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! \name Compile-Time Lookup Table
//! \{

//! Struct that holds `N` items of `T` type - output of lookup table generators.
template<typename T, size_t N>
struct LookupTable {
  T data[N];

  BL_INLINE_CONSTEXPR size_t size() const noexcept { return N; }
  BL_INLINE constexpr const T& operator[](size_t i) const noexcept { return data[i]; }
};

namespace Internal {
namespace {

// C++11 implementation based on <https://stackoverflow.com/a/17426611/410767>.
template <size_t... Ints>
struct index_sequence {
  using type = index_sequence;
  using value_type = size_t;
  static BL_INLINE_CONSTEXPR size_t size() noexcept { return sizeof...(Ints); }
};

template <class S1, class S2>
struct _merge_and_renumber;

template <size_t... I1, size_t... I2>
struct _merge_and_renumber<index_sequence<I1...>, index_sequence<I2...>>
  : index_sequence<I1..., (sizeof...(I1)+I2)...> {};

template <size_t N>
struct make_index_sequence
  : _merge_and_renumber<typename make_index_sequence<N/2>::type,
                        typename make_index_sequence<N - N/2>::type> {};

template<> struct make_index_sequence<0> : index_sequence<> {};
template<> struct make_index_sequence<1> : index_sequence<0> {};

template<typename T, size_t N, class Gen, size_t... Indexes>
BL_INLINE_CONSTEXPR LookupTable<T, N> make_lookup_table_impl(index_sequence<Indexes...>) noexcept {
  return LookupTable<T, N> {{ T(Gen::value(Indexes))... }};
}

template<typename Generator>
struct BitTableGeneratorAdapter {
  static BL_INLINE_CONSTEXPR uint32_t gen_byte(size_t index) noexcept {
    return (uint32_t(Generator::value(index + 0u)) << 0u) |
           (uint32_t(Generator::value(index + 1u)) << 1u) |
           (uint32_t(Generator::value(index + 2u)) << 2u) |
           (uint32_t(Generator::value(index + 3u)) << 3u) |
           (uint32_t(Generator::value(index + 4u)) << 4u) |
           (uint32_t(Generator::value(index + 5u)) << 5u) |
           (uint32_t(Generator::value(index + 6u)) << 6u) |
           (uint32_t(Generator::value(index + 7u)) << 7u) ;
  }

  static BL_INLINE_CONSTEXPR uint32_t gen32(size_t index) noexcept {
    return (uint32_t(gen_byte(index +  0u)) <<  0u) |
           (uint32_t(gen_byte(index +  8u)) <<  8u) |
           (uint32_t(gen_byte(index + 16u)) << 16u) |
           (uint32_t(gen_byte(index + 24u)) << 24u) ;
  }

  static BL_INLINE_CONSTEXPR uint64_t gen64(size_t index) noexcept {
    return (uint64_t(gen_byte(index +  0u)) <<  0u) |
           (uint64_t(gen_byte(index +  8u)) <<  8u) |
           (uint64_t(gen_byte(index + 16u)) << 16u) |
           (uint64_t(gen_byte(index + 24u)) << 24u) |
           (uint64_t(gen_byte(index + 32u)) << 32u) |
           (uint64_t(gen_byte(index + 40u)) << 40u) |
           (uint64_t(gen_byte(index + 48u)) << 48u) |
           (uint64_t(gen_byte(index + 56u)) << 56u) ;
  }

  static BL_INLINE_CONSTEXPR BLBitWord value(size_t index) noexcept {
    return sizeof(BLBitWord) <= 4 ? BLBitWord(gen32(index * 32u) & ~BLBitWord(0))
                                  : BLBitWord(gen64(index * 64u) & ~BLBitWord(0));
  }
};

} // {anonymous}
} // {Internal}

//! Creates a lookup table of `LookupTable<T[N]>` by using the generator `Gen`.
template<typename T, size_t N, class Gen>
static BL_INLINE_CONSTEXPR LookupTable<T, N> make_lookup_table() noexcept {
  // Make sure the table is a constant expression - we never want to have it runtime initialized.
  constexpr LookupTable<T, N> table = Internal::make_lookup_table_impl<T, N, Gen>(Internal::make_index_sequence<N>{});

  return table;
}

template<size_t N>
struct BitLookupTable {
  static inline constexpr size_t kBitsPerBitWord = sizeof(BLBitWord) * 8;
  static inline constexpr size_t kBitCount = N;
  static inline constexpr size_t kWordCount = (N + kBitsPerBitWord - 1) / (kBitsPerBitWord);

  LookupTable<BLBitWord, kWordCount> data;

  BL_INLINE_CONSTEXPR size_t size() const noexcept { return N; }

  BL_INLINE bool operator[](size_t i) const noexcept {
    BL_ASSERT(i < N);
    return bool((data[i / kBitsPerBitWord] >> (i % kBitsPerBitWord)) & 0x1u);
  }
};

template<size_t N, class Generator>
static BL_INLINE_CONSTEXPR BitLookupTable<N> make_bit_table() noexcept {
  return BitLookupTable<N>{
    Internal::make_lookup_table_impl<BLBitWord, BitLookupTable<N>::kWordCount,
                                  Internal::BitTableGeneratorAdapter<Generator>>(Internal::make_index_sequence<BitLookupTable<N>::kWordCount>{})
  };
}

#define BL_CONSTEXPR_TABLE(Name, Generator, T, N) \
  static constexpr const LookupTable<T, N> Name##_constexpr = make_lookup_table<T, N, Generator>(); \
  const LookupTable<T, N> Name = Name##_constexpr

//! \}

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_SUPPORT_LOOKUPTABLE_P_H_INCLUDED
