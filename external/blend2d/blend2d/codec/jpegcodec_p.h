// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_CODEC_JPEGCODEC_P_H_INCLUDED
#define BLEND2D_CODEC_JPEGCODEC_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/image_p.h>
#include <blend2d/core/imagecodec.h>
#include <blend2d/core/imagedecoder.h>
#include <blend2d/core/imageencoder.h>
#include <blend2d/core/pixelconverter.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/codec/jpeghuffman_p.h>
#include <blend2d/support/scopedallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_codec_impl
//! \{

namespace bl::Jpeg {

static constexpr uint32_t kDctSize = 8u;           //!< Size of JPEG's DCT block (N).
static constexpr uint32_t kDctSize2 = 8u * 8u;     //!< Size of JPEG's DCT block squared (NxN).

//! JPEG markers.
static constexpr uint32_t kMarkerNULL         = 0x00u; //!< A "stuff byte" used in Huffman stream to encode 0xFF, which is otherwise used as a marker.
static constexpr uint32_t kMarkerTEM          = 0x01u; //!< Temporary use in arithmetic coding.
static constexpr uint32_t kMarkerRES          = 0x02u; //!< Reserved (first) (0x02..0xBF).
static constexpr uint32_t kMarkerRES_LAST     = 0xBFu; //!< Reserved (last).

static constexpr uint32_t kMarkerSOF0         = 0xC0u; //!< Start of Frame 0 - Baseline DCT (Huffman).
static constexpr uint32_t kMarkerSOF1         = 0xC1u; //!< Start of Frame 1 - Sequential DCT (Huffman).
static constexpr uint32_t kMarkerSOF2         = 0xC2u; //!< Start of Frame 2 - Progressive DCT (Huffman).
static constexpr uint32_t kMarkerSOF3         = 0xC3u; //!< Start of Frame 3 - Lossless (Huffman).
static constexpr uint32_t kMarkerDHT          = 0xC4u; //!< Define Huffman Table (0xC4).
static constexpr uint32_t kMarkerSOF5         = 0xC5u; //!< Start of Frame 5 - Differential Sequential DCT (Huffman).
static constexpr uint32_t kMarkerSOF6         = 0xC6u; //!< Start of Frame 6 - Differential Progressive DCT (Huffman).
static constexpr uint32_t kMarkerSOF7         = 0xC7u; //!< Start of Frame 7 - Differential Lossless (Huffman).
static constexpr uint32_t kMarkerJPG          = 0xC8u; //!< JPEG Extensions (0xC8).
static constexpr uint32_t kMarkerSOF9         = 0xC9u; //!< Start of Frame 9 - Sequential DCT (Arithmetic).
static constexpr uint32_t kMarkerSOF10        = 0xCAu; //!< Start of Frame 10 - Progressive DCT (Arithmetic).
static constexpr uint32_t kMarkerSOF11        = 0xCBu; //!< Start of Frame 11 - Lossless (Arithmetic).
static constexpr uint32_t kMarkerDAC          = 0xCCu; //!< Define Arithmetic Coding (0xCC).
static constexpr uint32_t kMarkerSOF13        = 0xCDu; //!< Start of Frame 13 - Differential Sequential DCT (Arithmetic).
static constexpr uint32_t kMarkerSOF14        = 0xCEu; //!< Start of Frame 14 - Differential Progressive DCT (Arithmetic).
static constexpr uint32_t kMarkerSOF15        = 0xCFu; //!< Start of Frame 15 - Differential Lossless (Arithmetic).

static constexpr uint32_t kMarkerRST          = 0xD0u; //!< Restart Marker (first) (0xD0..0xD7)
static constexpr uint32_t kMarkerRST_LAST     = 0xD7u; //!< Restart Marker (last)
static constexpr uint32_t kMarkerSOI          = 0xD8u; //!< Start of Image (0xD8).
static constexpr uint32_t kMarkerEOI          = 0xD9u; //!< End of Image (0xD9).
static constexpr uint32_t kMarkerSOS          = 0xDAu; //!< Start of Scan (0xDA).
static constexpr uint32_t kMarkerDQT          = 0xDBu; //!< Define Quantization Table (0xDB).
static constexpr uint32_t kMarkerDNL          = 0xDCu; //!< Define Number of Lines (0xDC).
static constexpr uint32_t kMarkerDRI          = 0xDDu; //!< Define Restart Interval (0xDD).
static constexpr uint32_t kMarkerDHP          = 0xDEu; //!< Define Hierarchical Progression (0xDE)
static constexpr uint32_t kMarkerEXP          = 0xDFu; //!< Expand Reference Component (0xDF).

static constexpr uint32_t kMarkerAPP          = 0xE0u; //!< Application (first) (0xE0..0xEF).
static constexpr uint32_t kMarkerAPP_LAST     = 0xEFu; //!< Application (last).

static constexpr uint32_t kMarkerAPP0         = 0xE0u; //!< APP0 - JFIF, JFXX (0xE0).
static constexpr uint32_t kMarkerAPP1         = 0xE1u; //!< APP1 - EXIF, XMP (0xE1).
static constexpr uint32_t kMarkerAPP2         = 0xE2u; //!< APP2 - ICC (0xE2).
static constexpr uint32_t kMarkerAPP3         = 0xE3u; //!< APP3 - META (0xE3).
static constexpr uint32_t kMarkerAPP4         = 0xE4u; //!< APP4 (0xE4).
static constexpr uint32_t kMarkerAPP5         = 0xE5u; //!< APP5 (0xE5).
static constexpr uint32_t kMarkerAPP6         = 0xE6u; //!< APP6 (0xE6).
static constexpr uint32_t kMarkerAPP7         = 0xE7u; //!< APP7 (0xE7).
static constexpr uint32_t kMarkerAPP8         = 0xE8u; //!< APP8 (0xE8).
static constexpr uint32_t kMarkerAPP9         = 0xE9u; //!< APP9 (0xE9).
static constexpr uint32_t kMarkerAPP10        = 0xEAu; //!< APP10 (0xEA).
static constexpr uint32_t kMarkerAPP11        = 0xEBu; //!< APP11 (0xEB).
static constexpr uint32_t kMarkerAPP12        = 0xECu; //!< APP12 - Picture information (0xEC).
static constexpr uint32_t kMarkerAPP13        = 0xEDu; //!< APP13 - Adobe IRB (0xED).
static constexpr uint32_t kMarkerAPP14        = 0xEEu; //!< APP14 - Adobe (0xEE).
static constexpr uint32_t kMarkerAPP15        = 0xEFu; //!< APP15 (0xEF).

static constexpr uint32_t kMarkerEXT          = 0xF0u; //!< JPEG Extension (first) (0xF0..0xFD).
static constexpr uint32_t kMarkerEXT_LAST     = 0xFDu; //!< JPEG Extension (last).
static constexpr uint32_t kMarkerCOM          = 0xFEu; //!< Comment (0xFE).

static constexpr uint32_t kMarkerInvalid      = 0xFFu; //!< Invalid (0xFF), sometimes used as padding.

// JPEG colorspace constants:
static constexpr uint32_t kColorspaceNone     = 0; //!< Colorspace is unknown / unspecified.
static constexpr uint32_t kColorspaceY        = 1; //!< Colorspace is Y-only (grayscale).
static constexpr uint32_t kColorspaceRGB      = 2; //!< Colorspace is sRGB.
static constexpr uint32_t kColorspaceYCBCR    = 3; //!< Colorspace is YCbCr.
static constexpr uint32_t kColorspaceCMYK     = 4; //!< Colorspace is CMYK.
static constexpr uint32_t kColorspaceYCCK     = 5; //!< Colorspace is YCbCrK.
static constexpr uint32_t kColorspaceCount    = 6; //!< Count of JPEG's colorspace types.

// JPEG's density units specified by APP0-JFIF marker:
static constexpr uint32_t kDensityOnlyAspect  = 0;
static constexpr uint32_t kDensityPixelsPerIN = 1;
static constexpr uint32_t kDensityPixelsPerCM = 2;
static constexpr uint32_t kDensityCount       = 3;

// JPEG's thumbnail format specified by APP0-JFXX marker:
static constexpr uint32_t kThumbnailJPEG      = 0;
static constexpr uint32_t kThumbnailPAL8      = 1;
static constexpr uint32_t kThumbnailRGB24     = 2;
static constexpr uint32_t kThumbnailCount     = 3;

// JPEG's sampling point as specified by JFIF-APP0 marker:
static constexpr uint32_t kSamlingUnknown     = 0; //!< Unknown / not specified sampling point (no JFIF-APP0 marker).
static constexpr uint32_t kSamlingCositted    = 1; //!< Co-sitted sampling point (specified by JFIF-APP0 marker).
static constexpr uint32_t kSamlingCentered    = 2; //!< Centered sampling point (specified by JFIF-APP0 marker).

// JPEG's table class selector (DC, AC):
static constexpr uint32_t kTableDC            = 0; //!< DC table.
static constexpr uint32_t kTableAC            = 1; //!< AC table.
static constexpr uint32_t kTableCount         = 2; //!< Number of tables.

//! JPEG decoder flags - bits of information collected from JPEG markers.
enum class DecoderStatusFlags : uint32_t {
  kNoFlags  = 0u,
  kDoneSOI  = 0x00000001u,
  kDoneSOS  = 0x00000002u,
  kDoneEOI  = 0x00000004u,
  kDoneJFIF = 0x00000008u,
  kDoneJFXX = 0x00000010u,
  kDoneEXIF = 0x00000020u,
  kHasThumb = 0x80000000u
};

BL_DEFINE_ENUM_FLAGS(DecoderStatusFlags)

//! Tests whether the marker `m` is a SOF marker.
static BL_INLINE bool is_marker_sof(uint32_t m) noexcept { return m >= kMarkerSOF0 && m <= kMarkerSOF2; }
//! Tests whether the marker `m` is an RST marker.
static BL_INLINE bool is_marker_rst(uint32_t m) noexcept { return m >= kMarkerRST && m <= kMarkerRST_LAST; }
//! Tests whether the marker `m` is an APP marker.
static BL_INLINE bool is_marker_app(uint32_t m) noexcept { return m >= kMarkerAPP && m <= kMarkerAPP_LAST; }

template<typename T>
struct alignas(16) Block {
  T data[kDctSize2];

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

struct MCUInfo {
  //! MCU width/height in blocks (maximum sampling factor of all components).
  struct {
    uint8_t w;
    uint8_t h;
  } sf;

