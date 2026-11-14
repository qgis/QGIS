// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/rgba.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/qoicodec_p.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/lookuptable_p.h>

#if BL_TARGET_ARCH_BITS >= 64
  #define BL_QOI_USE_64_BIT_ARITHMETIC
#endif

namespace bl::Qoi {

// bl::Qoi::Codec - Globals
// ========================

static BLObjectEternalVirtualImpl<BLQoiCodecImpl, BLImageCodecVirt> qoi_codec;
static BLImageCodecCore qoi_codec_instance;
static BLImageDecoderVirt qoi_decoder_virt;
static BLImageEncoderVirt qoi_encoder_virt;

// bl::Qoi::Codec - Constants
// ==========================

static constexpr size_t kQoiHeaderSize = 14;
static constexpr size_t kQoiMagicSize = 4;
static constexpr size_t kQoiEndMarkerSize = 8;

static constexpr uint8_t kQoiOpIndex = 0x00; // 00xxxxxx
static constexpr uint8_t kQoiOpDiff  = 0x40; // 01xxxxxx
static constexpr uint8_t kQoiOpLuma  = 0x80; // 10xxxxxx
static constexpr uint8_t kQoiOpRun   = 0xC0; // 11xxxxxx
static constexpr uint8_t kQoiOpRgb   = 0xFE; // 11111110
static constexpr uint8_t kQoiOpRgba  = 0xFF; // 11111111

static constexpr uint32_t kQoiHashR = 3u;
static constexpr uint32_t kQoiHashG = 5u;
static constexpr uint32_t kQoiHashB = 7u;
static constexpr uint32_t kQoiHashA = 11u;
static constexpr uint32_t kQoiHashMask = 0x3Fu;

static constexpr uint8_t qoi_magic[kQoiMagicSize] = { 'q', 'o', 'i', 'f' };
static constexpr uint8_t qoi_end_marker[kQoiEndMarkerSize] = { 0, 0, 0, 0, 0, 0, 0, 1 };

// Lookup table generator that generates delta values for QOI_OP_DIFF and the first byte of QOI_OP_LUMA.
// Additionally, it provides values for a RLE run of a single pixel for a possible experimentation.
struct IndexDiffLumaTableGen {
  static constexpr uint32_t rgb(uint32_t r, uint32_t g, uint32_t b, uint32_t luma_mask) noexcept {
    return ((r & 0xFFu) << 24) |
           ((g & 0xFFu) << 16) |
           ((b & 0xFFu) <<  8) |
           ((luma_mask ) <<  0) ;
  }

  static constexpr uint32_t diff(uint32_t b0) noexcept {
    return rgb(((b0 >> 4) & 0x3u) - 2u, ((b0 >> 2) & 0x3u) - 2u, ((b0 >> 0) & 0x3u) - 2u, 0x00u);
  }

  static constexpr uint32_t luma(uint32_t b0) noexcept {
    return rgb(b0 - 40u, b0 - 32u, b0 - 40u, 0xFF);
  }

  static constexpr uint32_t value(size_t idx) noexcept {
    return idx < 64u  ? diff(uint32_t(idx      )) :
           idx < 128u ? luma(uint32_t(idx - 64u)) : 0u;
  }
};

static constexpr LookupTable<uint32_t, 129> qoiIndexDiffLumaLUT = make_lookup_table<uint32_t, 129, IndexDiffLumaTableGen>();

// bl::Qoi::Codec - Hashing
// ========================

static BL_INLINE uint32_t hashPixelAGxRBx64(uint64_t ag_rb) noexcept {
  ag_rb *= (uint64_t(kQoiHashA) << ( 8 + 2)) + (uint64_t(kQoiHashG) << (24 + 2)) +
           (uint64_t(kQoiHashR) << (40 + 2)) + (uint64_t(kQoiHashB) << (56 + 2)) ;
  return uint32_t(ag_rb >> 58);
}

static BL_INLINE uint32_t hashPixelAGxRBx32(uint32_t ag, uint32_t rb) noexcept {
  ag *= ((kQoiHashA << (0 + 2)) + (kQoiHashG << (16 + 2)));
  rb *= ((kQoiHashR << (8 + 2)) + (kQoiHashB << (24 + 2)));

  return (ag + rb) >> 26;
}

static BL_INLINE uint32_t hashPixelRGBA32(uint32_t pixel) noexcept {
#if defined(BL_QOI_USE_64_BIT_ARITHMETIC)
  return hashPixelAGxRBx64(((uint64_t(pixel) << 24) | pixel) & 0x00FF00FF00FF00FFu);
#else
  return hashPixelAGxRBx32(pixel & 0xFF00FF00u, pixel & 0x00FF00FFu);
#endif
}

static BL_INLINE uint32_t hashPixelA8(uint32_t a) noexcept {
  return (0xFFu * (kQoiHashR + kQoiHashG + kQoiHashB) + a * kQoiHashA) & kQoiHashMask;
}

// bl::Qoi::Codec - UnpackedPixel
// ==============================

#if defined(BL_QOI_USE_64_BIT_ARITHMETIC)
struct UnpackedPixel {
  uint64_t ag_rb; // Represents 0x00AA00GG00RR00BB.

