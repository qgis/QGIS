// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/format_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/pixelconverter.h>
#include <blend2d/core/rgba.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/bmpcodec_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/traits_p.h>

namespace bl::Bmp {

// bl::Bmp::Codec - Globals
// ========================

static BLObjectEternalVirtualImpl<BLBmpCodecImpl, BLImageCodecVirt> bmp_codec;
static BLImageCodecCore bmp_codec_instance;
static BLImageDecoderVirt bmp_decoder_virt;
static BLImageEncoderVirt bmp_encoder_virt;

static const char kBMPCompressionNameData[] =
  "RGB\0"            // #0
  "RLE8\0"           // #1
  "RLE4\0"           // #2
  "BitFields\0"      // #3
  "JPEG\0"           // #4
  "PNG\0"            // #5
  "AlphaBitFields\0" // #6
  "\0"               // #7
  "\0"               // #8
  "\0"               // #9
  "\0"               // #10
  "CMYK\0"           // #11
  "CMYK_RLE8\0"      // #12
  "\0";              // #13 (termination)

static const uint16_t kBMPCompressionNameIndex[] = {
  0,  // #0
  4,  // #1
  9,  // #2
  14, // #3
  24, // #4
  29, // #5
  33, // #6
  48, // #7
  49, // #8
  50, // #9
  51, // #10
  52, // #11
  57, // #12
  67, // #13 (termination)
};

// bl::Bmp::Decoder - Utilities
// ============================

static bool check_header_size(uint32_t header_size) noexcept {
  return header_size == kHeaderSizeOS2_V1 ||
         header_size == kHeaderSizeWIN_V1 ||
         header_size == kHeaderSizeWIN_V2 ||
         header_size == kHeaderSizeWIN_V3 ||
         header_size == kHeaderSizeWIN_V4 ||
         header_size == kHeaderSizeWIN_V5 ;
}

static bool check_depth(uint32_t depth) noexcept {
  return depth ==  1 ||
         depth ==  4 ||
         depth ==  8 ||
         depth == 16 ||
         depth == 24 ||
         depth == 32 ;
}

static bool check_image_size(const BLSizeI& size) noexcept {
  return uint32_t(size.w) <= BL_RUNTIME_MAX_IMAGE_SIZE &&
         uint32_t(size.h) <= BL_RUNTIME_MAX_IMAGE_SIZE ;
}

static bool check_bit_masks(const uint32_t* masks, uint32_t n) noexcept {
  uint32_t combined = 0;

  for (uint32_t i = 0; i < n; i++) {
    uint32_t m = masks[i];

    // RGB masks can't be zero.
    if (m == 0 && i != 3) {
      return false;
    }

    // Mask has to have consecutive bits set, masks like 000110011 are not allowed.
    if (m != 0 && !IntOps::is_bit_mask_consecutive(m)) {
      return false;
    }

    // Mask can't overlap with other.
    if ((combined & m) != 0) {
      return false;
    }

    combined |= m;
  }

  return true;
}

// bl::Bmp::Decoder - RLE4
// =======================

static BLResult decodeRLE4(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* p, size_t size, uint32_t w, uint32_t h, const BLRgba32* pal) noexcept {
  uint8_t* dst_data = dst_line;
  const uint8_t* end = p + size;

  uint32_t x = 0;
  uint32_t y = 0;

  for (;;) {
    if (PtrOps::bytes_until(p, end) < 2u) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    uint32_t b0 = p[0];
    uint32_t b1 = p[1]; p += 2;

    if (b0 != 0) {
      // RLE_FILL (b0 = Size, b1 = Pattern).
      uint32_t c0 = pal[b1 >> 4].value;
      uint32_t c1 = pal[b1 & 15].value;

      uint32_t i = bl_min<uint32_t>(b0, w - x);
      for (x += i; i >= 2; i -= 2, dst_data += 8) {
        MemOps::writeU32a(dst_data + 0, c0);
        MemOps::writeU32a(dst_data + 4, c1);
      }

      if (i) {
        MemOps::writeU32a(dst_data + 0, c0);
        dst_data += 4;
      }
    }
    else if (b1 >= kRleCount) {
      // Absolute (b1 = Size).
      uint32_t i = bl_min<uint32_t>(b1, w - x);
      uint32_t req_bytes = ((b1 + 3u) >> 1) & ~uint32_t(0x1);

      if (PtrOps::bytes_until(p, end) < req_bytes) {
        return bl_make_error(BL_ERROR_DATA_TRUNCATED);
      }

      for (x += i; i >= 4; i -= 4, dst_data += 16) {
        b0 = p[0];
        b1 = p[1]; p += 2;

        MemOps::writeU32a(dst_data +  0, pal[b0 >> 4].value);
        MemOps::writeU32a(dst_data +  4, pal[b0 & 15].value);
        MemOps::writeU32a(dst_data +  8, pal[b1 >> 4].value);
        MemOps::writeU32a(dst_data + 12, pal[b1 & 15].value);
      }

      if (i) {
        b0 = p[0];
        b1 = p[1]; p += 2;

        MemOps::writeU32a(dst_data, pal[b0 >> 4].value);
        dst_data += 4;

        if (--i) {
          MemOps::writeU32a(dst_data, pal[b0 & 15].value);
          dst_data += 4;

          if (--i) {
            MemOps::writeU32a(dst_data, pal[b1 >> 4].value);
            dst_data += 4;
          }
        }
      }
    }
    else {
      // RLE_SKIP (fill by a background pixel).
      uint32_t to_x = x;
      uint32_t to_y = y;

      if (b1 == kRleLine) {
        to_x = 0;
        to_y++;
      }
      else if (b1 == kRleMove) {
        if (PtrOps::bytes_until(p, end) < 2u) {
          return bl_make_error(BL_ERROR_DATA_TRUNCATED);
        }

        to_x += p[0];
        to_y += p[1]; p += 2;

        if (to_x > w || to_y > h) {
          return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);
        }
      }
      else {
        to_x = 0;
        to_y = h;
      }

      for (; y < to_y; y++) {
        for (x = w - x; x; x--, dst_data += 4) {
          MemOps::writeU32a(dst_data, kRleBackground);
        }

        dst_line += dst_stride;
        dst_data = dst_line;
      }

      for (; x < to_x; x++, dst_data += 4) {
        MemOps::writeU32a(dst_data, kRleBackground);
      }

      if (b1 == kRleStop || y == h) {
        return BL_SUCCESS;
      }
    }
  }
}

