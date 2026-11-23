// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/gradient_p.h>
#include <blend2d/pixelops/funcs_p.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/support/math_p.h>

namespace bl {
namespace PixelOps {
namespace Interpolation {

// bl::PixelOps - Interpolate PRGB32
// =================================

void BL_CDECL interpolate_prgb32(uint32_t* d_ptr, uint32_t d_size, const BLGradientStop* s_ptr, size_t s_size) noexcept {
  BL_ASSERT(d_ptr != nullptr);
  BL_ASSERT(d_size > 0);

  BL_ASSERT(s_ptr != nullptr);
  BL_ASSERT(s_size > 0);

  uint32_t* dSpanPtr = d_ptr;
  uint32_t i = d_size;

  uint32_t c0 = RgbaInternal::rgba32FromRgba64(s_ptr[0].rgba.value);
  uint32_t c1 = c0;

  uint32_t p0 = 0;
  uint32_t p1;

  size_t s_index = 0;
  double fWidth = double(int32_t(--d_size) << 8);

  uint32_t cp = Scalar::cvt_prgb32_8888_from_argb32_8888(c0);
  uint32_t cp_first = cp;

  if (s_size == 1)
    goto SolidLoop;

  do {
    c1 = RgbaInternal::rgba32FromRgba64(s_ptr[s_index].rgba.value);
    p1 = uint32_t(Math::round_to_int(s_ptr[s_index].offset * fWidth));

    dSpanPtr = d_ptr + (p0 >> 8);
    i = ((p1 >> 8) - (p0 >> 8));

    if (i == 0)
      c0 = c1;

    p0 = p1;
    i++;

SolidInit:
    cp = Scalar::cvt_prgb32_8888_from_argb32_8888(c0);
    if (c0 == c1) {
SolidLoop:
      do {
        dSpanPtr[0] = cp;
        dSpanPtr++;
      } while (--i);
    }
    else {
      dSpanPtr[0] = cp;
      dSpanPtr++;

      if (--i) {
        const uint32_t kShift = 23;
        const uint32_t kMask = 0xFFu << kShift;

        uint32_t r_pos = (c0 <<  7) & kMask;
        uint32_t gPos = (c0 << 15) & kMask;
        uint32_t b_pos = (c0 << 23) & kMask;

        uint32_t r_inc = (c1 <<  7) & kMask;
        uint32_t gInc = (c1 << 15) & kMask;
        uint32_t b_inc = (c1 << 23) & kMask;

        r_inc = uint32_t(int32_t(r_inc - r_pos) / int32_t(i));
        gInc = uint32_t(int32_t(gInc - gPos) / int32_t(i));
        b_inc = uint32_t(int32_t(b_inc - b_pos) / int32_t(i));

        r_pos += 1u << (kShift - 1);
        gPos += 1u << (kShift - 1);
        b_pos += 1u << (kShift - 1);

        if (RgbaInternal::isRgba32FullyOpaque(c0 & c1)) {
          // Both fully opaque, no need to premultiply.
          do {
            r_pos += r_inc;
            gPos += gInc;
            b_pos += b_inc;

            cp = 0xFF000000u + ((r_pos & kMask) >>  7) +
                               ((gPos & kMask) >> 15) +
                               ((b_pos & kMask) >> 23) ;

            dSpanPtr[0] = cp;
            dSpanPtr++;
          } while (--i);
        }
        else {
          // One or both having alpha, have to be premultiplied.
          uint32_t a_pos = (c0 >> 1) & kMask;
          uint32_t a_inc = (c1 >> 1) & kMask;

          a_inc = uint32_t(int32_t(a_inc - a_pos) / int32_t(i));
          a_pos += 1u << (kShift - 1);

          do {
            a_pos += a_inc;
            r_pos += r_inc;
            gPos += gInc;
            b_pos += b_inc;

            // `cp` contains red/blue components combined.
            uint32_t ca, cg;
            cp = ((b_pos & kMask) >> 23) +
                 ((r_pos & kMask) >>  7);
            ca = ((a_pos & kMask) >> 23);
            cg = ((gPos & kMask) >> 23);

            cp = (cp * ca) + 0x00800080u;
            cg = (cg * ca) + 0x00000080u;
            ca <<= 24;

            cp = ((cp + ((cp & 0xFF00FF00u) >> 8)) & 0xFF00FF00u) >> 8;
            cg = ((cg + ((cg & 0xFF00FF00u) >> 8)) & 0xFF00FF00u);

            cp += cg + ca;

            dSpanPtr[0] = cp;
            dSpanPtr++;
          } while (--i);
        }
      }

      c0 = c1;
    }
  } while (++s_index < s_size);

  // The last stop doesn't have to end at 1.0, in such case the remaining space is filled by the last color stop
  // (premultiplied). We jump to the main loop instead of filling the buffer here.
  i = uint32_t((size_t)((d_ptr + d_size + 1) - dSpanPtr));
  if (i != 0)
    goto SolidInit;

  // The first pixel has to be always set to the first stop's color. The main loop always honors the last color
  // value of the stop colliding with the previous offset index - for example if multiple stops have the same offset
  // [0.0] the first pixel will be the last stop's color. This is easier to fix here as we don't need extra conditions
  // in the main loop.
  d_ptr[0] = cp_first;
}

// bl::PixelOps - Interpolate PRGB64
// =================================

void BL_CDECL interpolate_prgb64(uint64_t* d_ptr, uint32_t d_size, const BLGradientStop* s_ptr, size_t s_size) noexcept {
  BL_ASSERT(d_ptr != nullptr);
  BL_ASSERT(d_size > 0);

  BL_ASSERT(s_ptr != nullptr);
  BL_ASSERT(s_size > 0);

  uint64_t* dSpanPtr = d_ptr;
  uint32_t i = d_size;

  uint64_t c0 = s_ptr[0].rgba.value;
  uint64_t c1 = c0;

  uint32_t p0 = 0;
  uint32_t p1;

  size_t s_index = 0;
  double fWidth = double(int32_t(--d_size) << 8);

  uint64_t cp = Scalar::cvt_prgb64_8888_from_argb64_8888(c0);
  uint64_t cp_first = cp;

  if (s_size == 1)
    goto SolidLoop;

  do {
    c1 = s_ptr[s_index].rgba.value;
    p1 = uint32_t(Math::round_to_int(s_ptr[s_index].offset * fWidth));

    dSpanPtr = d_ptr + (p0 >> 8);
    i = ((p1 >> 8) - (p0 >> 8));

    if (i == 0)
      c0 = c1;

    p0 = p1;
    i++;

SolidInit:
    cp = Scalar::cvt_prgb64_8888_from_argb64_8888(c0);
    if (c0 == c1) {
SolidLoop:
      do {
        dSpanPtr[0] = cp;
        dSpanPtr++;
      } while (--i);
    }
    else {
      dSpanPtr[0] = cp;
      dSpanPtr++;

      if (--i) {
        const uint32_t kShift = 15;
        const uint32_t kMask = 0xFFFFu << kShift;

        uint32_t r_pos = uint32_t((c0 >> (32 - kShift)) & kMask);
        uint32_t gPos = uint32_t((c0 >> (16 - kShift)) & kMask);
        uint32_t b_pos = uint32_t((c0 << (0  + kShift)) & kMask);

        uint32_t r_inc = uint32_t((c1 >> (32 - kShift)) & kMask);
        uint32_t gInc = uint32_t((c1 >> (16 - kShift)) & kMask);
        uint32_t b_inc = uint32_t((c1 << (0  + kShift)) & kMask);

        r_inc = uint32_t(int32_t(r_inc - r_pos) / int32_t(i));
        gInc = uint32_t(int32_t(gInc - gPos) / int32_t(i));
        b_inc = uint32_t(int32_t(b_inc - b_pos) / int32_t(i));

        r_pos += 1u << (kShift - 1);
        gPos += 1u << (kShift - 1);
        b_pos += 1u << (kShift - 1);

        if (RgbaInternal::isRgba64FullyOpaque(c0 & c1)) {
          // Both fully opaque, no need to premultiply.
          do {
            r_pos += r_inc;
            gPos += gInc;
            b_pos += b_inc;

            cp = (uint64_t(r_pos & kMask) << (32 - kShift)) |
                 (uint64_t(gPos & kMask) << (16 - kShift)) |
                 (uint64_t(b_pos & kMask) >> (0  + kShift)) | 0xFFFF000000000000u;

            dSpanPtr[0] = cp;
            dSpanPtr++;
          } while (--i);
        }
        else {
          // One or both having alpha, have to be premultiplied.
          uint32_t a_pos = uint32_t((c0 >> (48 - kShift)) & kMask);
          uint32_t a_inc = uint32_t((c1 >> (48 - kShift)) & kMask);

          a_inc = uint32_t(int32_t(a_inc - a_pos) / int32_t(i));
          a_pos += 1u << (kShift - 1);

          do {
            a_pos += a_inc;
            r_pos += r_inc;
            gPos += gInc;
            b_pos += b_inc;

            uint32_t ca = a_pos >> kShift;
            uint32_t cr = Scalar::udiv65535((r_pos >> kShift) * ca);
            uint32_t cg = Scalar::udiv65535((gPos >> kShift) * ca);
            uint32_t cb = Scalar::udiv65535((b_pos >> kShift) * ca);

            cp = RgbaInternal::packRgba64(cr, cg, cb, ca);
            dSpanPtr[0] = cp;
            dSpanPtr++;
          } while (--i);
        }
      }

      c0 = c1;
    }
  } while (++s_index < s_size);

  // The last stop doesn't have to end at 1.0, in such case the remaining space is filled by the last color stop
  // (premultiplied). We jump to the main loop instead of filling the buffer here.
  i = uint32_t((size_t)((d_ptr + d_size + 1) - dSpanPtr));
  if (i != 0)
    goto SolidInit;

  // The first pixel has to be always set to the first stop's color. The main loop always honors the last color
  // value of the stop colliding with the previous offset index - for example if multiple stops have the same offset
  // [0.0] the first pixel will be the last stop's color. This is easier to fix here as we don't need extra conditions
  // in the main loop.
  d_ptr[0] = cp_first;
}

} // {Interpolation}
} // {PixelOps}
} // {bl}