  static BL_INLINE UnpackedPixel unpack(uint32_t packed) noexcept {
    return UnpackedPixel{((uint64_t(packed) << 24) | packed) & 0x00FF00FF00FF00FFu};
  }

  static BL_INLINE UnpackedPixel unpackRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) noexcept {
    return UnpackedPixel{(uint64_t(a) << 48) | (uint64_t(g) << 32) | (uint64_t(r) << 16) | (uint64_t(b) << 0)};
  }

  template<bool kHasAlpha>
  BL_INLINE uint32_t pack() const noexcept {
    uint32_t rgba32 = uint32_t(ag_rb >> 24) | uint32_t(ag_rb & 0xFFFFFFFFu);
    if constexpr (kHasAlpha)
      return PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32);
    else
      return rgba32 | 0xFF000000u;
  }

  BL_INLINE uint32_t hash() const noexcept { return hashPixelAGxRBx64(ag_rb); }

  BL_INLINE void add(const UnpackedPixel& other) noexcept { ag_rb += other.ag_rb; }
  BL_INLINE void addRB(uint32_t value) noexcept { ag_rb += value; }
  BL_INLINE void mask() noexcept { ag_rb &= 0x00FF00FF00FF00FFu; }

  BL_INLINE void opRGBX(uint32_t hbyte0, const UnpackedPixel& other) noexcept {
    uint64_t msk = uint64_t(hbyte0 + 1) << 48;
    ag_rb = (ag_rb & msk) | (other.ag_rb & ~msk);
  }
};
#else
struct UnpackedPixel {
  uint32_t ag; // Represents 0xAA00GG00.
  uint32_t rb; // Represents 0x00RR00BB.

  static BL_INLINE UnpackedPixel unpack(uint32_t packed) noexcept {
    return UnpackedPixel{packed & 0xFF00FF00u, packed & 0x00FF00FFu };
  }

  static BL_INLINE UnpackedPixel unpackRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) noexcept {
    return UnpackedPixel{
      (uint32_t(a) << 24) | (uint32_t(g) << 8), // AG
      (uint32_t(r) << 16) | (uint32_t(b) << 0)  // RB
    };
  }

  template<bool kHasAlpha>
  BL_INLINE uint32_t pack() const noexcept {
    uint32_t rgba32 = ag | rb;

    if (kHasAlpha)
      return PixelOps::Scalar::cvt_prgb32_8888_from_argb32_8888(rgba32);
    else
      return rgba32 | 0xFF000000u;
  }

  BL_INLINE uint32_t hash() const noexcept { return hashPixelAGxRBx32(ag, rb); }

  BL_INLINE void add(const UnpackedPixel& other) noexcept { ag += other.ag; rb += other.rb; }
  BL_INLINE void addRB(uint32_t value) noexcept { rb += value; }
  BL_INLINE void mask() noexcept { ag &= 0xFF00FF00u; rb &= 0x00FF00FFu; }

  BL_INLINE void opRGBX(uint32_t hbyte0, const UnpackedPixel& other) noexcept {
    uint32_t msk = uint32_t(hbyte0 + 1) << 24;
    ag = (ag & msk) | (other.ag & ~msk);
    rb = other.rb;
  }
};
#endif

// bl::Qoi::Codec - Utilities
// ==========================

static BL_INLINE uint32_t* fillRgba32(uint32_t* dst, uint32_t value, size_t count) noexcept {
  for (size_t i = 0; i < count; i++)
    dst[i] = value;
  return dst + count;
}

// bl::Qoi::Decoder - Read Info (Internal)
// =======================================

// struct qoi_header {
//   char magic[4];      // magic bytes "qoif"
//   uint32_t width;     // image width in pixels (BE)
//   uint32_t height;    // image height in pixels (BE)
//   uint8_t channels;   // 3 = RGB, 4 = RGBA
//   uint8_t colorspace; // 0 = sRGB with linear alpha, 1 = all channels linear
// };