  //! MCU width/height in pixels (resolution of a single MCU).
  struct {
    uint8_t w;
    uint8_t h;
  } px;

  //! Number of MCUs in horizontal/vertical direction.
  struct {
    uint32_t w;
    uint32_t h;
  } count;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

struct DecoderComponent {
  //! Raster data.
  uint8_t* data;

  //! Component ID.
  uint8_t comp_id;
  //! Quantization table ID.
  uint8_t quant_id;
  //! DC Huffman-Table ID.
  uint8_t dc_id;
  //! AC Huffman-Table ID.
  uint8_t ac_id;

  //! Effective width.
  uint32_t px_w;
  //! Effective height.
  uint32_t px_h;
  //! Oversized width to match the total width requires by all MCUs.
  uint32_t os_w;
  //! Oversized height to match the total height required by all MCUs.
  uint32_t os_h;
  //! Number of 8x8 blocks in horizontal direction.
  uint32_t bl_w;
  //! Number of 8x8 blocks in vertical direction.
  uint32_t bl_h;

  //! Horizontal sampling factor (width).
  uint8_t sf_w;
  //! Vertical sampling factor (height).
  uint8_t sf_h;

  //! DC prediction (modified during decoding phase).
  int32_t dc_pred;
  //! Coefficients used only by progressive JPEGs.
  int16_t* coeff;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

//! Start of stream (SOS) data.
struct DecoderSOS {
  //! Maps a stream component index into the `DecoderComponent`.
  DecoderComponent* sc_comp[4];
  //! Count of components in this stream.
  uint8_t sc_count;
  //! Start of spectral selection.
  uint8_t ss_start;
  //! End of spectral selection.
  uint8_t ss_end;
  //! Successive approximation low bit.
  uint8_t sa_low_bit;
  //! Successive approximation high bit.
  uint8_t sa_high_bit;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

//! In case of RGB24 or PAL8 thumbnail data, the index points to the first byte describing W, H, and then data
//! follows. In case of an embedded JPEG the `index` points to the first byte of that JPEG. So to decode the RAW
//! uncompressed PAL8/RGB24 data, skip first two bytes, that describe W/H, which is already filled in this struct.
struct DecoderThumbnail {
  //! Thumbnail format, see thumbnail formats.
  uint8_t format;
  //! Reserved
  uint8_t reserved;
  //! Thumbnail width and height (8-bit, as in JFIF spec.).
  uint8_t w, h;
  //! Index of the thumbnail data from the beginning of the stream.
  size_t index;
  //! Thumbnail data size (raw data, the JFIF headers not included here).
  size_t size;

