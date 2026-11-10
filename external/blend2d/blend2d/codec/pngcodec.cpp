// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// The PNG codec is based on stb_image <https://github.com/nothings/stb>
// released into PUBLIC DOMAIN. Blend2D's PNG codec can be distributed
// under Blend2D's ZLIB license or under STB's PUBLIC DOMAIN as well.

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/array_p.h>
#include <blend2d/core/format.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/var_p.h>
#include <blend2d/codec/pngcodec_p.h>
#include <blend2d/codec/pngops_p.h>
#include <blend2d/compression/checksum_p.h>
#include <blend2d/compression/deflatedecoder_p.h>
#include <blend2d/compression/deflateencoder_p.h>
#include <blend2d/pixelops/scalar_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

namespace bl::Png {

// bl::Png::Codec - Globals
// ========================

static BLObjectEternalVirtualImpl<BLPngCodecImpl, BLImageCodecVirt> png_codec;
static BLImageCodecCore png_codec_instance;

static BLImageDecoderVirt png_decoder_virt;
static BLImageEncoderVirt png_encoder_virt;

// bl::Png::Codec - Constants
// ==========================

// PNG file signature (8 bytes).
static constexpr uint8_t kPngSignature[8] = { 0x89u, 0x50u, 0x4Eu, 0x47u, 0x0Du, 0x0Au, 0x1Au, 0x0Au };

// Allowed bits-per-sample per "ColorType"
static constexpr uint8_t kColorTypeBitDepthTable[7] = { 0x1Fu, 0u, 0x18u, 0x0Fu, 0x18u, 0u, 0x18u };

// Count of samples per "ColorType".
static constexpr uint8_t kColorTypeToSampleCountTable[7] = { 1u, 0u, 3u, 1u, 2u, 0u, 4u };

static constexpr uint32_t kPngSignatureSize = 8u;

static constexpr uint32_t kPngChunkHeaderSize = 8u;
static constexpr uint32_t kPngChunkCRCSize = 4u;
static constexpr uint32_t kPngChunkBaseSize = 12u;

static constexpr uint32_t kPngChunkDataSize_CgBI = 4u;
static constexpr uint32_t kPngChunkDataSize_IHDR = 13u;
static constexpr uint32_t kPngChunkDataSize_acTL = 8u;
static constexpr uint32_t kPngChunkDataSize_fcTL = 26;

// bl::Png::Codec - Utilities
// ==========================

static BL_INLINE bool check_color_type_and_bit_depth(uint32_t color_type, uint32_t depth) noexcept {
  // TODO: [PNG] 16-BPC.
  if (depth == 16)
    return false;

  return color_type < BL_ARRAY_SIZE(kColorTypeBitDepthTable) &&
         (kColorTypeBitDepthTable[color_type] & depth) != 0 &&
         IntOps::is_power_of_2(depth);
}

static BL_INLINE void create_grayscale_palette(BLRgba32* pal, uint32_t depth) noexcept {
  static const uint32_t scale_table[9] = { 0, 0xFF, 0x55, 0, 0x11, 0, 0, 0, 0x01 };
  BL_ASSERT(depth < BL_ARRAY_SIZE(scale_table));

  uint32_t scale = uint32_t(scale_table[depth]) * 0x00010101;
  uint32_t count = 1u << depth;
  uint32_t value = 0xFF000000;

  for (uint32_t i = 0; i < count; i++, value += scale)
    pal[i].value = value;
}

// bl::Png::Codec - Interlace / Deinterlace
// ========================================

// A single PNG interlace/deinterlace step related to the full image size.
struct InterlaceStep {
  uint32_t used;
  uint32_t width;
  uint32_t height;
  uint32_t bpl;

  uint32_t offset;
  uint32_t size;
};

// PNG deinterlace table data.
struct InterlaceTable {
  uint8_t x_off;
  uint8_t y_off;
  uint8_t x_pow;
  uint8_t y_pow;
};

// No interlacing.
static const InterlaceTable interlace_table_none[1] = {
  { 0, 0, 0, 0 }
};

// Passes start from zero to stay compatible with interlacing tables, however,
// this representation is not visually compatible with PNG spec, where passes
// are indexed from `1` (that's the only difference).
//
//        8x8 block
//   +-----------------+
//   | 0 5 3 5 1 5 3 5 |
//   | 6 6 6 6 6 6 6 6 |
//   | 4 5 4 5 4 5 4 5 |
//   | 6 6 6 6 6 6 6 6 |
//   | 2 5 3 5 2 5 3 5 |
//   | 6 6 6 6 6 6 6 6 |
//   | 4 5 4 5 4 5 4 5 |
//   | 6 6 6 6 6 6 6 6 |
//   +-----------------+
static const InterlaceTable interlaceTableAdam7[7] = {
  { 0, 0, 3, 3 },
  { 4, 0, 3, 3 },
  { 0, 4, 2, 3 },
  { 2, 0, 2, 2 },
  { 0, 2, 1, 2 },
  { 1, 0, 1, 1 },
  { 0, 1, 0, 1 }
};

static uint32_t calculate_interlace_steps(
  InterlaceStep* dst, const InterlaceTable* table, uint32_t step_count,
  uint32_t sample_depth, uint32_t sample_count,
  uint32_t w, uint32_t h) noexcept {

  // Byte-offset of each chunk.
  uint32_t offset = 0;

  for (uint32_t i = 0; i < step_count; i++, dst++) {
    const InterlaceTable& tab = table[i];

    uint32_t sx = 1 << tab.x_pow;
    uint32_t sy = 1 << tab.y_pow;
    uint32_t sw = (w + sx - tab.x_off - 1) >> tab.x_pow;
    uint32_t sh = (h + sy - tab.y_off - 1) >> tab.y_pow;

    // If the reference image contains fewer than five columns or fewer than
    // five rows, some passes will be empty, decoders must handle this case.
    uint32_t used = sw != 0 && sh != 0;

    // NOTE: No need to check for overflow at this point as we have already
    // calculated the total BPL of the whole image, and since interlacing is
    // splitting it into multiple images, it can't overflow the base size.
    uint32_t bpl = ((sw * sample_depth + 7) / 8) * sample_count + 1;
    uint32_t size = used ? bpl * sh : uint32_t(0);

    dst->used = used;
    dst->width = sw;
    dst->height = sh;
    dst->bpl = bpl;

    dst->offset = offset;
    dst->size = size;

    // Here we should be safe...
    bl::OverflowFlag of{};
    offset = IntOps::add_overflow(offset, size, &of);

    if (BL_UNLIKELY(of)) {
      return 0;
    }
  }

  return offset;
}

static BL_INLINE uint8_t combineByte1bpp(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3, uint32_t b4, uint32_t b5, uint32_t b6, uint32_t b7) noexcept {
  return uint8_t(((b0) & 0x80) | ((b1) & 0x40) | ((b2) & 0x20) | ((b3) & 0x10) | ((b4) & 0x08) | ((b5) & 0x04) | ((b6) & 0x02) | ((b7) & 0x01));
}

static BL_INLINE uint8_t combineByte2bpp(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) noexcept {
  return uint8_t(((b0) & 0xC0) + ((b1) & 0x30) + ((b2) & 0x0C) + ((b3) & 0x03));
}

static BL_INLINE uint8_t combineByte4bpp(uint32_t b0, uint32_t b1) noexcept {
  return uint8_t(((b0) & 0xF0) + ((b1) & 0x0F));
}

// Deinterlace a PNG image that has depth less than 8 bits. This is a bit tricky as one byte describes two or more
// pixels that can be fetched from 1st to 6th progressive images. Basically each bit depth is implemented separately
// as generic case would be very inefficient. Also, the destination image is handled pixel-by-pixel fetching data
// from all possible scanlines as necessary - this is a bit different when compared with `deinterlace_bytes()`.
template<uint32_t N>
static void deinterlace_bits(
  uint8_t* dst_line, intptr_t dst_stride, const BLPixelConverter& pc,
  uint8_t* tmp_line, intptr_t tmp_stride, const uint8_t* data, const InterlaceStep* steps,
  uint32_t w, uint32_t h) noexcept {

  const uint8_t* d0 = data + steps[0].offset;
  const uint8_t* d1 = data + steps[1].offset;
  const uint8_t* d2 = data + steps[2].offset;
  const uint8_t* d3 = data + steps[3].offset;
  const uint8_t* d4 = data + steps[4].offset;
  const uint8_t* d5 = data + steps[5].offset;

  BL_ASSERT(h != 0);

  // We store only to odd scanlines.
  uint32_t y = (h + 1) / 2;
  uint32_t n = 0;

  for (;;) {
    uint8_t* tmp_data = tmp_line + (intptr_t(n) * tmp_stride);
    uint32_t x = w;

    // 1-BPP
    // -----

    if (N == 1) {
      switch (n) {
        // [a b b b a b b b]
        // [0 5 3 5 1 5 3 5]
        case 0: {
          uint32_t a = 0, b;

          d0 += 1;
          d1 += (x >= 5);
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 32) {
            // Fetched every second iteration.
            if (!(a & 0x80000000))
              a = (uint32_t(*d0++)) + (uint32_t(*d1++) << 8) + 0x08000000u;

            b = (uint32_t(*d3++)      ) +
                (uint32_t(d5[0]) <<  8) +
                (uint32_t(d5[1]) << 16) ;
            d5 += 2;

            tmp_data[0] = combineByte1bpp(a     , b >>  9, b >> 2, b >> 10, a >> 12, b >> 11, b >> 5, b >> 12);
            tmp_data[1] = combineByte1bpp(a << 1, b >>  5, b     , b >>  6, a >> 11, b >>  7, b >> 3, b >>  8);
            tmp_data[2] = combineByte1bpp(a << 2, b >> 17, b << 2, b >> 18, a >> 10, b >> 19, b >> 1, b >> 20);
            tmp_data[3] = combineByte1bpp(a << 3, b >> 13, b << 4, b >> 14, a >>  9, b >> 15, b << 1, b >> 16);
            tmp_data += 4;

            a <<= 4;
            x -= 32;
          }

          if (!x)
            break;

          if (!(a & 0x80000000)) {
            a = uint32_t(*d0++);
            if (x >= 5)
              a += uint32_t(*d1++) << 8;
          }

          b = 0;
          if (x >=  3) b  = uint32_t(*d3++);
          if (x >=  2) b += uint32_t(*d5++) <<  8;
          if (x >= 18) b += uint32_t(*d5++) << 16;

          tmp_data[0] = combineByte1bpp(a     , b >>  9, b >> 2, b >> 10, a >> 12, b >> 11, b >> 5, b >> 12);
          if (x <= 8) break;

          tmp_data[1] = combineByte1bpp(a << 1, b >>  5, b     , b >>  6, a >> 11, b >>  7, b >> 3, b >>  8);
          if (x <= 16) break;

          tmp_data[2] = combineByte1bpp(a << 2, b >> 17, b << 2, b >> 18, a >> 10, b >> 19, b >> 1, b >> 20);
          if (x <= 24) break;

          tmp_data[3] = combineByte1bpp(a << 3, b >> 13, b << 4, b >> 14, a >>  9, b >> 15, b << 1, b >> 16);
          break;
        }

        // [a b a b a b a b]
        // [2 5 3 5 2 5 3 5]
        case 2: {
          uint32_t a, b;

          d2 += 1;
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 32) {
            a = uint32_t(*d2++) + (uint32_t(*d3++) << 8);
            b = uint32_t(d5[0]) + (uint32_t(d5[1]) << 8);
            d5 += 2;

            tmp_data[0] = combineByte1bpp(a     , b >>  1, a >> 10, b >>  2, a >>  3, b >>  3, a >> 13, b >>  4);
            tmp_data[1] = combineByte1bpp(a << 2, b <<  3, a >>  8, b <<  2, a >>  1, b <<  1, a >> 11, b      );
            tmp_data[2] = combineByte1bpp(a << 4, b >>  9, a >>  6, b >> 10, a <<  1, b >> 11, a >>  9, b >> 12);
            tmp_data[3] = combineByte1bpp(a << 6, b >>  5, a >>  4, b >>  6, a <<  3, b >>  7, a >>  7, b >>  8);
            tmp_data += 4;

            x -= 32;
          }

          if (!x)
            break;

          a = uint32_t(*d2++);
          b = 0;

          if (x >=  3) a += uint32_t(*d3++) << 8;
          if (x >=  2) b  = uint32_t(*d5++);
          if (x >= 18) b += uint32_t(*d5++) << 8;

          tmp_data[0] = combineByte1bpp(a     , b >>  1, a >> 10, b >>  2, a >>  3, b >>  3, a >> 13, b >>  4);
          if (x <=  8) break;

          tmp_data[1] = combineByte1bpp(a << 2, b <<  3, a >>  8, b <<  2, a >>  1, b <<  1, a >> 11, b      );
          if (x <= 16) break;

          tmp_data[2] = combineByte1bpp(a << 4, b >>  9, a >>  6, b >> 10, a <<  1, b >> 11, a >>  9, b >> 12);
          if (x <= 24) break;

          tmp_data[3] = combineByte1bpp(a << 6, b >>  5, a >>  4, b >>  6, a <<  3, b >>  7, a >>  7, b >>  8);
          break;
        }

        // [a b a b a b a b]
        // [4 5 4 5 4 5 4 5]
        case 1:
        case 3: {
          uint32_t a, b;

          d4 += 1;
          d5 += (x >= 2);

          while (x >= 16) {
            a = uint32_t(*d4++);
            b = uint32_t(*d5++);

            tmp_data[0] = combineByte1bpp(a     , b >> 1, a >> 1, b >> 2, a >> 2, b >> 3, a >> 3, b >> 4);
            tmp_data[1] = combineByte1bpp(a << 4, b << 3, a << 3, b << 2, a << 2, b << 1, a << 1, b     );
            tmp_data += 2;

            x -= 16;
          }

          if (!x)
            break;

          a = uint32_t(*d4++);
          b = 0;

          if (x >= 2)
            b = uint32_t(*d5++);

          tmp_data[0] = combineByte1bpp(a     , b >> 1, a >> 1, b >> 2, a >> 2, b >> 3, a >> 3, b >> 4);

          if (x <= 8) break;
          tmp_data[1] = combineByte1bpp(a << 4, b << 3, a << 3, b << 2, a << 2, b << 1, a << 1, b     );
          break;
        }
      }
    }