static BLResult decoder_read_info_internal(BLQoiDecoderImpl* decoder_impl, const uint8_t* data, size_t size) noexcept {
  if (size < kQoiHeaderSize) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  if (memcmp(qoi_magic, data, kQoiMagicSize) != 0) {
    return bl_make_error(BL_ERROR_INVALID_SIGNATURE);
  }

  uint32_t w = bl::MemOps::readU32uBE(data + 4);
  uint32_t h = bl::MemOps::readU32uBE(data + 8);

  if (w == 0 || h == 0) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  uint8_t channels = data[12];
  uint8_t colorspace = data[13];

  if (channels != 3u && channels != 4u) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  if (colorspace > 1u) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  if (w > BL_RUNTIME_MAX_IMAGE_SIZE || h > BL_RUNTIME_MAX_IMAGE_SIZE) {
    return bl_make_error(BL_ERROR_IMAGE_TOO_LARGE);
  }

  decoder_impl->buffer_index = 14u;
  decoder_impl->image_info.reset();
  decoder_impl->image_info.size.reset(int32_t(w), int32_t(h));
  decoder_impl->image_info.depth = uint16_t(channels * 8);
  decoder_impl->image_info.plane_count = 1;
  decoder_impl->image_info.frame_count = 1;

  memcpy(decoder_impl->image_info.format, "QOI", 4);
  memcpy(decoder_impl->image_info.compression, "RLE", 4);

  return BL_SUCCESS;
}

template<bool kHasAlpha>
static BL_INLINE BLResult decode_qoi_data(
  uint8_t* dst_row,
  intptr_t dst_stride,
  uint32_t w,
  uint32_t h,
  uint32_t packed_table[64],
  UnpackedPixel unpacked_table[64],
  const uint8_t* src,
  const uint8_t* end) noexcept {

  constexpr size_t kMinRemainingBytesOfNextChunk = kQoiEndMarkerSize + 1u;

  uint32_t* dst_ptr = reinterpret_cast<uint32_t*>(dst_row);
  uint32_t* dst_end = dst_ptr + w;

  uint32_t packed_pixel = 0xFF000000;
  UnpackedPixel unpacked_pixel = UnpackedPixel::unpack(packed_pixel);

  // Edge case: If the image starts with QOI_OP_RUN, the repeated pixel must be
  // added to the pixel table, otherwise the decoder may produce incorrect result.
  {
    uint32_t hbyte0 = src[0];

    if (hbyte0 >= kQoiOpRun && hbyte0 < kQoiOpRun + 62u) {
      uint32_t hash = unpacked_pixel.hash();
      packed_table[hash] = packed_pixel;
      unpacked_table[hash] = unpacked_pixel;
    }
  }

  for (;;) {
    size_t remaining = PtrOps::bytes_until(src, end);
    if (BL_UNLIKELY(remaining < kMinRemainingBytesOfNextChunk)) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    uint32_t hbyte0 = src[0];
    uint32_t hbyte1 = src[1];
    src++;

    if (hbyte0 < kQoiOpRun) {
      // QOI_OP_INDEX + QOI_OP_DIFF + QOI_OP_LUMA
      // ========================================

      if (hbyte0 < 64u) {
        // Handle QOI_OP_INDEX - 6-bit index to a pixel table (hbyte0 = 0b00xxxxxx).
        packed_pixel = packed_table[hbyte0];
        unpacked_pixel = unpacked_table[hbyte0];

        *dst_ptr = packed_pixel;
        if (BL_LIKELY(++dst_ptr != dst_end)) {
          if (!(hbyte1 < 64u)) {
            continue;
          }

          packed_pixel = packed_table[hbyte1];
          unpacked_pixel = unpacked_table[hbyte1];
          src++;

          *dst_ptr = packed_pixel;
          if (++dst_ptr != dst_end) {
            continue;
          }
        }
        hbyte0 = 0;
      }
      else {
        // Handle QOI_OP_DIFF and QOI_OP_LUMA chunks.
        {
          src += hbyte0 >> 7;

          uint32_t packed_delta = qoiIndexDiffLumaLUT[hbyte0 - 64u];
          hbyte1 &= packed_delta;
          packed_delta >>= 8;

          unpacked_pixel.addRB((hbyte1 | (hbyte1 << 12)) & 0x000F000Fu);
          unpacked_pixel.add(UnpackedPixel::unpack(packed_delta));
          unpacked_pixel.mask();
        }

store_pixel:
        hbyte0 = unpacked_pixel.hash();

        packed_pixel = unpacked_pixel.pack<kHasAlpha>();
        unpacked_table[hbyte0] = unpacked_pixel;

        *dst_ptr = packed_pixel;
        packed_table[hbyte0] = packed_pixel;

        if (++dst_ptr != dst_end)
          continue;

        hbyte0 = 0;
      }
    }
    else {
      // QOI_OP_RUN + QOI_OP_RGB + QOI_OP_RGBA
      // =====================================

      if (hbyte0 >= kQoiOpRgb) {
        // Handle both QOI_OP_RGB and QOI_OP_RGBA at the same time.
        unpacked_pixel.opRGBX(hbyte0, UnpackedPixel::unpackRGBA(hbyte1, src[1], src[2], src[3]));

        // Advance by either 3 (RGB) or 4 (RGBA) bytes.
        src += hbyte0 - 251u;
        goto store_pixel;
      }
      else {
        // Run-length encoding repeats the previous pixel by `(hbyte0 & 0x3F) + 1` times (N stored with a bias of -1).
        hbyte0 = (hbyte0 & 0x3Fu) + 1u;

store_rle:
        {
          size_t limit = (size_t)(dst_end - dst_ptr);
          size_t fill = bl_min<size_t>(hbyte0, limit);

          hbyte0 -= uint32_t(fill);
          dst_ptr = fillRgba32(dst_ptr, packed_pixel, fill);

          if (dst_ptr != dst_end) {
            continue;
          }
        }
      }
    }

    if (BL_UNLIKELY(--h == 0)) {
      return BL_SUCCESS;
    }

    dst_row += dst_stride;
    dst_ptr = reinterpret_cast<uint32_t*>(dst_row);
    dst_end = dst_ptr + w;

    // True if we are inside an unfinished QOI_OP_RUN that spans across two or more rows.
    if (hbyte0 != 0) {
      goto store_rle;
    }
  }
}

