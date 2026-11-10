// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/api-impl.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/image.h>
#include <blend2d/core/pixelconverter_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/tables/tables_p.h>

// bl::PixelConverter - Globals
// ============================

const BLPixelConverterOptions bl_pixel_converter_default_options {};

// bl::PixelConverter - Tables
// ===========================

// A table that contains shifts of native 32-bit pixel format. The only reason to have this in a table is a fact that
// a blue component is shifted by 8 (the same as green) to be at the right place, because there is no way to calculate
// the constants of component that has to stay within the low 8 bits as `scale` value is calculated by doubling the
// size until it reaches the required depth, so for example depth of 5 would scale to 10, depth 3 would scale to 9,
// and depths 1-2 would scale to 8.
static constexpr const uint8_t bl_pixel_converter_native32_from_foreign_shift_table[] = {
  16, // [0x00FF0000] R.
  8 , // [0x0000FF00] G.
  8 , // [0x0000FF00] B (shift to right by 8 to get the desired result).
  24  // [0xFF000000] A.
};

// bl::PixelConverter - Uninitialized
// ==================================

static BLResult BL_CDECL bl_convert_func_not_initialized(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_line, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  bl_unused(self, dst_data, dst_stride, src_line, src_stride, w, h, options);
  return bl_make_error(BL_ERROR_NOT_INITIALIZED);
}

// bl::PixelConverter - Utilities
// ==============================

static BL_INLINE bool bl_pixel_converter_is_indexed_depth(uint32_t depth) noexcept {
  return depth == 1 || depth == 2 || depth == 4 || depth == 8;
}

static bool bl_pixel_converter_palette_format_from_format_flags(BLFormatInfo& fi, BLFormatFlags flags) noexcept {
  // `fi` is now ARGB32 (non-premultiplied).
  fi = bl_format_info[BL_FORMAT_PRGB32];
  fi.clear_flags(BL_FORMAT_FLAG_PREMULTIPLIED);

  switch (flags & BL_FORMAT_FLAG_RGBA) {
    case BL_FORMAT_FLAG_ALPHA:
      return true;

    case BL_FORMAT_FLAG_RGB:
      fi.clear_flags(BL_FORMAT_FLAG_ALPHA);
      fi.sizes[3] = 0;
      fi.shifts[3] = 0;
      return true;

    case BL_FORMAT_FLAG_RGBA:
      fi.add_flags(BLFormatFlags(flags & BL_FORMAT_FLAG_PREMULTIPLIED));
      return true;

    default:
      return false;
  }
}

// bl::PixelConverter - Memory Management
// ======================================

static BL_INLINE void bl_pixel_converter_zero_initialize(BLPixelConverterCore* self) noexcept {
  memset(self, 0, sizeof(BLPixelConverterCore));
  self->convert_func = bl_convert_func_not_initialized;
}

static BL_INLINE void bl_pixel_converter_add_ref(BLPixelConverterCore* self) noexcept {
  BLPixelConverterData* d = bl_pixel_converter_get_data(self);
  if (!(d->internal_flags & BL_PIXEL_CONVERTER_INTERNAL_FLAG_DYNAMIC_DATA))
    return;

  bl_atomic_fetch_add_relaxed(d->ref_count);
}

static void bl_pixel_converter_release(BLPixelConverterCore* self) noexcept {
  BLPixelConverterData* d = bl_pixel_converter_get_data(self);

  uint32_t flags = d->internal_flags;
  if (!(flags & BL_PIXEL_CONVERTER_INTERNAL_FLAG_DYNAMIC_DATA))
    return;

  void* data_ptr = d->data_ptr;
  if (bl_atomic_fetch_sub_strong(d->ref_count) == 1) {
    if (flags & BL_PIXEL_CONVERTER_INTERNAL_FLAG_MULTI_STEP) {
      BLPixelConverterMultiStepContext* ctx = static_cast<BLPixelConverterMultiStepContext*>(data_ptr);
      bl_pixel_converter_reset(&ctx->first);
      bl_pixel_converter_reset(&ctx->second);
    }
    free(data_ptr);
  }
}

static BL_INLINE void bl_pixel_converter_copy_ref(BLPixelConverterCore* self, const BLPixelConverterCore* other) noexcept {
  memcpy(self, other, sizeof(BLPixelConverterCore));
  bl_pixel_converter_add_ref(self);
}

// bl::PixelConverter - Init & Destroy
// ===================================

BLResult bl_pixel_converter_init(BLPixelConverterCore* self) noexcept {
  bl_pixel_converter_zero_initialize(self);
  return BL_SUCCESS;
}

BLResult bl_pixel_converter_init_weak(BLPixelConverterCore* self, const BLPixelConverterCore* other) noexcept {
  bl_pixel_converter_copy_ref(self, other);
  return BL_SUCCESS;
}

BLResult bl_pixel_converter_destroy(BLPixelConverterCore* self) noexcept {
  bl_pixel_converter_release(self);
  self->convert_func = nullptr;
  return BL_SUCCESS;
}

// bl::PixelConverter - Reset
// ==========================

BLResult bl_pixel_converter_reset(BLPixelConverterCore* self) noexcept {
  bl_pixel_converter_release(self);
  bl_pixel_converter_zero_initialize(self);
  return BL_SUCCESS;
}

// bl::PixelConverter - Assign
// ===========================

BLResult bl_pixel_converter_assign(BLPixelConverterCore* self, const BLPixelConverterCore* other) noexcept {
  if (self == other)
    return BL_SUCCESS;

  bl_pixel_converter_release(self);
  bl_pixel_converter_copy_ref(self, other);
  return BL_SUCCESS;
}

// bl::PixelConverter - Create
// ===========================

BLResult bl_pixel_converter_create(BLPixelConverterCore* self, const BLFormatInfo* dst_info, const BLFormatInfo* src_info, BLPixelConverterCreateFlags create_flags) noexcept {
  BLFormatInfo di = *dst_info;
  BLFormatInfo si = *src_info;

  BL_PROPAGATE(di.sanitize());
  BL_PROPAGATE(si.sanitize());

  // Always create a new one and then swap it if the initialization succeeded.
  BLPixelConverterCore pc {};
  BL_PROPAGATE(bl_pixel_converter_init_internal(&pc, di, si, create_flags));

  bl_pixel_converter_release(self);
  memcpy(self, &pc, sizeof(BLPixelConverterCore));
  return BL_SUCCESS;
}

// bl::PixelConverter - Convert
// ============================

BLResult bl_pixel_converter_convert(const BLPixelConverterCore* self,
  void* dst_data, intptr_t dst_stride,
  const void* src_data, intptr_t src_stride,
  uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  return self->convert_func(self, static_cast<      uint8_t*>(dst_data), dst_stride,
                                 static_cast<const uint8_t*>(src_data), src_stride, w, h, options);
}

// bl::PixelConverter - Pixel Access
// =================================

struct BLPixelAccess8 {
  enum : uint32_t { kSize = 1 };

  static BL_INLINE uint32_t fetchA(const void* p) noexcept { return bl::MemOps::readU8(p); }
  static BL_INLINE uint32_t fetchU(const void* p) noexcept { return bl::MemOps::readU8(p); }

  static BL_INLINE void storeA(void* p, uint32_t v) noexcept { bl::MemOps::writeU8(p, uint16_t(v)); }
  static BL_INLINE void storeU(void* p, uint32_t v) noexcept { bl::MemOps::writeU8(p, uint16_t(v)); }
};

template<uint32_t ByteOrder>
struct BLPixelAccess16 {
  enum : uint32_t { kSize = 2 };

  static BL_INLINE uint32_t fetchA(const void* p) noexcept { return bl::MemOps::readU16<ByteOrder, 2>(p); }
  static BL_INLINE uint32_t fetchU(const void* p) noexcept { return bl::MemOps::readU16<ByteOrder, 1>(p); }

  static BL_INLINE void storeA(void* p, uint32_t v) noexcept { bl::MemOps::writeU16<ByteOrder, 2>(p, uint16_t(v)); }
  static BL_INLINE void storeU(void* p, uint32_t v) noexcept { bl::MemOps::writeU16<ByteOrder, 1>(p, uint16_t(v)); }
};

template<uint32_t ByteOrder>
struct BLPixelAccess24 {
  enum : uint32_t { kSize = 3 };

  static BL_INLINE uint32_t fetchA(const void* p) noexcept { return bl::MemOps::readU24u<ByteOrder>(p); }
  static BL_INLINE uint32_t fetchU(const void* p) noexcept { return bl::MemOps::readU24u<ByteOrder>(p); }

  static BL_INLINE void storeA(void* p, uint32_t v) noexcept { bl::MemOps::writeU24u<ByteOrder>(p, v); }
  static BL_INLINE void storeU(void* p, uint32_t v) noexcept { bl::MemOps::writeU24u<ByteOrder>(p, v); }
};

template<uint32_t ByteOrder>
struct BLPixelAccess32 {
  enum : uint32_t { kSize = 4 };

  static BL_INLINE uint32_t fetchA(const void* p) noexcept { return bl::MemOps::readU32<ByteOrder, 4>(p); }
  static BL_INLINE uint32_t fetchU(const void* p) noexcept { return bl::MemOps::readU32<ByteOrder, 1>(p); }

  static BL_INLINE void storeA(void* p, uint32_t v) noexcept { bl::MemOps::writeU32<ByteOrder, 4>(p, v); }
  static BL_INLINE void storeU(void* p, uint32_t v) noexcept { bl::MemOps::writeU32<ByteOrder, 1>(p, v); }
};

// bl::PixelConverter - Copy
// =========================

