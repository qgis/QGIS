// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/imagescale_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/rgba_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

namespace bl {

typedef void (BL_CDECL* ImageScaleFilterFunc)(double* dst, const double* t_array, size_t n) noexcept;

// bl::ImageScale - Ops
// ====================

struct ImageScaleOps {
  BLResult (BL_CDECL* weights)(ImageScaleContext::Data* d, uint32_t dir, ImageScaleFilterFunc filter_func) noexcept;
  void (BL_CDECL* horz[BL_FORMAT_MAX_VALUE + 1])(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept;
  void (BL_CDECL* vert[BL_FORMAT_MAX_VALUE + 1])(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept;
};
static ImageScaleOps image_scale_ops;

// bl::ImageScale - Filter Implementations
// =======================================

static void BL_CDECL image_scale_nearest_filter(double* dst, const double* t_array, size_t n) noexcept {
  for (size_t i = 0; i < n; i++) {
    double t = t_array[i];
    dst[i] = t <= 0.5 ? 1.0 : 0.0;
  }
}

static void BL_CDECL image_scale_bilinear_filter(double* dst, const double* t_array, size_t n) noexcept {
  for (size_t i = 0; i < n; i++) {
    double t = t_array[i];
    dst[i] = t < 1.0 ? 1.0 - t : 0.0;
  }
}

static void BL_CDECL image_scale_bicubic_filter(double* dst, const double* t_array, size_t n) noexcept {
  constexpr double k2Div3 = 2.0 / 3.0;

  // 0.5t^3 - t^2 + 2/3 == (0.5t - 1.0) t^2 + 2/3
  for (size_t i = 0; i < n; i++) {
    double t = t_array[i];
    dst[i] = t < 1.0 ? (t * 0.5 - 1.0) * Math::square(t) + k2Div3 :
             t < 2.0 ? Math::cube(2.0 - t) / 6.0 : 0.0;
  }
}

static BL_INLINE double lanczos(double x, double y) noexcept {
  double sin_x = Math::sin(x);
  double sin_y = Math::sin(y);

  return (sin_x * sin_y) / (x * y);
}

static void BL_CDECL image_scale_lanczos_filter(double* dst, const double* t_array, size_t n) noexcept {
  constexpr double r = 2.0;
  constexpr double x = Math::kPI;
  constexpr double y = Math::kPI_DIV_2;

  for (size_t i = 0; i < n; i++) {
    double t = t_array[i];
    dst[i] = t == 0.0 ? 1.0 : t <= r ? lanczos(t * x, t * y) : 0.0;
  }
}

// bl::ImageScale - Weights
// ========================

static BLResult BL_CDECL image_scale_weights(ImageScaleContext::Data* d, uint32_t dir, ImageScaleFilterFunc filter) noexcept {
  int32_t* weight_list = d->weight_list[dir];
  ImageScaleContext::Record* record_list = d->record_list[dir];

  int dst_size = d->dst_size[dir];
  int src_size = d->src_size[dir];
  int kernel_size = d->kernel_size[dir];

  double radius = d->radius[dir];
  double factor = d->factor[dir];
  double scale = d->scale[dir];
  int32_t is_unbound = 0;

  bl::ScopedBufferTmp<512> wMem;
  double* wData = static_cast<double*>(wMem.alloc(unsigned(kernel_size) * sizeof(double)));

  if (BL_UNLIKELY(!wData))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  for (int i = 0; i < dst_size; i++) {
    double wPos = (double(i) + 0.5) / scale - 0.5;
    double wSum = 0.0;

    int left = int(wPos - radius);
    int right = left + kernel_size;
    int wIndex;

    // Calculate all weights for the destination pixel.
    wPos -= left;
    for (wIndex = 0; wIndex < kernel_size; wIndex++, wPos -= 1.0) {
      wData[wIndex] = bl_abs(wPos * factor);
    }

    filter(wData, wData, unsigned(kernel_size));

    // Remove padded pixels from left and right.
    wIndex = 0;
    while (left < 0) {
      double w = wData[wIndex];
      wData[++wIndex] += w;
      left++;
    }

    int wCount = kernel_size;
    while (right > src_size) {
      BL_ASSERT(wCount > 0);
      double w = wData[--wCount];
      wData[wCount - 1] += w;
      right--;
    }

    record_list[i].pos = 0;
    record_list[i].count = 0;

    if (wIndex < wCount) {
      // Sum all weights.
      int j;

      for (j = wIndex; j < wCount; j++) {
        double w = wData[j];
        wSum += w;
      }

      int iStrongest = 0;
      int32_t iSum = 0;
      int32_t iMax = 0;

      double wScale = 65535 / wSum;
      for (j = wIndex; j < wCount; j++) {
        int32_t w = int32_t(wData[j] * wScale) >> 8;

        // Remove zero weight from the beginning of the list.
        if (w == 0 && wIndex == j) {
          wIndex++;
          left++;
          continue;
        }

        weight_list[j - wIndex] = w;
        iSum += w;
        is_unbound |= w;

        if (iMax < w) {
          iMax = w;
          iStrongest = j - wIndex;
        }
      }

      // Normalize the strongest weight so the sum matches `0x100`.
      if (iSum != 0x100)
        weight_list[iStrongest] += int32_t(0x100) - iSum;

      // `wCount` is now absolute size of weights in `weight_list`.
      wCount -= wIndex;

      // Remove all zero weights from the end of the weight array.
      while (wCount > 0 && weight_list[wCount - 1] == 0)
        wCount--;

      if (wCount) {
        BL_ASSERT(left >= 0);
        record_list[i].pos = uint32_t(left);
        record_list[i].count = uint32_t(wCount);
      }
    }

    weight_list += kernel_size;
  }

  d->is_unbound[dir] = is_unbound < 0;
  return BL_SUCCESS;
}

// bl::ImageScale - Horz
// =====================

static void BL_CDECL image_scale_horz_prgb32(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]);
  uint32_t sh = uint32_t(d->src_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[0]);