// bl::Bmp::Decoder - RLE8
// =======================

static BLResult decodeRLE8(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* p, size_t size, uint32_t w, uint32_t h, const BLRgba32* pal) noexcept {
  uint8_t* dst_data = dst_line;
  const uint8_t* end = p + size;

  uint32_t x = 0;
  uint32_t y = 0;

  for (;;) {
    if (PtrOps::bytes_until(p, end) < 2u) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    uint32_t b0 = p[0];
    uint32_t b1 = p[1]; p += 2;

    if (b0 != 0) {
      // RLE_FILL (b0 = Size, b1 = Pattern).
      uint32_t c0 = pal[b1].value;
      uint32_t i = bl_min<uint32_t>(b0, w - x);

      for (x += i; i; i--, dst_data += 4) {
        MemOps::writeU32a(dst_data, c0);
      }
    }
    else if (b1 >= kRleCount) {
      // Absolute (b1 = Size).
      uint32_t i = bl_min<uint32_t>(b1, w - x);
      uint32_t req_bytes = ((b1 + 1) >> 1) << 1;

      if (PtrOps::bytes_until(p, end) < req_bytes) {
        return bl_make_error(BL_ERROR_DATA_TRUNCATED);
      }

      for (x += i; i >= 2; i -= 2, dst_data += 8) {
        b0 = p[0];
        b1 = p[1]; p += 2;

        MemOps::writeU32a(dst_data + 0, pal[b0].value);
        MemOps::writeU32a(dst_data + 4, pal[b1].value);
      }

      if (i) {
        b0 = p[0]; p += 2;

        MemOps::writeU32a(dst_data, pal[b0].value);
        dst_data += 4;
      }
    }
    else {
      // RLE_SKIP (fill by a background pixel).
      uint32_t to_x = x;
      uint32_t to_y = y;

      if (b1 == kRleLine) {
        to_x = 0;
        to_y++;
      }
      else if (b1 == kRleMove) {
        if (PtrOps::bytes_until(p, end) < 2u) {
          return bl_make_error(BL_ERROR_DATA_TRUNCATED);
        }

        to_x += p[0];
        to_y += p[1]; p += 2;

        if (to_x > w || to_y > h) {
          return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);
        }
      }
      else {
        to_x = 0;
        to_y = h;
      }

      for (; y < to_y; y++) {
        for (x = w - x; x; x--, dst_data += 4) {
          MemOps::writeU32a(dst_data, kRleBackground);
        }

        dst_line += dst_stride;
        dst_data = dst_line;
      }

      for (; x < to_x; x++, dst_data += 4) {
        MemOps::writeU32a(dst_data, kRleBackground);
      }

      if (b1 == kRleStop || y == h) {
        return BL_SUCCESS;
      }
    }
  }
}

