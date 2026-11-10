// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if defined(BL_TARGET_OPT_SSE2)

#include <blend2d/core/geometry.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/ptrops_p.h>

namespace bl {
namespace TransformInternal {

// bl::Transform - MapPointDArray (SSE2)
// =====================================

static BLResult BL_CDECL map_pointd_array_identity_sse2(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  bl_unused(self);
  if (dst == src)
    return BL_SUCCESS;

  size_t i = size;
  if (PtrOps::both_aligned(dst, src, 16)) {
    while (i >= 4) {
      Vec2xF64 v0 = loada<Vec2xF64>(src + 0);
      Vec2xF64 v1 = loada<Vec2xF64>(src + 1);
      storea(dst + 0, v0);
      storea(dst + 1, v1);

      Vec2xF64 v2 = loada<Vec2xF64>(src + 2);
      Vec2xF64 v3 = loada<Vec2xF64>(src + 3);
      storea(dst + 2, v2);
      storea(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storea(dst, loada<Vec2xF64>(src));

      i--;
      dst++;
      src++;
    }
  }
  else {
    while (i) {
      storeu(dst, loadu<Vec2xF64>(src));

      i--;
      dst++;
      src++;
    }
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL map_pointd_array_translate_sse2(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec2xF64 m21_m20 = loadu<Vec2xF64>(&self->m20);

  if (PtrOps::both_aligned(dst, src, 16)) {
    while (i >= 4) {
      Vec2xF64 v0 = loada<Vec2xF64>(src + 0) + m21_m20;
      Vec2xF64 v1 = loada<Vec2xF64>(src + 1) + m21_m20;
      Vec2xF64 v2 = loada<Vec2xF64>(src + 2) + m21_m20;
      Vec2xF64 v3 = loada<Vec2xF64>(src + 3) + m21_m20;

      storea(dst + 0, v0);
      storea(dst + 1, v1);
      storea(dst + 2, v2);
      storea(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storea(dst, loada<Vec2xF64>(src) + m21_m20);

      i--;
      dst++;
      src++;
    }
  }
  else {
    while (i >= 4) {
      Vec2xF64 v0 = loadu<Vec2xF64>(src + 0) + m21_m20;
      Vec2xF64 v1 = loadu<Vec2xF64>(src + 1) + m21_m20;
      Vec2xF64 v2 = loadu<Vec2xF64>(src + 2) + m21_m20;
      Vec2xF64 v3 = loadu<Vec2xF64>(src + 3) + m21_m20;

      storeu(dst + 0, v0);
      storeu(dst + 1, v1);
      storeu(dst + 2, v2);
      storeu(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storeu(dst, loadu<Vec2xF64>(src) + m21_m20);

      i--;
      dst++;
      src++;
    }
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL map_pointd_array_scale_sse2(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec2xF64 m11_m00 = make128_f64(self->m11, self->m00);
  Vec2xF64 m21_m20 = loadu<Vec2xF64>(&self->m20);

  if (PtrOps::both_aligned(dst, src, 16)) {
    while (i >= 4) {
      Vec2xF64 v0 = loada<Vec2xF64>(src + 0) * m11_m00 + m21_m20;
      Vec2xF64 v1 = loada<Vec2xF64>(src + 1) * m11_m00 + m21_m20;
      Vec2xF64 v2 = loada<Vec2xF64>(src + 2) * m11_m00 + m21_m20;
      Vec2xF64 v3 = loada<Vec2xF64>(src + 3) * m11_m00 + m21_m20;

      storea(dst + 0, v0);
      storea(dst + 1, v1);
      storea(dst + 2, v2);
      storea(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storea(dst, (loada<Vec2xF64>(src) * m11_m00) + m21_m20);

      i--;
      dst++;
      src++;
    }
  }
  else {
    while (i >= 4) {
      Vec2xF64 v0 = loadu<Vec2xF64>(src + 0) * m11_m00 + m21_m20;
      Vec2xF64 v1 = loadu<Vec2xF64>(src + 1) * m11_m00 + m21_m20;
      Vec2xF64 v2 = loadu<Vec2xF64>(src + 2) * m11_m00 + m21_m20;
      Vec2xF64 v3 = loadu<Vec2xF64>(src + 3) * m11_m00 + m21_m20;

      storeu(dst + 0, v0);
      storeu(dst + 1, v1);
      storeu(dst + 2, v2);
      storeu(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storeu(dst, (loadu<Vec2xF64>(src) * m11_m00) + m21_m20);

      i--;
      dst++;
      src++;
    }
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL map_pointd_array_swap_sse2(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  Vec2xF64 m01_m10 = make128_f64(self->m01, self->m10);
  Vec2xF64 m21_m20 = loadu<Vec2xF64>(&self->m20);

  size_t i = size;
  if (bl::PtrOps::both_aligned(dst, src, 16)) {
    while (i >= 4) {
      Vec2xF64 v0 = swap_f64(loada<Vec2xF64>(src + 0)) * m01_m10 + m21_m20;
      Vec2xF64 v1 = swap_f64(loada<Vec2xF64>(src + 1)) * m01_m10 + m21_m20;
      Vec2xF64 v2 = swap_f64(loada<Vec2xF64>(src + 2)) * m01_m10 + m21_m20;
      Vec2xF64 v3 = swap_f64(loada<Vec2xF64>(src + 3)) * m01_m10 + m21_m20;

      storea(dst + 0, v0);
      storea(dst + 1, v1);
      storea(dst + 2, v2);
      storea(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storea(dst, (swap_f64(loada<Vec2xF64>(src)) * m01_m10) + m21_m20);

      i--;
      dst++;
      src++;
    }
  }
  else {
    while (i >= 4) {
      Vec2xF64 v0 = swap_f64(loadu<Vec2xF64>(src + 0)) * m01_m10 + m21_m20;
      Vec2xF64 v1 = swap_f64(loadu<Vec2xF64>(src + 1)) * m01_m10 + m21_m20;
      Vec2xF64 v2 = swap_f64(loadu<Vec2xF64>(src + 2)) * m01_m10 + m21_m20;
      Vec2xF64 v3 = swap_f64(loadu<Vec2xF64>(src + 3)) * m01_m10 + m21_m20;

      storeu(dst + 0, v0);
      storeu(dst + 1, v1);
      storeu(dst + 2, v2);
      storeu(dst + 3, v3);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      storeu(dst, swap_f64(loadu<Vec2xF64>(src)) * m01_m10 + m21_m20);

      i--;
      dst++;
      src++;
    }
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL map_pointd_array_affine_sse2(const BLMatrix2D* self, BLPoint* dst, const BLPoint* src, size_t size) noexcept {
  using namespace SIMD;

  size_t i = size;
  Vec2xF64 m11_m00 = make128_f64(self->m11, self->m00);
  Vec2xF64 m01_m10 = make128_f64(self->m01, self->m10);
  Vec2xF64 m21_m20 = loadu<Vec2xF64>(&self->m20);

  if (PtrOps::both_aligned(dst, src, 16)) {
    while (i >= 4) {
      Vec2xF64 v0 = loada<Vec2xF64>(src + 0);
      Vec2xF64 v1 = loada<Vec2xF64>(src + 1);
      Vec2xF64 v2 = loada<Vec2xF64>(src + 2);
      Vec2xF64 v3 = loada<Vec2xF64>(src + 3);

      storea(dst + 0, v0 * m11_m00 + swap_f64(v0) * m01_m10 + m21_m20);
      storea(dst + 1, v1 * m11_m00 + swap_f64(v1) * m01_m10 + m21_m20);
      storea(dst + 2, v2 * m11_m00 + swap_f64(v2) * m01_m10 + m21_m20);
      storea(dst + 3, v3 * m11_m00 + swap_f64(v3) * m01_m10 + m21_m20);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      Vec2xF64 v0 = loada<Vec2xF64>(src);
      storea(dst, v0 * m11_m00 + swap_f64(v0) * m01_m10 + m21_m20);

      i--;
      dst++;
      src++;
    }
  }
  else {
    while (i >= 4) {
      Vec2xF64 v0 = loadu<Vec2xF64>(src + 0);
      Vec2xF64 v1 = loadu<Vec2xF64>(src + 1);
      Vec2xF64 v2 = loadu<Vec2xF64>(src + 2);
      Vec2xF64 v3 = loadu<Vec2xF64>(src + 3);

      storeu(dst + 0, v0 * m11_m00 + swap_f64(v0) * m01_m10 + m21_m20);
      storeu(dst + 1, v1 * m11_m00 + swap_f64(v1) * m01_m10 + m21_m20);
      storeu(dst + 2, v2 * m11_m00 + swap_f64(v2) * m01_m10 + m21_m20);
      storeu(dst + 3, v3 * m11_m00 + swap_f64(v3) * m01_m10 + m21_m20);

      i -= 4;
      dst += 4;
      src += 4;
    }

    while (i) {
      Vec2xF64 v0 = loadu<Vec2xF64>(src);
      storeu(dst, v0 * m11_m00 + swap_f64(v0) * m01_m10 + m21_m20);

      i--;
      dst++;
      src++;
    }
  }

  return BL_SUCCESS;
}

// Transform - Runtime Registration (SSE2)
// =======================================

void bl_transform_rt_init_sse2(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);
  BLMapPointDArrayFunc* funcs = map_pointd_array_funcs;

  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_IDENTITY ], map_pointd_array_identity_sse2);
  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_TRANSLATE], map_pointd_array_translate_sse2);
  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_SCALE    ], map_pointd_array_scale_sse2);
  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_SWAP     ], map_pointd_array_swap_sse2);
  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_AFFINE   ], map_pointd_array_affine_sse2);
  bl_assign_func(&funcs[BL_TRANSFORM_TYPE_INVALID  ], map_pointd_array_affine_sse2);
}

} // {TransformInternal}
} // {bl}

#endif