  if (!d->is_unbound[ImageScaleContext::kDirHorz]) {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 4;
        const int32_t* wp = weight_list;

        uint32_t cr_cb = 0x00800080u;
        uint32_t ca_cg = 0x00800080u;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          uint32_t w0 = unsigned(wp[0]);

          ca_cg += ((p0 >> 8) & 0x00FF00FFu) * w0;
          cr_cb += ((p0     ) & 0x00FF00FFu) * w0;

          sp += 4;
          wp += 1;
        }

        MemOps::writeU32a(dp, (ca_cg & 0xFF00FF00u) + ((cr_cb & 0xFF00FF00u) >> 8));
        dp += 4;

        record_list += 1;
        weight_list += kernel_size;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 4;
        const int32_t* wp = weight_list;

        int32_t ca = 0x80;
        int32_t cr = 0x80;
        int32_t cg = 0x80;
        int32_t cb = 0x80;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          int32_t w0 = wp[0];

          ca += int32_t((p0 >> 24)        ) * w0;
          cr += int32_t((p0 >> 16) & 0xFFu) * w0;
          cg += int32_t((p0 >>  8) & 0xFFu) * w0;
          cb += int32_t((p0      ) & 0xFFu) * w0;

          sp += 4;
          wp += 1;
        }

        ca = bl_clamp<int32_t>(ca >> 8, 0, 255);
        cr = bl_clamp<int32_t>(cr >> 8, 0, ca);
        cg = bl_clamp<int32_t>(cg >> 8, 0, ca);
        cb = bl_clamp<int32_t>(cb >> 8, 0, ca);

        MemOps::writeU32a(dp, RgbaInternal::packRgba32(uint32_t(cr), uint32_t(cg), uint32_t(cb), uint32_t(ca)));
        dp += 4;

        record_list += 1;
        weight_list += kernel_size;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
}