// bl::Bmp::Decoder - Read Info (Internal)
// =======================================

static BLResult decoder_read_info_internal(BLBmpDecoderImpl* decoder_impl, const uint8_t* data, size_t size) noexcept {
  // Signature + BmpFile header + BmpInfo header size (18 bytes total).
  const size_t kBmpMinSize = 2 + 12 + 4;

  if (size < kBmpMinSize) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  // Read BMP file signature.
  if (data[0] != 'B' || data[1] != 'M') {
    return bl_make_error(BL_ERROR_INVALID_SIGNATURE);
  }

  const uint8_t* start = data;
  const uint8_t* end = data + size;

  // Read BMP file header.
  memcpy(&decoder_impl->file, data + 2, 12);
  data += 2 + 12;
  if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) decoder_impl->file.byte_swap();

  // First check if the header is supported by the decoder.
  uint32_t header_size = MemOps::readU32uLE(data);
  uint32_t file_and_info_header_size = 14 + header_size;

  if (!check_header_size(header_size)) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  // Read BMP info header.
  if (PtrOps::bytes_until(data, end) < header_size) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  memcpy(&decoder_impl->info, data, header_size);
  data += header_size;

  int32_t w, h;
  uint32_t depth;
  uint32_t plane_count;
  uint32_t compression = kCompressionRGB;
  bool rle_used = false;

  if (header_size == kHeaderSizeOS2_V1) {
    // Handle OS/2 BMP.
    if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
      decoder_impl->info.os2.byte_swap();
    }

    w = decoder_impl->info.os2.width;
    h = decoder_impl->info.os2.height;
    depth = decoder_impl->info.os2.bits_per_pixel;
    plane_count = decoder_impl->info.os2.planes;

    // Convert to Windows BMP, there is no difference except the header.
    decoder_impl->info.win.width = w;
    decoder_impl->info.win.height = h;
    decoder_impl->info.win.planes = uint16_t(plane_count);
    decoder_impl->info.win.bits_per_pixel = uint16_t(depth);
    decoder_impl->info.win.compression = compression;
  }
  else {
    // Handle Windows BMP.
    if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
      decoder_impl->info.win.byte_swap();
    }

    w = decoder_impl->info.win.width;
    h = decoder_impl->info.win.height;
    depth = decoder_impl->info.win.bits_per_pixel;
    plane_count = decoder_impl->info.win.planes;
    compression = decoder_impl->info.win.compression;
  }

  // Verify whether input data is ok.
  if (h == Traits::min_value<int32_t>() || w <= 0) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  if (plane_count != 1) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  if (h < 0) {
    h = -h;
  }

  decoder_impl->image_info.size.reset(w, h);
  decoder_impl->image_info.depth = uint16_t(depth);
  decoder_impl->image_info.plane_count = uint16_t(plane_count);
  decoder_impl->image_info.frame_count = 1;

  memcpy(decoder_impl->image_info.format, "BMP", 4);
  strncpy(decoder_impl->image_info.compression,
    kBMPCompressionNameData + kBMPCompressionNameIndex[bl_min<size_t>(compression, kCompressionValueCount)],
    sizeof(decoder_impl->image_info.compression) - 1);

  // Check if the compression field is correct when depth <= 8.
  if (compression != kCompressionRGB) {
    if (depth <= 8) {
      rle_used = (depth == 4 && compression == kCompressionRLE4) |
                (depth == 8 && compression == kCompressionRLE8) ;

      if (!rle_used) {
        return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
      }
    }
  }

  if (decoder_impl->file.image_offset < file_and_info_header_size)
    return bl_make_error(BL_ERROR_INVALID_DATA);

  // Check if the size is valid.
  if (!check_image_size(decoder_impl->image_info.size))
    return bl_make_error(BL_ERROR_IMAGE_TOO_LARGE);

  // Check if the depth is valid.
  if (!check_depth(decoder_impl->image_info.depth))
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);

  // Calculate a stride aligned to 32 bits.
  OverflowFlag of{};
  uint64_t stride = (((uint64_t(w) * uint64_t(depth) + 7u) / 8u) + 3u) & ~uint32_t(3);
  uint32_t image_size = IntOps::mul_overflow(uint32_t(stride & 0xFFFFFFFFu), uint32_t(h), &of);

  if (stride >= Traits::max_value<uint32_t>() || of) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  decoder_impl->stride = uint32_t(stride);

  // 1. OS/2 format doesn't specify image_size, it's always calculated.
  // 2. BMP allows `image_size` to be zero in case of uncompressed bitmaps.
  if (header_size == kHeaderSizeOS2_V1 || (decoder_impl->info.win.image_size == 0 && !rle_used)) {
    decoder_impl->info.win.image_size = image_size;
  }

  // Check if the `image_size` matches the calculated one. It's malformed if it doesn't.
  if (!rle_used && decoder_impl->info.win.image_size < image_size) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  decoder_impl->fmt.depth = depth;
  if (depth <= 8) {
    decoder_impl->fmt.flags = BLFormatFlags(BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_INDEXED);
  }
  else {
    decoder_impl->fmt.flags = BL_FORMAT_FLAG_RGB;

    if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
      decoder_impl->fmt.add_flags(BL_FORMAT_FLAG_BYTE_SWAP);
    }

    if (depth == 16) {
      decoder_impl->fmt.r_size = 5; decoder_impl->fmt.r_shift = 10;
      decoder_impl->fmt.g_size = 5; decoder_impl->fmt.g_shift = 5;
      decoder_impl->fmt.b_size = 5; decoder_impl->fmt.b_shift = 0;
    }

    if (depth == 24 || depth == 32) {
      decoder_impl->fmt.r_size = 8; decoder_impl->fmt.r_shift = 16;
      decoder_impl->fmt.g_size = 8; decoder_impl->fmt.g_shift = 8;
      decoder_impl->fmt.b_size = 8; decoder_impl->fmt.b_shift = 0;
    }
  }

  bool has_bit_fields = depth > 8 && header_size >= kHeaderSizeWIN_V2;
  if (header_size == kHeaderSizeWIN_V1) {
    // Use BITFIELDS if specified.
    if (compression == kCompressionBitFields || compression == kCompressionAlphaBitFields) {
      uint32_t channels = 3 + (compression == kCompressionAlphaBitFields);
      if (depth != 16 && depth != 32) {
        return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
      }

      if (PtrOps::bytes_until(data, end) < channels * 4) {
        return bl_make_error(BL_ERROR_DATA_TRUNCATED);
      }

      for (uint32_t i = 0; i < channels; i++) {
        decoder_impl->info.win.masks[i] = MemOps::readU32uLE(data + i * 4);
      }

      has_bit_fields = true;
      data += channels * 4;
    }
  }

  if (has_bit_fields) {
    // BitFields provided by info header must be continuous and non-overlapping.
    if (!check_bit_masks(decoder_impl->info.win.masks, 4))
      return bl_make_error(BL_ERROR_INVALID_DATA);

    FormatInternal::assign_absolute_masks(decoder_impl->fmt, decoder_impl->info.win.masks);
    if (decoder_impl->info.win.a_mask) {
      decoder_impl->fmt.add_flags(BLFormatFlags(BL_FORMAT_FLAG_ALPHA | BL_FORMAT_FLAG_PREMULTIPLIED));
    }
  }

  decoder_impl->buffer_index = PtrOps::byte_offset(start, data);
  return BL_SUCCESS;
}

