// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if defined(BL_TARGET_OPT_ASIMD_CRYPTO)

#include <blend2d/compression/checksum_p.h>
#include <blend2d/compression/checksumcrc32simdimpl_p.h>

namespace bl::Compression::Checksum {

// bl::Compression - CheckSum - CRC32 (ASIMD)
// ==========================================

uint32_t crc32_update_asimd(uint32_t checksum, const uint8_t* data, size_t size) noexcept {
  return crc32_update_clmul128(checksum, data, size);
}

} // {bl::Compression::Checksum}

#endif // BL_TARGET_OPT_ASIMD_CRYPTO