static void BL_CDECL image_scale_horz_xrgb32(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]);
  uint32_t sh = uint32_t(d->src_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[0]);

  if (!d->is_unbound[ImageScaleContext::kDirHorz]) {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 4;
        const int32_t* wp = weight_list;

        uint32_t cx_cg = 0x00008000u;
        uint32_t cr_cb = 0x00800080u;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          uint32_t w0 = unsigned(wp[0]);

          cx_cg += (p0 & 0x0000FF00u) * w0;
          cr_cb += (p0 & 0x00FF00FFu) * w0;

          sp += 4;
          wp += 1;
        }

        MemOps::writeU32a(dp, 0xFF000000u + (((cx_cg & 0x00FF0000u) | (cr_cb & 0xFF00FF00u)) >> 8));
        dp += 4;

        record_list += 1;
        weight_list += kernel_size;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 4;
        const int32_t* wp = weight_list;

        int32_t cr = 0x80;
        int32_t cg = 0x80;
        int32_t cb = 0x80;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          int32_t w0 = wp[0];

          cr += int32_t((p0 >> 16) & 0xFF) * w0;
          cg += int32_t((p0 >>  8) & 0xFF) * w0;
          cb += int32_t((p0      ) & 0xFF) * w0;

          sp += 4;
          wp += 1;
        }

        cr = bl_clamp<int32_t>(cr >> 8, 0, 255);
        cg = bl_clamp<int32_t>(cg >> 8, 0, 255);
        cb = bl_clamp<int32_t>(cb >> 8, 0, 255);

        MemOps::writeU32a(dp, RgbaInternal::packRgba32(uint32_t(cr), uint32_t(cg), uint32_t(cb), 0xFFu));
        dp += 4;

        record_list += 1;
        weight_list += kernel_size;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
}

static void BL_CDECL image_scale_horz_a8(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]);
  uint32_t sh = uint32_t(d->src_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[0]);

  if (!d->is_unbound[ImageScaleContext::kDirHorz]) {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 1;
        const int32_t* wp = weight_list;

        uint32_t ca = 0x80;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = sp[0];
          uint32_t w0 = unsigned(wp[0]);

          ca += p0 * w0;

          sp += 1;
          wp += 1;
        }

        dp[0] = uint8_t(ca >> 8);

        record_list += 1;
        weight_list += kernel_size;

        dp += 1;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < sh; y++) {
      const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirHorz];
      const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirHorz];

      uint8_t* dp = dst_line;

      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_line + record_list->pos * 1;
        const int32_t* wp = weight_list;

        int32_t ca = 0x80;

        for (uint32_t i = record_list->count; i; i--) {
          uint32_t p0 = sp[0];
          int32_t w0 = wp[0];

          ca += (int32_t)p0 * w0;

          sp += 1;
          wp += 1;
        }

        dp[0] = IntOps::clamp_to_byte(ca >> 8);

        record_list += 1;
        weight_list += kernel_size;

        dp += 1;
      }

      dst_line += dst_stride;
      src_line += src_stride;
    }
  }
}

// bl::ImageScale - Vert
// =====================

static void BL_CDECL image_scale_vert_prgb32(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]);
  uint32_t dh = uint32_t(d->dst_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[ImageScaleContext::kDirVert]);

  const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirVert];
  const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirVert];

  if (!d->is_unbound[ImageScaleContext::kDirVert]) {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t count = record_list->count;
      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        uint32_t cr_cb = 0x00800080;
        uint32_t ca_cg = 0x00800080;

        for (uint32_t i = count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          uint32_t w0 = unsigned(wp[0]);

          ca_cg += ((p0 >> 8) & 0x00FF00FF) * w0;
          cr_cb += ((p0     ) & 0x00FF00FF) * w0;

          sp += src_stride;
          wp += 1;
        }

        MemOps::writeU32a(dp, (ca_cg & 0xFF00FF00) + ((cr_cb & 0xFF00FF00) >> 8));
        dp += 4;
        src_data += 4;
      }

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t count = record_list->count;
      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        int32_t ca = 0x80;
        int32_t cr = 0x80;
        int32_t cg = 0x80;
        int32_t cb = 0x80;

        for (uint32_t i = count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          int32_t w0 = wp[0];

          ca += int32_t((p0 >> 24)        ) * w0;
          cr += int32_t((p0 >> 16) & 0xFFu) * w0;
          cg += int32_t((p0 >>  8) & 0xFFu) * w0;
          cb += int32_t((p0      ) & 0xFFu) * w0;

          sp += src_stride;
          wp += 1;
        }

        ca = bl_clamp<int32_t>(ca >> 8, 0, 255);
        cr = bl_clamp<int32_t>(cr >> 8, 0, ca);
        cg = bl_clamp<int32_t>(cg >> 8, 0, ca);
        cb = bl_clamp<int32_t>(cb >> 8, 0, ca);

        MemOps::writeU32a(dp, RgbaInternal::packRgba32(uint32_t(cr), uint32_t(cg), uint32_t(cb), uint32_t(ca)));
        dp += 4;
        src_data += 4;
      }

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
}