static BLResult decoder_read_frame_internal(BLQoiDecoderImpl* decoder_impl, BLImage* image_out, const uint8_t* data, size_t size) noexcept {
  if (size < kQoiHeaderSize)
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);

  const uint8_t* start = data;
  const uint8_t* end = data + size;

  uint32_t w = uint32_t(decoder_impl->image_info.size.w);
  uint32_t h = uint32_t(decoder_impl->image_info.size.h);

  uint32_t depth = decoder_impl->image_info.depth;
  BLFormat format = depth == 32 ? BL_FORMAT_PRGB32 : BL_FORMAT_XRGB32;

  data += kQoiHeaderSize;
  if (data >= end)
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);

  BLImageData image_data;
  BL_PROPAGATE(image_out->create(int(w), int(h), format));
  BL_PROPAGATE(image_out->make_mutable(&image_data));

  uint8_t* dst_row = static_cast<uint8_t*>(image_data.pixel_data);
  intptr_t dst_stride = image_data.stride;

  uint32_t packed_table[64];
  fillRgba32(packed_table, depth == 32 ? 0u : 0xFF000000u, 64);

  UnpackedPixel unpacked_table[64] {};

  if (depth == 32)
    BL_PROPAGATE(decode_qoi_data<true>(dst_row, dst_stride, w, h, packed_table, unpacked_table, data, end));
  else
    BL_PROPAGATE(decode_qoi_data<false>(dst_row, dst_stride, w, h, packed_table, unpacked_table, data, end));

  decoder_impl->buffer_index = PtrOps::byte_offset(start, data);
  decoder_impl->frame_index++;

  return BL_SUCCESS;
}

// bl::Qoi::Decoder - Interface
// ============================

static BLResult BL_CDECL decoder_restart_impl(BLImageDecoderImpl* impl) noexcept {
  BLQoiDecoderImpl* decoder_impl = static_cast<BLQoiDecoderImpl*>(impl);

  decoder_impl->last_result = BL_SUCCESS;
  decoder_impl->frame_index = 0;
  decoder_impl->buffer_index = 0;
  decoder_impl->image_info.reset();

  return BL_SUCCESS;
}

static BLResult BL_CDECL decoder_read_info_impl(BLImageDecoderImpl* impl, BLImageInfo* info_out, const uint8_t* data, size_t size) noexcept {
  BLQoiDecoderImpl* decoder_impl = static_cast<BLQoiDecoderImpl*>(impl);
  BLResult result = decoder_impl->last_result;

  if (decoder_impl->buffer_index == 0 && result == BL_SUCCESS) {
    result = decoder_read_info_internal(decoder_impl, data, size);
    if (result != BL_SUCCESS)
      decoder_impl->last_result = result;
  }

  if (info_out)
    memcpy(info_out, &decoder_impl->image_info, sizeof(BLImageInfo));

  return result;
}

