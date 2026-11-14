// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_BMPCODEC_P_H_INCLUDED
#define BLEND2D_CODEC_BMPCODEC_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/pixelconverter.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Bmp {

static constexpr uint32_t kHeaderSizeOS2_V1 = 12;
static constexpr uint32_t kHeaderSizeWIN_V1 = 40;
static constexpr uint32_t kHeaderSizeWIN_V2 = 52;
static constexpr uint32_t kHeaderSizeWIN_V3 = 56;
static constexpr uint32_t kHeaderSizeWIN_V4 = 108;
static constexpr uint32_t kHeaderSizeWIN_V5 = 12;

static constexpr uint32_t kCompressionRGB = 0;
static constexpr uint32_t kCompressionRLE8 = 1;
static constexpr uint32_t kCompressionRLE4 = 2;
static constexpr uint32_t kCompressionBitFields = 3;
static constexpr uint32_t kCompressionJPEG = 4;
static constexpr uint32_t kCompressionPNG = 5;
static constexpr uint32_t kCompressionAlphaBitFields = 6;
static constexpr uint32_t kCompressionCMYK = 11;
static constexpr uint32_t kCompressionCMYK_RLE8 = 12;
static constexpr uint32_t kCompressionCMYK_RLE4 = 1;
static constexpr uint32_t kCompressionValueCount = 13;

static constexpr uint32_t kColorSpaceCalibratedRGB = 0;
static constexpr uint32_t kColorSpaceDD_RGB = 1;
static constexpr uint32_t kColorSpaceDD_CMYK = 2;

static constexpr uint32_t kRleLine = 0;
static constexpr uint32_t kRleStop = 1;
static constexpr uint32_t kRleMove = 2;
static constexpr uint32_t kRleCount = 3;

// Specification describes that skipped pixels contain background color, transparent for us.
static constexpr uint32_t kRleBackground = 0u;

//! Bitmap file signature [2 Bytes].
struct BmpFileSignature {
  //! Bitmap signature data - "BM".
  uint8_t data[2];
};

//! Bitmap File Header [12 Bytes] (we don't count signature here, it's separate).
struct BmpFileHeader {
  //! Bitmap file size in bytes.
  uint32_t file_size;
  //! Reserved, should be zero.
  uint32_t reserved;
  //! Offset to image data (54, 124, ...).
  uint32_t image_offset;

  BL_INLINE void reset() noexcept { *this = BmpFileHeader{}; }

  BL_INLINE void byte_swap() {
    file_size          = IntOps::byteSwap32(file_size);
    image_offset       = IntOps::byteSwap32(image_offset);
  }
};

//! All bitmap headers in one union.
struct BmpInfoHeader {
  //! Helper that contains XYZ (colorspace).
  struct XYZ {
    uint32_t x, y, z;
  };

  //! Bitmap OS/2 Header [12 Bytes].
  struct OS2 {
    //! Header size (40, 52).
    uint32_t header_size;
    //! Bitmap width (16-bit value).
    int16_t width;
    //! Bitmap height (16-bit value).
    int16_t height;
    //! Number of color planes (always 1).
    uint16_t planes;
    //! Bits per pixel (1, 4, 8 or 24).
    uint16_t bits_per_pixel;

    BL_INLINE void byte_swap() noexcept {
      header_size    = IntOps::byteSwap32(header_size);
      width          = IntOps::byteSwap16(width);
      height         = IntOps::byteSwap16(height);
      planes         = IntOps::byteSwap16(planes);
      bits_per_pixel = IntOps::byteSwap16(bits_per_pixel);
    }
  };

  //! Windows Info Header [40..124 Bytes].
  struct Win {
    // Version 1.

    //! Header size (40, 52, 56, 108, 124).
    uint32_t header_size;
    //! Bitmap width.
    int32_t width;
    //! Bitmap height.
    int32_t height;
    //! Count of planes, always 1.
    uint16_t planes;
    //! Bits per pixel (1, 4, 8, 16, 24 or 32).
    uint16_t bits_per_pixel;
    //! Compression methods used.
    uint32_t compression;
    //! Image data size (in bytes).
    uint32_t image_size;
    //! Horizontal resolution in pixels per meter.
    uint32_t horz_resolution;
    //! Vertical resolution in pixels per meter.
    uint32_t vert_resolution;
    //! Number of colors in the image.
    uint32_t colors_used;
    //! Minimum number of important colors.
    uint32_t colors_important;