    // 2-BPP
    // -----

    else if (N == 2) {
      switch (n) {
        // [aa bb bb bb][aa bb bb bb]
        // [00 55 33 55][11 55 33 55]
        case 0: {
          uint32_t a = 0, b;

          d0 += 1;
          d1 += (x >= 5);
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 16) {
            // Fetched every second iteration.
            if (!(a & 0x80000000))
              a = (uint32_t(*d0++)     ) +
                  (uint32_t(*d1++) << 8) + 0x08000000;

            b = (uint32_t(*d3++)      ) +
                (uint32_t(d5[0]) <<  8) +
                (uint32_t(d5[1]) << 16) ;
            d5 += 2;

            tmp_data[0] = combineByte2bpp(a     , b >> 10, b >> 4, b >> 12);
            tmp_data[1] = combineByte2bpp(a >> 8, b >>  6, b >> 2, b >>  8);
            tmp_data[2] = combineByte2bpp(a << 2, b >> 18, b     , b >> 20);
            tmp_data[3] = combineByte2bpp(a >> 6, b >> 14, b << 2, b >> 16);
            tmp_data += 4;

            a <<= 4;
            x -= 16;
          }

          if (!x)
            break;

          if (!(a & 0x80000000)) {
            a = (uint32_t(*d0++));
            if (x >= 5)
              a += (uint32_t(*d1++) << 8);
          }

          b = 0;
          if (x >=  3) b  = (uint32_t(*d3++)      );
          if (x >=  2) b += (uint32_t(*d5++) <<  8);
          if (x >= 10) b += (uint32_t(*d5++) << 16);

          tmp_data[0] = combineByte2bpp(a     , b >> 10, b >> 4, b >> 12);
          if (x <=  4) break;

          tmp_data[1] = combineByte2bpp(a >> 8, b >>  6, b >> 2, b >>  8);
          if (x <=  8) break;

          tmp_data[2] = combineByte2bpp(a << 2, b >> 18, b     , b >> 20);
          if (x <= 12) break;

          tmp_data[3] = combineByte2bpp(a >> 6, b >> 14, b << 2, b >> 16);
          break;
        }

        // [aa bb aa bb][aa bb aa bb]
        // [22 55 33 55][22 55 33 55]
        case 2: {
          uint32_t a, b;

          d2 += 1;
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 16) {
            a = uint32_t(*d2++) + (uint32_t(*d3++) << 8);
            b = uint32_t(*d5++);

            tmp_data[0] = combineByte2bpp(a     , b >>  2, a >> 12, b >>  4);
            tmp_data[1] = combineByte2bpp(a << 2, b <<  2, a >> 10, b      );

            b = uint32_t(*d5++);

            tmp_data[2] = combineByte2bpp(a << 4, b >>  2, a >>  8, b >>  4);
            tmp_data[3] = combineByte2bpp(a << 6, b <<  2, a >>  6, b      );
            tmp_data += 4;

            x -= 16;
          }

          if (!x)
            break;

          a = uint32_t(*d2++);
          b = 0;

          if (x >=  3) a  = (uint32_t(*d3++) << 8);
          if (x >=  2) b  = (uint32_t(*d5++)     );
          if (x >= 10) b += (uint32_t(*d5++) << 8);

          tmp_data[0] = combineByte2bpp(a     , b >>  2, a >> 12, b >>  4);
          if (x <=  4) break;

          tmp_data[1] = combineByte2bpp(a << 2, b <<  2, a >> 10, b      );
          if (x <=  8) break;

          tmp_data[2] = combineByte2bpp(a << 4, b >> 10, a >>  8, b >> 12);
          if (x <= 12) break;

          tmp_data[3] = combineByte2bpp(a << 6, b >>  6, a >>  6, b >>  8);
          break;
        }