static BLResult BL_CDECL decoder_read_frame_impl(BLImageDecoderImpl* impl, BLImageCore* image_out, const uint8_t* data, size_t size) noexcept {
  BLQoiDecoderImpl* decoder_impl = static_cast<BLQoiDecoderImpl*>(impl);
  BL_PROPAGATE(decoder_read_info_impl(decoder_impl, nullptr, data, size));

  if (decoder_impl->frame_index)
    return bl_make_error(BL_ERROR_NO_MORE_DATA);

  BLResult result = decoder_read_frame_internal(decoder_impl, static_cast<BLImage*>(image_out), data, size);
  if (result != BL_SUCCESS)
    decoder_impl->last_result = result;
  return result;
}

static BLResult BL_CDECL decoder_create_impl(BLImageDecoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_DECODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLQoiDecoderImpl>(self, info));

  BLQoiDecoderImpl* decoder_impl = static_cast<BLQoiDecoderImpl*>(self->_d.impl);
  decoder_impl->ctor(&qoi_decoder_virt, &qoi_codec_instance);
  return decoder_restart_impl(decoder_impl);
}

static BLResult BL_CDECL decoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLQoiDecoderImpl* decoder_impl = static_cast<BLQoiDecoderImpl*>(impl);

  decoder_impl->dtor();
  return bl_object_free_impl(decoder_impl);
}

// bl::Qoi::Encoder - Interface
// ============================

// QOI isn't good for compressing alpha-only images - we can optimize the encoder's performance, but not the final size.
static uint8_t* encodeQoiDataA8(uint8_t* dst_data, uint32_t w, uint32_t h, const uint8_t* src_data, intptr_t src_stride) noexcept {
  // NOTE: Use an initial value which is not representable, because the encoder/decoder starts with RGB==0,
  // which would decode badly into RGBA formats (the components would be zero and thus it would not be the
  // same as when used by Blend2D, which defaults to having RGB components the same as 0xFF premultiplied).
  uint32_t pixel = 0xFFFFFFFFu;
  uint16_t pixel_table[64];

  for (size_t i = 0; i < 64; i++) {
    pixel_table[i] = 0xFFFFu;
  }

  src_stride -= intptr_t(w);
  uint32_t x = w;

  for (;;) {
    uint32_t p = *src_data++;

    // Run length encoding.
    if (p == pixel) {
      size_t n = 1;
      x--;

      for (;;) {
        uint32_t prev_x = x;

        while (x) {
          p = *src_data++;
          if (p != pixel)
            break;
          x--;
        }

        n += size_t(prev_x - x);

        if (x == 0 && --h != 0) {
          src_data += src_stride;
          x = w;
        }
        else {
          break;
        }
      }

      do {
        size_t run = bl_min<size_t>(n, 62u);
        *dst_data++ = uint8_t(run + (kQoiOpRun - 1u));
        n -= run;
      } while (n);

      if (!x) {
        return dst_data;
      }
    }

    uint32_t hash = hashPixelA8(p);

    if (pixel_table[hash] == p) {
      *dst_data++ = uint8_t(kQoiOpIndex | hash);
    }
    else {
      pixel_table[hash] = uint16_t(p);

      dst_data[0] = kQoiOpRgba;
      dst_data[1] = uint8_t(0xFFu);
      dst_data[2] = uint8_t(0xFFu);
      dst_data[3] = uint8_t(0xFFu);
      dst_data[4] = uint8_t(p);
      dst_data += 5;
    }

    pixel = p;

    if (--x != 0u)
      continue;

    if (--h == 0u)
      return dst_data;

    src_data += src_stride;
    x = w;
  }
}

