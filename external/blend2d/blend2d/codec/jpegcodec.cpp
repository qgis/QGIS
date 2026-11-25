// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// The JPEG codec is based on stb_image <https://github.com/nothings/stb>
// released into PUBLIC DOMAIN. Blend2D's JPEG codec can be distributed
// under Blend2D's ZLIB license or under STB's PUBLIC DOMAIN as well.

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/object_p.h>
#include <blend2d/core/runtime_p.h>
#include <blend2d/core/string_p.h>
#include <blend2d/codec/jpegcodec_p.h>
#include <blend2d/codec/jpeghuffman_p.h>
#include <blend2d/codec/jpegops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

namespace bl::Jpeg {

// bl::Jpeg::Codec - Globals
// =========================

static BLObjectEternalVirtualImpl<BLJpegCodecImpl, BLImageCodecVirt> jpeg_codec;
static BLImageCodecCore jpeg_codec_instance;

static BLImageDecoderVirt jpeg_decoder_virt;
/*
static BLImageEncoderVirt jpeg_encoder_virt;
*/

// bl::Jpeg::Decoder - DeZigZag Table
// ==================================

// Mapping table of zigzagged 8x8 data into a natural order.
static const uint8_t decoder_de_zig_zag_table[64 + 16] = {
  0 , 1 , 8 , 16, 9 , 2 , 3 , 10,
  17, 24, 32, 25, 18, 11, 4 , 5 ,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13, 6 , 7 , 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63,

  // These are not part of JPEG's spec, however, it's convenient as the decoder doesn't have to check whether
  // the coefficient index is out of bounds.
  63, 63, 63, 63, 63, 63, 63, 63,
  63, 63, 63, 63, 63, 63, 63, 63
};

// bl::Jpeg::Decoder - Process Marker
// ==================================

static BLResult decoder_process_marker(BLJpegDecoderImpl* decoder_impl, uint32_t m, const uint8_t* p, size_t remain, size_t& consumed_bytes) noexcept {
  // Should be zero when passed in.
  BL_ASSERT(consumed_bytes == 0);

  BLImageInfo& image_info = decoder_impl->image_info;

#define GET_PAYLOAD_SIZE(MinSize)                     \
  size_t size;                                        \
                                                      \
  do {                                                \
    if (remain < MinSize)                             \
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);  \
                                                      \
    size = MemOps::readU16uBE(p);                     \
    if (size < MinSize)                               \
      return bl_make_error(BL_ERROR_INVALID_DATA);    \
                                                      \
    if (size > remain)                                \
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);  \
                                                      \
    p += 2;                                           \
    remain = size - 2;                                \
  } while (false)

  // SOF - Start of Frame
  // --------------------
  //
  //        WORD - Size (Consumed by GET_PAYLOAD_...)
  //
  //   [00] BYTE - Precision `P`
  //   [01] WORD - Height `Y`
  //   [03] WORD - Width `X`
  //   [05] BYTE - Number of components `Nf`
  //
  //   [06] Specification of each component [0..Nf] {
  //        [00] BYTE Component identifier `id`
  //        [01] BYTE Horizontal `Hi` and vertical `Vi` sampling factor
  //        [02] BYTE Quantization table destination selector `TQi`
  //   }

  if (is_marker_sof(m)) {
    uint32_t sof_marker = m;

    // Forbid multiple SOF markers in a single JPEG file.
    if (decoder_impl->sof_marker) {
      return bl_make_error(BL_ERROR_JPEG_MULTIPLE_SOF);
    }

    // Check if SOF type is supported.
    if (sof_marker != kMarkerSOF0 && sof_marker != kMarkerSOF1 && sof_marker != kMarkerSOF2) {
      return bl_make_error(BL_ERROR_JPEG_UNSUPPORTED_SOF);
    }

    // 11 bytes is a minimum size of SOF describing exactly one component.
    GET_PAYLOAD_SIZE(2 + 6 + 3);

    uint32_t bpp = p[0];
    uint32_t h = MemOps::readU16uBE(p + 1);
    uint32_t w = MemOps::readU16uBE(p + 3);
    uint32_t component_count = p[5];

    if (size != 8 + 3 * component_count)
      return bl_make_error(BL_ERROR_JPEG_INVALID_SOF);

    // Advance header.
    p += 6;

    if (w == 0) {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    // TODO: [JPEG] Unsupported delayed height (0).
    if (h == 0) {
      return bl_make_error(BL_ERROR_JPEG_UNSUPPORTED_FEATURE);
    }

    if (w > BL_RUNTIME_MAX_IMAGE_SIZE || h > BL_RUNTIME_MAX_IMAGE_SIZE) {
      return bl_make_error(BL_ERROR_IMAGE_TOO_LARGE);
    }

    // Check number of components and SOF size.
    if ((component_count != 1 && component_count != 3)) {
      return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
    }

    // TODO: [JPEG] 16-BPC.
    if (bpp != 8) {
      return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
    }

    // Maximum horizontal/vertical sampling factor of all components.
    uint32_t mcu_sf_w = 1;
    uint32_t mcu_sf_h = 1;

    uint32_t i, j;
    for (i = 0; i < component_count; i++, p += 3) {
      DecoderComponent* comp = &decoder_impl->comp[i];

      // Check if the ID doesn't collide with previous components.
      uint32_t comp_id = p[0];
      for (j = 0; j < i; j++) {
        if (decoder_impl->comp[j].comp_id == comp_id) {
          return bl_make_error(BL_ERROR_INVALID_DATA);
        }
      }

      // TODO: [JPEG] Is this necessary?
      // Required by JFIF.
      if (comp_id != i + 1) {
        // Some version of JpegTran outputs non-JFIF-compliant files!
        if (comp_id != i) {
          return bl_make_error(BL_ERROR_INVALID_DATA);
        }
      }

      // Horizontal/Vertical sampling factor.
      uint32_t sf = p[1];
      uint32_t sf_w = sf >> 4;
      uint32_t sf_h = sf & 15;

      if (sf_w == 0 || sf_w > 4 || sf_h == 0 || sf_h > 4) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      // Bail to 1 if there is only one component as it contributes to nothing.
      if (component_count == 1) {
        sf_w = 1;
        sf_h = 1;
      }

      // Quantization ID.
      uint32_t quant_id = p[2];
      if (quant_id > 3) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      // Save to DecoderComponent.
      comp->comp_id  = uint8_t(comp_id);
      comp->sf_w     = uint8_t(sf_w);
      comp->sf_h     = uint8_t(sf_h);
      comp->quant_id = uint8_t(quant_id);

      // We need to know maximum horizontal and vertical sampling factor to calculate the correct MCU size (WxH).
      mcu_sf_w = bl_max(mcu_sf_w, sf_w);
      mcu_sf_h = bl_max(mcu_sf_h, sf_h);
    }

    // Compute interleaved MCU info.
    uint32_t mcu_px_w = mcu_sf_w * kDctSize;
    uint32_t mcu_px_h = mcu_sf_h * kDctSize;

    uint32_t mcu_count_w = (w + mcu_px_w - 1) / mcu_px_w;
    uint32_t mcu_count_h = (h + mcu_px_h - 1) / mcu_px_h;
    bool is_baseline = sof_marker != kMarkerSOF2;

    for (i = 0; i < component_count; i++) {
      DecoderComponent* comp = &decoder_impl->comp[i];

      // Number of effective pixels (e.g. for non-interleaved MCU).
      comp->px_w = (w * uint32_t(comp->sf_w) + mcu_sf_w - 1) / mcu_sf_w;
      comp->px_h = (h * uint32_t(comp->sf_h) + mcu_sf_h - 1) / mcu_sf_h;

      // Allocate enough memory for all blocks even those that won't be used fully.
      comp->bl_w = mcu_count_w * uint32_t(comp->sf_w);
      comp->bl_h = mcu_count_h * uint32_t(comp->sf_h);

      comp->os_w = comp->bl_w * kDctSize;
      comp->os_h = comp->bl_h * kDctSize;

      comp->data = static_cast<uint8_t*>(decoder_impl->allocator.alloc(comp->os_w * comp->os_h));
      if (comp->data == nullptr) {
        return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
      }

      if (!is_baseline) {
        uint32_t kBlock8x8UInt16 = kDctSize2 * uint32_t(sizeof(int16_t));
        size_t coeff_size = comp->bl_w * comp->bl_h * kBlock8x8UInt16;
        int16_t* coeff_data = static_cast<int16_t*>(decoder_impl->allocator.alloc(coeff_size, 16));

        if (coeff_data == nullptr) {
          return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
        }

        comp->coeff = coeff_data;
        memset(comp->coeff, 0, coeff_size);
      }
    }

    // Everything seems ok, store the image information.
    image_info.flags = 0;
    image_info.size.reset(int(w), int(h));
    image_info.depth = uint16_t(component_count * bpp);
    image_info.plane_count = uint16_t(component_count);
    image_info.frame_count = 1;

    if (!is_baseline) {
      image_info.flags |= BL_IMAGE_INFO_FLAG_PROGRESSIVE;
    }

    decoder_impl->sof_marker = uint8_t(sof_marker);
    decoder_impl->delayed_height = (h == 0);
    decoder_impl->mcu.sf.w = uint8_t(mcu_sf_w);
    decoder_impl->mcu.sf.h = uint8_t(mcu_sf_h);
    decoder_impl->mcu.px.w = uint8_t(mcu_px_w);
    decoder_impl->mcu.px.h = uint8_t(mcu_px_h);
    decoder_impl->mcu.count.w = mcu_count_w;
    decoder_impl->mcu.count.h = mcu_count_h;

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // DHT - Define Huffman Table
  // --------------------------
  //
  //        WORD - Size (Consumed by GET_PAYLOAD_...)
  //
  //   [00] BYTE - Table class `tc` and table identifier `ti`.
  //   [01] 16xB - The count of Huffman codes of size 1..16.
  //
  //   [17] .... - The one byte symbols sorted by Huffman code. The number of symbols is the sum of the 16 code counts.

  if (m == kMarkerDHT) {
    GET_PAYLOAD_SIZE(2 + 17);

    while (remain) {
      uint32_t q = *p++;
      remain--;

      uint32_t table_class = q >> 4; // Table class.
      uint32_t table_id = q & 15; // Table id (0-3).

      // Invalid class or id.
      if (table_class >= kTableCount || table_id > 3) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      size_t table_size_in_bytes;
      if (table_class == kTableDC) {
        BL_PROPAGATE(build_huffman_dc(&decoder_impl->dc_table[table_id], p, remain, &table_size_in_bytes));
        decoder_impl->dc_table_mask = uint8_t(decoder_impl->dc_table_mask | IntOps::lsb_bit_at<uint32_t>(table_id));
      }
      else {
        BL_PROPAGATE(build_huffman_ac(&decoder_impl->ac_table[table_id], p, remain, &table_size_in_bytes));
        decoder_impl->ac_table_mask = uint8_t(decoder_impl->ac_table_mask | IntOps::lsb_bit_at<uint32_t>(table_id));
      }

      p += table_size_in_bytes;
      remain -= table_size_in_bytes;
    }

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // DQT - Define Quantization Table
  // -------------------------------
  //
  //        WORD - Size (Consumed by GET_PAYLOAD_...)
  //
  //   [00] BYTE - Quantization value size `quant_sz` (0-1) and table identifier `quant_id`.
  //   [01] .... - 64 or 128 bytes depending on `qs`.

  if (m == kMarkerDQT) {
    GET_PAYLOAD_SIZE(2 + 65);

    while (remain >= 65) {
      uint32_t q = *p++;

      uint32_t q_size = q >> 4;
      uint32_t q_id = q & 15;

      if (q_size > 1 || q_id > 3) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      uint16_t* q_table = decoder_impl->q_table[q_id].data;
      uint32_t required_size = 1 + 64 * (q_size + 1);

      if (required_size > remain) {
        break;
      }

      if (q_size == 0) {
        for (uint32_t k = 0; k < 64; k++, p++) {
          q_table[decoder_de_zig_zag_table[k]] = *p;
        }
      }
      else {
        for (uint32_t k = 0; k < 64; k++, p += 2) {
          q_table[decoder_de_zig_zag_table[k]] = uint16_t(MemOps::readU16uBE(reinterpret_cast<const uint16_t*>(p)));
        }
      }

      decoder_impl->q_table_mask = uint8_t(decoder_impl->q_table_mask | IntOps::lsb_bit_at<uint32_t>(q_id));
      remain -= required_size;
    }

    if (remain != 0) {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // DRI - Define Restart Interval
  // -----------------------------
  //
  //        WORD - Size (Consumed by GET_PAYLOAD_...)
  //
  //   [00] WORD - Restart interval.

  if (m == kMarkerDRI) {
    if (remain < 4) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    size_t size = MemOps::readU16uBE(p + 0);
    uint32_t ri = MemOps::readU16uBE(p + 2);

    // DRI payload should be 4 bytes.
    if (size != 4) {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    decoder_impl->restart_interval = ri;
    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // SOS - Start of Scan
  // -------------------
  //
  //        WORD - Size (Consumed by GET_PAYLOAD_...)
  //
  //   [00] BYTE - Number of components in this SOS:
  //
  //   [01] Specification of each component - {
  //        [00] BYTE - Component ID
  //        [01] BYTE - DC and AC Selector
  //   }
  //
  //   [01 + NumComponents * 2]:
  //        [00] BYTE - Spectral Selection Start
  //        [01] BYTE - Spectral Selection End
  //        [02] BYTE - Successive Approximation High/Low

  if (m == kMarkerSOS) {
    GET_PAYLOAD_SIZE(2 + 6);

    uint32_t sof_marker = decoder_impl->sof_marker;
    uint32_t component_count = image_info.plane_count;

    uint32_t sc_count = *p++;
    uint32_t sc_mask = 0;

    if (size != 6 + sc_count * 2) {
      return bl_make_error(BL_ERROR_IMAGE_UNKNOWN_FILE_FORMAT);
    }

    if (sc_count < 1 || sc_count > component_count) {
      return bl_make_error(BL_ERROR_JPEG_INVALID_SOS);
    }

    uint32_t ss_start    = uint32_t(p[sc_count * 2 + 0]);
    uint32_t ss_end      = uint32_t(p[sc_count * 2 + 1]);
    uint32_t sa_low_bit  = uint32_t(p[sc_count * 2 + 2]) & 15;
    uint32_t sa_high_bit = uint32_t(p[sc_count * 2 + 2]) >> 4;

    if (sof_marker == kMarkerSOF0 || sof_marker == kMarkerSOF1) {
      if (ss_start != 0 || sa_low_bit != 0 || sa_high_bit != 0) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      // The value should be 63, but it's zero sometimes.
      ss_end = 63;
    }

    if (sof_marker == kMarkerSOF2) {
      if (ss_start > 63 || ss_end > 63 || ss_start > ss_end || sa_low_bit > 13 || sa_high_bit > 13) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      // AC & DC cannot be merged in a progressive JPEG.
      if (ss_start == 0 && ss_end != 0) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }
    }

    DecoderSOS& sos = decoder_impl->sos;
    sos.sc_count    = uint8_t(sc_count);
    sos.ss_start    = uint8_t(ss_start);
    sos.ss_end      = uint8_t(ss_end);
    sos.sa_low_bit  = uint8_t(sa_low_bit);
    sos.sa_high_bit = uint8_t(sa_high_bit);

    for (uint32_t i = 0; i < sc_count; i++, p += 2) {
      uint32_t comp_id = p[0];
      uint32_t index = 0;

      while (decoder_impl->comp[index].comp_id != comp_id) {
        if (++index >= component_count) {
          return bl_make_error(BL_ERROR_JPEG_INVALID_SOS);
        }
      }

      // One huffman stream shouldn't overwrite the same component.
      if (IntOps::bit_test(sc_mask, index)) {
        return bl_make_error(BL_ERROR_JPEG_INVALID_SOS);
      }

      sc_mask |= IntOps::lsb_bit_at<uint32_t>(index);

      uint32_t selector = p[1];
      uint32_t ac_id = selector & 15;
      uint32_t dc_id = selector >> 4;

      // Validate AC & DC selectors.
      if (ac_id > 3 || (!IntOps::bit_test(decoder_impl->ac_table_mask, ac_id) && ss_end  > 0)) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      if (dc_id > 3 || (!IntOps::bit_test(decoder_impl->dc_table_mask, dc_id) && ss_end == 0)) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      // Link the current component to the `index` and update AC & DC selectors.
      DecoderComponent* comp = &decoder_impl->comp[index];
      comp->dc_id = uint8_t(dc_id);
      comp->ac_id = uint8_t(ac_id);
      sos.sc_comp[i] = comp;
    }

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // APP - Application
  // -----------------

  if (is_marker_app(m)) {
    GET_PAYLOAD_SIZE(2);

    // APP0 - "JFIF\0"
    // ---------------

    if (m == kMarkerAPP0 && remain >= 5 && memcmp(p, "JFIF", 5) == 0) {
      if (bl_test_flag(decoder_impl->status_flags, DecoderStatusFlags::kDoneJFIF)) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      if (remain < 14) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      uint32_t jfif_major = p[5];
      uint32_t jfif_minor = p[6];

      // Check the density unit, correct it to aspect-only if it's wrong, but
      // don't fail as of one wrong value won't make any difference anyway.
      uint32_t density_unit = p[7];
      uint32_t x_density = MemOps::readU16uBE(p + 8);
      uint32_t y_density = MemOps::readU16uBE(p + 10);

      switch (density_unit) {
        case kDensityOnlyAspect:
          // TODO: [JPEG]
          break;

        case kDensityPixelsPerIN:
          image_info.density.reset(double(int(x_density)) * 39.3701, double(int(y_density)) * 39.3701);
          break;

        case kDensityPixelsPerCM:
          image_info.density.reset(double(int(x_density * 100)), double(int(y_density * 100)));
          break;

        default:
          break;
      }

      uint32_t thumb_w = p[12];
      uint32_t thumb_h = p[13];

      decoder_impl->status_flags |= DecoderStatusFlags::kDoneJFIF;
      decoder_impl->jfif_major = uint8_t(jfif_major);
      decoder_impl->jfif_minor = uint8_t(jfif_minor);

      if (thumb_w && thumb_h) {
        uint32_t thumb_size = thumb_w * thumb_h * 3;

        if (thumb_size + 14 < remain) {
          return bl_make_error(BL_ERROR_INVALID_DATA);
        }

        DecoderThumbnail& thumb = decoder_impl->thumb;
        thumb.format = kThumbnailRGB24;
        thumb.w = uint8_t(thumb_w);
        thumb.h = uint8_t(thumb_h);
        thumb.index = decoder_impl->buffer_index + 18;
        thumb.size = thumb_size;
        decoder_impl->status_flags |= DecoderStatusFlags::kHasThumb;
      }
    }

    // APP0 - "JFXX\0"
    // ---------------

    if (m == kMarkerAPP0 && remain >= 5 && memcmp(p, "JFXX", 5) == 0) {
      if (bl_test_flag(decoder_impl->status_flags, DecoderStatusFlags::kDoneJFXX)) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      if (remain < 6) {
        return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      uint32_t format = p[5];
      uint32_t thumb_w = 0;
      uint32_t thumb_h = 0;
      uint32_t thumb_size = 0;

      switch (format) {
        case kThumbnailJPEG:
          // Cannot overflow as the payload size is just 16-bit uint.
          thumb_size = uint32_t(remain - 6);
          break;

        case kThumbnailPAL8:
          thumb_w = p[6];
          thumb_h = p[7];
          thumb_size = 768 + thumb_w * thumb_h;
          break;

        case kThumbnailRGB24:
          thumb_w = p[6];
          thumb_h = p[7];
          thumb_size = thumb_w * thumb_h * 3;
          break;

        default:
          return bl_make_error(BL_ERROR_INVALID_DATA);
      }

      if (thumb_size + 6 > remain)
        return bl_make_error(BL_ERROR_INVALID_DATA);

      decoder_impl->thumb.format = uint8_t(format);
      decoder_impl->thumb.w = uint8_t(thumb_w);
      decoder_impl->thumb.h = uint8_t(thumb_h);
      decoder_impl->thumb.index = decoder_impl->buffer_index + 10;
      decoder_impl->thumb.size = thumb_size;

      decoder_impl->status_flags |= DecoderStatusFlags::kDoneJFXX | DecoderStatusFlags::kHasThumb;
    }

    // APP1 - "EXIF\0\0"
    // -----------------
    /*
    // TODO: [JPEG] This would require some work to make this possible.
    if (m == kMarkerAPP1 && remain >= 6 && memcmp(p, "Exif\0", 6) == 0) {
      // These should be only one EXIF marker in the whole JPEG image, not sure
      // what to do if there is more...
      if (!(decoder_impl->status_flags & BL_JPEG_DECODER_DONE_EXIF)) {
        p += 6;
        remain -= 6;

        // Need at least more 8 bytes required by TIFF header.
        if (remain < 8)
          return bl_make_error(kErrorExifInvalidHeader);

        // Check if the EXIF marker has a proper TIFF header.
        uint32_t byte_order;
        uint32_t do_byte_swap;

        if (memcmp(p, blJpegExifLE, 4))
          byte_order = BL_BYTE_ORDER_LE;
        else if (memcmp(p, blJpegExifBE, 4))
          byte_order = BL_BYTE_ORDER_BE;
        else
          return bl_make_error(kErrorExifInvalidHeader);

        do_byte_swap = byte_order != BL_BYTE_ORDER_NATIVE;
        decoder_impl->status_flags |= BL_JPEG_DECODER_DONE_EXIF;
      }
    }
    */

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // COM - Comment
  // -------------

  if (m == kMarkerCOM) {
    GET_PAYLOAD_SIZE(2);

    consumed_bytes = size;
    return BL_SUCCESS;
  }

  // EOI - End of Image
  // ------------------

  if (m == kMarkerEOI) {
    decoder_impl->status_flags |= DecoderStatusFlags::kDoneEOI;
    return BL_SUCCESS;
  }

  // Invalid / Unknown
  // -----------------

  return bl_make_error(BL_ERROR_INVALID_DATA);

#undef GET_PAYLOAD_SIZE
}

// bl::Jpeg::Decoder - Process Stream
// ==================================

struct DecoderRun {
  //! Component linked with the run.
  DecoderComponent* comp;

  //! Current data pointer (advanced during decoding).
  uint8_t* data;
  //! De-quantization table pointer.
  const Block<uint16_t>* q_table;

  //! Count of 8x8 blocks required by a single MCU, calculated as `sf_w * sf_h`.
  uint32_t count;
  //! Stride.
  uint32_t stride;
  //! Horizontal/Vertical advance per MCU.
  uint32_t advance[2];

  //! Offsets of all blocks of this component that are part of a single MCU.
  intptr_t offset[16];
};

// Called after a restart marker (RES) has been reached.
static BLResult decoder_handle_restart(BLJpegDecoderImpl* decoder_impl, DecoderBitStream& stream, const uint8_t* pEnd) noexcept {
  if (stream.restart_counter == 0 || --stream.restart_counter != 0)
    return BL_SUCCESS;

  // I think this shouldn't be necessary to refill the code buffer/size as all bytes should have been consumed.
  // However, since the spec is so vague, I'm not sure if this is necessary, recommended, or forbidden :(
  DecoderBitReader reader(stream);
  reader.refill();

  if (!reader.at_end() || (size_t)(pEnd - reader.ptr) < 2 || !is_marker_rst(reader.ptr[1]))
    return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);

  // Skip the marker and flush entropy bits.
  reader.flush();
  reader.advance(2);
  reader.done(stream);

  stream.eob_run = 0;
  stream.restart_counter = decoder_impl->restart_interval;

  // Reset DC predictions.
  DecoderComponent* comp = decoder_impl->comp;
  comp[0].dc_pred = 0;
  comp[1].dc_pred = 0;
  comp[2].dc_pred = 0;
  comp[3].dc_pred = 0;
  return BL_SUCCESS;
}

//! Decode a baseline 8x8 block.
static BLResult decoder_read_baseline_block(BLJpegDecoderImpl* decoder_impl, DecoderBitStream& stream, DecoderComponent* comp, int16_t* dst) noexcept {
  const DecoderHuffmanTable* dc_table = &decoder_impl->dc_table[comp->dc_id];
  const DecoderHuffmanTable* ac_table = &decoder_impl->ac_table[comp->ac_id];

  DecoderBitReader reader(stream);
  reader.refill();

  // Decode DC - Maximum Bytes Consumed: 4 (unescaped)
  // -------------------------------------------------

  uint32_t s;
  int32_t dc_pred = comp->dc_pred;
  BL_PROPAGATE(reader.read_code(s, dc_table));

  if (s) {
    reader.refill_if_32bit();
    BL_PROPAGATE(reader.require_bits(s));

    int32_t dc_val = reader.read_signed(s);
    dc_pred += dc_val;
    comp->dc_pred = dc_pred;
  }
  dst[0] = int16_t(dc_pred);

  // Decode AC - Maximum Bytes Consumed: 4 * 63 (unescaped)
  // ------------------------------------------------------

  uint32_t k = 1;
  const int16_t* ac_accel = decoder_impl->ac_table[comp->ac_id].ac_accel;

  do {
    reader.refill();

    uint32_t c = reader.peek<uint32_t>(kHuffmanAccelBits);
    int32_t ac = ac_accel[c];

    // Fast AC.
    if (ac) {
      s = (ac & 15);       // Size.
      k += (ac >> 4) & 15; // Skip.
      ac >>= 8;
      reader.drop(s);
      dst[decoder_de_zig_zag_table[k++]] = int16_t(ac);
    }
    else {
      BL_PROPAGATE(reader.read_code(ac, ac_table));
      s = ac & 15;
      ac >>= 4;

      if (s == 0) {
        // End block.
        if (ac != 0xF)
          break;
        k += 16;
      }
      else {
        k += uint32_t(ac);

        reader.refill_if_32bit();
        BL_PROPAGATE(reader.require_bits(s));

        ac = reader.read_signed(s);
        dst[decoder_de_zig_zag_table[k++]] = int16_t(ac);
      }
    }
  } while (k < 64);

  reader.done(stream);
  return BL_SUCCESS;
}

//! Decode a progressive 8x8 block (AC or DC coefficients, but never both).
static BLResult decoder_read_progressive_block(BLJpegDecoderImpl* decoder_impl, DecoderBitStream& stream, DecoderComponent* comp, int16_t* dst) noexcept {
  DecoderBitReader reader(stream);
  reader.refill();

  uint32_t k     = uint32_t(decoder_impl->sos.ss_start);
  uint32_t k_end = uint32_t(decoder_impl->sos.ss_end) + 1;
  uint32_t shift = decoder_impl->sos.sa_low_bit;

  // Decode DC - Maximum Bytes Consumed: 4 (unescaped)
  // -------------------------------------------------

  if (k == 0) {
    const DecoderHuffmanTable* dc_table = &decoder_impl->dc_table[comp->dc_id];
    uint32_t s;

    if (decoder_impl->sos.sa_high_bit == 0) {
      // Initial scan for DC coefficient.
      int32_t dc_pred = comp->dc_pred;
      BL_PROPAGATE(reader.read_code(s, dc_table));

      if (s) {
        reader.refill_if_32bit();
        BL_PROPAGATE(reader.require_bits(s));

        int32_t dc_val = reader.read_signed(s);
        dc_pred += dc_val;
        comp->dc_pred = dc_pred;
      }

      dst[0] = int16_t(IntOps::shl(dc_pred, shift));
    }
    else {
      // Refinement scan for DC coefficient.
      BL_PROPAGATE(reader.require_bits(1));

      s = reader.read_bit<uint32_t>();
      dst[0] = int16_t(int32_t(dst[0]) + int32_t(s << shift));
    }

    k++;
  }

  // Decode AC - Maximum Bytes Consumed: max(4 * 63, 8) (unescaped)
  // --------------------------------------------------------------

  if (k < k_end) {
    const DecoderHuffmanTable* ac_table = &decoder_impl->ac_table[comp->ac_id];
    const int16_t* ac_accel = decoder_impl->ac_table[comp->ac_id].ac_accel;

    if (decoder_impl->sos.sa_high_bit == 0) {
      // Initial scan for AC coefficients.
      if (stream.eob_run) {
        stream.eob_run--;
        return BL_SUCCESS;
      }

      do {
        // Fast AC.
        reader.refill();
        int32_t r = ac_accel[reader.peek(kHuffmanAccelBits)];

        if (r) {
          int32_t s = r & 15;
          k += (r >> 4) & 15;
          reader.drop(uint32_t(s));

          uint32_t zig = decoder_de_zig_zag_table[k++];
          dst[zig] = int16_t(IntOps::shl(r >> 8, shift));
        }
        else {
          BL_PROPAGATE(reader.read_code(r, ac_table));
          reader.refill_if_32bit();

          int32_t s = r & 15;
          r >>= 4;

          if (s == 0) {
            if (r < 15) {
              uint32_t eob_run = 0;
              if (r) {
                BL_PROPAGATE(reader.require_bits(uint32_t(r)));
                eob_run = reader.read_unsigned(uint32_t(r));
              }
              stream.eob_run = eob_run + (1u << r) - 1;
              break;
            }
            k += 16;
          }
          else {
            k += uint32_t(r);
            r = reader.read_signed(uint32_t(s));

            uint32_t zig = decoder_de_zig_zag_table[k++];
            dst[zig] = int16_t(IntOps::shl(r, shift));
          }
        }
      } while (k < k_end);
    }
    else {
      // Refinement scan for AC coefficients.
      int32_t bit = int32_t(1) << shift;
      if (stream.eob_run) {
        do {
          int16_t* p = &dst[decoder_de_zig_zag_table[k++]];
          int32_t pVal = *p;

          if (pVal) {
            BL_PROPAGATE(reader.require_bits(1));
            uint32_t b = reader.read_bit<uint32_t>();

            reader.refill();
            if (b && (pVal & bit) == 0)
              *p = int16_t(pVal + (pVal > 0 ? bit : -bit));
          }
        } while (k < k_end);
        stream.eob_run--;
      }
      else {
        do {
          int32_t r, s;

          reader.refill();
          BL_PROPAGATE(reader.read_code(r, ac_table));

          reader.refill_if_32bit();
          s = r & 15;
          r >>= 4;

          if (s == 0) {
            if (r < 15) {
              uint32_t eob_run = 0;
              if (r) {
                BL_PROPAGATE(reader.require_bits(uint32_t(r)));
                eob_run = reader.read_unsigned(uint32_t(r));
              }
              stream.eob_run = eob_run + (1u << r) - 1;
              r = 64; // Force end of block.
            }
            // r=15 s=0 already does the right thing (write 16 0s).
          }
          else {
            if (BL_UNLIKELY(s != 1))
              return bl_make_error(BL_ERROR_DECOMPRESSION_FAILED);

            BL_PROPAGATE(reader.require_bits(1));
            uint32_t sign = reader.read_bit<uint32_t>();
            s = sign ? bit : -bit;
          }

          // Advance by `r`.
          while (k < k_end) {
            int16_t* p = &dst[decoder_de_zig_zag_table[k++]];
            int32_t pVal = *p;

            if (pVal) {
              uint32_t b;

              reader.refill();
              BL_PROPAGATE(reader.require_bits(1));

              b = reader.read_bit<uint32_t>();
              if (b && (pVal & bit) == 0)
                *p = int16_t(pVal + (pVal > 0 ? bit : -bit));
            }
            else {
              if (r == 0) {
                *p = int16_t(s);
                break;
              }
              r--;
            }
          }
        } while (k < k_end);
      }
    }
  }

  reader.done(stream);
  return BL_SUCCESS;
}

static BLResult decoder_process_stream(BLJpegDecoderImpl* decoder_impl, const uint8_t* p, size_t remain, size_t& consumed_bytes) noexcept {
  DecoderSOS& sos = decoder_impl->sos;

  const uint8_t* start = p;
  const uint8_t* end = p + remain;

  // Initialize
  // ----------

  // Just needed to determine the logic.
  uint32_t sof_marker = decoder_impl->sof_marker;

  // Whether the stream is baseline or progressive. Progressive streams use multiple SOS markers to progressively
  // update the image being decoded.
  bool is_baseline = sof_marker != kMarkerSOF2;

  // If this is a baseline stream then the unit-size is 1 byte, because the block of coefficients is immediately
  // IDCTed to pixel values after it is decoded. However, progressive decoding cannot use this space optimization
  // as coefficients are updated progressively.
  uint32_t unit_size = is_baseline ? 1 : 2;

  // Initialize the entropy stream.
  DecoderBitStream stream;
  stream.reset(p, end);
  stream.restart_counter = decoder_impl->restart_interval;

  uint32_t i;
  uint32_t sc_count = sos.sc_count;

  uint32_t mcu_x = 0;
  uint32_t mcu_y = 0;

  // TODO: [JPEG] This is not right, we must calculate MCU W/H every time.
  uint32_t mcu_w = decoder_impl->mcu.count.w;
  uint32_t mcu_h = decoder_impl->mcu.count.h;

  // A single component's decoding doesn't use interleaved MCUs.
  if (sc_count == 1) {
    DecoderComponent* comp = sos.sc_comp[0];
    mcu_w = (comp->px_w + kDctSize - 1) / kDctSize;
    mcu_h = (comp->px_h + kDctSize - 1) / kDctSize;
  }

  // Initialize decoder runs (each run specifies one component per scan).
  DecoderRun runs[4];
  for (i = 0; i < sc_count; i++) {
    DecoderRun* run = &runs[i];
    DecoderComponent* comp = sos.sc_comp[i];

    uint32_t sf_w = sc_count > 1 ? uint32_t(comp->sf_w) : uint32_t(1);
    uint32_t sf_h = sc_count > 1 ? uint32_t(comp->sf_h) : uint32_t(1);

    uint32_t count = 0;
    uint32_t offset = 0;

    if (is_baseline) {
      uint32_t stride = comp->os_w * unit_size;

      for (uint32_t y = 0; y < sf_h; y++) {
        for (uint32_t x = 0; x < sf_w; x++) {
          run->offset[count++] = intptr_t(offset + x * unit_size * kDctSize);
        }
        offset += stride * kDctSize;
      }

      run->comp = comp;
      run->data = comp->data;
      run->q_table = &decoder_impl->q_table[comp->quant_id];

      run->count = count;
      run->stride = stride;
      run->advance[0] = sf_w * unit_size * kDctSize;
      run->advance[1] = run->advance[0] + (sf_h * kDctSize - 1) * stride;
    }
    else {
      uint32_t block_size = unit_size * kDctSize2;
      uint32_t block_stride = comp->bl_w * block_size;

      for (uint32_t y = 0; y < sf_h; y++) {
        for (uint32_t x = 0; x < sf_w; x++) {
          run->offset[count++] = intptr_t(offset + x * block_size);
        }
        offset += block_stride;
      }

      run->comp = comp;
      run->data = reinterpret_cast<uint8_t*>(comp->coeff);
      run->q_table = nullptr;

      run->count = count;
      run->stride = 0;

      run->advance[0] = sf_w * block_size;
      run->advance[1] = sf_h * block_stride - (mcu_w - 1) * run->advance[0];
    }
  }

  // SOF0/1 - Baseline / Extended
  // ----------------------------

  if (sof_marker == kMarkerSOF0 || sof_marker == kMarkerSOF1) {
    Block<int16_t> tmp_block;

    for (;;) {
      // Increment it here so we can use `mcu_x == mcu_w` in the inner loop.
      mcu_x++;

      // Decode all blocks required by a single MCU.
      for (i = 0; i < sc_count; i++) {
        DecoderRun* run = &runs[i];
        uint8_t* block_data = run->data;
        uint32_t block_count = run->count;

        for (uint32_t n = 0; n < block_count; n++) {
          tmp_block.reset();
          BL_PROPAGATE(decoder_read_baseline_block(decoder_impl, stream, run->comp, tmp_block.data));
          opts.idct8(block_data + run->offset[n], intptr_t(run->stride), tmp_block.data, run->q_table->data);
        }

        run->data = block_data + run->advance[mcu_x == mcu_w];
      }

      // Advance.
      if (mcu_x == mcu_w) {
        if (++mcu_y == mcu_h) {
          break;
        }
        mcu_x = 0;
      }

      // Restart.
      BL_PROPAGATE(decoder_handle_restart(decoder_impl, stream, end));
    }
  }

  // SOF2 - Progressive
  // ------------------

  else if (sof_marker == kMarkerSOF2) {
    for (;;) {
      // Increment it here so we can use `mcu_x == mcu_w` in the inner loop.
      mcu_x++;

      // Decode all blocks required by a single MCU.
      for (i = 0; i < sc_count; i++) {
        DecoderRun* run = &runs[i];

        uint8_t* block_data = run->data;
        uint32_t block_count = run->count;

        for (uint32_t n = 0; n < block_count; n++) {
          BL_PROPAGATE(decoder_read_progressive_block(decoder_impl, stream, run->comp,
            reinterpret_cast<int16_t*>(block_data + run->offset[n])));
        }

        run->data = block_data + run->advance[mcu_x == mcu_w];
      }

      // Advance.
      if (mcu_x == mcu_w) {
        if (++mcu_y == mcu_h) {
          break;
        }
        mcu_x = 0;
      }

      // Restart.
      BL_PROPAGATE(decoder_handle_restart(decoder_impl, stream, end));
    }
  }

  // End
  // ---

  else {
    BL_NOT_REACHED();
  }

  p = stream.ptr;

  // Skip zeros at the end of the entropy stream that was not consumed `refill()`
  while (p != end && p[0] == 0x00) {
    p++;
  }

  consumed_bytes = (size_t)(p - start);
  return BL_SUCCESS;
}

// bl:::Jpeg::Decoder - Process MCUs
// =================================

static BLResult decoder_process_mcus(BLJpegDecoderImpl* decoder_impl) noexcept {
  if (decoder_impl->sof_marker == kMarkerSOF2) {
    uint32_t component_count = decoder_impl->image_info.plane_count;

    // Dequantize & IDCT.
    for (uint32_t n = 0; n < component_count; n++) {
      DecoderComponent& comp = decoder_impl->comp[n];

      uint32_t w = (comp.px_w + 7) >> 3;
      uint32_t h = (comp.px_h + 7) >> 3;
      const Block<uint16_t>* q_table = &decoder_impl->q_table[comp.quant_id];

      for (uint32_t j = 0; j < h; j++) {
        for (uint32_t i = 0; i < w; i++) {
          int16_t *data = comp.coeff + 64 * (i + j * comp.bl_w);
          opts.idct8(comp.data + comp.os_w * j * 8 + i * 8, intptr_t(comp.os_w), data, q_table->data);
        }
      }
    }
  }

  return BL_SUCCESS;
}

// bl::Jpeg::Decoder - ConvertToRGB
// ================================

struct DecoderUpsample {
  uint8_t* line[2];

  // Expansion factor in each axis.
  uint32_t hs, vs;
  // Horizontal pixels pre-expansion.
  uint32_t w_lores;
  // How far through vertical expansion we are.
  uint32_t ystep;
  // Which pre-expansion row we're on.
  uint32_t ypos;
  // Selected upsample function.
  uint8_t* (BL_CDECL* upsample)(uint8_t* out, uint8_t* in0, uint8_t* in1, uint32_t w, uint32_t hs) noexcept;
};

static BLResult decoder_convert_to_rgb(BLJpegDecoderImpl* decoder_impl, BLImageData& dst) noexcept {
  uint32_t w = uint32_t(decoder_impl->image_info.size.w);
  uint32_t h = uint32_t(decoder_impl->image_info.size.h);

  BL_ASSERT(uint32_t(dst.size.w) >= w);
  BL_ASSERT(uint32_t(dst.size.h) >= h);

  uint8_t* dst_line = static_cast<uint8_t*>(dst.pixel_data);
  intptr_t dst_stride = dst.stride;

  bl::ScopedBufferTmp<1024 * 3 + 16> tmp_mem;

  // Allocate a line buffer that's big enough for up-sampling off the edges with up-sample factor of 4.
  uint32_t component_count = decoder_impl->image_info.plane_count;
  BL_ASSERT(component_count > 0u && component_count <= 4u);

  uint32_t line_stride = IntOps::align_up(w + 3, 16);
  uint8_t* line_buffer = static_cast<uint8_t*>(tmp_mem.alloc(line_stride * component_count));

  if (BL_UNLIKELY(!line_buffer))
    return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

  DecoderUpsample upsample[4];
  uint8_t* pPlane[4];
  uint8_t* pBuffer[4];

  for (uint32_t k = 0; k < component_count; k++) {
    DecoderComponent& comp = decoder_impl->comp[k];
    DecoderUpsample* r = &upsample[k];

    pBuffer[k] = line_buffer + k * line_stride;

    r->hs      = uint32_t(decoder_impl->mcu.sf.w / comp.sf_w);
    r->vs      = uint32_t(decoder_impl->mcu.sf.h / comp.sf_h);
    r->ystep   = r->vs >> 1;
    r->w_lores = (w + r->hs - 1) / r->hs;
    r->ypos    = 0;
    r->line[0] = comp.data;
    r->line[1] = comp.data;

    if      (r->hs == 1 && r->vs == 1) r->upsample = opts.upsample_1x1;
    else if (r->hs == 1 && r->vs == 2) r->upsample = opts.upsample_1x2;
    else if (r->hs == 2 && r->vs == 1) r->upsample = opts.upsample_2x1;
    else if (r->hs == 2 && r->vs == 2) r->upsample = opts.upsample_2x2;
    else                               r->upsample = opts.upsample_any;
  }

  // Now go ahead and resample.
  for (uint32_t y = 0; y < h; y++, dst_line += dst_stride) {
    for (uint32_t k = 0; k < component_count; k++) {
      DecoderComponent& comp = decoder_impl->comp[k];
      DecoderUpsample* r = &upsample[k];

      int y_bot = r->ystep >= (r->vs >> 1);
      pPlane[k] = r->upsample(pBuffer[k], r->line[y_bot], r->line[1 - y_bot], r->w_lores, r->hs);

      if (++r->ystep >= r->vs) {
        r->ystep = 0;
        r->line[0] = r->line[1];

        if (++r->ypos < comp.px_h) {
          r->line[1] += comp.os_w;
        }
      }
    }

    uint8_t* pY = pPlane[0];
    if (component_count == 3) {
      opts.conv_ycbcr8_to_rgb32(dst_line, pY, pPlane[1], pPlane[2], w);
    }
    else {
      for (uint32_t x = 0; x < w; x++) {
        MemOps::writeU32a(dst_line + x * 4, 0xFF000000u + uint32_t(pY[x]) * 0x010101u);
      }
    }
  }

  return BL_SUCCESS;
}

// bl::Jpeg::Decoder - Read Internal
// =================================

static BLResult decoder_read_info_impl_internal(BLJpegDecoderImpl* decoder_impl, const uint8_t* p, size_t size) noexcept {
  // JPEG file signature is 2 bytes (0xFF, 0xD8) followed by markers, SOF
  // (start of file) marker contains 1 byte signature and at least 8 bytes of
  // data describing basic information of the image.
  if (size < 2 + 8 + 1)
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);

  const uint8_t* start = p;
  const uint8_t* end = p + size;

  // Check JPEG signature (SOI marker).
  if (p[0] != 0xFF || p[1] != kMarkerSOI)
    return bl_make_error(BL_ERROR_INVALID_SIGNATURE);

  memcpy(decoder_impl->image_info.format, "JPEG", 5);
  memcpy(decoder_impl->image_info.compression, "HUFFMAN", 8);

  p += 2;
  decoder_impl->status_flags |= DecoderStatusFlags::kDoneSOI;

  // Process markers until SOF.
  for (;;) {
    decoder_impl->buffer_index = (size_t)(p - start);

    if ((size_t)(end - p) < 2)
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);

    if (p[0] != 0xFF)
      return bl_make_error(BL_ERROR_INVALID_DATA);

    uint32_t m = p[1];
    p += 2;

    // Some files have an extra padding (0xFF) after their blocks, ignore it.
    if (m == kMarkerInvalid) {
      while (p != end && (m = p[0]) == kMarkerInvalid) {
        p++;
      }

      if (p == end) {
        break;
      }

      p++;
    }

    size_t consumed_bytes = 0;
    BL_PROPAGATE(decoder_process_marker(decoder_impl, m, p, (size_t)(end - p), consumed_bytes));

    BL_ASSERT(consumed_bytes < (size_t)(end - p));
    p += consumed_bytes;

    // Terminate after SOF has been processed, the rest is handled by `decode()`.
    if (is_marker_sof(m)) {
      break;
    }
  }

  decoder_impl->buffer_index = (size_t)(p - start);
  return BL_SUCCESS;
}

static BLResult decoder_read_frame_impl_internal(BLJpegDecoderImpl* decoder_impl, BLImage* image_out, const uint8_t* p, size_t size) noexcept {
  const uint8_t* start = p;
  const uint8_t* end = p + size;

  if (size < decoder_impl->buffer_index)
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);

  p += decoder_impl->buffer_index;

  // Process markers.
  //
  // We are already after SOF, which was processed by `decoder_read_info_impl_internal`.
  for (;;) {
    decoder_impl->buffer_index = (size_t)(p - start);
    if ((size_t)(end - p) < 2) {
      return bl_make_error(BL_ERROR_DATA_TRUNCATED);
    }

    if (p[0] != 0xFF) {
      return bl_make_error(BL_ERROR_INVALID_DATA);
    }

    uint32_t m = p[1];
    p += 2;

    // Some files have an extra padding (0xFF) after their blocks, ignore it.
    if (m == kMarkerInvalid) {
      while (p != end && (m = p[0]) == kMarkerInvalid) {
        p++;
      }

      if (p == end) {
        break;
      }
      p++;
    }

    // Process the marker.
    {
      size_t consumed_bytes = 0;
      BL_PROPAGATE(decoder_process_marker(decoder_impl, m, p, (size_t)(end - p), consumed_bytes));

      BL_ASSERT((size_t)(end - p) >= consumed_bytes);
      p += consumed_bytes;
    }

    // EOI - terminate.
    if (m == kMarkerEOI) {
      break;
    }

    // SOS - process the entropy coded data-stream that follows SOS.
    if (m == kMarkerSOS) {
      size_t consumed_bytes = 0;
      BL_PROPAGATE(decoder_process_stream(decoder_impl, p, (size_t)(end - p), consumed_bytes));

      BL_ASSERT((size_t)(end - p) >= consumed_bytes);
      p += consumed_bytes;
      decoder_impl->status_flags |= DecoderStatusFlags::kDoneSOS;
    }
  }

  // Process MCUs.
  BL_PROPAGATE(decoder_process_mcus(decoder_impl));

  // Create the final image and convert YCbCr -> RGB.
  uint32_t w = uint32_t(decoder_impl->image_info.size.w);
  uint32_t h = uint32_t(decoder_impl->image_info.size.h);
  BLFormat format = BL_FORMAT_XRGB32;
  BLImageData image_data;

  BL_PROPAGATE(image_out->create(int(w), int(h), format));
  BL_PROPAGATE(image_out->make_mutable(&image_data));
  BL_PROPAGATE(decoder_convert_to_rgb(decoder_impl, image_data));

  decoder_impl->buffer_index = (size_t)(p - start);
  decoder_impl->frame_index++;

  return BL_SUCCESS;
}

// bl::Jpeg::Decoder - Interface
// =============================

static BLResult BL_CDECL decoder_restart_impl(BLImageDecoderImpl* impl) noexcept {
  BLJpegDecoderImpl* decoder_impl = static_cast<BLJpegDecoderImpl*>(impl);

  decoder_impl->last_result = BL_SUCCESS;
  decoder_impl->frame_index = 0;
  decoder_impl->buffer_index = 0;

  decoder_impl->allocator.reset();
  decoder_impl->image_info.reset();
  decoder_impl->status_flags = DecoderStatusFlags::kNoFlags;
  decoder_impl->restart_interval = 0;
  decoder_impl->sof_marker = 0;
  decoder_impl->colorspace = 0;
  decoder_impl->delayed_height = 0;
  decoder_impl->jfif_major = 0;
  decoder_impl->jfif_minor = 0;
  decoder_impl->dc_table_mask = 0;
  decoder_impl->ac_table_mask = 0;
  decoder_impl->q_table_mask = 0;
  decoder_impl->mcu.reset();
  decoder_impl->sos.reset();
  decoder_impl->thumb.reset();
  memset(decoder_impl->comp, 0, sizeof(decoder_impl->comp));

  return BL_SUCCESS;
}

static BLResult BL_CDECL decoder_read_info_impl(BLImageDecoderImpl* impl, BLImageInfo* info_out, const uint8_t* p, size_t size) noexcept {
  BLJpegDecoderImpl* decoder_impl = static_cast<BLJpegDecoderImpl*>(impl);
  BLResult result = decoder_impl->last_result;

  if (decoder_impl->buffer_index == 0 && result == BL_SUCCESS) {
    result = decoder_read_info_impl_internal(decoder_impl, p, size);
    if (result != BL_SUCCESS)
      decoder_impl->last_result = result;
  }

  if (info_out)
    memcpy(info_out, &decoder_impl->image_info, sizeof(BLImageInfo));

  return result;
}

static BLResult BL_CDECL decoder_read_frame_impl(BLImageDecoderImpl* impl, BLImageCore* image_out, const uint8_t* p, size_t size) noexcept {
  BLJpegDecoderImpl* decoder_impl = static_cast<BLJpegDecoderImpl*>(impl);
  BL_PROPAGATE(decoder_read_info_impl(decoder_impl, nullptr, p, size));

  if (decoder_impl->frame_index)
    return bl_make_error(BL_ERROR_NO_MORE_DATA);

  BLResult result = decoder_read_frame_impl_internal(decoder_impl, static_cast<BLImage*>(image_out), p, size);
  if (result != BL_SUCCESS)
    decoder_impl->last_result = result;
  return result;
}

static BLResult BL_CDECL bl_jpeg_decoder_impl_create(BLImageDecoderCore* self) noexcept {
  BLObjectInfo info = BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_DECODER);
  BL_PROPAGATE(ObjectInternal::alloc_impl_t<BLJpegDecoderImpl>(self, info));

  BLJpegDecoderImpl* decoder_impl = static_cast<BLJpegDecoderImpl*>(self->_d.impl);
  decoder_impl->ctor(&jpeg_decoder_virt, &jpeg_codec_instance);
  bl_call_ctor(decoder_impl->allocator);
  return decoder_restart_impl(decoder_impl);
}

static BLResult BL_CDECL decoder_destroy_impl(BLObjectImpl* impl) noexcept {
  BLJpegDecoderImpl* decoder_impl = static_cast<BLJpegDecoderImpl*>(impl);

  decoder_impl->allocator.reset();
  decoder_impl->dtor();
  return bl_object_free_impl(decoder_impl);
}

// bl::Jpeg::Codec - Interface
// ===========================

static BLResult BL_CDECL codec_destroy_impl(BLObjectImpl* impl) noexcept {
  // Built-in codecs are never destroyed.
  bl_unused(impl);
  return BL_SUCCESS;
}

static uint32_t BL_CDECL codec_inspect_data_impl(const BLImageCodecImpl* impl, const uint8_t* data, size_t size) noexcept {
  bl_unused(impl);

  // JPEG minimum size and signature (SOI).
  if (size < 2 || data[0] != 0xFF || data[1] != kMarkerSOI)
    return 0;

  // JPEG signature has to be followed by a marker that starts with 0xFF.
  if (size > 2 && data[2] != 0xFF)
    return 0;

  return 100;
}

static BLResult BL_CDECL codec_create_decoder_impl(const BLImageCodecImpl* impl, BLImageDecoderCore* dst) noexcept {
  bl_unused(impl);

  BLImageDecoderCore tmp;
  BL_PROPAGATE(bl_jpeg_decoder_impl_create(&tmp));
  return bl_image_decoder_assign_move(dst, &tmp);
}

static BLResult BL_CDECL codec_create_encoder_impl(const BLImageCodecImpl* impl, BLImageEncoderCore* dst) noexcept {
  bl_unused(impl);
  bl_unused(dst);

  return bl_make_error(BL_ERROR_IMAGE_ENCODER_NOT_PROVIDED);

  // TODO: [JPEG] Encoder
  /*
  BLImageEncoderCore tmp;
  BL_PROPAGATE(bl_jpeg_encoder_impl_create(tmp._d));
  return bl_image_encoder_assign_move(dst, &tmp);
  */
}

// bl::Jpeg::Codec - Runtime Registration
// ======================================

void jpeg_codec_on_init(BLRuntimeContext* rt, BLArray<BLImageCodec>* codecs) noexcept {
  using namespace bl::Jpeg;

  bl_unused(rt);

  BL_DEFINE_STATIC_STRING(jpeg_extensions, "jpg|jpeg|jif|jfi|jfif");

  // Initialize JPEG opts.
  opts.idct8 = idct8;
  opts.conv_ycbcr8_to_rgb32 = rgb32_from_ycbcr8;

#ifdef BL_BUILD_OPT_SSE2
  opts.idct8 = idct8_sse2;
  opts.conv_ycbcr8_to_rgb32 = rgb32_from_ycbcr8_sse2;
#endif

  opts.upsample_1x1 = upsample_1x1;
  opts.upsample_1x2 = upsample_1x2;
  opts.upsample_2x1 = upsample_2x1;
  opts.upsample_2x2 = upsample_2x2;
  opts.upsample_any = upsample_generic;

  // Initialize JPEG codec.
  jpeg_codec.virt.base.destroy = codec_destroy_impl;
  jpeg_codec.virt.base.get_property = bl_object_impl_get_property;
  jpeg_codec.virt.base.set_property = bl_object_impl_set_property;
  jpeg_codec.virt.inspect_data = codec_inspect_data_impl;
  jpeg_codec.virt.create_decoder = codec_create_decoder_impl;
  jpeg_codec.virt.create_encoder = codec_create_encoder_impl;

  jpeg_codec.impl->ctor(&jpeg_codec.virt);
  jpeg_codec.impl->features =
    BL_IMAGE_CODEC_FEATURE_READ  |
    BL_IMAGE_CODEC_FEATURE_WRITE |
    BL_IMAGE_CODEC_FEATURE_LOSSY ;
  jpeg_codec.impl->name.dcast().assign("JPEG");
  jpeg_codec.impl->vendor.dcast().assign("Blend2D");
  jpeg_codec.impl->mime_type.dcast().assign("image/jpeg");
  bl::StringInternal::init_static(&jpeg_codec.impl->extensions, jpeg_extensions);

  jpeg_codec_instance._d.init_dynamic(BLObjectInfo::from_type_with_marker(BL_OBJECT_TYPE_IMAGE_CODEC), &jpeg_codec.impl);

  // Initialize JPEG decoder virtual functions.
  jpeg_decoder_virt.base.destroy = decoder_destroy_impl;
  jpeg_decoder_virt.base.get_property = bl_object_impl_get_property;
  jpeg_decoder_virt.base.set_property = bl_object_impl_set_property;
  jpeg_decoder_virt.restart = decoder_restart_impl;
  jpeg_decoder_virt.read_info = decoder_read_info_impl;
  jpeg_decoder_virt.read_frame = decoder_read_frame_impl;

  // Initialize JPEG encoder virtual functions.
  // TODO: [JPEG] Encoder

  codecs->append(jpeg_codec_instance.dcast());
}

} // {bl::Jpeg}
