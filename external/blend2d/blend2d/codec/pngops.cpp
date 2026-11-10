// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/pngops_p.h>

namespace bl::Png::Ops {

FunctionTable func_table;

// bl::Png::Ops - Inverse Filter
// ==============================

static BLResult BL_CDECL inverse_filter_impl(uint8_t* p, uint32_t bpp, uint32_t bpl, uint32_t h) noexcept {
  BL_ASSERT(bpp > 0u);
  BL_ASSERT(bpl > 1u);
  BL_ASSERT(h   > 0u);

  uint32_t y = h;
  uint8_t* u = nullptr;

  // Subtract one BYTE that is used to store the `filter` ID - it's always processed and not part of pixel data.
  bpl--;

  // First row uses a special filter that doesn't access the previous row,
  // which is assumed to contain all zeros.
  uint32_t filter_type = *p++;

  if (filter_type >= kFilterTypeCount)
    filter_type = kFilterTypeNone;

  filter_type = simplify_filter_of_first_row(filter_type);

  for (;;) {
    uint32_t i;

    switch (filter_type) {
      case kFilterTypeSub: {
        for (i = bpl - bpp; i != 0; i--, p++)
          p[bpp] = apply_sum_filter(p[bpp], p[0]);

        p += bpp;
        break;
      }

      case kFilterTypeUp: {
        BL_ASSERT(u != nullptr);
        for (i = bpl; i != 0; i--, p++, u++)
          p[0] = apply_sum_filter(p[0], u[0]);
        break;
      }

      case kFilterTypeAvg: {
        BL_ASSERT(u != nullptr);
        for (i = 0; i < bpp; i++)
          p[i] = apply_sum_filter(p[i], u[i] >> 1);

        u += bpp;
        for (i = bpl - bpp; i != 0; i--, p++, u++)
          p[bpp] = apply_sum_filter(p[bpp], apply_avg_filter(p[0], u[0]));

        p += bpp;
        break;
      }

      case kFilterTypePaeth: {
        BL_ASSERT(u != nullptr);
        for (i = 0; i < bpp; i++)
          p[i] = apply_sum_filter(p[i], u[i]);

        for (i = bpl - bpp; i != 0; i--, p++, u++)
          p[bpp] = apply_sum_filter(p[bpp], apply_paeth_filter(p[0], u[bpp], u[0]));

        p += bpp;
        break;
      }

      case kFilterTypeAvg0: {
        for (i = bpl - bpp; i != 0; i--, p++)
          p[bpp] = apply_sum_filter(p[bpp], p[0] >> 1);

        p += bpp;
        break;
      }

      case kFilterTypeNone:
      default:
        p += bpl;
        break;
    }

    if (--y == 0)
      break;

    u = p - bpl;
    filter_type = *p++;

    if (filter_type >= kFilterTypeCount)
      filter_type = kFilterTypeNone;
  }

  return BL_SUCCESS;
}

void init_func_table_ref(FunctionTable& ft) noexcept {
  ft.inverse_filter[1] = inverse_filter_impl;
  ft.inverse_filter[2] = inverse_filter_impl;
  ft.inverse_filter[3] = inverse_filter_impl;
  ft.inverse_filter[4] = inverse_filter_impl;
  ft.inverse_filter[6] = inverse_filter_impl;
  ft.inverse_filter[8] = inverse_filter_impl;
}

void init_func_table(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  // Initialize optimized PNG functions.
  FunctionTable& ft = func_table;

#if !defined(BL_BUILD_OPT_SSE2) && !defined(BL_BUILD_OPT_ASIMD)
  init_func_table_ref(ft);
#endif

#if defined(BL_BUILD_OPT_SSE2)
  if (bl_runtime_has_sse2(rt)) {
    init_func_table_sse2(ft);
  }
  else {
    init_func_table_ref(ft);
  }
#endif // BL_BUILD_OPT_SSE2

#if defined(BL_BUILD_OPT_AVX)
  if (bl_runtime_has_avx(rt)) {
    init_func_table_avx(ft);
  }
#endif // BL_BUILD_OPT_AVX

#if defined(BL_BUILD_OPT_ASIMD)
  if (bl_runtime_has_asimd(rt)) {
    init_func_table_asimd(ft);
  }
  else {
    init_func_table_ref(ft);
  }
#endif // BL_BUILD_OPT_ASIMD
}

} // {bl::Png::Ops}