  BL_INLINE void reset() noexcept { memset(this, 0, sizeof(*this)); }
};

BL_HIDDEN void jpeg_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept;

} // {bl::Jpeg}

struct BLJpegDecoderImpl : public BLImageDecoderImpl {
  //! JPEG memory allocator (can allocate aligned blocks and keep track of them).
  bl::ScopedAllocator allocator;
  //! JPEG image information.
  BLImageInfo image_info;

  //! JPEG decoder flags.
  bl::Jpeg::DecoderStatusFlags status_flags;
  //! Restart interval as specified by DRI marker.
  uint32_t restart_interval;
  //! SOF marker (so we can select right decompression algorithm), initially zero.
  uint8_t sof_marker;
  //! Colorspace.
  uint8_t colorspace;
  //! True if the data contains zero height (delayed height).
  uint8_t delayed_height;
  //!< JFIF major version (if present).
  uint8_t jfif_major;
  //!< JFIF minor version (if present).
  uint8_t jfif_minor;
  //! Mask of all defined DC tables.
  uint8_t dc_table_mask;
  //! Mask of all defined AC tables.
  uint8_t ac_table_mask;
  //! Mask of all defined (de)quantization tables.
  uint8_t q_table_mask;

  //! JPEG decoder MCU information.
  bl::Jpeg::MCUInfo mcu;
  //! JPEG decoder's current stream data (defined and overwritten by SOS markers).
  bl::Jpeg::DecoderSOS sos;
  //! JPEG decoder thumbnail data.
  bl::Jpeg::DecoderThumbnail thumb;
  //! JPEG decoder components.
  bl::Jpeg::DecoderComponent comp[4];
  //! JPEG Huffman DC tables.
  bl::Jpeg::DecoderHuffmanDCTable dc_table[4];
  //! JPEG Huffman AC tables.
  bl::Jpeg::DecoderHuffmanACTable ac_table[4];
  //! JPEG quantization tables.
  bl::Jpeg::Block<uint16_t> q_table[4];
};

struct BLJpegEncoderImpl : public BLImageEncoderImpl {};

struct BLJpegCodecImpl : public BLImageCodecImpl {};

//! \}
//! \endcond

#endif // BLEND2D_CODEC_JPEGCODEC_P_H_INCLUDED