static void BL_CDECL image_scale_vert_xrgb32(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]);
  uint32_t dh = uint32_t(d->dst_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[ImageScaleContext::kDirVert]);

  const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirVert];
  const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirVert];

  if (!d->is_unbound[ImageScaleContext::kDirVert]) {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t count = record_list->count;
      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        uint32_t cx_cg = 0x00008000u;
        uint32_t cr_cb = 0x00800080u;

        for (uint32_t i = count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          uint32_t w0 = unsigned(wp[0]);

          cx_cg += (p0 & 0x0000FF00u) * w0;
          cr_cb += (p0 & 0x00FF00FFu) * w0;

          sp += src_stride;
          wp += 1;
        }

        MemOps::writeU32a(dp, 0xFF000000u + (((cx_cg & 0x00FF0000u) | (cr_cb & 0xFF00FF00u)) >> 8));
        dp += 4;
        src_data += 4;
      }

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t count = record_list->count;
      for (uint32_t x = 0; x < dw; x++) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        int32_t cr = 0x80;
        int32_t cg = 0x80;
        int32_t cb = 0x80;

        for (uint32_t i = count; i; i--) {
          uint32_t p0 = MemOps::readU32a(sp);
          int32_t w0 = wp[0];

          cr += int32_t((p0 >> 16) & 0xFFu) * w0;
          cg += int32_t((p0 >>  8) & 0xFFu) * w0;
          cb += int32_t((p0      ) & 0xFFu) * w0;

          sp += src_stride;
          wp += 1;
        }

        cr = bl_clamp<int32_t>(cr >> 8, 0, 255);
        cg = bl_clamp<int32_t>(cg >> 8, 0, 255);
        cb = bl_clamp<int32_t>(cb >> 8, 0, 255);

        MemOps::writeU32a(dp, RgbaInternal::packRgba32(uint32_t(cr), uint32_t(cg), uint32_t(cb), 0xFFu));
        dp += 4;
        src_data += 4;
      }

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
}