static uint8_t* encodeQoiDataXRGB32(uint8_t* dst_data, uint32_t w, uint32_t h, const uint8_t* src_data, intptr_t src_stride) noexcept {
  BLRgba32 pixel = BLRgba32(0xFF000000u);
  uint32_t pixel_table[64] {};

  uint32_t x = w;
  src_stride -= intptr_t(w) * 4;

  for (;;) {
    BLRgba32 p = BLRgba32(MemOps::readU32a(src_data) | 0xFF000000u);
    src_data += 4;

    // Run length encoding.
    if (p == pixel) {
      size_t n = 1;
      x--;

      for (;;) {
        uint32_t prev_x = x;

        while (x) {
          p = BLRgba32(MemOps::readU32a(src_data) | 0xFF000000u);
          src_data += 4;
          if (p != pixel)
            break;
          x--;
        }

        n += size_t(prev_x - x);

        if (x == 0 && --h != 0) {
          src_data += src_stride;
          x = w;
        }
        else {
          break;
        }
      }

      do {
        size_t run = bl_min<size_t>(n, 62u);
        *dst_data++ = uint8_t(run + (kQoiOpRun - 1u));
        n -= run;
      } while (n);

      if (!x) {
        return dst_data;
      }
    }

    uint32_t hash = hashPixelRGBA32(p.value);

    if (pixel_table[hash] == p.value) {
      *dst_data++ = uint8_t(kQoiOpIndex | hash);
    }
    else {
      pixel_table[hash] = p.value;

      uint32_t dr = uint32_t(p.r()) - uint32_t(pixel.r());
      uint32_t dg = uint32_t(p.g()) - uint32_t(pixel.g());
      uint32_t db = uint32_t(p.b()) - uint32_t(pixel.b());

      uint32_t xr = uint32_t(dr + 2u) & 0xFFu;
      uint32_t xg = uint32_t(dg + 2u) & 0xFFu;
      uint32_t xb = uint32_t(db + 2u) & 0xFFu;

      if ((xr | xg | xb) <= 0x3u) {
        *dst_data++ = uint8_t(kQoiOpDiff | (xr << 4) | (xg << 2) | xb);
      }
      else {
        uint32_t dg_r = dr - dg;
        uint32_t dg_b = db - dg;

        xr = (dg_r + 8u) & 0xFFu;
        xg = (dg  + 32u) & 0xFFu;
        xb = (dg_b + 8u) & 0xFFu;

        if ((xr | xb) <= 0xFu && xg <= 0x3F) {
          dst_data[0] = uint8_t(kQoiOpLuma | xg);
          dst_data[1] = uint8_t((xr << 4) | xb);
          dst_data += 2;
        }
        else {
          dst_data[0] = kQoiOpRgb;
          dst_data[1] = uint8_t(p.r());
          dst_data[2] = uint8_t(p.g());
          dst_data[3] = uint8_t(p.b());
          dst_data += 4;
        }
      }
    }

    pixel = p;

    if (--x != 0u)
      continue;

    if (--h == 0u)
      return dst_data;

    src_data += src_stride;
    x = w;
  }
}

static uint8_t* encodeQoiDataPRGB32(uint8_t* dst_data, uint32_t w, uint32_t h, const uint8_t* src_data, intptr_t src_stride) noexcept {
  BLRgba32 pixelPM = BLRgba32(0xFF000000u);
  BLRgba32 pixelNP = BLRgba32(0xFF000000u);
  uint32_t pixel_table[64] {};

  uint32_t x = w;
  src_stride -= intptr_t(w) * 4;

  for (;;) {
    BLRgba32 pm = BLRgba32(MemOps::readU32a(src_data));
    src_data += 4;

    // Run length encoding.
    if (pm == pixelPM) {
      size_t n = 1;
      x--;

      for (;;) {
        uint32_t prev_x = x;

        while (x) {
          pm = BLRgba32(MemOps::readU32a(src_data));
          src_data += 4;
          if (pm != pixelPM)
            break;
          x--;
        }

        n += size_t(prev_x - x);

        if (x == 0 && --h != 0) {
          src_data += src_stride;
          x = w;
        }
        else {
          break;
        }
      }

      do {
        size_t run = bl_min<size_t>(n, 62u);
        *dst_data++ = uint8_t(run + (kQoiOpRun - 1u));
        n -= run;
      } while (n);

      if (!x) {
        return dst_data;
      }
    }

    BLRgba32 np = BLRgba32(PixelOps::Scalar::cvt_argb32_8888_from_prgb32_8888(pm.value));
    uint32_t hash = hashPixelRGBA32(np.value);

    if (pixel_table[hash] == np.value) {
      *dst_data++ = uint8_t(kQoiOpIndex | hash);
    }
    else {
      pixel_table[hash] = np.value;

      // To use delta, the previous pixel needs to have the same alpha value unfortunately.
      if (pixelNP.a() == np.a()) {
        uint32_t dr = uint32_t(np.r()) - uint32_t(pixelNP.r());
        uint32_t dg = uint32_t(np.g()) - uint32_t(pixelNP.g());
        uint32_t db = uint32_t(np.b()) - uint32_t(pixelNP.b());

        uint32_t xr = (dr + 2u) & 0xFFu;
        uint32_t xg = (dg + 2u) & 0xFFu;
        uint32_t xb = (db + 2u) & 0xFFu;

        if ((xr | xg | xb) <= 0x3u) {
          *dst_data++ = uint8_t(kQoiOpDiff | (xr << 4) | (xg << 2) | xb);
        }
        else {
          uint32_t dg_r = dr - dg;
          uint32_t dg_b = db - dg;

          xr = (dg_r + 8) & 0xFFu;
          xg = (dg  + 32) & 0xFFu;
          xb = (dg_b + 8) & 0xFFu;

          if ((xr | xb) <= 0xFu && xg <= 0x3Fu) {
            dst_data[0] = uint8_t(kQoiOpLuma | xg);
            dst_data[1] = uint8_t((xr << 4) | xb);
            dst_data += 2;
          }
          else {
            dst_data[0] = kQoiOpRgb;
            dst_data[1] = uint8_t(np.r());
            dst_data[2] = uint8_t(np.g());
            dst_data[3] = uint8_t(np.b());
            dst_data += 4;
          }
        }
      }
      else {
        dst_data[0] = kQoiOpRgba;
        dst_data[1] = uint8_t(np.r());
        dst_data[2] = uint8_t(np.g());
        dst_data[3] = uint8_t(np.b());
        dst_data[4] = uint8_t(np.a());
        dst_data += 5;
      }
    }

    pixelPM = pm;
    pixelNP = np;

    if (--x != 0u)
      continue;

    if (--h == 0u)
      return dst_data;

    src_data += src_stride;
    x = w;
  }
}