static BLResult decoder_read_frame_internal(BLBmpDecoderImpl* decoder_impl, BLImage* image_out, const uint8_t* data, size_t size) noexcept {
  const uint8_t* start = data;
  const uint8_t* end = data + size;

  // Image info.
  uint32_t w = uint32_t(decoder_impl->image_info.size.w);
  uint32_t h = uint32_t(decoder_impl->image_info.size.h);

  BLFormat format = decoder_impl->fmt.sizes[3] ? BL_FORMAT_PRGB32 : BL_FORMAT_XRGB32;
  uint32_t depth = decoder_impl->image_info.depth;
  uint32_t file_and_info_header_size = 14 + decoder_impl->info.header_size;

  if (size < file_and_info_header_size) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  // Palette.
  BLRgba32 pal[256];
  uint32_t pal_size;

  if (depth <= 8) {
    const uint8_t* pPal = data + file_and_info_header_size;
    pal_size = decoder_impl->file.image_offset - file_and_info_header_size;

    uint32_t pal_entity_size = decoder_impl->info.header_size == kHeaderSizeOS2_V1 ? 3 : 4;
    uint32_t pal_bytes_total;

    pal_size = bl_min<uint32_t>(pal_size / pal_entity_size, 256);
    pal_bytes_total = pal_size * pal_entity_size;

    if (PtrOps::bytes_until(pPal, end) < pal_bytes_total) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    // Stored as BGR|BGR (OS/2) or BGRX|BGRX (Windows).
    uint32_t i = 0;
    while (i < pal_size) {
      pal[i++] = BLRgba32(pPal[2], pPal[1], pPal[0], 0xFF);
      pPal += pal_entity_size;
    }

    // All remaining entries should be opaque black.
    while (i < 256) {
      pal[i++] = BLRgba32(0, 0, 0, 0xFF);
    }
  }

  // Move the cursor to the beginning of the image data and check if the whole
  // image content specified by `info.win.image_size` is present in the buffer.
  if (decoder_impl->file.image_offset >= size ||
      size - decoder_impl->file.image_offset < decoder_impl->info.win.image_size
  ) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  data += decoder_impl->file.image_offset;

  // Make sure that the destination image has the correct pixel format and size.
  BLImageData image_data;
  BL_PROPAGATE(image_out->create(int(w), int(h), format));
  BL_PROPAGATE(image_out->make_mutable(&image_data));

  uint8_t* dst_line = static_cast<uint8_t*>(image_data.pixel_data);
  intptr_t dst_stride = image_data.stride;

  // Flip vertically.
  if (decoder_impl->info.win.height > 0) {
    dst_line += intptr_t(h - 1) * dst_stride;
    dst_stride = -dst_stride;
  }

  // Decode.
  if (depth == 4 && decoder_impl->info.win.compression == kCompressionRLE4) {
    BL_PROPAGATE(decodeRLE4(dst_line, dst_stride, data, decoder_impl->info.win.image_size, w, h, pal));
  }
  else if (depth == 8 && decoder_impl->info.win.compression == kCompressionRLE8) {
    BL_PROPAGATE(decodeRLE8(dst_line, dst_stride, data, decoder_impl->info.win.image_size, w, h, pal));
  }
  else {
    BLPixelConverter pc;

    if (depth <= 8) {
      decoder_impl->fmt.palette = pal;
    }

    BL_PROPAGATE(pc.create(bl_format_info[format], decoder_impl->fmt,
      BLPixelConverterCreateFlags(
        BL_PIXEL_CONVERTER_CREATE_FLAG_DONT_COPY_PALETTE |
        BL_PIXEL_CONVERTER_CREATE_FLAG_ALTERABLE_PALETTE)));
    pc.convert_rect(dst_line, dst_stride, data, intptr_t(decoder_impl->stride), w, h);

    if (depth <= 8) {
      decoder_impl->fmt.palette = nullptr;
    }
  }

  decoder_impl->buffer_index = PtrOps::byte_offset(start, data);
  decoder_impl->frame_index++;

  return BL_SUCCESS;
}