        // [aa bb aa bb][aa bb aa bb]
        // [44 55 44 55][44 55 44 55]
        case 1:
        case 3: {
          uint32_t a, b;

          d4 += 1;
          d5 += (x >= 2);

          while (x >= 8) {
            a = uint32_t(*d4++);
            b = uint32_t(*d5++);

            tmp_data[0] = combineByte2bpp(a     , b >> 2, a >> 2, b >> 4);
            tmp_data[1] = combineByte2bpp(a << 4, b << 2, a << 2, b     );
            tmp_data += 2;

            x -= 8;
          }

          if (!x)
            break;

          a = uint32_t(*d4++);
          b = 0;

          if (x >= 2)
            b = uint32_t(*d5++);

          tmp_data[0] = combineByte2bpp(a     , b >> 10, b >> 4, b >> 12);
          if (x <=  4) break;

          tmp_data[1] = combineByte2bpp(a >> 8, b >>  6, b >> 2, b >>  8);
          break;
        }
      }
    }

    // 4-BPP
    // -----

    else if (N == 4) {
      switch (n) {
        // [aaaa bbbb][bbbb bbbb][aaaa bbbb][bbbb bbbb]
        // [0000 5555][3333 5555][1111 5555][3333 5555]
        case 0: {
          uint32_t a = 0, b;

          d0 += 1;
          d1 += (x >= 5);
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 8) {
            // Fetched every second iteration.
            if (!(a & 0x80000000))
              a = (uint32_t(*d0++)      ) +
                  (uint32_t(*d1++) <<  8) + 0x08000000;

            b = (uint32_t(*d3++)      ) +
                (uint32_t(d5[0]) <<  8) +
                (uint32_t(d5[1]) << 16) ;
            d5 += 2;

            tmp_data[0] = combineByte4bpp(a     , b >> 12);
            tmp_data[1] = combineByte4bpp(b     , b >>  8);
            tmp_data[2] = combineByte4bpp(a >> 8, b >> 20);
            tmp_data[3] = combineByte4bpp(b << 4, b >> 16);
            tmp_data += 4;

            a <<= 4;
            x -= 8;
          }

          if (!x)
            break;

          if (!(a & 0x80000000)) {
            a = (uint32_t(*d0++));
            if (x >= 5)
              a += (uint32_t(*d1++) << 8);
          }

          b = 0;
          if (x >= 3) b  = (uint32_t(*d3++)      );
          if (x >= 2) b += (uint32_t(*d5++) <<  8);
          if (x >= 6) b += (uint32_t(*d5++) << 16);

          tmp_data[0] = combineByte4bpp(a, b >> 12);
          if (x <= 2) break;

          tmp_data[1] = combineByte4bpp(b, b >> 8);
          if (x <= 4) break;

          tmp_data[2] = combineByte4bpp(a >> 8, b >> 20);
          if (x <= 6) break;

          tmp_data[3] = combineByte4bpp(b << 4, b >> 16);
          break;
        }

        // [aaaa bbbb][aaaa bbbb][aaaa bbbb][aaaa bbbb]
        // [2222 5555][3333 5555][2222 5555][3333 5555]
        case 2: {
          uint32_t a, b;

          d2 += 1;
          d3 += (x >= 3);
          d5 += (x >= 2);

          while (x >= 8) {
            a = uint32_t(*d2++) + (uint32_t(*d3++) << 8);
            b = uint32_t(*d5++);
            tmp_data[0] = combineByte4bpp(a, b >> 4);
            tmp_data[1] = combineByte4bpp(a >> 8, b);

            b = uint32_t(*d5++);
            tmp_data[2] = combineByte4bpp(a << 4, b >> 4);
            tmp_data[3] = combineByte4bpp(a >> 4, b);
            tmp_data += 4;

            x -= 8;
          }

          if (!x)
            break;

          a = uint32_t(*d2++);
          b = 0;

          if (x >= 3) a += (uint32_t(*d3++) << 8);
          if (x >= 2) b  = (uint32_t(*d5++)     );

          tmp_data[0] = combineByte4bpp(a, b >> 4);
          if (x <= 2) break;

          tmp_data[1] = combineByte4bpp(a >> 8, b);
          if (x <= 4) break;

          b = uint32_t(*d5++);
          tmp_data[2] = combineByte4bpp(a << 4, b >> 4);
          if (x <= 6) break;

          tmp_data[3] = combineByte4bpp(a >> 4, b);
          break;
        }

        // [aaaa bbbb aaaa bbbb][aaaa bbbb aaaa bbbb]
        // [4444 5555 4444 5555][4444 5555 4444 5555]
        case 1:
        case 3: {
          uint32_t a, b;

          d4 += 1;
          d5 += (x >= 2);

          while (x >= 4) {
            a = uint32_t(*d4++);
            b = uint32_t(*d5++);

            tmp_data[0] = combineByte4bpp(a, b >> 4);
            tmp_data[1] = combineByte4bpp(a << 4, b);
            tmp_data += 2;

            x -= 4;
          }

          if (!x)
            break;

          a = uint32_t(*d4++);
          b = 0;

          if (x >= 2)
            b = uint32_t(*d5++);

          tmp_data[0] = combineByte4bpp(a, b >> 4);
          if (x <= 2) break;

          tmp_data[1] = combineByte4bpp(a << 4, b);
          break;
        }
      }
    }

    // Don't change to `||`, both have to be executed!
    if (uint32_t(--y == 0) | uint32_t(++n == 4)) {
      pc.convert_rect(dst_line, dst_stride * 2, tmp_line, tmp_stride, w, n);
      dst_line += dst_stride * 8;

      if (y == 0)
        break;
      n = 0;
    }
  }
}

// Copy `N` bytes from unaligned `src` into aligned `dst`. Allows us to handle
// some special cases if the CPU supports unaligned reads/writes from/to memory.
template<uint32_t N>
static BL_INLINE const uint8_t* copy_bytes(uint8_t* dst, const uint8_t* src) noexcept {
  if (N == 2) {
    MemOps::writeU16a(dst, MemOps::readU16u(src));
  }
  else if (N == 4) {
    MemOps::writeU32a(dst, MemOps::readU32u(src));
  }
  else if (N == 8) {
    MemOps::writeU32a(dst + 0, MemOps::readU32u(src + 0));
    MemOps::writeU32a(dst + 4, MemOps::readU32u(src + 4));
  }
  else {
    if (N >= 1) dst[0] = src[0];
    if (N >= 2) dst[1] = src[1];
    if (N >= 3) dst[2] = src[2];
    if (N >= 4) dst[3] = src[3];
    if (N >= 5) dst[4] = src[4];
    if (N >= 6) dst[5] = src[5];
    if (N >= 7) dst[6] = src[6];
    if (N >= 8) dst[7] = src[7];
  }
  return src + N;
}

template<uint32_t N>
static void deinterlace_bytes(
  uint8_t* dst_line, intptr_t dst_stride, const BLPixelConverter& pc,
  uint8_t* tmp_line, intptr_t tmp_stride, const uint8_t* data, const InterlaceStep* steps,
  uint32_t w, uint32_t h) noexcept {

  const uint8_t* d0 = data + steps[0].offset;
  const uint8_t* d1 = data + steps[1].offset;
  const uint8_t* d2 = data + steps[2].offset;
  const uint8_t* d3 = data + steps[3].offset;
  const uint8_t* d4 = data + steps[4].offset;
  const uint8_t* d5 = data + steps[5].offset;

  BL_ASSERT(h != 0);

  // We store only to odd scanlines.
  uint32_t y = (h + 1) / 2;
  uint32_t n = 0;
  uint32_t x_max = w * N;

  for (;;) {
    uint8_t* tmp_data = tmp_line + (intptr_t(n) * tmp_stride);
    uint32_t x;

    switch (n) {
      // [05351535]
      case 0: {
        d0 += 1;
        d1 += (w >= 5);
        d3 += (w >= 3);
        d5 += (w >= 2);

        for (x = 0 * N; x < x_max; x += 8 * N) d0 = copy_bytes<N>(tmp_data + x, d0);
        for (x = 4 * N; x < x_max; x += 8 * N) d1 = copy_bytes<N>(tmp_data + x, d1);
        for (x = 2 * N; x < x_max; x += 4 * N) d3 = copy_bytes<N>(tmp_data + x, d3);
        for (x = 1 * N; x < x_max; x += 2 * N) d5 = copy_bytes<N>(tmp_data + x, d5);

        break;
      }

      // [25352535]
      case 2: {
        d2 += 1;
        d3 += (w >= 3);
        d5 += (w >= 2);

        for (x = 0 * N; x < x_max; x += 4 * N) d2 = copy_bytes<N>(tmp_data + x, d2);
        for (x = 2 * N; x < x_max; x += 4 * N) d3 = copy_bytes<N>(tmp_data + x, d3);
        for (x = 1 * N; x < x_max; x += 2 * N) d5 = copy_bytes<N>(tmp_data + x, d5);

        break;
      }

      // [45454545]
      case 1:
      case 3: {
        d4 += 1;
        d5 += (w >= 2);

        for (x = 0 * N; x < x_max; x += 2 * N) d4 = copy_bytes<N>(tmp_data + x, d4);
        for (x = 1 * N; x < x_max; x += 2 * N) d5 = copy_bytes<N>(tmp_data + x, d5);

        break;
      }
    }

    // Don't change to `||`, both have to be executed!
    if (uint32_t(--y == 0) | uint32_t(++n == 4)) {
      pc.convert_rect(dst_line, dst_stride * 2, tmp_line, tmp_stride, w, n);
      dst_line += dst_stride * 8;

      if (y == 0)
        break;
      n = 0;
    }
  }
}

// bl::Png::Codec - Decoder - ChunkReader
// ======================================

namespace {

class ChunkReader {
public:
  const uint8_t* ptr {};
  const uint8_t* end {};

  BL_INLINE_NODEBUG ChunkReader(const uint8_t* ptr, const uint8_t* end) noexcept
    : ptr(ptr),
      end(end) {}

  BL_INLINE_NODEBUG size_t remaining_bytes() const noexcept { return PtrOps::bytes_until(ptr, end); }

  BL_INLINE void advance(size_t size) noexcept {
    BL_ASSERT(size <= remaining_bytes());
    ptr += size;
  }

  BL_INLINE void advance_chunk_header() noexcept {
    BL_ASSERT(remaining_bytes() >= kPngChunkHeaderSize);
    ptr += kPngChunkHeaderSize;
  }

  BL_INLINE void advance_checksum() noexcept {
    BL_ASSERT(remaining_bytes() >= kPngChunkCRCSize);
    ptr += kPngChunkCRCSize;
  }

  BL_INLINE_NODEBUG bool at_end() const noexcept { return ptr == end; }
  BL_INLINE_NODEBUG bool has_chunk() const noexcept { return remaining_bytes() >= kPngChunkBaseSize; }

  BL_INLINE_NODEBUG bool has_chunk_with_size(size_t size) const noexcept {
    // Always called after `has_chunk()` with the advertized size of the chunk, so we always have at least 12 bytes.
    BL_ASSERT(remaining_bytes() >= kPngChunkBaseSize);

    return remaining_bytes() - kPngChunkBaseSize >= size;
  }

  BL_INLINE uint32_t read_chunk_size() const noexcept {
    BL_ASSERT(has_chunk());
    return MemOps::readU32uBE(ptr + 0);
  }

  BL_INLINE uint32_t read_chunk_tag() const noexcept {
    BL_ASSERT(has_chunk());
    return MemOps::readU32uBE(ptr + 4);
  }

  BL_INLINE uint32_t readUInt8(size_t offset) const noexcept {
    BL_ASSERT(offset + 1u <= remaining_bytes());
    return MemOps::readU8(ptr + offset);
  }

  BL_INLINE uint32_t readUInt16(size_t offset) const noexcept {
    BL_ASSERT(offset + 2u <= remaining_bytes());
    return MemOps::readU16uBE(ptr + offset);
  }

  BL_INLINE uint32_t readUInt32(size_t offset) const noexcept {
    BL_ASSERT(offset + 4u <= remaining_bytes());
    return MemOps::readU32uBE(ptr + offset);
  }
};

} // {anonymous}

// bl::Png::Codec - Decoder - API
// ==============================