static void BL_CDECL bl_image_scale_vert_bytes(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride, uint32_t wScale) noexcept {
  uint32_t dw = uint32_t(d->dst_size[0]) * wScale;
  uint32_t dh = uint32_t(d->dst_size[1]);
  uint32_t kernel_size = uint32_t(d->kernel_size[ImageScaleContext::kDirVert]);

  const ImageScaleContext::Record* record_list = d->record_list[ImageScaleContext::kDirVert];
  const int32_t* weight_list = d->weight_list[ImageScaleContext::kDirVert];

  if (!d->is_unbound[ImageScaleContext::kDirVert]) {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t x = dw;
      uint32_t i = 0;
      uint32_t count = record_list->count;

      if (((intptr_t)dp & 0x7) == 0)
        goto BoundLarge;
      i = 8u - uint32_t((uintptr_t)dp & 0x7u);

BoundSmall:
      x -= i;
      do {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        uint32_t c0 = 0x80;

        for (uint32_t j = count; j; j--) {
          uint32_t p0 = sp[0];
          uint32_t w0 = unsigned(wp[0]);

          c0 += p0 * w0;

          sp += src_stride;
          wp += 1;
        }

        dp[0] = (uint8_t)(c0 >> 8);
        dp += 1;
        src_data += 1;
      } while (--i);

BoundLarge:
      while (x >= 8) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        uint32_t c0 = 0x00800080u;
        uint32_t c1 = 0x00800080u;
        uint32_t c2 = 0x00800080u;
        uint32_t c3 = 0x00800080u;

        for (uint32_t j = count; j; j--) {
          uint32_t p0 = MemOps::readU32a(sp + 0u);
          uint32_t p1 = MemOps::readU32a(sp + 4u);
          uint32_t w0 = unsigned(wp[0]);

          c0 += ((p0     ) & 0x00FF00FFu) * w0;
          c1 += ((p0 >> 8) & 0x00FF00FFu) * w0;
          c2 += ((p1     ) & 0x00FF00FFu) * w0;
          c3 += ((p1 >> 8) & 0x00FF00FFu) * w0;

          sp += src_stride;
          wp += 1;
        }

        MemOps::writeU32a(dp + 0u, ((c0 & 0xFF00FF00u) >> 8) + (c1 & 0xFF00FF00u));
        MemOps::writeU32a(dp + 4u, ((c2 & 0xFF00FF00u) >> 8) + (c3 & 0xFF00FF00u));

        dp += 8;
        src_data += 8;
        x -= 8;
      }

      i = x;
      if (i != 0)
        goto BoundSmall;

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
  else {
    for (uint32_t y = 0; y < dh; y++) {
      const uint8_t* src_data = src_line + intptr_t(record_list->pos) * src_stride;
      uint8_t* dp = dst_line;

      uint32_t x = dw;
      uint32_t i = 0;
      uint32_t count = record_list->count;

      if (((size_t)dp & 0x3) == 0)
        goto UnboundLarge;
      i = 4u - uint32_t((uintptr_t)dp & 0x3u);

UnboundSmall:
      x -= i;
      do {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        int32_t c0 = 0x80;

        for (uint32_t j = count; j; j--) {
          uint32_t p0 = sp[0];
          int32_t w0 = wp[0];

          c0 += int32_t(p0) * w0;

          sp += src_stride;
          wp += 1;
        }

        dp[0] = (uint8_t)(uint32_t)bl_clamp<int32_t>(c0 >> 8, 0, 255);
        dp += 1;
        src_data += 1;
      } while (--i);

UnboundLarge:
      while (x >= 4) {
        const uint8_t* sp = src_data;
        const int32_t* wp = weight_list;

        int32_t c0 = 0x80;
        int32_t c1 = 0x80;
        int32_t c2 = 0x80;
        int32_t c3 = 0x80;

        for (uint32_t j = count; j; j--) {
          uint32_t p0 = MemOps::readU32a(sp);
          uint32_t w0 = unsigned(wp[0]);

          c0 += ((p0      ) & 0xFF) * w0;
          c1 += ((p0 >>  8) & 0xFF) * w0;
          c2 += ((p0 >> 16) & 0xFF) * w0;
          c3 += ((p0 >> 24)       ) * w0;

          sp += src_stride;
          wp += 1;
        }

        MemOps::writeU32a(dp, uint32_t(bl_clamp<int32_t>(c0 >> 8, 0, 255)      ) |
                           uint32_t(bl_clamp<int32_t>(c1 >> 8, 0, 255) <<  8) |
                           uint32_t(bl_clamp<int32_t>(c2 >> 8, 0, 255) << 16) |
                           uint32_t(bl_clamp<int32_t>(c3 >> 8, 0, 255) << 24));
        dp += 4;
        src_data += 4;
        x -= 4;
      }

      i = x;
      if (i != 0)
        goto UnboundSmall;

      record_list += 1;
      weight_list += kernel_size;

      dst_line += dst_stride;
    }
  }
}

static void BL_CDECL image_scale_vert_a8(const ImageScaleContext::Data* d, uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride) noexcept {
  bl_image_scale_vert_bytes(d, dst_line, dst_stride, src_line, src_stride, 1);
}

// bl::ImageScaleContext - Reset
// =============================

BLResult ImageScaleContext::reset() noexcept {
  free(data);
  data = nullptr;
  return BL_SUCCESS;
}

// bl::ImageScaleContext - Create
// ==============================