BLResult bl_convert_copy(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t bytes_per_pixel = bl_pixel_converter_get_data(self)->mem_copy_data.bytes_per_pixel;
  const size_t byte_width = size_t(w) * bytes_per_pixel;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(byte_width) + gap;
  src_stride -= uintptr_t(byte_width);

  for (uint32_t y = h; y != 0; y--) {
    size_t i = byte_width;

    if (!bl::MemOps::kUnalignedMem32 && bl::PtrOps::have_equal_alignment(dst_data, src_data, 4)) {
      while (i && ((uintptr_t)dst_data) & 0x03) {
        *dst_data++ = *src_data++;
        i--;
      }

      while (i >= 16) {
        uint32_t p0 = bl::MemOps::readU32a(src_data +  0);
        uint32_t p1 = bl::MemOps::readU32a(src_data +  4);
        uint32_t p2 = bl::MemOps::readU32a(src_data +  8);
        uint32_t p3 = bl::MemOps::readU32a(src_data + 12);

        bl::MemOps::writeU32a(dst_data +  0, p0);
        bl::MemOps::writeU32a(dst_data +  4, p1);
        bl::MemOps::writeU32a(dst_data +  8, p2);
        bl::MemOps::writeU32a(dst_data + 12, p3);

        dst_data += 16;
        src_data += 16;
        i -= 16;
      }

      while (i >= 4) {
        bl::MemOps::writeU32a(dst_data, bl::MemOps::readU32a(src_data));
        dst_data += 4;
        src_data += 4;
        i -= 4;
      }
    }
    else {
      while (i >= 16) {
        uint32_t p0 = bl::MemOps::readU32u(src_data +  0);
        uint32_t p1 = bl::MemOps::readU32u(src_data +  4);
        uint32_t p2 = bl::MemOps::readU32u(src_data +  8);
        uint32_t p3 = bl::MemOps::readU32u(src_data + 12);

        bl::MemOps::writeU32u(dst_data +  0, p0);
        bl::MemOps::writeU32u(dst_data +  4, p1);
        bl::MemOps::writeU32u(dst_data +  8, p2);
        bl::MemOps::writeU32u(dst_data + 12, p3);

        dst_data += 16;
        src_data += 16;
        i -= 16;
      }

      while (i >= 4) {
        bl::MemOps::writeU32u(dst_data, bl::MemOps::readU32u(src_data));
        dst_data += 4;
        src_data += 4;
        i -= 4;
      }
    }

    while (i) {
      *dst_data++ = *src_data++;
      i--;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Copy|Or
// ============================

BLResult bl_convert_copy_or_8888(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const uint32_t fill_mask = bl_pixel_converter_get_data(self)->mem_copy_data.fill_mask;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  for (uint32_t y = h; y != 0; y--) {
    uint32_t i = w;
    if (!bl::MemOps::kUnalignedMem32 && bl::PtrOps::both_aligned(dst_data, src_data, 4)) {
      while (i >= 4) {
        uint32_t p0 = bl::MemOps::readU32a(src_data +  0);
        uint32_t p1 = bl::MemOps::readU32a(src_data +  4);
        uint32_t p2 = bl::MemOps::readU32a(src_data +  8);
        uint32_t p3 = bl::MemOps::readU32a(src_data + 12);

        bl::MemOps::writeU32a(dst_data +  0, p0 | fill_mask);
        bl::MemOps::writeU32a(dst_data +  4, p1 | fill_mask);
        bl::MemOps::writeU32a(dst_data +  8, p2 | fill_mask);
        bl::MemOps::writeU32a(dst_data + 12, p3 | fill_mask);

        dst_data += 16;
        src_data += 16;
        i -= 4;
      }

      while (i) {
        bl::MemOps::writeU32a(dst_data, bl::MemOps::readU32a(src_data) | fill_mask);
        dst_data += 4;
        src_data += 4;
        i--;
      }
    }
    else {
      while (i >= 4) {
        uint32_t p0 = bl::MemOps::readU32u(src_data +  0);
        uint32_t p1 = bl::MemOps::readU32u(src_data +  4);
        uint32_t p2 = bl::MemOps::readU32u(src_data +  8);
        uint32_t p3 = bl::MemOps::readU32u(src_data + 12);

        bl::MemOps::writeU32u(dst_data +  0, p0 | fill_mask);
        bl::MemOps::writeU32u(dst_data +  4, p1 | fill_mask);
        bl::MemOps::writeU32u(dst_data +  8, p2 | fill_mask);
        bl::MemOps::writeU32u(dst_data + 12, p3 | fill_mask);

        dst_data += 16;
        src_data += 16;
        i -= 4;
      }

      while (i) {
        bl::MemOps::writeU32u(dst_data, bl::MemOps::readU32u(src_data) | fill_mask);
        dst_data += 4;
        src_data += 4;
        i--;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Premultiply & Unpremultiply
// ================================================

static BLResult BL_CDECL bl_convert_premultiply_8888(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::PremultiplyData& d = bl_pixel_converter_get_data(self)->premultiply_data;
  const uint32_t alpha_shift = d.alpha_shift;
  const uint32_t alpha_mask = 0xFFu << alpha_shift;
  const uint32_t fill_mask = d.fill_mask;

  for (uint32_t y = h; y != 0; y--) {
    if (!bl::MemOps::kUnalignedMem32 && bl::PtrOps::both_aligned(dst_data, src_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32a(src_data);
        uint32_t a = (pix >> alpha_shift) & 0xFFu;

        pix |= alpha_mask;

        uint32_t c0 = ((pix     ) & 0x00FF00FFu) * a + 0x00800080u;
        uint32_t c1 = ((pix >> 8) & 0x00FF00FFu) * a + 0x00800080u;

        c0 = (c0 + ((c0 >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;
        c1 = (c1 + ((c1 >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;

        bl::MemOps::writeU32a(dst_data, (c0 >> 8) | c1 | fill_mask);

        dst_data += 4;
        src_data += 4;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32u(src_data);
        uint32_t a = (pix >> alpha_shift) & 0xFFu;

        pix |= alpha_mask;

        uint32_t c0 = ((pix     ) & 0x00FF00FFu) * a + 0x00800080u;
        uint32_t c1 = ((pix >> 8) & 0x00FF00FFu) * a + 0x00800080u;

        c0 = (c0 + ((c0 >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;
        c1 = (c1 + ((c1 >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;

        bl::MemOps::writeU32u(dst_data, (c0 >> 8) | c1 | fill_mask);

        dst_data += 4;
        src_data += 4;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<uint32_t A_Shift>
static BLResult BL_CDECL bl_convert_unpremultiply_8888(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  bl_unused(self);

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * 4;

  const uint32_t R_Shift = (A_Shift +  8u) % 32u;
  const uint32_t G_Shift = (A_Shift + 16u) % 32u;
  const uint32_t B_Shift = (A_Shift + 24u) % 32u;

  for (uint32_t y = h; y != 0; y--) {
    if (!bl::MemOps::kUnalignedMem32 && bl::PtrOps::both_aligned(dst_data, src_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32a(src_data);
        uint32_t r = (pix >> R_Shift) & 0xFFu;
        uint32_t g = (pix >> G_Shift) & 0xFFu;
        uint32_t b = (pix >> B_Shift) & 0xFFu;
        uint32_t a = (pix >> A_Shift) & 0xFFu;

        bl::PixelOps::Scalar::unpremultiply_rgb_8bit(r, g, b, a);
        bl::MemOps::writeU32a(dst_data, (r << R_Shift) | (g << G_Shift) | (b << B_Shift) | (a << A_Shift));

        dst_data += 4;
        src_data += 4;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32u(src_data);
        uint32_t r = (pix >> R_Shift) & 0xFFu;
        uint32_t g = (pix >> G_Shift) & 0xFFu;
        uint32_t b = (pix >> B_Shift) & 0xFFu;
        uint32_t a = (pix >> A_Shift) & 0xFFu;

        bl::PixelOps::Scalar::unpremultiply_rgb_8bit(r, g, b, a);
        bl::MemOps::writeU32u(dst_data, (r << R_Shift) | (g << G_Shift) | (b << B_Shift) | (a << A_Shift));

        dst_data += 4;
        src_data += 4;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - A8 From PRGB32/ARGB32
// ==========================================

BLResult bl_convert_a8_from_8888(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::X8FromRgb32Data& d = bl_pixel_converter_get_data(self)->x8FromRgb32Data;
  const size_t src_bpp = d.bytes_per_pixel;

#if BL_BYTE_ORDER == 1234
  const size_t srcAI = d.alpha_shift / 8u;
#else
  const size_t srcAI = (24u - d.alpha_shift) / 8u;
#endif

  src_data += srcAI;

  for (uint32_t y = h; y != 0; y--) {
    for (uint32_t i = w; i != 0; i--) {
      dst_data[0] = src_data[0];
      dst_data += 1;
      src_data += src_bpp;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - RGB32 From A8/L8
// =====================================

BLResult bl_convert_8888_from_x8(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w);

  const BLPixelConverterData::Rgb32FromX8Data& d = bl_pixel_converter_get_data(self)->rgb32FromX8Data;
  const uint32_t fill_mask = d.fill_mask;
  const uint32_t zero_mask = d.zero_mask;

  for (uint32_t y = h; y != 0; y--) {
    if (!bl::MemOps::kUnalignedMem32 && bl::IntOps::is_aligned(dst_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        bl::MemOps::writeU32a(dst_data, ((uint32_t(src_data[0]) * 0x01010101u) & zero_mask) | fill_mask);
        dst_data += 4;
        src_data += 1;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        bl::MemOps::writeU32u(dst_data, ((uint32_t(src_data[0]) * 0x01010101u) & zero_mask) | fill_mask);
        dst_data += 4;
        src_data += 1;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Any <- Indexed1
// ====================================

// Instead of doing a table lookup each time we create a XOR mask that is used to get the second color value from
// the first one. This allows to remove the lookup completely. The only requirement is that we need all zeros or
// ones depending on the source value (see the implementation, it uses signed right shift to fill these bits in).

template<typename PixelAccess>
static BLResult BL_CDECL bl_convert_any_from_indexed1(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_line, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const uint32_t kPixelSize = PixelAccess::kSize;
  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * kPixelSize + gap;

  const BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;
  uint32_t c0 = PixelAccess::fetchA(d.embedded.table8 + 0 * kPixelSize);
  uint32_t cm = PixelAccess::fetchA(d.embedded.table8 + 1 * kPixelSize) ^ c0; // XOR mask.

  if (c0 == 0 && cm == (0xFFFFFFFFu >> (32 - kPixelSize))) {
    // Special case for zeros and all ones.
    for (uint32_t y = h; y != 0; y--) {
      const uint8_t* src_data = src_line;
      uint32_t i = w;

      while (i >= 8) {
        uint32_t b0 = uint32_t(*src_data++) << 24;
        uint32_t b1 = b0 << 1;

        PixelAccess::storeU(dst_data + 0 * kPixelSize, bl::IntOps::sar(b0, 31)); b0 <<= 2;
        PixelAccess::storeU(dst_data + 1 * kPixelSize, bl::IntOps::sar(b1, 31)); b1 <<= 2;
        PixelAccess::storeU(dst_data + 2 * kPixelSize, bl::IntOps::sar(b0, 31)); b0 <<= 2;
        PixelAccess::storeU(dst_data + 3 * kPixelSize, bl::IntOps::sar(b1, 31)); b1 <<= 2;
        PixelAccess::storeU(dst_data + 4 * kPixelSize, bl::IntOps::sar(b0, 31)); b0 <<= 2;
        PixelAccess::storeU(dst_data + 5 * kPixelSize, bl::IntOps::sar(b1, 31)); b1 <<= 2;
        PixelAccess::storeU(dst_data + 6 * kPixelSize, bl::IntOps::sar(b0, 31));
        PixelAccess::storeU(dst_data + 7 * kPixelSize, bl::IntOps::sar(b1, 31));

        dst_data += 8 * kPixelSize;
        i -= 8;
      }

      if (i) {
        uint32_t b0 = uint32_t(*src_data++) << 24;
        do {
          PixelAccess::storeU(dst_data, bl::IntOps::sar(b0, 31));
          dst_data += kPixelSize;
          b0 <<= 1;
        } while (--i);
      }

      dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
      dst_data += dst_stride;
      src_line += src_stride;
    }
  }
  else {
    // Generic case for any other combination.
    for (uint32_t y = h; y != 0; y--) {
      const uint8_t* src_data = src_line;
      uint32_t i = w;

      while (i >= 8) {
        uint32_t b0 = uint32_t(*src_data++) << 24;
        uint32_t b1 = b0 << 1;

        PixelAccess::storeU(dst_data + 0 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b0, 31))); b0 <<= 2;
        PixelAccess::storeU(dst_data + 1 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b1, 31))); b1 <<= 2;
        PixelAccess::storeU(dst_data + 2 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b0, 31))); b0 <<= 2;
        PixelAccess::storeU(dst_data + 3 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b1, 31))); b1 <<= 2;
        PixelAccess::storeU(dst_data + 4 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b0, 31))); b0 <<= 2;
        PixelAccess::storeU(dst_data + 5 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b1, 31))); b1 <<= 2;
        PixelAccess::storeU(dst_data + 6 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b0, 31)));
        PixelAccess::storeU(dst_data + 7 * kPixelSize, c0 ^ (cm & bl::IntOps::sar(b1, 31)));

        dst_data += 8 * kPixelSize;
        i -= 8;
      }

      if (i) {
        uint32_t b0 = uint32_t(*src_data++) << 24;
        do {
          PixelAccess::storeU(dst_data, c0 ^ (cm & bl::IntOps::sar(b0, 31)));
          dst_data += kPixelSize;
          b0 <<= 1;
        } while (--i);
      }

      dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
      dst_data += dst_stride;
      src_line += src_stride;
    }
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Any <- Indexed2
// ====================================

template<typename PixelAccess>
static BLResult BL_CDECL bl_convert_any_from_indexed2(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_line, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const uint32_t kPixelSize = PixelAccess::kSize;
  const uint32_t kShiftToLeadingByte = (bl::IntOps::bit_size_of<uintptr_t>() - 8);
  const uint32_t kShiftToTableIndex  = (bl::IntOps::bit_size_of<uintptr_t>() - 2);

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * kPixelSize + gap;

  const BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;
  const uint8_t* table = d.embedded.table8;

  for (uint32_t y = h; y != 0; y--) {
    const uint8_t* src_data = src_line;
    uint32_t i = w;

    while (i >= 4) {
      uintptr_t b0 = uintptr_t(*src_data++) << kShiftToLeadingByte;

      uint32_t p0 = PixelAccess::fetchA(table + (b0 >> kShiftToTableIndex) * kPixelSize); b0 <<= 2;
      uint32_t p1 = PixelAccess::fetchA(table + (b0 >> kShiftToTableIndex) * kPixelSize); b0 <<= 2;
      uint32_t p2 = PixelAccess::fetchA(table + (b0 >> kShiftToTableIndex) * kPixelSize); b0 <<= 2;
      uint32_t p3 = PixelAccess::fetchA(table + (b0 >> kShiftToTableIndex) * kPixelSize);

      PixelAccess::storeU(dst_data + 0 * kPixelSize, p0);
      PixelAccess::storeU(dst_data + 1 * kPixelSize, p1);
      PixelAccess::storeU(dst_data + 2 * kPixelSize, p2);
      PixelAccess::storeU(dst_data + 3 * kPixelSize, p3);

      dst_data += 4 * kPixelSize;
      i -= 4;
    }

    if (i) {
      uintptr_t b0 = uintptr_t(*src_data++) << kShiftToLeadingByte;
      do {
        PixelAccess::storeU(dst_data, PixelAccess::fetchA(table + (b0 >> kShiftToTableIndex) * kPixelSize));
        dst_data += kPixelSize;
        b0 <<= 2;
      } while (--i);
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_line += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Any <- Indexed4
// ====================================

template<typename PixelAccess>
static BLResult BL_CDECL bl_convert_any_from_indexed4(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_line, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const uint32_t kPixelSize = PixelAccess::kSize;

  const BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;
  const uint8_t* table = d.embedded.table8;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * kPixelSize + gap;

  for (uint32_t y = h; y != 0; y--) {
    const uint8_t* src_data = src_line;
    uint32_t i = w;

    while (i >= 2) {
      uintptr_t b0 = *src_data++;

      uint32_t p0 = PixelAccess::fetchA(table + (b0 >> 4) * kPixelSize);
      uint32_t p1 = PixelAccess::fetchA(table + (b0 & 15) * kPixelSize);

      PixelAccess::storeU(dst_data + 0 * kPixelSize, p0);
      PixelAccess::storeU(dst_data + 1 * kPixelSize, p1);

      dst_data += 2 * kPixelSize;
      i -= 2;
    }

    if (i) {
      uintptr_t b0 = src_data[0];
      PixelAccess::storeU(dst_data, PixelAccess::fetchA(table + (b0 >> 4) * kPixelSize));
      dst_data += kPixelSize;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_line += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Any <- Indexed8
// ====================================

// Special case - used when no copy of the palette is required.
static BLResult BL_CDECL bl_convert_a8_from_indexed8_pal32(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) + gap;
  src_stride -= uintptr_t(w);

  const BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;
  const uint32_t* table = d.dynamic.table32;

  for (uint32_t y = h; y != 0; y--) {
    for (uint32_t i = w; i != 0; i--) {
      uintptr_t b0 = *src_data++;
      *dst_data++ = uint8_t(table[b0] >> 24);
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess>
static BLResult BL_CDECL bl_convert_any_from_indexed8(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const uint32_t kPixelSize = PixelAccess::kSize;
  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * kPixelSize + gap;
  src_stride -= uintptr_t(w);

  const BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;
  const uint8_t* table = d.dynamic.table8;

  for (uint32_t y = h; y != 0; y--) {
    for (uint32_t i = w; i != 0; i--) {
      uintptr_t b0 = *src_data++;
      PixelAccess::storeU(dst_data, PixelAccess::fetchA(table + b0 * kPixelSize));
      dst_data += kPixelSize;
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - ByteShuffle
// ================================

// TODO:

// bl::PixelConverter - Native32 <- XRGB|ARGB|PRGB
// ===============================================

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_xrgb32_from_xrgb_any(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * PixelAccess::kSize;

  const BLPixelConverterData::NativeFromForeign& d = bl_pixel_converter_get_data(self)->native_from_foreign;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];

  uint32_t r_scale = d.scale[0];
  uint32_t gScale = d.scale[1];
  uint32_t b_scale = d.scale[2];

  uint32_t fill_mask = d.fill_mask;

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, 4) && bl::IntOps::is_aligned(src_data, PixelAccess::kSize)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchA(src_data);
        uint32_t r = (((pix >> r_shift) & r_mask) * r_scale) & 0x00FF0000u;
        uint32_t g = (((pix >> g_shift) & g_mask) * gScale) & 0x0000FF00u;
        uint32_t b = (((pix >> b_shift) & b_mask) * b_scale) >> 8;

        bl::MemOps::writeU32a(dst_data, r | g | b | fill_mask);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchU(src_data);
        uint32_t r = (((pix >> r_shift) & r_mask) * r_scale) & 0x00FF0000u;
        uint32_t g = (((pix >> g_shift) & g_mask) * gScale) & 0x0000FF00u;
        uint32_t b = (((pix >> b_shift) & b_mask) * b_scale) >> 8;

        bl::MemOps::writeU32u(dst_data, r | g | b | fill_mask);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_prgb32_from_argb_any(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * PixelAccess::kSize;

  const BLPixelConverterData::NativeFromForeign& d = bl_pixel_converter_get_data(self)->native_from_foreign;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];
  uint32_t a_mask = d.masks[3];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];
  uint32_t a_shift = d.shifts[3];

  uint32_t r_scale = d.scale[0];
  uint32_t gScale = d.scale[1];
  uint32_t b_scale = d.scale[2];
  uint32_t a_scale = d.scale[3];

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, 4) && bl::IntOps::is_aligned(src_data, PixelAccess::kSize)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchA(src_data);
        uint32_t _a = ((((pix >> a_shift) & a_mask) * a_scale) >> 24);
        uint32_t ag = ((((pix >> g_shift) & g_mask) * gScale) >>  8);
        uint32_t rb = ((((pix >> r_shift) & r_mask) * r_scale) & 0x00FF0000u) |
                      ((((pix >> b_shift) & b_mask) * b_scale) >>  8);

        ag |= 0x00FF0000u;
        rb *= _a;
        ag *= _a;

        rb += 0x00800080u;
        ag += 0x00800080u;

        rb = (rb + ((rb >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;
        ag = (ag + ((ag >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;

        rb >>= 8;
        bl::MemOps::writeU32a(dst_data, ag + rb);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchU(src_data);
        uint32_t _a = ((((pix >> a_shift) & a_mask) * a_scale) >> 24);
        uint32_t ag = ((((pix >> g_shift) & g_mask) * gScale) >>  8);
        uint32_t rb = ((((pix >> r_shift) & r_mask) * r_scale) & 0x00FF0000u) |
                      ((((pix >> b_shift) & b_mask) * b_scale) >>  8);

        ag |= 0x00FF0000u;
        rb *= _a;
        ag *= _a;

        rb += 0x00800080u;
        ag += 0x00800080u;

        rb = (rb + ((rb >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;
        ag = (ag + ((ag >> 8) & 0x00FF00FFu)) & 0xFF00FF00u;

        rb >>= 8;
        bl::MemOps::writeU32u(dst_data, ag | rb);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_prgb32_from_prgb_any(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * PixelAccess::kSize;

  const BLPixelConverterData::NativeFromForeign& d = bl_pixel_converter_get_data(self)->native_from_foreign;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];
  uint32_t a_mask = d.masks[3];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];
  uint32_t a_shift = d.shifts[3];

  uint32_t r_scale = d.scale[0];
  uint32_t gScale = d.scale[1];
  uint32_t b_scale = d.scale[2];
  uint32_t a_scale = d.scale[3];

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, 4) && bl::IntOps::is_aligned(src_data, PixelAccess::kSize)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchA(src_data);
        uint32_t r = ((pix >> r_shift) & r_mask) * r_scale;
        uint32_t g = ((pix >> g_shift) & g_mask) * gScale;
        uint32_t b = ((pix >> b_shift) & b_mask) * b_scale;
        uint32_t a = ((pix >> a_shift) & a_mask) * a_scale;

        uint32_t ag = (a + (g     )) & 0xFF00FF00u;
        uint32_t rb = (r + (b >> 8)) & 0x00FF00FFu;

        bl::MemOps::writeU32a(dst_data, ag | rb);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchU(src_data);
        uint32_t g = ((pix >> g_shift) & g_mask) * gScale;
        uint32_t r = ((pix >> r_shift) & r_mask) * r_scale;
        uint32_t b = ((pix >> b_shift) & b_mask) * b_scale;
        uint32_t a = ((pix >> a_shift) & a_mask) * a_scale;

        uint32_t ag = (a + (g     )) & 0xFF00FF00u;
        uint32_t rb = (r + (b >> 8)) & 0x00FF00FFu;

        bl::MemOps::writeU32u(dst_data, ag | rb);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_argb32_from_prgb_any(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * 4 + gap;
  src_stride -= uintptr_t(w) * PixelAccess::kSize;

  const BLPixelConverterData::NativeFromForeign& d = bl_pixel_converter_get_data(self)->native_from_foreign;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];
  uint32_t a_mask = d.masks[3];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];
  uint32_t a_shift = d.shifts[3];

  uint32_t r_scale = d.scale[0];
  uint32_t gScale = d.scale[1];
  uint32_t b_scale = d.scale[2];
  uint32_t a_scale = d.scale[3];

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, 4) && bl::IntOps::is_aligned(src_data, PixelAccess::kSize)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchA(src_data);
        uint32_t r = (((pix >> r_shift) & r_mask) * r_scale) >> 16;
        uint32_t g = (((pix >> g_shift) & g_mask) * gScale) >> 8;
        uint32_t b = (((pix >> b_shift) & b_mask) * b_scale) >> 8;
        uint32_t a = (((pix >> a_shift) & a_mask) * a_scale) >> 24;

        bl::PixelOps::Scalar::unpremultiply_rgb_8bit(r, g, b, a);
        bl::MemOps::writeU32a(dst_data, (a << 24) | (r << 16) | (g << 8) | b);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = PixelAccess::fetchU(src_data);
        uint32_t r = (((pix >> r_shift) & r_mask) * r_scale) >> 16;
        uint32_t g = (((pix >> g_shift) & g_mask) * gScale) >> 8;
        uint32_t b = (((pix >> b_shift) & b_mask) * b_scale) >> 8;
        uint32_t a = (((pix >> a_shift) & a_mask) * a_scale) >> 24;

        bl::PixelOps::Scalar::unpremultiply_rgb_8bit(r, g, b, a);
        bl::MemOps::writeU32u(dst_data, (a << 24) | (r << 16) | (g << 8) | b);

        dst_data += 4;
        src_data += PixelAccess::kSize;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - XRGB|ARGB|PRGB <- Native32
// ===============================================

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_xrgb_any_from_xrgb32(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * PixelAccess::kSize + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::ForeignFromNative& d = bl_pixel_converter_get_data(self)->foreign_from_native;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];

  uint32_t fill_mask = d.fill_mask;

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, PixelAccess::kSize) && bl::IntOps::is_aligned(src_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32a(src_data);

        uint32_t r = ((pix >> 16) & 0xFFu) * 0x01010101u;
        uint32_t g = ((pix >>  8) & 0xFFu) * 0x01010101u;
        uint32_t b = ((pix      ) & 0xFFu) * 0x01010101u;

        PixelAccess::storeA(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) | fill_mask);
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32u(src_data);

        uint32_t r = ((pix >> 16) & 0xFFu) * 0x01010101u;
        uint32_t g = ((pix >>  8) & 0xFFu) * 0x01010101u;
        uint32_t b = ((pix      ) & 0xFFu) * 0x01010101u;

        PixelAccess::storeU(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) | fill_mask);
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_argb_any_from_prgb32(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * PixelAccess::kSize + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::ForeignFromNative& d = bl_pixel_converter_get_data(self)->foreign_from_native;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];
  uint32_t a_mask = d.masks[3];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];
  uint32_t a_shift = d.shifts[3];

  const uint32_t* unpremultiply_rcp = bl::common_table.unpremultiply_rcp;

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, PixelAccess::kSize) && bl::IntOps::is_aligned(src_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32a(src_data);

        uint32_t a = pix >> 24;
        uint32_t rcp = unpremultiply_rcp[a];

        uint32_t r = ((((pix >> 16) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;
        uint32_t g = ((((pix >>  8) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;
        uint32_t b = ((((pix      ) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;

        a *= 0x01010101u;
        PixelAccess::storeA(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) |
                                     ((a >> a_shift) & a_mask));
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32u(src_data);

        uint32_t a = pix >> 24;
        uint32_t rcp = unpremultiply_rcp[a];

        uint32_t r = ((((pix >> 16) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;
        uint32_t g = ((((pix >>  8) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;
        uint32_t b = ((((pix      ) & 0xFFu) * rcp + 0x8000u) >> 16) * 0x01010101u;

        a *= 0x01010101u;
        PixelAccess::storeU(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) |
                                     ((a >> a_shift) & a_mask));
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

template<typename PixelAccess, bool AlwaysUnaligned>
static BLResult BL_CDECL bl_convert_prgb_any_from_prgb32(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  if (!options)
    options = &bl_pixel_converter_default_options;

  const size_t gap = options->gap;
  dst_stride -= uintptr_t(w) * PixelAccess::kSize + gap;
  src_stride -= uintptr_t(w) * 4;

  const BLPixelConverterData::ForeignFromNative& d = bl_pixel_converter_get_data(self)->foreign_from_native;
  uint32_t r_mask = d.masks[0];
  uint32_t g_mask = d.masks[1];
  uint32_t b_mask = d.masks[2];
  uint32_t a_mask = d.masks[3];

  uint32_t r_shift = d.shifts[0];
  uint32_t g_shift = d.shifts[1];
  uint32_t b_shift = d.shifts[2];
  uint32_t a_shift = d.shifts[3];

  for (uint32_t y = h; y != 0; y--) {
    if (!AlwaysUnaligned && bl::IntOps::is_aligned(dst_data, PixelAccess::kSize) && bl::IntOps::is_aligned(src_data, 4)) {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32a(src_data);

        uint32_t r = ((pix >> 16) & 0xFFu) * 0x01010101u;
        uint32_t g = ((pix >>  8) & 0xFFu) * 0x01010101u;
        uint32_t b = ((pix      ) & 0xFFu) * 0x01010101u;
        uint32_t a = ((pix >> 24)        ) * 0x01010101u;

        PixelAccess::storeA(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) |
                                     ((a >> a_shift) & a_mask));
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }
    else {
      for (uint32_t i = w; i != 0; i--) {
        uint32_t pix = bl::MemOps::readU32u(src_data);

        uint32_t r = ((pix >> 16) & 0xFFu) * 0x01010101u;
        uint32_t g = ((pix >>  8) & 0xFFu) * 0x01010101u;
        uint32_t b = ((pix      ) & 0xFFu) * 0x01010101u;
        uint32_t a = ((pix >> 24)        ) * 0x01010101u;

        PixelAccess::storeU(dst_data, ((r >> r_shift) & r_mask) |
                                     ((g >> g_shift) & g_mask) |
                                     ((b >> b_shift) & b_mask) |
                                     ((a >> a_shift) & a_mask));
        dst_data += PixelAccess::kSize;
        src_data += 4;
      }
    }

    dst_data = bl_pixel_converter_fill_gap(dst_data, gap);
    dst_data += dst_stride;
    src_data += src_stride;
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Init - Utilities
// =====================================

static BL_INLINE BLResult bl_pixel_converter_init_func_generic(BLPixelConverterCore* self, BLPixelConverterFunc func, uint32_t flags = 0) noexcept {
  self->convert_func = func;
  self->internal_flags = uint8_t(flags | BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED);
  return BL_SUCCESS;
}

static BL_INLINE BLResult bl_pixel_converter_init_func_opt(BLPixelConverterCore* self, BLPixelConverterFunc func, uint32_t flags = 0) noexcept {
  self->convert_func = func;
  self->internal_flags = uint8_t(flags | BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED |
                                        BL_PIXEL_CONVERTER_INTERNAL_FLAG_OPTIMIZED);
  return BL_SUCCESS;
}

static uint32_t bl_pixel_converter_calc_rgb_mask32(const BLFormatInfo& fmt_info) noexcept {
  uint32_t mask = 0;
  for (uint32_t i = 0; i < 3; i++)
    if (fmt_info.sizes[i])
      mask |= bl::IntOps::non_zero_lsb_mask<uint32_t>(fmt_info.sizes[i]) << fmt_info.shifts[i];
  return mask;
}

static uint32_t bl_pixel_converter_calc_fill_mask32(const BLFormatInfo& fmt_info) noexcept {
  uint32_t mask = 0;
  for (uint32_t i = 0; i < 4; i++)
    if (fmt_info.sizes[i])
      mask |= bl::IntOps::non_zero_lsb_mask<uint32_t>(fmt_info.sizes[i]) << fmt_info.shifts[i];
  return ~mask;
}

static void bl_pixel_converter_calc_pshufb_predicate_32_from_24(uint32_t out[4], const BLFormatInfo& dst_info, const BLFormatInfo& src_info) noexcept {
  BL_ASSERT(dst_info.depth == 32);
  BL_ASSERT(src_info.depth == 24);

  BL_ASSERT(dst_info.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);
  BL_ASSERT(src_info.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);

  uint32_t r_index = uint32_t(src_info.shifts[0]) / 8u;
  uint32_t gIndex = uint32_t(src_info.shifts[1]) / 8u;
  uint32_t b_index = uint32_t(src_info.shifts[2]) / 8u;

  uint32_t predicate = 0x80808080u;
  predicate ^= (0x80u ^ r_index) << dst_info.shifts[0];
  predicate ^= (0x80u ^ gIndex) << dst_info.shifts[1];
  predicate ^= (0x80u ^ b_index) << dst_info.shifts[2];

  uint32_t increment = (0x03u << dst_info.shifts[0]) |
                       (0x03u << dst_info.shifts[1]) |
                       (0x03u << dst_info.shifts[2]) ;

  out[0] = predicate; predicate += increment;
  out[1] = predicate; predicate += increment;
  out[2] = predicate; predicate += increment;
  out[3] = predicate;
}

static void bl_pixel_converter_calc_pshufb_predicate_32_from_32(uint32_t out[4], const BLFormatInfo& dst_info, const BLFormatInfo& src_info) noexcept {
  BL_ASSERT(dst_info.depth == 32);
  BL_ASSERT(src_info.depth == 32);

  BL_ASSERT(dst_info.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);
  BL_ASSERT(src_info.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);

  uint32_t r_index = uint32_t(src_info.shifts[0]) / 8u;
  uint32_t gIndex = uint32_t(src_info.shifts[1]) / 8u;
  uint32_t b_index = uint32_t(src_info.shifts[2]) / 8u;
  uint32_t a_index = uint32_t(src_info.shifts[3]) / 8u;

  uint32_t predicate = 0x80808080u;
  predicate ^= (0x80u ^ r_index) << dst_info.shifts[0];
  predicate ^= (0x80u ^ gIndex) << dst_info.shifts[1];
  predicate ^= (0x80u ^ b_index) << dst_info.shifts[2];

  uint32_t increment = (0x04u << dst_info.shifts[0]) |
                       (0x04u << dst_info.shifts[1]) |
                       (0x04u << dst_info.shifts[2]) ;

  if (src_info.sizes[3] != 0 && dst_info.sizes[3] != 0) {
    predicate ^= (0x80u ^ a_index) << dst_info.shifts[3];
    increment |= (0x04u         ) << dst_info.shifts[3];
  }

  out[0] = predicate; predicate += increment;
  out[1] = predicate; predicate += increment;
  out[2] = predicate; predicate += increment;
  out[3] = predicate;
}

// bl::PixelConverter - Init - Indexed
// ===================================

static BLResult bl_pixel_converter_init_indexed(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  BLPixelConverterData::IndexedData& d = bl_pixel_converter_get_data(self)->indexed_data;

  // Bail if the source depth doesn't match any supported one.
  if (BL_UNLIKELY(!bl_pixel_converter_is_indexed_depth(si.depth)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLFormatInfo palette_format_info;
  if (BL_UNLIKELY(!bl_pixel_converter_palette_format_from_format_flags(palette_format_info, si.flags)))
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  bool dont_copy_palette = (create_flags & BL_PIXEL_CONVERTER_CREATE_FLAG_DONT_COPY_PALETTE) != 0;
  bool alterable_palette = (create_flags & BL_PIXEL_CONVERTER_CREATE_FLAG_ALTERABLE_PALETTE) != 0;

  // Special case - avoid making the copy of the palette for known conversions.
  if (di.depth == 8 && si.depth == 8 && dont_copy_palette) {
    if ((di.flags & (BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_BYTE_ALIGNED)) == (BL_FORMAT_FLAG_ALPHA | BL_FORMAT_FLAG_BYTE_ALIGNED)) {
      d.convert_func = bl_convert_a8_from_indexed8_pal32;
      d.internal_flags = BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED;
      d.dynamic.table = si.palette;
      return BL_SUCCESS;
    }
  }

  // We need a temporary pixel converter to convert the palette to the destination pixel format. This operation should
  // not allocate any memory as the converter will convert native pixel format (BLRgba32) into a possibly non-native
  // one although a native pixel format is used most of the time.
  BLPixelConverterCore pal_cvt;
  BL_PROPAGATE(bl_pixel_converter_init_internal(&pal_cvt, di, palette_format_info, BL_PIXEL_CONVERTER_CREATE_NO_FLAGS));

  // If the source depth is 8 bits it means that we either use the source format's palette or make a copy of it
  // depending on `create_flags` and the destination format as well.
  void* palette = nullptr;
  uint32_t palette_size = 1u << si.depth;
  uint32_t palette_size_in_bytes = palette_size * (di.depth / 8u);
  uint32_t internal_flags = BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED;

  if (palette_size_in_bytes > sizeof(d.embedded.table8)) {
    if (dont_copy_palette && ((pal_cvt.internal_flags & BL_PIXEL_CONVERTER_INTERNAL_FLAG_RAW_COPY) || alterable_palette)) {
      palette = si.palette;
      d.dynamic.table = palette;
    }
    else {
      palette = malloc(palette_size_in_bytes + sizeof(size_t));
      internal_flags |= BL_PIXEL_CONVERTER_INTERNAL_FLAG_DYNAMIC_DATA;

      if (!palette) {
        bl_pixel_converter_reset(&pal_cvt);
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
      }
    }
  }
  else {
    palette = d.embedded.table8;
  }

  pal_cvt.convert_func(&pal_cvt, static_cast<uint8_t*>(palette), 0, reinterpret_cast<const uint8_t*>(si.palette), 0, palette_size, 1, nullptr);
  bl_pixel_converter_reset(&pal_cvt);

  BLPixelConverterFunc func = nullptr;
  switch (di.depth) {
    case 8:
      switch (si.depth) {
        case 1: func = bl_convert_any_from_indexed1<BLPixelAccess8>; break;
        case 2: func = bl_convert_any_from_indexed2<BLPixelAccess8>; break;
        case 4: func = bl_convert_any_from_indexed4<BLPixelAccess8>; break;
        case 8: func = bl_convert_any_from_indexed8<BLPixelAccess8>; break;
      }
      break;

    case 16:
      switch (si.depth) {
        case 1: func = bl_convert_any_from_indexed1<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>>; break;
        case 2: func = bl_convert_any_from_indexed2<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>>; break;
        case 4: func = bl_convert_any_from_indexed4<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>>; break;
        case 8: func = bl_convert_any_from_indexed8<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>>; break;
      }
      break;

    case 24:
      switch (si.depth) {
        case 1: func = bl_convert_any_from_indexed1<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>>; break;
        case 2: func = bl_convert_any_from_indexed2<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>>; break;
        case 4: func = bl_convert_any_from_indexed4<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>>; break;
        case 8: func = bl_convert_any_from_indexed8<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>>; break;
      }
      break;

    case 32:
      switch (si.depth) {
        case 1: func = bl_convert_any_from_indexed1<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>>; break;
        case 2: func = bl_convert_any_from_indexed2<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>>; break;
        case 4: func = bl_convert_any_from_indexed4<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>>; break;
        case 8: func = bl_convert_any_from_indexed8<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>>; break;
      }
      break;
  }

  d.convert_func = func;
  d.internal_flags = uint8_t(internal_flags);

  if (internal_flags & BL_PIXEL_CONVERTER_INTERNAL_FLAG_DYNAMIC_DATA) {
    size_t* ref_count = bl::PtrOps::offset<size_t>(palette, palette_size_in_bytes);
    *ref_count = 1;

    d.dynamic.table = palette;
    d.dynamic.ref_count = ref_count;
  }

  if (!func) {
    bl_pixel_converter_reset(self);
    return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
  }

  return BL_SUCCESS;
}

// bl::PixelConverter - Init - Simple
// ==================================

static BLResult bl_pixel_converter_init_copy_8888_with_fill_mask(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si) noexcept {
  BLPixelConverterData::MemCopyData& d = bl_pixel_converter_get_data(self)->mem_copy_data;

  d.internal_flags = BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED | BL_PIXEL_CONVERTER_INTERNAL_FLAG_RAW_COPY ;
  d.bytes_per_pixel = uint8_t(di.depth / 8u);

  // Required to handle Copy32, XRGB32<-PRGB32, and PRGB32<-XRGB32 conversions.
  uint32_t common_flags = di.flags & si.flags;
  if (!(common_flags & BL_FORMAT_FLAG_ALPHA)) {
    if (di.flags & BL_FORMAT_FLAG_ALPHA)
      d.fill_mask = 0xFFu << di.shifts[3];
    else
      d.fill_mask = bl_pixel_converter_calc_fill_mask32(di);
  }

#ifdef BL_BUILD_OPT_AVX2
  if (bl_runtime_has_avx2(&bl_runtime_context)) {
    d.convert_func = bl_convert_copy_or_8888_avx2;
    return BL_SUCCESS;
  }
#endif

#ifdef BL_BUILD_OPT_SSE2
  if (bl_runtime_has_sse2(&bl_runtime_context)) {
    d.convert_func = bl_convert_copy_or_8888_sse2;
    return BL_SUCCESS;
  }
#endif

  d.convert_func = bl_convert_copy_or_8888;
  return BL_SUCCESS;
}

static BLResult bl_pixel_converter_init_premultiply_8888(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si) noexcept {
  BLPixelConverterData::PremultiplyData& d = bl_pixel_converter_get_data(self)->premultiply_data;

  bool dst_has_alpha = (di.flags & BL_FORMAT_FLAG_ALPHA) != 0;
  uint32_t a_shift = dst_has_alpha ? di.shifts[3] : si.shifts[3];
  uint32_t fill_mask = uint32_t(dst_has_alpha ? 0 : 0xFF) << a_shift;

  d.alpha_shift = uint8_t(a_shift);
  d.fill_mask = fill_mask;

#ifdef BL_BUILD_OPT_AVX2
  if (bl_runtime_has_avx2(&bl_runtime_context)) {
    if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_trailing_alpha_avx2);
    if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_leading_alpha_avx2);
  }
#endif

#ifdef BL_BUILD_OPT_SSE2
  if (bl_runtime_has_sse2(&bl_runtime_context)) {
    if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_trailing_alpha_sse2);
    if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_leading_alpha_sse2);
  }
#endif

  return bl_pixel_converter_init_func_generic(self, bl_convert_premultiply_8888);
}

static BLResult bl_pixel_converter_init_unpremultiply8888(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si) noexcept {
  bl_unused(si);

  BLPixelConverterData::PremultiplyData& d = bl_pixel_converter_get_data(self)->premultiply_data;

  uint32_t a_shift = di.shifts[3];
  d.alpha_shift = uint8_t(a_shift);

#ifdef BL_BUILD_OPT_AVX2
  if (bl_runtime_has_avx2(&bl_runtime_context)) {
    if (bl_runtime_context.optimization_info.has_fast_pmulld()) {
      if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_trailing_alpha_pmulld_avx2);
      if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_leading_alpha_pmulld_avx2);
    }
    else {
      if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_trailing_alpha_float_avx2);
      if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_leading_alpha_float_avx2);
    }
  }
#endif

#ifdef BL_BUILD_OPT_SSE2
  if (bl_runtime_has_sse2(&bl_runtime_context)) {
    if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_trailing_alpha_sse2);
    if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_unpremultiply_8888_leading_alpha_sse2);
  }
#endif

  if (a_shift == 0) return bl_pixel_converter_init_func_generic(self, bl_convert_unpremultiply_8888<0>);
  if (a_shift == 24) return bl_pixel_converter_init_func_generic(self, bl_convert_unpremultiply_8888<24>);

  return BL_RESULT_NOTHING;
}

static BLResult bl_pixel_converter_init_simple(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(create_flags);

  uint32_t depth = di.depth;
  uint32_t common_flags = di.flags & si.flags;

  const uint32_t kA = BL_FORMAT_FLAG_ALPHA;
  const uint32_t kP = BL_FORMAT_FLAG_PREMULTIPLIED;

  if (bl::FormatInternal::has_same_rgb_layout(di, si)) {
    if (bl::FormatInternal::has_same_alpha_layout(di, si)) {
      // Memory copy.
      if (di.flags == si.flags) {
        // Don't copy undefined bytes in 8888 formats, it's better to set them to 0xFF.
        if (depth == 32 && !(di.flags & BL_FORMAT_FLAG_ALPHA) && (di.flags & BL_FORMAT_FLAG_UNDEFINED_BITS))
          return bl_pixel_converter_init_copy_8888_with_fill_mask(self, di, si);

        BLPixelConverterData::MemCopyData& d = bl_pixel_converter_get_data(self)->mem_copy_data;
        d.internal_flags = BL_PIXEL_CONVERTER_INTERNAL_FLAG_INITIALIZED |
                          BL_PIXEL_CONVERTER_INTERNAL_FLAG_RAW_COPY ;
        d.bytes_per_pixel = uint8_t(di.depth / 8u);

#ifdef BL_BUILD_OPT_AVX2
        if (bl_runtime_has_avx2(&bl_runtime_context)) {
          d.convert_func = bl_convert_copy_avx2;
          return BL_SUCCESS;
        }
#endif

#ifdef BL_BUILD_OPT_SSE2
        if (bl_runtime_has_sse2(&bl_runtime_context)) {
          d.convert_func = bl_convert_copy_sse2;
          return BL_SUCCESS;
        }
#endif

        d.convert_func = bl_convert_copy;
        return BL_SUCCESS;
      }

      // Premultiply / Unpremultiply.
      if (bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_BYTE_ALIGNED) && di.flags == (si.flags ^ kP)) {
        // Premultiply / Unpremultiply: 32-bit format where the alpha is either first or last.
        if (depth == 32) {
          // If we can do any alpha index it's okay, but generally prefer only
          // AlphaFirst|AlphaLast - other layouts are very unlikely to be used.
          if (di.flags & kP)
            BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_premultiply_8888(self, di, si));
          else
            BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_unpremultiply8888(self, di, si));
        }
      }
    }
    else if (depth == 32 && bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_BYTE_ALIGNED)) {
      // Copy:
      //   PRGB32 <- XRGB32 - Copy with or-mask.
      //   ARGB32 <- XRGB32 - Copy with or-mask.
      //   XRGB32 <- PRGB32 - Copy with or-mask.
      if ((!(di.flags & kA) && (si.flags & kP)) || (!(si.flags & kA) && (di.flags & kA)))
        return bl_pixel_converter_init_copy_8888_with_fill_mask(self, di, si);

      // Premultiply:
      //   XRGB32 <- ARGB32 - Premultiply with or-mask.
      if (!(di.flags & kA) && (si.flags & kA))
        return bl_pixel_converter_init_premultiply_8888(self, di, si);
    }
  }
  else {
#ifdef BL_BUILD_OPT_SSSE3
    if (bl_runtime_has_ssse3(&bl_runtime_context)) {
      if (depth == 32 && bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_BYTE_ALIGNED)) {
        // Handle the following conversions (PSHUFB|OR):
        //   XRGB32 <- XRGB32 - Shuffle with or-mask
        //   ARGB32 <- XRGB32 - Shuffle with or-mask (opaque alpha)
        //   PRGB32 <- XRGB32 - Shuffle with or-mask (opaque alpha)
        //   ARGB32 <- ARGB32 - Shuffle
        //   XRGB32 <- PRGB32 - Shuffle with or-mask (no unpremultiply)
        //   PRGB32 <- PRGB32 - Shuffle
        bool same_alpha = (di.flags & (kA | kP)) == (si.flags & (kA | kP));
        bool dst_alpha = (di.flags & kA) != 0;
        bool src_alpha = (si.flags & kA) != 0;

        if (same_alpha || !src_alpha || (!dst_alpha && bl::IntOps::bit_match(si.flags, kP))) {
          BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
          bl_pixel_converter_calc_pshufb_predicate_32_from_32(d.shufb_predicate, di, si);

          if (!(di.flags & kA))
            d.fill_mask = bl_pixel_converter_calc_fill_mask32(di);
          else if (!(si.flags & kA))
            d.fill_mask = 0xFFu << di.shifts[3];

#ifdef BL_BUILD_OPT_AVX2
          if (bl_runtime_has_avx2(&bl_runtime_context))
            return bl_pixel_converter_init_func_opt(self, bl_convert_copy_shufb_8888_avx2);
#endif

          return bl_pixel_converter_init_func_opt(self, bl_convert_copy_shufb_8888_ssse3);
        }

        // Handle the following conversions (Premultiply|Shufb)
        //   PRGB32 <- ARGB32 - Shuffle with premultiply
        //   XRGB32 <- ARGB32 - Shuffle with premultiply
        if (((di.flags & kP) || !(di.flags & kA)) && (si.flags & (kA | kP)) == kA) {
          uint32_t a_shift = di.shifts[3];

          BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
          bl_pixel_converter_calc_pshufb_predicate_32_from_32(d.shufb_predicate, di, si);

#ifdef BL_BUILD_OPT_AVX2
          if (bl_runtime_has_avx2(&bl_runtime_context)) {
            if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_trailing_alpha_shufb_avx2);
            if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_leading_alpha_shufb_avx2);
          }
#endif

          if (a_shift == 0) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_trailing_alpha_shufb_ssse3);
          if (a_shift == 24) return bl_pixel_converter_init_func_opt(self, bl_convert_premultiply_8888_leading_alpha_shufb_ssse3);
        }
      }
    }
#endif
  }

  return BL_RESULT_NOTHING;
}

// bl::PixelConverter - Init - 8 From 8888
// =======================================

static BLResult bl_pixel_converter_init_8_from_8888(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(create_flags);
  BLPixelConverterData::X8FromRgb32Data& d = bl_pixel_converter_get_data(self)->x8FromRgb32Data;

  uint32_t common_flags = di.flags & si.flags;
  if (bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_ALPHA | BL_FORMAT_FLAG_BYTE_ALIGNED)) {
    d.bytes_per_pixel = uint8_t(si.depth / 8u);
    d.alpha_shift = uint8_t(si.shifts[3]);
    return bl_pixel_converter_init_func_generic(self, bl_convert_a8_from_8888);
  }

  return BL_RESULT_NOTHING;
}

// bl::PixelConverter - Init - 8888 From 8
// =======================================

static BLResult bl_pixel_converter_init_8888_from_8(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(create_flags);
  uint32_t rgb_mask = bl_pixel_converter_calc_rgb_mask32(di);

  BLPixelConverterData::Rgb32FromX8Data& d = bl_pixel_converter_get_data(self)->rgb32FromX8Data;
  d.zero_mask = 0xFFFFFFFFu;

  if (!(si.flags & BL_FORMAT_FLAG_ALPHA)) {
    // ?RGB32 <- L8.
    d.fill_mask = ~rgb_mask;
  }
  else if (bl::IntOps::bit_match(di.flags, BL_FORMAT_FLAG_ALPHA | BL_FORMAT_FLAG_PREMULTIPLIED)) {
    // PRGB32 <- A8 - RGB channels are set to A, alpha channel is kept.
  }
  else if (bl::IntOps::bit_match(di.flags, BL_FORMAT_FLAG_ALPHA)) {
    // ARGB32 <- A8 - RGB channels are set to 255, alpha channel is kept.
    d.fill_mask = rgb_mask;
  }
  else {
    // XRGB32 <- A8 - RGB channels are set to A, alpha channel is set to 255.
    d.fill_mask = ~rgb_mask;
  }

#ifdef BL_BUILD_OPT_SSE2
  if (bl_runtime_has_sse2(&bl_runtime_context))
    return bl_pixel_converter_init_func_opt(self, bl_convert_8888_from_x8_sse2);
#endif

  return bl_pixel_converter_init_func_generic(self, bl_convert_8888_from_x8);
}

// bl::PixelConverter - Init - 8888 From 888
// =========================================

static BLResult bl_pixel_converter_init_8888_from_888(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(self, create_flags);

  uint32_t common_flags = di.flags & si.flags;

  // Maybe unused if there are no optimizations.
  bl_unused(common_flags);

  // This is only possible with SSSE3 and AVX2 enabled converters.
#ifdef BL_BUILD_OPT_SSSE3
  if (bl_runtime_has_ssse3(&bl_runtime_context)) {
    // We expect both formats to provide RGB components and to be BYTE aligned.
    if (!(common_flags & BL_FORMAT_FLAG_RGB))
      return BL_RESULT_NOTHING;

    BLPixelConverterData::ShufbData& d = bl_pixel_converter_get_data(self)->shufb_data;
    d.fill_mask = ~bl_pixel_converter_calc_rgb_mask32(di);
    bl_pixel_converter_calc_pshufb_predicate_32_from_24(d.shufb_predicate, di, si);

#ifdef BL_BUILD_OPT_AVX2
    if (bl_runtime_has_avx2(&bl_runtime_context))
      return bl_pixel_converter_init_func_opt(self, bl_convert_rgb32_from_rgb24_shufb_avx2);
#endif

    return bl_pixel_converter_init_func_opt(self, bl_convert_rgb32_from_rgb24_shufb_ssse3);
  }
#endif

  return BL_RESULT_NOTHING;
}

// bl::PixelConverter - Init - NativeFromForeign
// =============================================

static BLResult bl_pixel_converter_init_8888_from_foreign(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(create_flags);

  BL_ASSERT(di.depth == 32);
  BL_ASSERT(di.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);

  if (di.r_shift != 16 || di.g_shift != 8 || di.b_shift != 0)
    return BL_RESULT_NOTHING;

  BLPixelConverterData::NativeFromForeign& d = bl_pixel_converter_get_data(self)->native_from_foreign;

  bool isSrcRGBA           = (si.flags & BL_FORMAT_FLAG_ALPHA) != 0;
  bool is_src_premultiplied  = (si.flags & BL_FORMAT_FLAG_PREMULTIPLIED) != 0;
  bool hasSrcHostBO        = (si.flags & BL_FORMAT_FLAG_BYTE_SWAP) == 0;

  if (!isSrcRGBA)
    d.fill_mask = 0xFF000000u;

  for (uint32_t i = 0; i < 4; i++) {
    uint32_t size = si.sizes[i];
    uint32_t shift = si.shifts[i];

    d.masks[i] = 0;
    d.shifts[i] = uint8_t(shift);
    d.scale[i] = 0;

    if (size == 0)
      continue;

    // Discard all bits that are below 8 most significant ones.
    if (size > 8) {
      shift += (size - 8);
      size = 8;
    }

    d.masks[i] = bl::IntOps::non_zero_lsb_mask<uint32_t>(size);
    d.shifts[i] = uint8_t(shift);

    // Calculate a scale constant that will be used to expand bits in case that the source contains less than 8 bits.
    // We do it by adding `size`  to the `scaled_size` until we reach the required bit-depth.
    uint32_t scale = 0x1;
    uint32_t scaled_size = size;

    while (scaled_size < 8) {
      scale = (scale << size) | 1;
      scaled_size += size;
    }

    // Shift scale in a way that it contains MSB of the mask and the right position.
    uint32_t scaled_shift = bl_pixel_converter_native32_from_foreign_shift_table[i] - (scaled_size - 8);
    scale <<= scaled_shift;
    d.scale[i] = scale;
  }

  // Special case of converting LUM to RGB.
  if (si.flags & BL_FORMAT_FLAG_LUM) {
    // TODO:
  }

  // Generic conversion.
  BLPixelConverterFunc func = nullptr;
  switch (si.depth) {
    case 16:
      // TODO:
      if (is_src_premultiplied)
        func = hasSrcHostBO ? bl_convert_prgb32_from_prgb_any<BLPixelAccess16<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem16>
                            : bl_convert_prgb32_from_prgb_any<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
      else if (isSrcRGBA)
        func = hasSrcHostBO ? bl_convert_prgb32_from_argb_any<BLPixelAccess16<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem16>
                            : bl_convert_prgb32_from_argb_any<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
      else
        func = hasSrcHostBO ? bl_convert_xrgb32_from_xrgb_any<BLPixelAccess16<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem16>
                            : bl_convert_xrgb32_from_xrgb_any<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
      break;

    case 24:
      if (is_src_premultiplied)
        func = hasSrcHostBO ? bl_convert_prgb32_from_prgb_any<BLPixelAccess24<BL_BYTE_ORDER_NATIVE >, true>
                            : bl_convert_prgb32_from_prgb_any<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
      else if (isSrcRGBA)
        func = hasSrcHostBO ? bl_convert_prgb32_from_argb_any<BLPixelAccess24<BL_BYTE_ORDER_NATIVE >, true>
                            : bl_convert_prgb32_from_argb_any<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
      else
        func = hasSrcHostBO ? bl_convert_xrgb32_from_xrgb_any<BLPixelAccess24<BL_BYTE_ORDER_NATIVE >, true>
                            : bl_convert_xrgb32_from_xrgb_any<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
      break;

    case 32:
      if (is_src_premultiplied)
        func = hasSrcHostBO ? bl_convert_prgb32_from_prgb_any<BLPixelAccess32<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem32>
                            : bl_convert_prgb32_from_prgb_any<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
      else if (isSrcRGBA)
        func = hasSrcHostBO ? bl_convert_prgb32_from_argb_any<BLPixelAccess32<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem32>
                            : bl_convert_prgb32_from_argb_any<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
      else
        func = hasSrcHostBO ? bl_convert_xrgb32_from_xrgb_any<BLPixelAccess32<BL_BYTE_ORDER_NATIVE >, bl::MemOps::kUnalignedMem32>
                            : bl_convert_xrgb32_from_xrgb_any<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
      break;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  return bl_pixel_converter_init_func_generic(self, func);
}

// bl::PixelConverter - Init - ForeignFromNative
// =============================================

static BLResult bl_pixel_converter_init_foreign_from_8888(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(si, create_flags);

  BL_ASSERT(si.depth == 32);
  BL_ASSERT(si.flags & BL_FORMAT_FLAG_BYTE_ALIGNED);

  if (si.r_shift != 16 || si.g_shift != 8 || si.b_shift != 0)
    return BL_RESULT_NOTHING;

  if (di.flags & BL_FORMAT_FLAG_INDEXED) {
    // TODO:
    return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
  }
  else {
    BLPixelConverterData::ForeignFromNative& d = bl_pixel_converter_get_data(self)->foreign_from_native;

    bool isDstRGBA          = (di.flags & BL_FORMAT_FLAG_ALPHA) != 0;
    bool is_dst_premultiplied = (di.flags & BL_FORMAT_FLAG_PREMULTIPLIED) != 0;
    bool hasDstHostBO       = (di.flags & BL_FORMAT_FLAG_BYTE_SWAP) == 0;

    for (uint32_t i = 0; i < 4; i++) {
      uint32_t mask = 0;
      uint32_t size = di.sizes[i];
      uint32_t shift = di.shifts[i];

      if (size != 0) {
        mask = bl::IntOps::non_zero_lsb_mask<uint32_t>(size) << shift;
        shift = 32 - size - shift;
      }

      d.masks[i] = mask;
      d.shifts[i] = uint8_t(shift);
    }

    BLPixelConverterFunc func = nullptr;
    switch (di.depth) {
      case 16:
        if (is_dst_premultiplied)
          func = hasDstHostBO ? bl_convert_prgb_any_from_prgb32<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem16>
                              : bl_convert_prgb_any_from_prgb32<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
        else if (isDstRGBA)
          func = hasDstHostBO ? bl_convert_argb_any_from_prgb32<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem16>
                              : bl_convert_argb_any_from_prgb32<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
        else
          func = hasDstHostBO ? bl_convert_xrgb_any_from_xrgb32<BLPixelAccess16<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem16>
                              : bl_convert_xrgb_any_from_xrgb32<BLPixelAccess16<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem16>;
        break;

      case 24:
        if (is_dst_premultiplied)
          func = hasDstHostBO ? bl_convert_prgb_any_from_prgb32<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>, true>
                              : bl_convert_prgb_any_from_prgb32<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
        else if (isDstRGBA)
          func = hasDstHostBO ? bl_convert_argb_any_from_prgb32<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>, true>
                              : bl_convert_argb_any_from_prgb32<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
        else
          func = hasDstHostBO ? bl_convert_xrgb_any_from_xrgb32<BLPixelAccess24<BL_BYTE_ORDER_NATIVE>, true>
                              : bl_convert_xrgb_any_from_xrgb32<BLPixelAccess24<BL_BYTE_ORDER_SWAPPED>, true>;
        break;

      case 32:
        if (is_dst_premultiplied)
          func = hasDstHostBO ? bl_convert_prgb_any_from_prgb32<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem32>
                              : bl_convert_prgb_any_from_prgb32<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
        else if (isDstRGBA)
          func = hasDstHostBO ? bl_convert_argb_any_from_prgb32<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem32>
                              : bl_convert_argb_any_from_prgb32<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
        else
          func = hasDstHostBO ? bl_convert_xrgb_any_from_xrgb32<BLPixelAccess32<BL_BYTE_ORDER_NATIVE>, bl::MemOps::kUnalignedMem32>
                              : bl_convert_xrgb_any_from_xrgb32<BLPixelAccess32<BL_BYTE_ORDER_SWAPPED>, bl::MemOps::kUnalignedMem32>;
        break;

      default:
        return bl_make_error(BL_ERROR_INVALID_VALUE);
    }

    return bl_pixel_converter_init_func_generic(self, func);
  }
}

// bl::PixelConverter - Init - Multi-Step
// ======================================

static BLResult BL_CDECL bl_convert_multi_step(
  const BLPixelConverterCore* self,
  uint8_t* dst_data, intptr_t dst_stride,
  const uint8_t* src_data, intptr_t src_stride, uint32_t w, uint32_t h, const BLPixelConverterOptions* options) noexcept {

  const BLPixelConverterData::MultiStepData& d = bl_pixel_converter_get_data(self)->multi_step_data;
  uint32_t intermediate_pixel_count = d.intermediate_pixel_count;

  // NOTE: We use uintptr_t so the buffer gets properly aligned. In general we don't need a higher alignment than
  // 32-bit or 64-bit depending on the target.
  uintptr_t intermediate_storage[BL_PIXEL_CONVERTER_MULTISTEP_BUFFER_SIZE / sizeof(uintptr_t)];
  uint8_t* intermediate_data = reinterpret_cast<uint8_t*>(intermediate_storage);

  const BLPixelConverterMultiStepContext* ctx = d.ctx;
  BLPixelConverterFunc src_to_intermediate = ctx->first.convert_func;
  BLPixelConverterFunc intermediate_to_dst = ctx->second.convert_func;

  if (!options)
    options = &bl_pixel_converter_default_options;
  BLPixelConverterOptions work_opt = *options;

  if (w > intermediate_pixel_count) {
    // Process part of the scanline at a time.
    uint8_t* dst_line = dst_data;
    const uint8_t* src_line = src_data;

    int base_origin_x = work_opt.origin.x;
    uint32_t dst_bytes_per_pixel = d.dst_bytes_per_pixel;
    uint32_t src_bytes_per_pixel = d.src_bytes_per_pixel;

    for (uint32_t y = h; y; y--) {
      uint32_t i = w;

      work_opt.origin.x = base_origin_x;
      dst_data = dst_line;
      src_data = src_line;

      while (i) {
        uint32_t n = bl_min(i, intermediate_pixel_count);

        src_to_intermediate(&ctx->first, intermediate_data, 0, src_data, src_stride, n, 1, nullptr);
        intermediate_to_dst(&ctx->second, dst_data, dst_stride, intermediate_data, 0, n, 1, &work_opt);

        dst_data += n * dst_bytes_per_pixel;
        src_data += n * src_bytes_per_pixel;
        work_opt.origin.x += int(n);

        i -= n;
      }

      dst_line += dst_stride;
      src_line += src_stride;
      work_opt.origin.y++;
    }

    return BL_SUCCESS;
  }
  else if (h > intermediate_pixel_count || w * h > intermediate_pixel_count) {
    // Process at least one scanline at a time.
    for (uint32_t y = h; y; y--) {
      src_to_intermediate(&ctx->first, intermediate_data, 0, src_data, src_stride, w, 1, nullptr);
      intermediate_to_dst(&ctx->second, dst_data, dst_stride, intermediate_data, 0, w, 1, &work_opt);

      dst_data += dst_stride;
      src_data += src_stride;
      work_opt.origin.y++;
    }

    return BL_SUCCESS;
  }
  else {
    // Process all scanlines as the `intermediate_buffer` is large enough.
    intptr_t intermediate_stride = intptr_t(w) * d.intermediate_bytes_per_pixel;
    src_to_intermediate(&ctx->first, intermediate_data, intermediate_stride, src_data, src_stride, w, h, nullptr);
    return intermediate_to_dst(&ctx->second, dst_data, dst_stride, intermediate_data, intermediate_stride, w, h, &work_opt);
  }
}

static BLResult bl_pixel_converter_init_multi_step_internal(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& intermediate, const BLFormatInfo& si) noexcept {
  BLPixelConverterMultiStepContext* ctx =
    static_cast<BLPixelConverterMultiStepContext*>(malloc(sizeof(BLPixelConverterMultiStepContext)));

  if (BL_UNLIKELY(!ctx))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  BLResult result;
  BLPixelConverterCreateFlags custom_flags = BL_PIXEL_CONVERTER_CREATE_FLAG_NO_MULTI_STEP;

  memset(ctx, 0, sizeof(*ctx));
  if ((result = bl_pixel_converter_init_internal(&ctx->first, intermediate, si, custom_flags)) != BL_SUCCESS ||
      (result = bl_pixel_converter_init_internal(&ctx->first, di, intermediate, custom_flags)) != BL_SUCCESS) {
    bl_pixel_converter_reset(&ctx->first);
    bl_pixel_converter_reset(&ctx->second);
    free(ctx);
    return result;
  }

  BLPixelConverterData::MultiStepData& d = bl_pixel_converter_get_data(self)->multi_step_data;
  d.dst_bytes_per_pixel = uint8_t(di.depth / 8u);
  d.src_bytes_per_pixel = uint8_t(si.depth / 8u);
  d.intermediate_bytes_per_pixel = uint8_t(intermediate.depth / 8u);
  d.intermediate_pixel_count = BL_PIXEL_CONVERTER_MULTISTEP_BUFFER_SIZE / d.intermediate_bytes_per_pixel;

  ctx->ref_count = 1;
  d.ref_count = (size_t*)&ctx->ref_count;
  d.ctx = ctx;

  uint32_t internal_flags = BL_PIXEL_CONVERTER_INTERNAL_FLAG_MULTI_STEP |
                           BL_PIXEL_CONVERTER_INTERNAL_FLAG_DYNAMIC_DATA;
  return bl_pixel_converter_init_func_generic(self, bl_convert_multi_step, internal_flags);
}

static BLResult bl_pixel_converter_init_multi_step(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  bl_unused(create_flags);

  // We have foreign pixel formats on both input and output. This means that we will create two converters and
  // convert through a native pixel format as otherwise it would not be possible to convert the pixels by using
  // built-in converters.

  const uint32_t kA = BL_FORMAT_FLAG_ALPHA;
  const uint32_t kP = BL_FORMAT_FLAG_PREMULTIPLIED;

  uint32_t common_flags = di.flags & si.flags;
  if (common_flags & BL_FORMAT_FLAG_RGB) {
    // Temporary format information.
    BLFormatInfo intermediate = bl_format_info[BL_FORMAT_PRGB32];
    if ((di.flags & (kA | kP)) == kA)
      intermediate.clear_flags(BL_FORMAT_FLAG_PREMULTIPLIED);
    if (!(di.flags & kA) || !(si.flags & kA))
      intermediate = bl_format_info[BL_FORMAT_XRGB32];
    return bl_pixel_converter_init_multi_step_internal(self, di, intermediate, si);
  }

  return BL_RESULT_NOTHING;
}

// bl::PixelConverter - Init - Internal
// ====================================

BLResult bl_pixel_converter_init_internal(BLPixelConverterCore* self, const BLFormatInfo& di, const BLFormatInfo& si, BLPixelConverterCreateFlags create_flags) noexcept {
  uint32_t common_flags = di.flags & si.flags;
  // Convert - Indexed destination is not supported.
  if (di.flags & BL_FORMAT_FLAG_INDEXED)
    return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);

  // Convert - Any from Indexed.
  if (si.flags & BL_FORMAT_FLAG_INDEXED)
    return bl_pixel_converter_init_indexed(self, di, si, create_flags);

  // Convert - MemCopy | Native | ShufB | Premultiply | Unpremultiply.
  if (di.depth == si.depth)
    BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_simple(self, di, si, create_flags));

  if (di.depth == 8 && si.depth == 32) {
    // Convert - A8 <- ARGB32|PRGB32.
    if (bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_ALPHA | BL_FORMAT_FLAG_BYTE_ALIGNED))
      BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_8_from_8888(self, di, si, create_flags));
  }

  // Convert - ?RGB32 <- A8|L8.
  if (di.depth == 32 && si.depth == 8) {
    if (bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_BYTE_ALIGNED) && (di.flags & BL_FORMAT_FLAG_RGB) != 0)
      BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_8888_from_8(self, di, si, create_flags));
  }

  // Convert - ?RGB32 <- RGB24.
  if (di.depth == 32 && si.depth == 24) {
    if (bl::IntOps::bit_match(common_flags, BL_FORMAT_FLAG_BYTE_ALIGNED | BL_FORMAT_FLAG_RGB))
      BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_8888_from_888(self, di, si, create_flags));
  }

  // Convert - ?RGB32 <- Foreign.
  if (di.depth == 32 && bl::IntOps::bit_match(di.flags, BL_FORMAT_FLAG_BYTE_ALIGNED))
    BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_8888_from_foreign(self, di, si, create_flags));

  // Convert - Foreign <- ?RGB32.
  if (si.depth == 32 && bl::IntOps::bit_match(si.flags, BL_FORMAT_FLAG_BYTE_ALIGNED))
    BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_foreign_from_8888(self, di, si, create_flags));

  // Convert - Foreign <- Foreign.
  if (!(create_flags & BL_PIXEL_CONVERTER_CREATE_FLAG_NO_MULTI_STEP))
    BL_PROPAGATE_IF_NOT_NOTHING(bl_pixel_converter_init_multi_step(self, di, si, create_flags));

  // Probably extreme case that is not implemented.
  return bl_make_error(BL_ERROR_NOT_IMPLEMENTED);
}