static BLResult BL_CDECL decoder_restart_impl(BLImageDecoderImpl* impl) noexcept {
  BLPngDecoderImpl* decoder_impl = static_cast<BLPngDecoderImpl*>(impl);

  decoder_impl->last_result = BL_SUCCESS;
  decoder_impl->frame_index = 0;
  decoder_impl->buffer_index = 0;

  decoder_impl->image_info.reset();
  decoder_impl->status_flags = DecoderStatusFlags::kNone;
  decoder_impl->color_type = 0;
  decoder_impl->sample_depth = 0;
  decoder_impl->sample_count = 0;
  decoder_impl->output_format = uint8_t(BL_FORMAT_NONE);
  decoder_impl->color_key.reset();
  decoder_impl->palette_size = 0;
  decoder_impl->first_fctl_offset = 0;
  decoder_impl->prev_ctrl = FCTL{};
  decoder_impl->frame_ctrl = FCTL{};

  return BL_SUCCESS;
}

static BLResult decoderReadFCTL(BLPngDecoderImpl* decoder_impl, size_t chunk_offset, BLArrayView<uint8_t> chunk) noexcept {
  if (BL_UNLIKELY(chunk.size < kPngChunkDataSize_fcTL)) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  uint32_t n = MemOps::readU32uBE(chunk.data + 0u);
  uint32_t w = MemOps::readU32uBE(chunk.data + 4u);
  uint32_t h = MemOps::readU32uBE(chunk.data + 8u);
  uint32_t x = MemOps::readU32uBE(chunk.data + 12u);
  uint32_t y = MemOps::readU32uBE(chunk.data + 16u);
  uint32_t delay_num = MemOps::readU16uBE(chunk.data + 20u);
  uint32_t delay_den = MemOps::readU16uBE(chunk.data + 22u);
  uint32_t dispose_op = MemOps::readU8(chunk.data + 24u);
  uint32_t blend_op = MemOps::readU8(chunk.data + 25u);

  if (BL_UNLIKELY(x >= uint32_t(decoder_impl->image_info.size.w) ||
                  y >= uint32_t(decoder_impl->image_info.size.h) ||
                  w > uint32_t(decoder_impl->image_info.size.w) - x ||
                  h > uint32_t(decoder_impl->image_info.size.h) - y ||
                  dispose_op > kAPNGDisposeOpMaxValue ||
                  blend_op > kAPNGBlendOpMaxValue)) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  if (decoder_impl->first_fctl_offset == 0) {
    decoder_impl->first_fctl_offset = chunk_offset;
  }

  decoder_impl->prev_ctrl = decoder_impl->frame_ctrl;
  decoder_impl->frame_ctrl.sequence_number = n;
  decoder_impl->frame_ctrl.w = w;
  decoder_impl->frame_ctrl.h = h;
  decoder_impl->frame_ctrl.x = x;
  decoder_impl->frame_ctrl.y = y;
  decoder_impl->frame_ctrl.delay_num = uint16_t(delay_num);
  decoder_impl->frame_ctrl.delay_den = uint16_t(delay_den);
  decoder_impl->frame_ctrl.dispose_op = uint8_t(dispose_op);
  decoder_impl->frame_ctrl.blend_op = uint8_t(blend_op);
  decoder_impl->add_flag(DecoderStatusFlags::kRead_fcTL);

  return BL_SUCCESS;
}

