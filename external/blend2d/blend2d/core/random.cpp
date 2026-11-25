// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/random_p.h>
#include <blend2d/simd/simd_p.h>

// bl::Random - API - Reset
// ========================

BL_API_IMPL BLResult bl_random_reset(BLRandom* self, uint64_t seed) noexcept {
  bl::RandomInternal::reset_seed(self, seed);
  return BL_SUCCESS;
}

// bl::Random - API - Next
// =======================

BL_API_IMPL double bl_random_next_double(BLRandom* self) noexcept {
  return bl::RandomInternal::next_double(self);
}

BL_API_IMPL uint32_t bl_random_next_uint32(BLRandom* self) noexcept {
  return bl::RandomInternal::next_uint32(self);
}

BL_API_IMPL uint64_t bl_random_next_uint64(BLRandom* self) noexcept {
  return bl::RandomInternal::next_uint64(self);
}
