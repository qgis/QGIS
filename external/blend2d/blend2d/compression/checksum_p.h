// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_COMPRESSION_CHECKSUM_P_H_INCLUDED
#define BLEND2D_COMPRESSION_CHECKSUM_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>

//! \cond INTERNAL

namespace bl::Compression::Checksum {

struct FunctionTable {
  using Adler32Func = uint32_t (BL_CDECL*)(uint32_t checksum, const uint8_t* data, size_t size) noexcept;
  using Crc32Func = uint32_t (BL_CDECL*)(uint32_t checksum, const uint8_t* data, size_t size) noexcept;

  Adler32Func adler32;
  Crc32Func crc32;
};

BL_HIDDEN extern FunctionTable function_table;
BL_HIDDEN extern const uint32_t crc32_table[];

// Initial value used by CRC32 checksum.
static constexpr uint32_t kCrc32Initial = 0xFFFFFFFFu;

// Initial value used by ADLER32 checksum.
static constexpr uint32_t kAdler32Initial = 0x00000001u;

// The Adler32 divisor - highest prime that fits into 16 bits.
static constexpr uint32_t kAdler32Divisor = 65521u;

// kAdler32MaxBytesPerChunk is the most bytes that can be processed without the possibility of s2 overflowing when
// it is represented as an unsigned 32-bit integer. To get the correct worst-case value, we must assume that every
// byte in the input equals 0xFF and that s1 and s2 started with the highest possible values modulo the divisor.
static constexpr uint32_t kAdler32MaxBytesPerChunk = 5552u;

namespace {
static BL_INLINE uint32_t crc32_update_byte(uint32_t checksum, uint8_t b) noexcept { return (checksum >> 8) ^ crc32_table[(checksum ^ b) & 0xFFu]; }
static BL_INLINE uint32_t crc32_finalize(uint32_t checksum) noexcept { return ~checksum; }
} // {anonymous}

BL_HIDDEN uint32_t BL_CDECL crc32(const uint8_t* data, size_t size) noexcept;
BL_HIDDEN uint32_t BL_CDECL crc32_update_ref(uint32_t checksum, const uint8_t* data, size_t size) noexcept;

#if defined(BL_BUILD_OPT_SSE4_2)
BL_HIDDEN uint32_t BL_CDECL crc32_update_sse4_2(uint32_t checksum, const uint8_t* data, size_t size) noexcept;
#endif // BL_BUILD_OPT_SSE4_2

#if defined(BL_BUILD_OPT_ASIMD_CRYPTO)
BL_HIDDEN uint32_t BL_CDECL crc32_update_asimd(uint32_t checksum, const uint8_t* data, size_t size) noexcept;
#endif // BL_BUILD_OPT_ASIMD_CRYPTO

BL_HIDDEN uint32_t BL_CDECL adler32(const uint8_t* data, size_t size) noexcept;
BL_HIDDEN uint32_t BL_CDECL adler32_update_ref(uint32_t checksum, const uint8_t* data, size_t size) noexcept;

#if defined(BL_BUILD_OPT_SSE2)
BL_HIDDEN uint32_t BL_CDECL adler32_update_sse2(uint32_t checksum, const uint8_t* data, size_t size) noexcept;
#endif // BL_TARGET_OPT_SSE2

#if defined(BL_BUILD_OPT_ASIMD)
BL_HIDDEN uint32_t BL_CDECL adler32_update_asimd(uint32_t checksum, const uint8_t* data, size_t size) noexcept;
#endif // BL_TARGET_OPT_ASIMD

} // {bl::Compression::Checksum}

//! \endcond

#endif // BLEND2D_COMPRESSION_CHECKSUM_P_H_INCLUDED