// bl::Bmp::Decoder - Interface
// ============================

static BLResult BL_CDECL decoder_restart_impl(BLImageDecoderImpl* impl) noexcept {
  BLBmpDecoderImpl* decoder_impl = static_cast<BLBmpDecoderImpl*>(impl);

  decoder_impl->last_result = BL_SUCCESS;
  decoder_impl->frame_index = 0;
  decoder_impl->buffer_index = 0;
  decoder_impl->image_info.reset();
  decoder_impl->file.reset();
  decoder_impl->info.reset();
  decoder_impl->fmt.reset();
  decoder_impl->stride = 0;

  return BL_SUCCESS;
}

static BLResult BL_CDECL decoder_read_info_impl(BLImageDecoderImpl* impl, BLImageInfo* info_out, const uint8_t* data, size_t size) noexcept {
  BLBmpDecoderImpl* decoder_impl = static_cast<BLBmpDecoderImpl*>(impl);
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
  BLBmpDecoderImpl* decoder_impl = static_cast<BLBmpDecoderImpl*>(impl);
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
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLBmpDecoderImpl>(self, info));

  BLBmpDecoderImpl* decoder_impl = static_cast<BLBmpDecoderImpl*>(self->_d.impl);
  decoder_impl->ctor(&bmp_decoder_virt, &bmp_codec_instance);
  return decoder_restart_impl(decoder_impl);
}