static BLResult decoder_read_info_internal(BLPngDecoderImpl* decoder_impl, const uint8_t* p, size_t size) noexcept {
  const size_t kMinSize_PNG = kPngSignatureSize + kPngChunkBaseSize + kPngChunkDataSize_IHDR;
  const size_t kMinSize_CgBI = kMinSize_PNG + kPngChunkBaseSize + kPngChunkDataSize_CgBI;

  if (BL_UNLIKELY(size < kMinSize_PNG)) {
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  // Check PNG signature.
  if (BL_UNLIKELY(memcmp(p, kPngSignature, kPngSignatureSize) != 0)) {
    return bl_make_error(BL_ERROR_INVALID_SIGNATURE);
  }

  ChunkReader chunk_reader(p + kPngSignatureSize, p + size - kPngSignatureSize);

  // Already verified by `kMinSize_PNG` check - so it must be true.
  BL_ASSERT(chunk_reader.has_chunk());

  // Expect 'IHDR' or 'CgBI' chunk.
  uint32_t chunk_tag = chunk_reader.read_chunk_tag();
  uint32_t chunk_size = chunk_reader.read_chunk_size();

  // Read 'CgBI' Chunk (4 Bytes)
  // ---------------------------

  // Support "CgBI" aka "CoreGraphicsBrokenImage" - a violation of the PNG Spec:
  //   1. http://www.jongware.com/pngdefry.html
  //   2. http://iphonedevwiki.net/index.php/CgBI_file_format
  if (chunk_tag == BL_MAKE_TAG('C', 'g', 'B', 'I')) {
    if (BL_UNLIKELY(chunk_size != kPngChunkDataSize_CgBI)) {
      return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
    }

    if (BL_UNLIKELY(size < kMinSize_CgBI)) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    decoder_impl->add_flag(DecoderStatusFlags::kRead_CgBI);

    // Skip "CgBI" chunk and read the next chunk tag/size, which must be 'IHDR'.
    chunk_reader.advance(kPngChunkBaseSize + kPngChunkDataSize_CgBI);

    chunk_tag = chunk_reader.read_chunk_tag();
    chunk_size = chunk_reader.read_chunk_size();
  }

  // Read 'IHDR' Chunk (13 Bytes)
  // ----------------------------

  if (chunk_tag != BL_MAKE_TAG('I', 'H', 'D', 'R') || chunk_size != kPngChunkDataSize_IHDR) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  uint32_t w           = chunk_reader.readUInt32(kPngChunkHeaderSize + 0u);
  uint32_t h           = chunk_reader.readUInt32(kPngChunkHeaderSize + 4u);
  uint32_t sample_depth = chunk_reader.readUInt8(kPngChunkHeaderSize + 8u);
  uint32_t color_type   = chunk_reader.readUInt8(kPngChunkHeaderSize + 9u);
  uint32_t compression = chunk_reader.readUInt8(kPngChunkHeaderSize + 10u);
  uint32_t filter      = chunk_reader.readUInt8(kPngChunkHeaderSize + 11u);
  uint32_t progressive = chunk_reader.readUInt8(kPngChunkHeaderSize + 12u);

  chunk_reader.advance(kPngChunkBaseSize + kPngChunkDataSize_IHDR);

  // Width/Height can't be zero or greater than `2^31 - 1`.
  if (BL_UNLIKELY(w == 0 || h == 0)) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  if (BL_UNLIKELY(w >= 0x80000000u || h >= 0x80000000u)) {
    return bl_make_error(BL_ERROR_IMAGE_TOO_LARGE);
  }

  if (BL_UNLIKELY(!check_color_type_and_bit_depth(color_type, sample_depth))) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  // Compression and filter has to be zero, progressive can be [0, 1].
  if (BL_UNLIKELY(compression != 0 || filter != 0 || progressive >= 2)) {
    return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
  }

  // Setup the image information.
  decoder_impl->add_flag(DecoderStatusFlags::kRead_IHDR);
  decoder_impl->color_type = uint8_t(color_type);
  decoder_impl->sample_depth = uint8_t(sample_depth);
  decoder_impl->sample_count = kColorTypeToSampleCountTable[color_type];

  decoder_impl->image_info.size.reset(int(w), int(h));
  decoder_impl->image_info.depth = uint16_t(sample_depth * uint32_t(decoder_impl->sample_count));
  decoder_impl->image_info.frame_count = 1;
  decoder_impl->image_info.flags = progressive ? BL_IMAGE_INFO_FLAG_PROGRESSIVE : BL_IMAGE_INFO_FLAG_NO_FLAGS;

  memcpy(decoder_impl->image_info.format, "PNG", 4);
  memcpy(decoder_impl->image_info.compression, "DEFLATE", 8);

  BLFormat output_format = color_type == kColorType2_RGB ? BL_FORMAT_XRGB32 : BL_FORMAT_PRGB32;

  decoder_impl->output_format = uint8_t(output_format);
  decoder_impl->buffer_index = PtrOps::byte_offset(p, chunk_reader.ptr);

  // Read Extra Chunks to Detect APNG
  // --------------------------------

  while (chunk_reader.has_chunk()) {
    chunk_tag = chunk_reader.read_chunk_tag();
    chunk_size = chunk_reader.read_chunk_size();

    if (BL_UNLIKELY(!chunk_reader.has_chunk_with_size(chunk_size))) {
      break;
    }

    if (chunk_tag == BL_MAKE_TAG('a', 'c', 'T', 'L')) {
      // Animated PNG chunk.
      if (chunk_size != kPngChunkDataSize_acTL) {
        // Don't refuse the file, but don't mark it as APNG (we would just treat it as a regular PNG if 'acTL' is broken).
        break;
      }

      uint32_t frame_count = chunk_reader.readUInt32(kPngChunkHeaderSize + 0u);
      uint32_t repeat_count = chunk_reader.readUInt32(kPngChunkHeaderSize + 4u);

      if (frame_count <= 1) {
        break;
      }

      decoder_impl->image_info.frame_count = frame_count;
      decoder_impl->image_info.repeat_count = repeat_count;
      memcpy(decoder_impl->image_info.format, "APNG", 5);
      decoder_impl->add_flag(DecoderStatusFlags::kRead_acTL);
      break;
    }

    if ((chunk_tag == BL_MAKE_TAG('I', 'H', 'D', 'R')) ||
        (chunk_tag == BL_MAKE_TAG('P', 'L', 'T', 'E')) ||
        (chunk_tag == BL_MAKE_TAG('I', 'D', 'A', 'T')) ||
        (chunk_tag == BL_MAKE_TAG('I', 'E', 'N', 'D'))) {
      break;
    }

    chunk_reader.advance(size_t(kPngChunkBaseSize) + size_t(chunk_size));
  }

  return BL_SUCCESS;
}

// Reads initial chunks and stops at the beginning of pixel data ('IDAT' and 'fdAT') or 'IEND'.
static BLResult decoder_read_important_chunks(BLPngDecoderImpl* decoder_impl, const uint8_t* p, size_t size) noexcept {
  // Don't read beyond the user provided buffer.
  if (BL_UNLIKELY(size < decoder_impl->buffer_index)) {
    return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  ChunkReader chunk_reader(p + decoder_impl->buffer_index, p + size);
  for (;;) {
    if (BL_UNLIKELY(!chunk_reader.has_chunk())) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    uint32_t chunk_tag = chunk_reader.read_chunk_tag();
    uint32_t chunk_size = chunk_reader.read_chunk_size();

    if (BL_UNLIKELY(!chunk_reader.has_chunk_with_size(chunk_size))) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    if (chunk_tag == BL_MAKE_TAG('P', 'L', 'T', 'E')) {
      // Read 'PLTE' Chunk (Once)
      // ------------------------

      // 1. There must not be more than one PLTE chunk.
      // 2. It must precede the first IDAT chunk (also tRNS chunk).
      // 3. Contains 1...256 RGB palette entries.
      if (decoder_impl->has_flag(DecoderStatusFlags::kRead_PLTE | DecoderStatusFlags::kRead_tRNS)) {
        return bl_make_error(BL_ERROR_PNG_INVALID_PLTE);
      }

      if (chunk_size == 0 || chunk_size > 768 || (chunk_size % 3) != 0) {
        return bl_make_error(BL_ERROR_PNG_INVALID_PLTE);
      }

      chunk_reader.advance_chunk_header();

      uint32_t i = 0u;
      uint32_t palette_size = chunk_size / 3;

      decoder_impl->add_flag(DecoderStatusFlags::kRead_PLTE);
      decoder_impl->palette_size = palette_size;

      while (i < palette_size) {
        decoder_impl->palette_data[i++] = BLRgba32(chunk_reader.readUInt8(0u),
                                              chunk_reader.readUInt8(1u),
                                              chunk_reader.readUInt8(2u));
        chunk_reader.advance(3);
      }

      while (i < 256u) {
        decoder_impl->palette_data[i++] = BLRgba32(0x00, 0x00, 0x00, 0xFF);
      }

      chunk_reader.advance(4u); // CRC32
    }
    else if (chunk_tag == BL_MAKE_TAG('t', 'R', 'N', 'S')) {
      // Read 'tRNS' Chunk (Once)
      // ------------------------

      uint32_t color_type = decoder_impl->color_type;

      // 1. There must not be more than one 'tRNS' chunk.
      // 2. It must precede the first 'IDAT' chunk and follow a 'PLTE' chunk, if any.
      // 3. It is prohibited for color types 4 and 6.
      if (decoder_impl->has_flag(DecoderStatusFlags::kRead_tRNS)) {
        return bl_make_error(BL_ERROR_PNG_INVALID_TRNS);
      }

      if (color_type == kColorType4_LUMA || color_type == kColorType6_RGBA) {
        return bl_make_error(BL_ERROR_PNG_INVALID_TRNS);
      }

      if (color_type == kColorType0_LUM) {
        // For color type 0 (grayscale), the tRNS chunk contains a single gray level value, stored in the format:
        //   [0..1] Gray:  2 bytes, range 0 .. (2^depth)-1
        if (chunk_size != 2u) {
          return bl_make_error(BL_ERROR_PNG_INVALID_TRNS);
        }

        uint32_t gray = chunk_reader.readUInt16(kPngChunkHeaderSize);
        decoder_impl->color_key.reset(gray, gray, gray, 0u);
        decoder_impl->add_flag(DecoderStatusFlags::kHasColorKey);

        chunk_reader.advance(kPngChunkBaseSize + 2u);
      }
      else if (color_type == kColorType2_RGB) {
        // For color type 2 (truecolor), the tRNS chunk contains a single RGB color value, stored in the format:
        //   [0..1] Red:   2 bytes, range 0 .. (2^depth)-1
        //   [2..3] Green: 2 bytes, range 0 .. (2^depth)-1
        //   [4..5] Blue:  2 bytes, range 0 .. (2^depth)-1
        if (chunk_size != 6u) {
          return bl_make_error(BL_ERROR_PNG_INVALID_TRNS);
        }

        uint32_t r = chunk_reader.readUInt16(kPngChunkHeaderSize + 0u);
        uint32_t g = chunk_reader.readUInt16(kPngChunkHeaderSize + 2u);
        uint32_t b = chunk_reader.readUInt16(kPngChunkHeaderSize + 4u);

        decoder_impl->color_key.reset(r, g, b, 0u);
        decoder_impl->add_flag(DecoderStatusFlags::kHasColorKey);

        chunk_reader.advance(kPngChunkBaseSize + 6u);
      }
      else {
        // For color type 3 (indexed color), the tRNS chunk contains a series of one-byte alpha values, corresponding
        // to entries in the PLTE chunk.
        BL_ASSERT(color_type == kColorType3_PAL);

        // 1. Has to follow PLTE if color type is 3.
        // 2. The tRNS chunk can contain 1...pal_size alpha values, but in general it can contain less than `pal_size`
        //    values, in that case the remaining alpha values are assumed to be 255.
        if (!decoder_impl->has_flag(DecoderStatusFlags::kRead_PLTE) || chunk_size == 0u || chunk_size > decoder_impl->palette_size) {
          return bl_make_error(BL_ERROR_PNG_INVALID_TRNS);
        }

        chunk_reader.advance_chunk_header();

        for (uint32_t i = 0; i < chunk_size; i++) {
          decoder_impl->palette_data[i].setA(chunk_reader.readUInt8(i));
        }

        chunk_reader.advance(chunk_size + 4u);
      }

      decoder_impl->add_flag(DecoderStatusFlags::kRead_tRNS);
    }
    else if (chunk_tag == BL_MAKE_TAG('I', 'H', 'D', 'R') ||
             chunk_tag == BL_MAKE_TAG('I', 'D', 'A', 'T') ||
             chunk_tag == BL_MAKE_TAG('I', 'E', 'N', 'D') ||
             chunk_tag == BL_MAKE_TAG('f', 'c', 'T', 'L')) {
      // Stop - these will be read by a different function.
      break;
    }
    else {
      if (chunk_tag == BL_MAKE_TAG('f', 'c', 'T', 'L') && decoder_impl->isAPNG()) {
        if (decoder_impl->has_fctl()) {
          return bl_make_error(BL_ERROR_INVALID_DATA);
        }

        BL_PROPAGATE(
          decoderReadFCTL(
            decoder_impl,
            PtrOps::byte_offset(p, chunk_reader.ptr),
            BLArrayView<uint8_t>{chunk_reader.ptr + kPngChunkHeaderSize, chunk_size}));
      }

      // Skip unknown or known, but unsupported chunks.
      chunk_reader.advance(kPngChunkBaseSize + chunk_size);
    }
  }

  // Create a pixel converter capable of converting PNG pixel data to BLImage pixel data.
  BLFormatInfo png_fmt {};
  png_fmt.depth = decoder_impl->sample_depth;

  if (BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE) {
    png_fmt.add_flags(BL_FORMAT_FLAG_BYTE_SWAP);
  }

  if (decoder_impl->color_type == kColorType0_LUM && decoder_impl->sample_depth <= 8) {
    // Treat grayscale images up to 8bpp as indexed and create a dummy palette.
    create_grayscale_palette(decoder_impl->palette_data, decoder_impl->sample_depth);

    // Handle color-key properly.
    if (decoder_impl->has_color_key() && decoder_impl->color_key.r() < (1u << decoder_impl->sample_depth)) {
      decoder_impl->palette_data[decoder_impl->color_key.r()] = BLRgba32(0u);
    }

    png_fmt.add_flags(BLFormatFlags(BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_INDEXED));
    png_fmt.palette = decoder_impl->palette_data;
  }
  else if (decoder_impl->color_type == kColorType3_PAL) {
    png_fmt.add_flags(BLFormatFlags(BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_INDEXED));
    png_fmt.palette = decoder_impl->palette_data;
  }
  else {
    png_fmt.depth *= decoder_impl->sample_count;

    if (decoder_impl->color_type == kColorType0_LUM) {
      // TODO: [PNG] 16-BPC.
    }
    else if (decoder_impl->color_type == kColorType2_RGB) {
      png_fmt.add_flags(BL_FORMAT_FLAG_RGB);
      png_fmt.r_size = 8; png_fmt.r_shift = 16;
      png_fmt.g_size = 8; png_fmt.g_shift = 8;
      png_fmt.b_size = 8; png_fmt.b_shift = 0;
    }
    else if (decoder_impl->color_type == kColorType4_LUMA) {
      png_fmt.add_flags(BL_FORMAT_FLAG_LUMA);
      png_fmt.r_size = 8; png_fmt.r_shift = 8;
      png_fmt.g_size = 8; png_fmt.g_shift = 8;
      png_fmt.b_size = 8; png_fmt.b_shift = 8;
      png_fmt.a_size = 8; png_fmt.a_shift = 0;
    }
    else if (decoder_impl->color_type == kColorType6_RGBA) {
      png_fmt.add_flags(BL_FORMAT_FLAG_RGBA);
      png_fmt.r_size = 8; png_fmt.r_shift = 24;
      png_fmt.g_size = 8; png_fmt.g_shift = 16;
      png_fmt.b_size = 8; png_fmt.b_shift = 8;
      png_fmt.a_size = 8; png_fmt.a_shift = 0;
    }

    if (decoder_impl->isCGBI()) {
      BLInternal::swap(png_fmt.r_shift, png_fmt.b_shift);
      if (png_fmt.has_flag(BL_FORMAT_FLAG_ALPHA)) {
        png_fmt.add_flags(BL_FORMAT_FLAG_PREMULTIPLIED);
      }
    }
  }

  BL_PROPAGATE(decoder_impl->pixel_converter.create(bl_format_info[decoder_impl->output_format], png_fmt,
    BLPixelConverterCreateFlags(
      BL_PIXEL_CONVERTER_CREATE_FLAG_DONT_COPY_PALETTE |
      BL_PIXEL_CONVERTER_CREATE_FLAG_ALTERABLE_PALETTE)));

  decoder_impl->buffer_index = PtrOps::byte_offset(p, chunk_reader.ptr);
  return BL_SUCCESS;
}

static void copy_pixels(uint8_t* dst_data, intptr_t dst_stride, const uint8_t* src_data, intptr_t src_stride, size_t w, uint32_t h) noexcept {
  for (uint32_t i = 0; i < h; i++) {
    memcpy(dst_data, src_data, w);
    dst_data += dst_stride;
    src_data += src_stride;
  }
}

static void zero_pixels(uint8_t* dst_data, intptr_t dst_stride, size_t w, uint32_t h) noexcept {
  for (uint32_t i = 0; i < h; i++) {
    memset(dst_data, 0, w);
    dst_data += dst_stride;
  }
}

static BLResult decoder_read_pixel_data(BLPngDecoderImpl* decoder_impl, BLImage* image_out, const uint8_t* input, size_t size) noexcept {
  // Number of bytes to overallocate so the DEFLATE decoder doesn't have to run the slow loop at the end.
  constexpr uint32_t kOutputSizeScratch = 1024u;

  // Make sure we won't initialize our chunk reader out of range.
  if (BL_UNLIKELY(size < decoder_impl->buffer_index)) {
    return bl_make_error(BL_ERROR_INVALID_STATE);
  }

  ChunkReader chunk_reader(input + decoder_impl->buffer_index, input + size);

  uint32_t x = 0u;
  uint32_t y = 0u;
  uint32_t w = uint32_t(decoder_impl->image_info.size.w);
  uint32_t h = uint32_t(decoder_impl->image_info.size.h);

  // Advance Chunks
  // --------------

  uint32_t frame_tag =
    (decoder_impl->frame_index == 0u)
      ? BL_MAKE_TAG('I', 'D', 'A', 'T')
      : BL_MAKE_TAG('f', 'd', 'A', 'T');

  // Process all preceding chunks, which are not 'IDAT' or 'fdAT'.
  for (;;) {
    if (BL_UNLIKELY(!chunk_reader.has_chunk())) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    uint32_t chunk_tag = chunk_reader.read_chunk_tag();
    uint32_t chunk_size = chunk_reader.read_chunk_size();

    if (BL_UNLIKELY(!chunk_reader.has_chunk_with_size(chunk_size))) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    if (chunk_tag == frame_tag) {
      // Found a frame chunk.
      break;
    }

    if (chunk_tag == BL_MAKE_TAG('I', 'H', 'D', 'R')) {
      return bl_make_error(BL_ERROR_PNG_MULTIPLE_IHDR);
    }

    if (chunk_tag == BL_MAKE_TAG('I', 'E', 'N', 'D')) {
      return bl_make_error(BL_ERROR_PNG_INVALID_IEND);
    }

    if (chunk_tag == BL_MAKE_TAG('f', 'c', 'T', 'L') && decoder_impl->isAPNG()) {
      if (decoder_impl->has_fctl()) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }
      BL_PROPAGATE(
        decoderReadFCTL(
          decoder_impl,
          PtrOps::byte_offset(input, chunk_reader.ptr),
          BLArrayView<uint8_t>{chunk_reader.ptr + 8u, chunk_size}));
    }

    chunk_reader.advance(size_t(kPngChunkBaseSize) + chunk_size);
  }

  // Handle APNG Frame Window
  // ------------------------

  if (decoder_impl->frame_index != 0u) {
    if (!decoder_impl->has_fctl()) {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    x = decoder_impl->frame_ctrl.x;
    y = decoder_impl->frame_ctrl.y;
    w = decoder_impl->frame_ctrl.w;
    h = decoder_impl->frame_ctrl.h;
  }

  // Decode Pixel Data (DEFLATE)
  // ---------------------------

  uint32_t sample_depth = decoder_impl->sample_depth;
  uint32_t sample_count = decoder_impl->sample_count;

  uint32_t progressive = (decoder_impl->image_info.flags & BL_IMAGE_INFO_FLAG_PROGRESSIVE) != 0;
  uint32_t step_count = progressive ? 7 : 1;

  InterlaceStep steps[7];
  uint32_t png_pixel_data_size = calculate_interlace_steps(steps, progressive ? interlaceTableAdam7 : interlace_table_none, step_count, sample_depth, sample_count, w, h);

  if (BL_UNLIKELY(png_pixel_data_size == 0)) {
    return bl_make_error(BL_ERROR_INVALID_DATA);
  }

  BL_PROPAGATE(decoder_impl->deflate_decoder.init(decoder_impl->deflate_format(), Compression::Deflate::DecoderOptions::kNeverReallocOutputBuffer));
  BL_PROPAGATE(decoder_impl->png_pixel_data.clear());
  BL_PROPAGATE(decoder_impl->png_pixel_data.reserve(size_t(png_pixel_data_size) + kOutputSizeScratch));

  // Read 'IDAT' or 'fdAT' chunks - once the first chunk is found, it's either the only chunk or there are consecutive
  // chunks of the same type. It's not allowed that the chunks are interleaved with chunks of a different chunk tag.
  {
    uint32_t chunk_size = chunk_reader.read_chunk_size();
    // Was already checked...
    BL_ASSERT(chunk_reader.has_chunk_with_size(chunk_size));

    for (;;) {
      // Zero chunks are allowed, however, they don't contain any data, thus don't call the DEFLATE decoder with these.
      const uint8_t* chunk_data = chunk_reader.ptr + kPngChunkHeaderSize;
      chunk_reader.advance(size_t(kPngChunkBaseSize) + chunk_size);

      if (frame_tag == BL_MAKE_TAG('f', 'd', 'A', 'T')) {
        // The 'fdAT' chunk starts with 4 bytes specifying the sequence.
        if (BL_UNLIKELY(chunk_size < 4u)) {
          return bl_make_error(BL_ERROR_INVALID_DATA);
        }

        chunk_data += 4u;
        chunk_size -= 4u;
      }

      if (chunk_size > 0u) {
        // When the decompression is done, verify whether the decompressed data size matches the PNG pixel data size.
        BLResult result = decoder_impl->deflate_decoder.decode(decoder_impl->png_pixel_data, BLDataView{chunk_data, chunk_size});
        if (result == BL_SUCCESS) {
          if (decoder_impl->png_pixel_data.size() != png_pixel_data_size) {
            return bl_make_error(BL_ERROR_INVALID_DATA);
          }
          break;
        }

        // The decoder returns this error (which is not traced) in case that the input data was not enough to
        // decompress the data. It's not an error if more pixel data chunks follow.
        if (result != BL_ERROR_DATA_TRUNCATED) {
          return result;
        }
      }

      // Consecutive chunks required.
      if (BL_UNLIKELY(!chunk_reader.has_chunk())) {
        return bl_make_error(BL_ERROR_DATA_TRUNCATED);
      }

      chunk_size = chunk_reader.read_chunk_size();
      if (BL_UNLIKELY(!chunk_reader.has_chunk_with_size(chunk_size))) {
        return bl_make_error(BL_ERROR_DATA_TRUNCATED);
      }

      uint32_t chunk_tag = chunk_reader.read_chunk_tag();
      if (BL_UNLIKELY(chunk_tag != frame_tag)) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }
    }
  }

  decoder_impl->clear_flag(DecoderStatusFlags::kRead_fcTL);
  decoder_impl->buffer_index = PtrOps::byte_offset(input, chunk_reader.ptr);

  uint8_t* png_pixel_ptr = const_cast<uint8_t*>(decoder_impl->png_pixel_data.data());
  uint32_t bytes_per_pixel = bl_max<uint32_t>((sample_depth * sample_count) / 8, 1);

  // Apply Inverse Filter
  // --------------------

  // If progressive `step_count` is 7 and `steps` array contains all windows.
  for (uint32_t i = 0; i < step_count; i++) {
    InterlaceStep& step = steps[i];
    if (step.used) {
      BL_PROPAGATE(Ops::func_table.inverse_filter[bytes_per_pixel](png_pixel_ptr + step.offset, bytes_per_pixel, step.bpl, step.height));
    }
  }

  // Deinterlace & Copy/Blend
  // ------------------------

  BLImageData image_data;

  if (decoder_impl->frame_index == 0u) {
    BL_PROPAGATE(image_out->create(int(w), int(h), BLFormat(decoder_impl->output_format)));
  }
  else {
    // The animation requires that the user passes an image that has the previous content, but we only want to verify
    // its size and pixel format.
    if (BL_UNLIKELY(image_out->size() != decoder_impl->image_info.size || image_out->format() != BLFormat(decoder_impl->output_format))) {
      return bl_make_error(BL_ERROR_INVALID_STATE);
    }
  }

  BL_PROPAGATE(image_out->make_mutable(&image_data));

  intptr_t dst_stride = image_data.stride;
  uint8_t* dst_pixels = static_cast<uint8_t*>(image_data.pixel_data);

  if (decoder_impl->frame_index != 0u) {
    size_t bpp = image_out->depth() / 8u;
    size_t prev_area_width_in_bytes = decoder_impl->prev_ctrl.w * bpp;

    switch (decoder_impl->prev_ctrl.dispose_op) {
      case kAPNGDisposeOpBackground: {
        zero_pixels(
          dst_pixels + intptr_t(decoder_impl->prev_ctrl.y) * dst_stride + intptr_t(decoder_impl->prev_ctrl.x * bpp),
          dst_stride,
          prev_area_width_in_bytes,
          decoder_impl->prev_ctrl.h);
        break;
      }

      case kAPNGDisposeOpPrevious: {
        const uint8_t* saved_pixels = static_cast<const uint8_t*>(decoder_impl->previous_pixel_buffer.get());

        copy_pixels(
          dst_pixels + intptr_t(decoder_impl->prev_ctrl.y) * dst_stride + intptr_t(decoder_impl->prev_ctrl.x * bpp),
          dst_stride,
          saved_pixels,
          intptr_t(prev_area_width_in_bytes),
          prev_area_width_in_bytes,
          decoder_impl->prev_ctrl.h);
        break;
      }

      default: {
        // Do nothing if the dispose op is kAPNGDisposeOpNone.
        break;
      }
    }

    dst_pixels += intptr_t(y) * dst_stride + intptr_t(x * bpp);

    if (decoder_impl->frame_ctrl.dispose_op == kAPNGDisposeOpPrevious) {
      size_t copy_area_width_in_bytes = w * bpp;
      uint8_t* saved_pixels = static_cast<uint8_t*>(decoder_impl->previous_pixel_buffer.alloc(h * copy_area_width_in_bytes));

      if (BL_UNLIKELY(!saved_pixels)) {
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
      }

      copy_pixels(saved_pixels, intptr_t(copy_area_width_in_bytes), dst_pixels, dst_stride, copy_area_width_in_bytes, h);
    }

    // TODO: [APNG] kAPNGBlendOpOver is currently not supported.
    //
    // if (decoder_impl->frame_ctrl.blend_op == kBlendOpOver) {
    // }
  }

  if (progressive) {
    // PNG interlacing requires 7 steps, where 7th handles all even scanlines (indexing from 1). This means that we
    // can, in general, reuse the buffer required by 7th step as a temporary to merge steps 1-6. To achieve this,
    // we need to:
    //
    //   1. Convert all even scanlines already ready by 7th step to `dst`. This makes the buffer ready to be reused.
    //   2. Merge pixels from steps 1-6 into that buffer.
    //   3. Convert all odd scanlines (from the reused buffer) to `dst`.
    //
    // We, in general, process 4 odd scanlines at a time, so we need the 7th buffer to have enough space to hold them
    // as well, if not, we allocate an extra buffer and use it instead. This approach is good as small images would
    // probably require the extra buffer, but larger images can reuse the 7th.
    BL_ASSERT(steps[6].width == w);
    BL_ASSERT(steps[6].height == h / 2); // Half of the rows, rounded down.

    uint32_t depth = sample_depth * sample_count;
    uint32_t tmp_height = bl_min<uint32_t>((h + 1u) / 2u, 4u);
    uint32_t tmp_bpl = steps[6].bpl;
    uint32_t tmp_size;

    if (steps[6].height) {
      decoder_impl->pixel_converter.convert_rect(dst_pixels + dst_stride, dst_stride * 2, png_pixel_ptr + 1u + steps[6].offset, intptr_t(tmp_bpl), w, steps[6].height);
    }

    // Align `tmp_bpl` so we can use aligned memory writes and reads while using it.
    tmp_bpl = IntOps::align_up(tmp_bpl, 16);
    tmp_size = tmp_bpl * tmp_height;

    ScopedBuffer tmp_alloc;
    uint8_t* tmp_pixel_ptr;

    // Decide whether to alloc an extra buffer or to reuse 7th.
    if (steps[6].size < tmp_size + 15) {
      tmp_pixel_ptr = static_cast<uint8_t*>(tmp_alloc.alloc(tmp_size + 15));
    }
    else {
      tmp_pixel_ptr = png_pixel_ptr + steps[6].offset;
    }

    if (!tmp_pixel_ptr) {
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
    }

    tmp_pixel_ptr = IntOps::align_up(tmp_pixel_ptr, 16);
    switch (depth) {
      case 1 : deinterlace_bits<1>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 2 : deinterlace_bits<2>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 4 : deinterlace_bits<4>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 8 : deinterlace_bytes<1>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 16: deinterlace_bytes<2>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 24: deinterlace_bytes<3>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
      case 32: deinterlace_bytes<4>(dst_pixels, dst_stride, decoder_impl->pixel_converter, tmp_pixel_ptr, intptr_t(tmp_bpl), png_pixel_ptr, steps, w, h); break;
    }
  }
  else {
    BL_ASSERT(steps[0].width == w);
    BL_ASSERT(steps[0].height == h);

    decoder_impl->pixel_converter.convert_rect(dst_pixels, dst_stride, png_pixel_ptr + 1, intptr_t(steps[0].bpl), w, h);
  }

  decoder_impl->frame_index++;
  if (decoder_impl->isAPNG() && decoder_impl->frame_index >= decoder_impl->image_info.frame_count) {
    // Restart the animation to create a loop.
    decoder_impl->frame_index = 0;
    decoder_impl->buffer_index = decoder_impl->first_fctl_offset;
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL decoder_read_info_impl(BLImageDecoderImpl* impl, BLImageInfo* info_out, const uint8_t* data, size_t size) noexcept {
  BLPngDecoderImpl* decoder_impl = static_cast<BLPngDecoderImpl*>(impl);
  BLResult result = decoder_impl->last_result;

  if (decoder_impl->buffer_index == 0u && result == BL_SUCCESS) {
    result = decoder_read_info_internal(decoder_impl, data, size);
    if (result != BL_SUCCESS) {
      decoder_impl->last_result = result;
    }
  }

  if (info_out) {
    memcpy(info_out, &decoder_impl->image_info, sizeof(BLImageInfo));
  }

  return result;
}

static BLResult BL_CDECL decoder_read_frame_impl(BLImageDecoderImpl* impl, BLImageCore* image_out, const uint8_t* data, size_t size) noexcept {
  BLPngDecoderImpl* decoder_impl = static_cast<BLPngDecoderImpl*>(impl);
  BL_PROPAGATE(decoder_read_info_impl(decoder_impl, nullptr, data, size));

  if (decoder_impl->frame_index == 0u && decoder_impl->first_fctl_offset == 0u) {
    BLResult result = decoder_read_important_chunks(decoder_impl, data, size);
    if (result != BL_SUCCESS) {
      decoder_impl->last_result = result;
      return result;
    }
  }
  else if (!decoder_impl->isAPNG()) {
    return bl_make_error(BL_ERROR_NO_MORE_DATA);
  }

  {
    BLResult result = decoder_read_pixel_data(decoder_impl, static_cast<BLImage*>(image_out), data, size);
    if (result != BL_SUCCESS) {
      decoder_impl->last_result = result;
      return result;
    }
    return BL_SUCCESS;
  }
}

static BLResult BL_CDECL decoder_create_impl(BLImageDecoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_DECODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLPngDecoderImpl>(self, info));

  BLPngDecoderImpl* decoder_impl = static_cast<BLPngDecoderImpl*>(self->_d.impl);
  decoder_impl->ctor(&png_decoder_virt, &png_codec_instance);
  return decoder_restart_impl(decoder_impl);
}

static BLResult BL_CDECL decoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLPngDecoderImpl* decoder_impl = static_cast<BLPngDecoderImpl*>(impl);
  decoder_impl->dtor();
  return bl_object_free_impl(decoder_impl);
}

// bl::Png::Codec - Encoder - OutputBuffer
// =======================================

class OutputBuffer {
public:
  uint8_t* _data = nullptr;
  uint8_t* _ptr = nullptr;
  uint8_t* _end = nullptr;

  BL_INLINE OutputBuffer() noexcept {}

  BL_INLINE OutputBuffer(uint8_t* data, size_t size) noexcept
    : _data(data),
      _ptr(data),
      _end(data + size) {}

  BL_INLINE uint8_t* ptr() const noexcept { return _ptr; }
  BL_INLINE uint8_t* end() const noexcept { return _end; }

  BL_INLINE size_t index() const noexcept { return PtrOps::byte_offset(_data, _ptr); }
  BL_INLINE size_t capacity() const noexcept { return PtrOps::byte_offset(_data, _end); }
  BL_INLINE size_t remaining_size() const noexcept { return PtrOps::bytes_until(_ptr, _end); }

  BL_INLINE void reset() noexcept {
    _data = nullptr;
    _ptr = nullptr;
    _end = nullptr;
  }

  BL_INLINE void reset(uint8_t* data, size_t size) noexcept {
    _data = data;
    _ptr = data;
    _end = data + size;
  }

  BL_INLINE void append_byte(uint8_t value) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    *_ptr++ = value;
  }

  BL_INLINE void appendUInt16(uint16_t value) noexcept {
    BL_ASSERT(remaining_size() >= 2);

    MemOps::writeU16u(_ptr, value);
    _ptr += 2;
  }

  BL_INLINE void appendUInt16LE(uint16_t value) noexcept {
    BL_ASSERT(remaining_size() >= 2);

    MemOps::writeU16uLE(_ptr, value);
    _ptr += 2;
  }

  BL_INLINE void appendUInt16BE(uint16_t value) noexcept {
    BL_ASSERT(remaining_size() >= 2);

    MemOps::writeU16uBE(_ptr, value);
    _ptr += 2;
  }

  BL_INLINE void appendUInt32(uint32_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU32u(_ptr, value);
    _ptr += 4;
  }

  BL_INLINE void appendUInt32LE(uint32_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU32uLE(_ptr, value);
    _ptr += 4;
  }

  BL_INLINE void appendUInt32BE(uint32_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU32uBE(_ptr, value);
    _ptr += 4;
  }

  BL_INLINE void appendUInt64(uint64_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU64u(_ptr, value);
    _ptr += 8;
  }

  BL_INLINE void appendUInt64LE(uint64_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU64uLE(_ptr, value);
    _ptr += 8;
  }

  BL_INLINE void appendUInt64BE(uint64_t value) noexcept {
    BL_ASSERT(remaining_size() >= 4);

    MemOps::writeU64uBE(_ptr, value);
    _ptr += 8;
  }

  BL_INLINE void append_data(const uint8_t* data, size_t size) noexcept {
    BL_ASSERT(remaining_size() >= size);

    memcpy(_ptr, data, size);
    _ptr += size;
  }
};