static BLResult BL_CDECL encoder_restart_impl(BLImageEncoderImpl* impl) noexcept {
  BLQoiEncoderImpl* encoder_impl = static_cast<BLQoiEncoderImpl*>(impl);
  encoder_impl->last_result = BL_SUCCESS;
  encoder_impl->frame_index = 0;
  encoder_impl->buffer_index = 0;
  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_write_frame_impl(BLImageEncoderImpl* impl, BLArrayCore* dst, const BLImageCore* image) noexcept {
  BLQoiEncoderImpl* encoder_impl = static_cast<BLQoiEncoderImpl*>(impl);
  BL_PROPAGATE(encoder_impl->last_result);

  BLArray<uint8_t>& buf = *static_cast<BLArray<uint8_t>*>(dst);
  const BLImage& img = *static_cast<const BLImage*>(image);

  if (img.is_empty())
    return bl_make_error(BL_ERROR_INVALID_VALUE);

  BLImageData image_data;
  BL_PROPAGATE(img.get_data(&image_data));

  uint32_t w = uint32_t(image_data.size.w);
  uint32_t h = uint32_t(image_data.size.h);
  uint32_t format = image_data.format;

  uint32_t channels = format == BL_FORMAT_XRGB32 ? 3u : 4u;
  uint32_t max_bytes_per_encoded_pixel = channels + 1u;

  // NOTE: This should never overflow.
  uint64_t max_size = uint64_t(w) * uint64_t(h) * uint64_t(max_bytes_per_encoded_pixel) + kQoiHeaderSize + kQoiEndMarkerSize;

  if (BL_UNLIKELY(max_size >= uint64_t(SIZE_MAX)))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  uint8_t* dst_data;
  BL_PROPAGATE(buf.modify_op(BL_MODIFY_OP_ASSIGN_FIT, size_t(max_size), &dst_data));

  uint8_t* dst_ptr = dst_data;
  memcpy(dst_ptr, qoi_magic, kQoiMagicSize);
  MemOps::writeU32uBE(dst_ptr + 4, w);
  MemOps::writeU32uBE(dst_ptr + 8, h);
  dst_ptr[12] = uint8_t(channels);
  dst_ptr[13] = 0;
  dst_ptr += 14;

  const uint8_t* src_line = static_cast<const uint8_t*>(image_data.pixel_data);

  switch (format) {
    case BL_FORMAT_A8:
      dst_ptr = encodeQoiDataA8(dst_ptr, w, h, src_line, image_data.stride);
      break;

    case BL_FORMAT_XRGB32:
      dst_ptr = encodeQoiDataXRGB32(dst_ptr, w, h, src_line, image_data.stride);
      break;

    case BL_FORMAT_PRGB32:
      dst_ptr = encodeQoiDataPRGB32(dst_ptr, w, h, src_line, image_data.stride);
      break;

    default:
      ArrayInternal::set_size(dst, 0);
      return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  memcpy(dst_ptr, qoi_end_marker, kQoiEndMarkerSize);
  dst_ptr += kQoiEndMarkerSize;

  ArrayInternal::set_size(dst, PtrOps::byte_offset(dst_data, dst_ptr));
  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_create_impl(BLImageEncoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_ENCODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLQoiEncoderImpl>(self, info));

  BLQoiEncoderImpl* encoder_impl = static_cast<BLQoiEncoderImpl*>(self->_d.impl);
  encoder_impl->ctor(&qoi_encoder_virt, &qoi_codec_instance);
  return encoder_restart_impl(encoder_impl);
}

static BLResult BL_CDECL encoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLQoiEncoderImpl* encoder_impl = static_cast<BLQoiEncoderImpl*>(impl);

  encoder_impl->dtor();
  return bl_object_free_impl(encoder_impl);
}

// bl::Qoi::Codec - Interface
// ==========================

static BLResult BL_CDECL codec_destroy_impl(BLObjectImpl* impl) noexcept {
  // Built-in codecs are never destroyed.
  bl_unused(impl);
  return BL_SUCCESS;
};

static uint32_t BL_CDECL codec_inspect_data_impl(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl);

  if (size == 0)
    return 0;

  size_t magic_size = bl_min<size_t>(size, kQoiMagicSize);
  if (memcmp(qoi_magic, data, magic_size) != 0)
    return 0;

  if (size < 12)
    return uint32_t(magic_size);

  uint32_t w = bl::MemOps::readU32uBE(data + 4);
  uint32_t h = bl::MemOps::readU32uBE(data + 8);

  if (w == 0 || h == 0)
    return 0;

  if (size < 14)
    return uint32_t(magic_size + 1u);

  uint8_t channels = data[12];
  uint8_t colorspace = data[13];

  if (channels != 3u && channels != 4u)
    return 0;

  if (colorspace > 1u)
    return 0;

  // A valid QOI header.
  return 100;
}