    // Version 2 and 3.

    union {
      uint32_t masks[4];
      struct {
        //! Mask identifying bits of red component.
        uint32_t r_mask;
        //! Mask identifying bits of green component.
        uint32_t g_mask;
        //! Mask identifying bits of blue component.
        uint32_t b_mask;
        //! Mask identifying bits of alpha component [Version 3+ only].
        uint32_t a_mask;
      };
    };

    // Version 4.

    //! Color space type.
    uint32_t colorspace;
    //! Coordinates of red endpoint.
    XYZ r;
    //! Coordinates of green endpoint.
    XYZ g;
    //! Coordinates of blue endpoint.
    XYZ b;
    //! Gamma red coordinate scale value.
    uint32_t r_gamma;
    //! Gamma green coordinate scale value.
    uint32_t gGamma;
    //! Gamma blue coordinate scale value.
    uint32_t b_gamma;

    // Version 5.

    //! Rendering intent for bitmap.
    uint32_t intent;
    //! ProfileData offset (in bytes), from the beginning of `BmpInfoHeader::Win`.
    uint32_t profile_data;
    //! Size, in bytes, of embedded profile data.
    uint32_t profile_size;
    //! Reserved, should be zero.
    uint32_t reserved;

    BL_INLINE void byte_swap() noexcept {
      header_size      = IntOps::byteSwap32(header_size);
      width            = IntOps::byteSwap32(width);
      height           = IntOps::byteSwap32(height);
      planes           = IntOps::byteSwap16(planes);
      bits_per_pixel   = IntOps::byteSwap16(bits_per_pixel);
      compression      = IntOps::byteSwap32(compression);
      image_size       = IntOps::byteSwap32(image_size);
      horz_resolution  = IntOps::byteSwap32(horz_resolution);
      vert_resolution  = IntOps::byteSwap32(vert_resolution);
      colors_used      = IntOps::byteSwap32(colors_used);
      colors_important = IntOps::byteSwap32(colors_important);
      r_mask           = IntOps::byteSwap32(r_mask);
      g_mask           = IntOps::byteSwap32(g_mask);
      b_mask           = IntOps::byteSwap32(b_mask);
      a_mask           = IntOps::byteSwap32(a_mask);
      colorspace       = IntOps::byteSwap32(colorspace);
      r.x              = IntOps::byteSwap32(r.x);
      r.y              = IntOps::byteSwap32(r.y);
      r.z              = IntOps::byteSwap32(r.z);
      g.x              = IntOps::byteSwap32(g.x);
      g.y              = IntOps::byteSwap32(g.y);
      g.z              = IntOps::byteSwap32(g.z);
      b.x              = IntOps::byteSwap32(b.x);
      b.y              = IntOps::byteSwap32(b.y);
      b.z              = IntOps::byteSwap32(b.z);
      r_gamma          = IntOps::byteSwap32(r_gamma);
      gGamma           = IntOps::byteSwap32(gGamma);
      b_gamma          = IntOps::byteSwap32(b_gamma);
      intent           = IntOps::byteSwap32(intent);
      profile_data     = IntOps::byteSwap32(profile_data);
      profile_size     = IntOps::byteSwap32(profile_size);
    }
  };

  union {
    uint32_t header_size;
    OS2 os2;
    Win win;
  };

  BL_INLINE void reset() noexcept { *this = BmpInfoHeader{}; }
};

BL_HIDDEN void bmp_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept;

} // {bl::Bmp}

struct BLBmpDecoderImpl : public BLImageDecoderImpl {
  BLImageInfo image_info;
  bl::Bmp::BmpFileHeader file;
  bl::Bmp::BmpInfoHeader info;
  BLFormatInfo fmt;
  uint32_t stride;
};

struct BLBmpEncoderImpl : public BLImageEncoderImpl {};

struct BLBmpCodecImpl : public BLImageCodecImpl {};

//! \}
//! \endcond

#endif // BLEND2D_CODEC_BMPCODEC_P_H_INCLUDED