// bl::Png::Codec - Encoder - ChunkWriter
// ======================================

class ChunkWriter {
public:
  uint8_t* chunk_data = nullptr;

  BL_INLINE void start(OutputBuffer& output, uint32_t tag) noexcept {
    chunk_data = output.ptr();
    output.appendUInt32BE(0);
    output.appendUInt32BE(tag);
  }

  BL_INLINE void done(OutputBuffer& output) noexcept {
    const uint8_t* start = chunk_data + 8;
    size_t chunk_length = PtrOps::byte_offset(start, output.ptr());

    // PNG Specification: CRC is calculated on the preceding bytes in the chunk, including
    // the chunk type code and chunk data fields, but not including the length field.
    MemOps::writeU32uBE(chunk_data, uint32_t(chunk_length));
    output.appendUInt32BE(Compression::Checksum::crc32(start - 4, chunk_length + 4));
  }
};

// bl::Png::Codec - Encoder - API
// ==============================

static BLResult BL_CDECL encoder_restart_impl(BLImageEncoderImpl* impl) noexcept {
  BLPngEncoderImpl* encoder_impl = static_cast<BLPngEncoderImpl*>(impl);

  encoder_impl->last_result = BL_SUCCESS;
  encoder_impl->frame_index = 0;
  encoder_impl->buffer_index = 0;
  encoder_impl->compression_level = 6;

  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_get_property_impl(const BLObjectImpl* impl, const char* name, size_t name_size, BLVarCore* value_out) noexcept {
  const BLPngEncoderImpl* encoder_impl = static_cast<const BLPngEncoderImpl*>(impl);

  if (bl_match_property(name, name_size, "compression")) {
    return bl_var_assign_uint64(value_out, encoder_impl->compression_level);
  }

  return bl_object_impl_get_property(encoder_impl, name, name_size, value_out);
}

static BLResult BL_CDECL encoder_set_property_impl(BLObjectImpl* impl, const char* name, size_t name_size, const BLVarCore* value) noexcept {
  BLPngEncoderImpl* encoder_impl = static_cast<BLPngEncoderImpl*>(impl);

  if (bl_match_property(name, name_size, "compression")) {
    uint64_t v;
    BL_PROPAGATE(bl_var_to_uint64(value, &v));
    encoder_impl->compression_level = uint8_t(bl_min<uint64_t>(v, 12));
    return BL_SUCCESS;
  }

  return bl_object_impl_set_property(encoder_impl, name, name_size, value);
}

static BLResult filter_image_data(uint8_t* data, intptr_t stride, uint32_t bits_per_pixel, uint32_t w, uint32_t h) noexcept {
  bl_unused(bits_per_pixel, w);

  for (uint32_t y = 0; y < h; y++) {
    data[0] = 0;
    data += stride;
  }

  return BL_SUCCESS;
}

static BLResult BL_CDECL encoder_write_frame_impl(BLImageEncoderImpl* impl, BLArrayCore* dst, const BLImageCore* image) noexcept {
  BLPngEncoderImpl* encoder_impl = static_cast<BLPngEncoderImpl*>(impl);
  BL_PROPAGATE(encoder_impl->last_result);

  BLArray<uint8_t>& buf = *static_cast<BLArray<uint8_t>*>(dst);
  const BLImage& img = *static_cast<const BLImage*>(image);

  if (img.is_empty()) {
    return bl_make_error(BL_ERROR_INVALID_VALUE);
  }

  BLImageData image_data;
  BL_PROPAGATE(img.get_data(&image_data));

  uint32_t w = uint32_t(image_data.size.w);
  uint32_t h = uint32_t(image_data.size.h);
  uint32_t format = image_data.format;

  // Setup target PNG format and other information.
  BLFormatInfo png_format_info {};
  uint8_t png_bit_depth = 0;
  uint8_t png_color_type = 0;

  switch (format) {
    case BL_FORMAT_PRGB32:
      png_format_info.depth = 32;
      png_format_info.flags = BLFormatFlags(BL_FORMAT_FLAG_RGBA | BL_FORMAT_FLAG_BE);
      png_format_info.set_sizes(8, 8, 8, 8);
      png_format_info.set_shifts(24, 16, 8, 0);
      png_bit_depth = 8;
      png_color_type = 6;
      break;

    case BL_FORMAT_XRGB32:
      png_format_info.depth = 24;
      png_format_info.flags = BLFormatFlags(BL_FORMAT_FLAG_RGB | BL_FORMAT_FLAG_BE);
      png_format_info.set_sizes(8, 8, 8, 0);
      png_format_info.set_shifts(16, 8, 0, 0);
      png_bit_depth = 8;
      png_color_type = 2;
      break;

    case BL_FORMAT_A8:
      png_format_info.depth = 8;
      png_format_info.flags = BL_FORMAT_FLAG_ALPHA;
      png_format_info.set_sizes(0, 0, 0, 8);
      png_format_info.set_shifts(0, 0, 0, 0);
      png_bit_depth = 8;
      png_color_type = 0;
      break;
  }

  // Setup pixel converter and convert the input image to PNG representation.
  BLPixelConverter pc;
  BL_PROPAGATE(pc.create(png_format_info, bl_format_info[format]));

  size_t uncompressed_stride = ((w * png_format_info.depth + 7) / 8u) + 1;
  size_t uncompressed_data_size = uncompressed_stride * h;

  ScopedBuffer uncompressed_buffer;
  uint8_t* uncompressed_data = static_cast<uint8_t*>(uncompressed_buffer.alloc(uncompressed_data_size));

  if (BL_UNLIKELY(!uncompressed_data)) {
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
  }

  BL_PROPAGATE(pc.convert_rect(uncompressed_data + 1, intptr_t(uncompressed_stride), image_data.pixel_data, image_data.stride, w, h));
  BL_PROPAGATE(filter_image_data(uncompressed_data, intptr_t(uncompressed_stride), png_format_info.depth, w, h));

  // Setup a deflate encoder - higher compression levels require more space, so init it now.
  Compression::Deflate::Encoder deflate_encoder;
  BL_PROPAGATE(deflate_encoder.init(Compression::Deflate::FormatType::kZlib, encoder_impl->compression_level));

  // Create PNG file.
  size_t output_worst_case_size = deflate_encoder.minimum_output_buffer_size(uncompressed_data_size);

  size_t ihdr_size = kPngChunkBaseSize + kPngChunkDataSize_IHDR;
  size_t idat_size = kPngChunkBaseSize + output_worst_case_size;
  size_t iend_size = kPngChunkBaseSize;

  size_t reserve_bytes = kPngSignatureSize + ihdr_size + idat_size + iend_size;
  uint8_t* output_data;
  BL_PROPAGATE(buf.modify_op(BL_MODIFY_OP_APPEND_FIT, reserve_bytes, &output_data));

  // Prepare output buffer and chunk writer.
  OutputBuffer output(output_data, reserve_bytes);
  ChunkWriter chunk;

  // Write PNG signature.
  output.append_data(kPngSignature, kPngSignatureSize);

  // Write IHDR chunk.
  chunk.start(output, BL_MAKE_TAG('I', 'H', 'D', 'R'));
  output.appendUInt32BE(w);        // Image width.
  output.appendUInt32BE(h);        // Image height.
  output.append_byte(png_bit_depth);  // Bit depth (1, 2, 4, 8, 16).
  output.append_byte(png_color_type); // Color type (0, 2, 3, 4, 6).
  output.append_byte(0u);           // Compression method, must be zero.
  output.append_byte(0u);           // Filter method, must be zero.
  output.append_byte(0u);           // Interlace method (0 == no interlacing).
  chunk.done(output);

  // Write IDAT chunk.
  chunk.start(output, BL_MAKE_TAG('I', 'D', 'A', 'T'));
  output._ptr += deflate_encoder.compress_to(output.ptr(), output.remaining_size(), uncompressed_data, uncompressed_data_size);
  chunk.done(output);

  // Write IEND chunk.
  chunk.start(output, BL_MAKE_TAG('I', 'E', 'N', 'D'));
  chunk.done(output);

  ArrayInternal::set_size(dst, PtrOps::byte_offset(buf.data(), output.ptr()));
  return BL_SUCCESS;
}

static BLResult encoder_create_impl(BLImageEncoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_ENCODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLPngEncoderImpl>(self, info));

  BLPngEncoderImpl* encoder_impl = static_cast<BLPngEncoderImpl*>(self->_d.impl);
  encoder_impl->ctor(&png_encoder_virt, &png_codec_instance);
  return encoder_restart_impl(encoder_impl);
}