BLResult ImageScaleContext::create(const BLSizeI& to, const BLSizeI& from, uint32_t filter) noexcept {
  ImageScaleFilterFunc filter_func;
  double r = 0.0;

  // Setup Parameters
  // ----------------

  if (!Geometry::is_valid(to) || !Geometry::is_valid(from))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  switch (filter) {
    case BL_IMAGE_SCALE_FILTER_NEAREST : filter_func = image_scale_nearest_filter ; r = 1.0; break;
    case BL_IMAGE_SCALE_FILTER_BILINEAR: filter_func = image_scale_bilinear_filter; r = 1.0; break;
    case BL_IMAGE_SCALE_FILTER_BICUBIC : filter_func = image_scale_bicubic_filter ; r = 2.0; break;
    case BL_IMAGE_SCALE_FILTER_LANCZOS : filter_func = image_scale_lanczos_filter ; r = 2.0; break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  // Setup Weights
  // -------------

  double scale[2];
  double factor[2];
  double radius[2];
  int kernel_size[2];
  int is_unbound[2];

  scale[0] = double(to.w) / double(from.w);
  scale[1] = double(to.h) / double(from.h);

  factor[0] = 1.0;
  factor[1] = 1.0;

  radius[0] = r;
  radius[1] = r;

  if (scale[0] < 1.0) { factor[0] = scale[0]; radius[0] = r / scale[0]; }
  if (scale[1] < 1.0) { factor[1] = scale[1]; radius[1] = r / scale[1]; }

  kernel_size[0] = Math::ceil_to_int(1.0 + 2.0 * radius[0]);
  kernel_size[1] = Math::ceil_to_int(1.0 + 2.0 * radius[1]);

  is_unbound[0] = false;
  is_unbound[1] = false;

  size_t wWeightDataSize = size_t(to.w) * unsigned(kernel_size[0]) * sizeof(int32_t);
  size_t hWeightDataSize = size_t(to.h) * unsigned(kernel_size[1]) * sizeof(int32_t);
  size_t wRecordDataSize = size_t(to.w) * sizeof(Record);
  size_t hRecordDataSize = size_t(to.h) * sizeof(Record);
  size_t data_size = sizeof(Data) + wWeightDataSize + hWeightDataSize + wRecordDataSize + hRecordDataSize;

  if (this->data)
    free(this->data);

  this->data = static_cast<Data*>(malloc(data_size));
  if (BL_UNLIKELY(!this->data))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  // Init data.
  Data* d = this->data;
  d->dst_size[0] = to.w;
  d->dst_size[1] = to.h;
  d->src_size[0] = from.w;
  d->src_size[1] = from.h;
  d->kernel_size[0] = kernel_size[0];
  d->kernel_size[1] = kernel_size[1];
  d->is_unbound[0] = is_unbound[0];
  d->is_unbound[1] = is_unbound[1];

  d->scale[0] = scale[0];
  d->scale[1] = scale[1];
  d->factor[0] = factor[0];
  d->factor[1] = factor[1];
  d->radius[0] = radius[0];
  d->radius[1] = radius[1];

  // Distribute the memory buffer.
  uint8_t* data_ptr = PtrOps::offset<uint8_t>(d, sizeof(Data));

  d->weight_list[kDirHorz] = reinterpret_cast<int32_t*>(data_ptr); data_ptr += wWeightDataSize;
  d->weight_list[kDirVert] = reinterpret_cast<int32_t*>(data_ptr); data_ptr += hWeightDataSize;
  d->record_list[kDirHorz] = reinterpret_cast<Record*>(data_ptr); data_ptr += wRecordDataSize;
  d->record_list[kDirVert] = reinterpret_cast<Record*>(data_ptr);

  // Built-in filters will probably never fail, however, custom filters can and
  // it wouldn't be safe to just continue.
  image_scale_ops.weights(d, kDirHorz, filter_func);
  image_scale_ops.weights(d, kDirVert, filter_func);

  return BL_SUCCESS;
}

// bl::ImageScale - Process
// ========================

BLResult ImageScaleContext::process_horz_data(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride, uint32_t format) const noexcept {
  BL_ASSERT(is_initialized());
  image_scale_ops.horz[format](this->data, dst_line, dst_stride, src_line, src_stride);
  return BL_SUCCESS;
}

BLResult ImageScaleContext::process_vert_data(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride, uint32_t format) const noexcept {
  BL_ASSERT(is_initialized());
  image_scale_ops.vert[format](this->data, dst_line, dst_stride, src_line, src_stride);
  return BL_SUCCESS;
}

} // {bl}

// bl::ImageScale - Runtime Registration
// =====================================

void bl_image_scale_rt_init(BLRuntimeContext* rt) noexcept {
  bl_unused(rt);

  bl::image_scale_ops.weights = bl::image_scale_weights;

  bl::image_scale_ops.horz[BL_FORMAT_PRGB32] = bl::image_scale_horz_prgb32;
  bl::image_scale_ops.horz[BL_FORMAT_XRGB32] = bl::image_scale_horz_xrgb32;
  bl::image_scale_ops.horz[BL_FORMAT_A8    ] = bl::image_scale_horz_a8;

  bl::image_scale_ops.vert[BL_FORMAT_PRGB32] = bl::image_scale_vert_prgb32;
  bl::image_scale_ops.vert[BL_FORMAT_XRGB32] = bl::image_scale_vert_xrgb32;
  bl::image_scale_ops.vert[BL_FORMAT_A8    ] = bl::image_scale_vert_a8;
}