static BLResult BL_CDECL codec_create_decoder_impl(const BLImageCodecImpl* impl, BLImageDecoderCore* dst) noexcept {
  bl_unused(impl);

  BLImageDecoderCore tmp;
  BL_PROPAGATE(decoder_create_impl(&tmp));
  return bl_image_decoder_assign_move(dst, &tmp);
}

static BLResult BL_CDECL codec_create_encoder_impl(const BLImageCodecImpl* impl, BLImageEncoderCore* dst) noexcept {
  bl_unused(impl);

  BLImageEncoderCore tmp;
  BL_PROPAGATE(encoder_create_impl(&tmp));
  return bl_image_encoder_assign_move(dst, &tmp);
}

// bl::Qoi::Codec - Runtime Registration
// =====================================

void qoi_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept {
  using namespace bl::Qoi;

  bl_unused(rt);

  // Initialize QOI codec.
  qoi_codec.virt.base.destroy = codec_destroy_impl;
  qoi_codec.virt.base.get_property = bl_object_impl_get_property;
  qoi_codec.virt.base.set_property = bl_object_impl_set_property;
  qoi_codec.virt.inspect_data = codec_inspect_data_impl;
  qoi_codec.virt.create_decoder = codec_create_decoder_impl;
  qoi_codec.virt.create_encoder = codec_create_encoder_impl;

  qoi_codec.impl->ctor(&qoi_codec.virt);
  qoi_codec.impl->features = BLImageCodecFeatures(BL_IMAGE_CODEC_FEATURE_READ     |
                                                 BL_IMAGE_CODEC_FEATURE_WRITE    |
                                                 BL_IMAGE_CODEC_FEATURE_LOSSLESS);
  qoi_codec.impl->name.dcast().assign("QOI");
  qoi_codec.impl->vendor.dcast().assign("Blend2D");
  qoi_codec.impl->mime_type.dcast().assign("image/qoi");
  qoi_codec.impl->extensions.dcast().assign("qoi");

  qoi_codec_instance._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC), &qoi_codec.impl);

  // Initialize QOI decoder virtual functions.
  qoi_decoder_virt.base.destroy = decoder_destroy_impl;
  qoi_decoder_virt.base.get_property = bl_object_impl_get_property;
  qoi_decoder_virt.base.set_property = bl_object_impl_set_property;
  qoi_decoder_virt.restart = decoder_restart_impl;
  qoi_decoder_virt.read_info = decoder_read_info_impl;
  qoi_decoder_virt.read_frame = decoder_read_frame_impl;

  // Initialize QOI encoder virtual functions.
  qoi_encoder_virt.base.destroy = encoder_destroy_impl;
  qoi_encoder_virt.base.get_property = bl_object_impl_get_property;
  qoi_encoder_virt.base.set_property = bl_object_impl_set_property;
  qoi_encoder_virt.restart = encoder_restart_impl;
  qoi_encoder_virt.write_frame = encoder_write_frame_impl;

  codecs->append(qoi_codec_instance.dcast());
}

} // {bl::Qoi}