static BLResult BL_CDECL encoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLPngEncoderImpl* encoder_impl = static_cast<BLPngEncoderImpl*>(impl);
  encoder_impl->dtor();
  return bl_object_free_impl(encoder_impl);
}

// bl::Png::Codec - Codec API
// ==========================

static BLResult BL_CDECL codec_destroy_impl(BLObjectImpl* impl) noexcept {
  // Built-in codecs are never destroyed.
  bl_unused(impl);
  return BL_SUCCESS;
}

static uint32_t BL_CDECL codec_inspect_data_impl(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl);

  // Minimum PNG size and signature.
  if (size < kPngSignatureSize || memcmp(data, kPngSignature, kPngSignatureSize) != 0) {
    return 0;
  }

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

// bl::Png::Codec - Runtime Registration
// =====================================

void png_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept {
  using namespace bl::Png;

  Ops::init_func_table(rt);

  // Initialize PNG codec.
  png_codec.virt.base.destroy = codec_destroy_impl;
  png_codec.virt.base.get_property = bl_object_impl_get_property;
  png_codec.virt.base.set_property = bl_object_impl_set_property;
  png_codec.virt.inspect_data = codec_inspect_data_impl;
  png_codec.virt.create_decoder = codec_create_decoder_impl;
  png_codec.virt.create_encoder = codec_create_encoder_impl;

  png_codec.impl->ctor(&png_codec.virt);
  png_codec.impl->features =
    BL_IMAGE_CODEC_FEATURE_READ     |
    BL_IMAGE_CODEC_FEATURE_WRITE    |
    BL_IMAGE_CODEC_FEATURE_LOSSLESS ;
  png_codec.impl->name.dcast().assign("PNG");
  png_codec.impl->vendor.dcast().assign("Blend2D");
  png_codec.impl->mime_type.dcast().assign("image/png");
  png_codec.impl->extensions.dcast().assign("png");

  png_codec_instance._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC), &png_codec.impl);

  // Initialize PNG decoder virtual functions.
  png_decoder_virt.base.destroy = decoder_destroy_impl;
  png_decoder_virt.base.get_property = bl_object_impl_get_property;
  png_decoder_virt.base.set_property = bl_object_impl_set_property;
  png_decoder_virt.restart = decoder_restart_impl;
  png_decoder_virt.read_info = decoder_read_info_impl;
  png_decoder_virt.read_frame = decoder_read_frame_impl;

  // Initialize PNG encoder virtual functions.
  png_encoder_virt.base.destroy = encoder_destroy_impl;
  png_encoder_virt.base.get_property = encoder_get_property_impl;
  png_encoder_virt.base.set_property = encoder_set_property_impl;
  png_encoder_virt.restart = encoder_restart_impl;
  png_encoder_virt.write_frame = encoder_write_frame_impl;

  codecs->append(png_codec_instance.dcast());
}

} // {bl::Png}