static BLResult BL_CDECL decoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLBmpDecoderImpl* decoder_impl = static_cast<BLBmpDecoderImpl*>(impl);

  decoder_impl->dtor();
  return bl_object_free_impl(decoder_impl);
}

// bl::Bmp::Encoder - Interface
// ============================

static BLResult BL_CDECL encoder_restart_impl(BLImageEncoderImpl* impl) noexcept {
  BLBmpEncoderImpl* encoder_impl = static_cast<BLBmpEncoderImpl*>(impl);
  encoder_impl->last_result = BL_SUCCESS;
  encoder_impl->frame_index = 0;
  encoder_impl->buffer_index = 0;
  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_write_frame_impl(BLImageEncoderImpl* impl, BLArrayCore* dst, const BLImageCore* image) noexcept {
  BLBmpEncoderImpl* encoder_impl = static_cast<BLBmpEncoderImpl*>(impl);
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

  uint32_t header_size = kHeaderSizeWIN_V1;
  uint32_t bpl = 0;
  uint32_t gap = 0;
  uint32_t palette_size = 0;

  BLPixelConverter pc;
  BmpFileHeader file {};
  BmpInfoHeader info {};
  BLFormatInfo bmp_fmt {};

  info.win.width = int32_t(w);
  info.win.height = int32_t(h);
  info.win.planes = 1;
  info.win.compression = kCompressionRGB;
  info.win.colorspace = kColorSpaceDD_RGB;

  switch (format) {
    case BL_FORMAT_PRGB32: {
      // NOTE: Version 3 would be okay, but not all tools can read BMPv3.
      header_size = kHeaderSizeWIN_V4;
      bpl = w * 4;
      bmp_fmt.depth = 32;
      bmp_fmt.flags = BLFormatFlags(BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_PREMULTIPLIED);
      bmp_fmt.r_size = 8; bmp_fmt.r_shift = 16;
      bmp_fmt.g_size = 8; bmp_fmt.g_shift = 8;
      bmp_fmt.b_size = 8; bmp_fmt.b_shift = 0;
      bmp_fmt.a_size = 8; bmp_fmt.a_shift = 24;
      break;
    }

    case BL_FORMAT_XRGB32: {
      bpl = w * 3;
      gap = IntOps::align_up_diff(bpl, 4);
      bmp_fmt.depth = 24;
      bmp_fmt.flags = BL_FORMAT_FLAG_RGB;
      bmp_fmt.r_size = 8; bmp_fmt.r_shift = 16;
      bmp_fmt.g_size = 8; bmp_fmt.g_shift = 8;
      bmp_fmt.b_size = 8; bmp_fmt.b_shift = 0;
      break;
    }

    case BL_FORMAT_A8: {
      bpl = w;
      gap = IntOps::align_up_diff(bpl, 4);
      bmp_fmt.depth = 8;
      palette_size = 256 * 4;
      info.win.colors_used = 256;
      break;
    }
  }

  uint32_t image_offset = 2 + 12 + header_size + palette_size;
  uint32_t image_size = (bpl + gap) * h;
  uint32_t file_size = image_offset + image_size;

  file.file_size = file_size;
  file.image_offset = image_offset;
  info.win.header_size = header_size;
  info.win.bits_per_pixel = uint16_t(bmp_fmt.depth);
  info.win.image_size = image_size;

  if (palette_size == 0) {
    info.win.r_mask = (bmp_fmt.r_size ? IntOps::non_zero_lsb_mask<uint32_t>(bmp_fmt.r_size) : uint32_t(0)) << bmp_fmt.r_shift;
    info.win.g_mask = (bmp_fmt.g_size ? IntOps::non_zero_lsb_mask<uint32_t>(bmp_fmt.g_size) : uint32_t(0)) << bmp_fmt.g_shift;
    info.win.b_mask = (bmp_fmt.b_size ? IntOps::non_zero_lsb_mask<uint32_t>(bmp_fmt.b_size) : uint32_t(0)) << bmp_fmt.b_shift;
    info.win.a_mask = (bmp_fmt.a_size ? IntOps::non_zero_lsb_mask<uint32_t>(bmp_fmt.a_size) : uint32_t(0)) << bmp_fmt.a_shift;

    if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE)
      bmp_fmt.add_flags(BL_FORMAT_FLAG_BYTE_SWAP);

    // This should never fail as only a limited subset of possibilities exist
    // that are guaranteed by the implementation.
    BLResult result = pc.create(bmp_fmt, bl_format_info[format]);
    BL_ASSERT(result == BL_SUCCESS);

    // Maybe unused in release mode.
    bl_unused(result);
  }

  uint8_t* dst_data;
  BL_PROPAGATE(buf.modify_op(BL_MODIFY_OP_ASSIGN_FIT, file_size, &dst_data));

  const uint8_t* src_data = static_cast<const uint8_t*>(image_data.pixel_data);
  intptr_t src_stride = image_data.stride;

  if (BL_BYTE_ORDER_NATIVE != BL_BYTE_ORDER_LE) {
    file.byte_swap();
    info.win.byte_swap();
  }

  // Write file signature.
  memcpy(dst_data, "BM", 2);
  dst_data += 2;

  // Write file header.
  memcpy(dst_data, &file, 12);
  dst_data += 12;

  // Write info header.
  memcpy(dst_data, &info, header_size);
  dst_data += header_size;

  // Write palette and pixel data.
  if (palette_size == 0) {
    BLPixelConverterOptions opt {};
    opt.gap = gap;
    pc.convert_rect(dst_data, intptr_t(bpl + gap), src_data + (intptr_t(h - 1) * src_stride), -src_stride, w, h, &opt);
  }
  else {
    size_t i;

    for (i = 0; i < 256u; i++, dst_data += 4) {
      dst_data[0] = uint8_t(i);
      dst_data[1] = uint8_t(i);
      dst_data[2] = uint8_t(i);
      dst_data[3] = uint8_t(0xFFu);
    }

    for (i = h; i; i--) {
      memcpy(dst_data, src_data + intptr_t(i - 1) * src_stride, bpl);
      dst_data += bpl;
      MemOps::fill_inline_t(dst_data, uint8_t(0), gap);
      dst_data += gap;
    }
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_create_impl(BLImageEncoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_ENCODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLBmpEncoderImpl>(self, info));

  BLBmpEncoderImpl* encoder_impl = static_cast<BLBmpEncoderImpl*>(self->_d.impl);
  encoder_impl->ctor(&bmp_encoder_virt, &bmp_codec_instance);
  return encoder_restart_impl(encoder_impl);
}

static BLResult BL_CDECL encoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLBmpEncoderImpl* encoder_impl = static_cast<BLBmpEncoderImpl*>(impl);

  encoder_impl->dtor();
  return bl_object_free_impl(encoder_impl);
}

// bl::Bmp::Codec - Interface
// ==========================

static BLResult BL_CDECL codec_destroy_impl(BLObjectImpl* impl) noexcept {
  // Built-in codecs are never destroyed.
  bl_unused(impl);
  return BL_SUCCESS;
};

static uint32_t BL_CDECL codec_inspect_data_impl(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl);

  // BMP minimum size and signature (BM).
  if (size < 2 || data[0] != 0x42 || data[1] != 0x4D)
    return 0;

  // Return something low as we cannot validate the header.
  if (size < 18)
    return 1;

  // Check whether `data` contains a correct BMP header.
  uint32_t header_size = MemOps::readU32uLE(data + 14);
  if (!check_header_size(header_size))
    return 0;

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

// bl::Bmp::Codec - Runtime Registration
// =====================================

void bmp_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept {
  using namespace bl::Bmp;

  bl_unused(rt);

  // Initialize BMP codec.
  bmp_codec.virt.base.destroy = codec_destroy_impl;
  bmp_codec.virt.base.get_property = bl_object_impl_get_property;
  bmp_codec.virt.base.set_property = bl_object_impl_set_property;
  bmp_codec.virt.inspect_data = codec_inspect_data_impl;
  bmp_codec.virt.create_decoder = codec_create_decoder_impl;
  bmp_codec.virt.create_encoder = codec_create_encoder_impl;

  bmp_codec.impl->ctor(&bmp_codec.virt);
  bmp_codec.impl->features = BLImageCodecFeatures(BL_IMAGE_CODEC_FEATURE_READ     |
                                                  BL_IMAGE_CODEC_FEATURE_WRITE    |
                                                  BL_IMAGE_CODEC_FEATURE_LOSSLESS);
  bmp_codec.impl->name.dcast().assign("BMP");
  bmp_codec.impl->vendor.dcast().assign("Blend2D");
  bmp_codec.impl->mime_type.dcast().assign("image/x-bmp");
  bmp_codec.impl->extensions.dcast().assign("bmp|ras");

  bmp_codec_instance._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC), &bmp_codec.impl);

  // Initialize BMP decoder virtual functions.
  bmp_decoder_virt.base.destroy = decoder_destroy_impl;
  bmp_decoder_virt.base.get_property = bl_object_impl_get_property;
  bmp_decoder_virt.base.set_property = bl_object_impl_set_property;
  bmp_decoder_virt.restart = decoder_restart_impl;
  bmp_decoder_virt.read_info = decoder_read_info_impl;
  bmp_decoder_virt.read_frame = decoder_read_frame_impl;

  // Initialize BMP encoder virtual functions.
  bmp_encoder_virt.base.destroy = encoder_destroy_impl;
  bmp_encoder_virt.base.get_property = bl_object_impl_get_property;
  bmp_encoder_virt.base.set_property = bl_object_impl_set_property;
  bmp_encoder_virt.restart = encoder_restart_impl;
  bmp_encoder_virt.write_frame = encoder_write_frame_impl;

  codecs->append(bmp_codec_instance.dcast());
}

} // {bl::Bmp}
