// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/font_p.h>
#include <blend2d/core/matrix_p.h>
#include <blend2d/core/path_p.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/opentype/otface_p.h>
#include <blend2d/opentype/otglyf_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/lookuptable_p.h>
#include <blend2d/support/memops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/scopedbuffer_p.h>

namespace bl::OpenType {
namespace GlyfImpl {

// bl::OpenType::GlyfImpl - FlagToSizeTable
// =========================================

// This table provides information about the number of bytes vertex data consumes per each flag. It's used to
// calculate the size of X and Y arrays of all contours a simple glyph is composed of to speed up decoding.
struct FlagToSizeGen {
  static constexpr uint32_t value(size_t i) noexcept {
    return ((uint32_t(i & (GlyfTable::Simple::kXIsByte >> 1)) ? 1u : (i & (GlyfTable::Simple::kXIsSameOrXByteIsPositive >> 1)) ? 0u : 2u) <<  0) |
           ((uint32_t(i & (GlyfTable::Simple::kYIsByte >> 1)) ? 1u : (i & (GlyfTable::Simple::kYIsSameOrYByteIsPositive >> 1)) ? 0u : 2u) << 16) ;
  }
};

static constexpr const auto vertex_size_table_ = make_lookup_table<uint32_t, ((GlyfTable::Simple::kImportantFlagsMask + 1) >> 1), FlagToSizeGen>();

const LookupTable<uint32_t, ((GlyfTable::Simple::kImportantFlagsMask + 1) >> 1)> vertex_size_table = vertex_size_table_;

// bl::OpenType::GlyfImpl - GetGlyphBounds
// =======================================

static const uint8_t bl_blank_glyph_data[sizeof(GlyfTable::GlyphData)] = { 0 };

static BLResult BL_CDECL get_glyph_bounds(
  const BLFontFaceImpl* face_impl,
  const uint32_t* glyph_data,
  intptr_t glyph_advance,
  BLBoxI* boxes,
  size_t count) noexcept {

  BLResult result = BL_SUCCESS;

  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);
  RawTable glyf_table = ot_face_impl->glyf.glyf_table;
  RawTable loca_table = ot_face_impl->glyf.loca_table;
  uint32_t loca_offset_size = ot_face_impl->loca_offset_size();

  const uint8_t* blank_glyph_data = bl_blank_glyph_data;

  for (size_t i = 0; i < count; i++) {
    BLGlyphId glyph_id = glyph_data[0] & 0xFFFFu;
    glyph_data = PtrOps::offset(glyph_data, glyph_advance);

    size_t offset;
    size_t end_off;

    // NOTE: Maximum glyph_id is 65535, so we are always safe here regarding multiplying the `glyph_id` by 2 or 4
    // to calculate the correct index.
    if (loca_offset_size == 2) {
      size_t index = size_t(glyph_id) * 2u;
      if (BL_UNLIKELY(index + sizeof(UInt16) * 2 > loca_table.size))
        goto InvalidData;

      offset = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 0)->value()) * 2u;
      end_off = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 2)->value()) * 2u;
    }
    else {
      size_t index = size_t(glyph_id) * 4u;
      if (BL_UNLIKELY(index + sizeof(UInt32) * 2 > loca_table.size))
        goto InvalidData;

      offset = reinterpret_cast<const UInt32*>(loca_table.data + index + 0)->value();
      end_off = reinterpret_cast<const UInt32*>(loca_table.data + index + 4)->value();
    }

    if (BL_LIKELY(end_off <= glyf_table.size)) {
      const uint8_t* gPtr = blank_glyph_data;
      if (offset < end_off) {
        gPtr = glyf_table.data + offset;
        size_t remaining_size = end_off - offset;

        if (BL_UNLIKELY(remaining_size < sizeof(GlyfTable::GlyphData)))
          goto InvalidData;
      }

      int x_min = reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->x_min();
      int x_max = reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->x_max();

      // Y coordinates in fonts are bottom to top, we convert them to top-to-bottom.
      int y_min = -reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->y_max();
      int y_max = -reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->y_min();

      boxes[i].reset(x_min, y_min, x_max, y_max);
      continue;
    }

    // Invalid data or the glyph is not defined. In either case we just zero the box.
InvalidData:
    boxes[i].reset();
    result = BL_ERROR_INVALID_DATA;
  }

  return result;
}

// bl::OpenType::GlyfImpl - GetGlyphOutlines
// =========================================

namespace {

class GlyfVertexDecoder {
public:
  const uint8_t* _xCoordPtr;
  const uint8_t* _yCoordPtr;
  const uint8_t* _end_ptr;

  double _m00;
  double _m01;
  double _m10;
  double _m11;

  BL_INLINE GlyfVertexDecoder(const uint8_t* xCoordPtr, const uint8_t* yCoordPtr, const uint8_t* end_ptr, const BLMatrix2D& transform) noexcept
    : _xCoordPtr(xCoordPtr),
      _yCoordPtr(yCoordPtr),
      _end_ptr(end_ptr),
      _m00(transform.m00),
      _m01(transform.m01),
      _m10(transform.m10),
      _m11(transform.m11) {}

  BL_INLINE BLPoint decode_next(uint32_t flags) noexcept {
    int x16 = 0;
    int y16 = 0;

    if (flags & GlyfTable::Simple::kXIsByte) {
      BL_ASSERT(_xCoordPtr <= _end_ptr - 1);
      x16 = int(_xCoordPtr[0]);
      if (!(flags & GlyfTable::Simple::kXIsSameOrXByteIsPositive))
        x16 = -x16;
      _xCoordPtr += 1;
    }
    else if (!(flags & GlyfTable::Simple::kXIsSameOrXByteIsPositive)) {
      BL_ASSERT(_xCoordPtr <= _end_ptr - 2);
      x16 = MemOps::readI16uBE(_xCoordPtr);
      _xCoordPtr += 2;
    }

    if (flags & GlyfTable::Simple::kYIsByte) {
      BL_ASSERT(_yCoordPtr <= _end_ptr - 1);
      y16 = int(_yCoordPtr[0]);
      if (!(flags & GlyfTable::Simple::kYIsSameOrYByteIsPositive))
        y16 = -y16;
      _yCoordPtr += 1;
    }
    else if (!(flags & GlyfTable::Simple::kYIsSameOrYByteIsPositive)) {
      BL_ASSERT(_yCoordPtr <= _end_ptr - 2);
      y16 = MemOps::readI16uBE(_yCoordPtr);
      _yCoordPtr += 2;
    }

    return BLPoint(double(x16) * _m00 + double(y16) * _m10, double(x16) * _m01 + double(y16) * _m11);
  }
};

} // {anonymous}

static BLResult BL_CDECL get_glyph_outlines(
  const BLFontFaceImpl* face_impl,
  BLGlyphId glyph_id,
  const BLMatrix2D* transform,
  BLPath* out,
  size_t* contour_count_out,
  ScopedBuffer* tmp_buffer) noexcept {

  const OTFaceImpl* ot_face_impl = static_cast<const OTFaceImpl*>(face_impl);

  typedef GlyfTable::Simple Simple;
  typedef GlyfTable::Compound Compound;

  if (BL_UNLIKELY(glyph_id >= ot_face_impl->face_info.glyph_count))
    return bl_make_error(BL_ERROR_INVALID_GLYPH);

  RawTable glyf_table = ot_face_impl->glyf.glyf_table;
  RawTable loca_table = ot_face_impl->glyf.loca_table;
  uint32_t loca_offset_size = ot_face_impl->loca_offset_size();

  const uint8_t* gPtr = nullptr;
  size_t remaining_size = 0;
  size_t compound_level = 0;

  // Only matrix and compound_flags are important in the root entry.
  CompoundEntry compound_data[CompoundEntry::kMaxLevel];
  compound_data[0].gPtr = nullptr;
  compound_data[0].remaining_size = 0;
  compound_data[0].compound_flags = Compound::kArgsAreXYValues;
  compound_data[0].transform = *transform;

  PathAppender appender;
  size_t contour_count_total = 0;

  for (;;) {
    size_t offset;
    size_t end_off;

    // NOTE: Maximum glyph_id is 65535, so we are always safe here regarding multiplying the `glyph_id` by 2 or 4
    // to calculate the correct index.
    if (loca_offset_size == 2) {
      size_t index = size_t(glyph_id) * 2u;
      if (BL_UNLIKELY(index + sizeof(UInt16) * 2u > loca_table.size))
        goto InvalidData;
      offset = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 0)->value()) * 2u;
      end_off = uint32_t(reinterpret_cast<const UInt16*>(loca_table.data + index + 2)->value()) * 2u;
    }
    else {
      size_t index = size_t(glyph_id) * 4u;
      if (BL_UNLIKELY(index + sizeof(UInt32) * 2u > loca_table.size))
        goto InvalidData;
      offset = reinterpret_cast<const UInt32*>(loca_table.data + index + 0)->value();
      end_off = reinterpret_cast<const UInt32*>(loca_table.data + index + 4)->value();
    }

    // Simple or Empty Glyph
    // ---------------------

    if (BL_UNLIKELY(offset >= end_off || end_off > glyf_table.size)) {
      // Only ALLOWED when `offset == end_off`.
      if (BL_UNLIKELY(offset != end_off || end_off > glyf_table.size))
        goto InvalidData;
    }
    else {
      gPtr = glyf_table.data + offset;
      remaining_size = end_off - offset;

      if (BL_UNLIKELY(remaining_size < sizeof(GlyfTable::GlyphData)))
        goto InvalidData;

      int contour_count_signed = reinterpret_cast<const GlyfTable::GlyphData*>(gPtr)->number_of_contours();
      if (contour_count_signed > 0) {
        size_t contour_count = size_t(unsigned(contour_count_signed));
        OverflowFlag of{};

        // Minimum data size is:
        //   10                     [GlyphData header]
        //   (number_of_contours * 2) [end_pts_of_contours]
        //   2                      [instruction_length]
        gPtr += sizeof(GlyfTable::GlyphData);
        remaining_size = IntOps::sub_overflow(remaining_size, sizeof(GlyfTable::GlyphData) + contour_count * 2u + 2u, &of);
        if (BL_UNLIKELY(of))
          goto InvalidData;

        const UInt16* contour_array = reinterpret_cast<const UInt16*>(gPtr);
        gPtr += contour_count * 2u;
        contour_count_total += contour_count;

        // We don't use hinting instructions, so skip them.
        size_t instruction_count = MemOps::readU16uBE(gPtr);
        remaining_size = IntOps::sub_overflow(remaining_size, instruction_count, &of);
        if (BL_UNLIKELY(of))
          goto InvalidData;

        gPtr += 2u + instruction_count;
        const uint8_t* gEnd = gPtr + remaining_size;

        // Number of vertices in TrueType sense (could be less than a number of points required by BLPath
        // representation, especially if TT outline contains consecutive off-curve points).
        size_t tt_vertex_count = size_t(contour_array[contour_count - 1].value()) + 1u;

        // Only try to decode vertices if there is more than 1.
        if (tt_vertex_count > 1u) {
          // Read TrueType Flags Data
          // ------------------------

          uint8_t* fDataPtr = static_cast<uint8_t*>(tmp_buffer->alloc(tt_vertex_count));
          if (BL_UNLIKELY(!fDataPtr))
            return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

          // Sizes of x_coordinates[] and y_coordinates[] arrays in TrueType data.
          size_t xCoordinatesSize;
          size_t yCoordinatesSize;

          // Number of consecutive off curve vertices making a spline. We need this number to be able to calculate the
          // number of BLPath vertices we will need to convert this glyph into BLPath data.
          size_t off_curve_spline_count = 0;

          constexpr uint32_t kOffCurveSplineMask = Simple::kOnCurvePoint | (Simple::kOnCurvePoint << 7);

          {
            // Number of bytes required by both X and Y coordinates as a packed `(Y << 16) | X` value.
            //
            // NOTE: We know that the maximum number of contours of a single glyph is 32767, thus we can store both the
            // number of X and Y vertices in a single unsigned 32-bit integer, as each vertex has 2 bytes maximum, which
            // would be 65534.
            size_t xy_coordinates_size = 0;

            // We parse flags one-by-one and calculate the size required by vertices by using our FLAG tables so we don't
            // have to do bounds checking during vertex decoding.
            size_t i = 0;
            uint32_t f = Simple::kOnCurvePoint;

            do {
              if (BL_UNLIKELY(gPtr == gEnd))
                goto InvalidData;

              uint32_t tt_flag = *gPtr++ & Simple::kImportantFlagsMask;
              uint32_t vertex_size = vertex_size_table[tt_flag >> 1];

              f = ((f << 7) | tt_flag) & 0xFFu;
              fDataPtr[i++] = uint8_t(f);

              xy_coordinates_size += vertex_size;
              off_curve_spline_count += uint32_t((f & kOffCurveSplineMask) == 0);

              // Most of flags are not repeated. Some contours have no repeated flags at all, so make this likely.
              if (BL_LIKELY(!(f & Simple::kRepeatFlag)))
                continue;

              if (BL_UNLIKELY(gPtr == gEnd))
                goto InvalidData;

              // When `kRepeatFlag` is set it means that the next byte contains how many times it should repeat
              // (the specification doesn't mention zero length, so we won't fail and just silently consume the byte).
              size_t n = *gPtr++;
              if (BL_UNLIKELY(n > tt_vertex_count - i))
                goto InvalidData;

              f = ((f << 7) | tt_flag) & 0xFFu;

              xy_coordinates_size += uint32_t(n) * vertex_size;
              off_curve_spline_count += n * size_t((f & Simple::kOnCurvePoint) == 0);

              MemOps::fill_small(fDataPtr + i, uint8_t(f), n);
              i += n;
            } while (i < tt_vertex_count);

            xCoordinatesSize = xy_coordinates_size & 0xFFFFu;
            yCoordinatesSize = xy_coordinates_size >> 16;
          }

          remaining_size = PtrOps::bytes_until(gPtr, gEnd);
          if (BL_UNLIKELY(xCoordinatesSize + yCoordinatesSize > remaining_size))
            goto InvalidData;

          // Read TrueType Vertex Data
          // -------------------------

          // Vertex data in `glyf` table doesn't map 1:1 to how BLPath stores its data. Multiple off-point curves in
          // TrueType data are decomposed into a quad spline, which is one vertex larger (BLPath doesn't offer multiple
          // off-point quads). This means that the number of vertices required by BLPath can be greater than the number
          // of vertices stored in TrueType 'glyf' data. However, we should know exactly how many vertices we have to
          // add to `tt_vertex_count` as we calculated `off_curve_spline_count` during flags decoding.
          //
          // The number of resulting vertices is thus:
          //   - `tt_vertex_count` - base number of vertices stored in TrueType data.
          //   - `off_curve_spline_count` - the number of additional vertices we will need to add for each off-curve spline
          //     used in TrueType data.
          //   - `contour_count` - Number of contours, we multiply this by 3 as we want to include one 'MoveTo', 'Close',
          //     and one additional off-curve spline point per each contour in case it starts - ends with an off-curve
          //     point.
          size_t max_vertex_count = tt_vertex_count + off_curve_spline_count + contour_count * 3;

          // Increase max_vertex_count if the path was not allocated yet - this avoids a possible realloc of compound glyphs.
          if (out->capacity() == 0 && compound_level > 0)
            max_vertex_count += 128;

          BL_PROPAGATE(appender.begin_append(out, max_vertex_count));

          // Since we know exactly how many bytes both vertex arrays consume we can decode both X and Y coordinates at
          // the same time. This gives us also the opportunity to start appending to BLPath immediately.
          GlyfVertexDecoder vertex_decoder(gPtr, gPtr + xCoordinatesSize, gEnd, compound_data[compound_level].transform);

          // Vertices are stored relative to each other, this is the current point.
          BLPoint current_pt(compound_data[compound_level].transform.m20, compound_data[compound_level].transform.m21);

          // Current vertex index in TT sense, advanced until `tt_vertex_count`, which must be end index of the last contour.
          size_t i = 0;

          for (size_t contour_index = 0; contour_index < contour_count; contour_index++) {
            size_t iEnd = size_t(contour_array[contour_index].value()) + 1;
            if (BL_UNLIKELY(iEnd <= i || iEnd > tt_vertex_count))
              goto InvalidData;

            // We need to be able to handle a case in which the contour data starts off-curve.
            size_t off_curve_start = SIZE_MAX;

            // We do the first vertex here as we want to emit 'MoveTo' and we want to remember it for a possible
            // off-curve start.
            uint32_t f = fDataPtr[i];
            current_pt += vertex_decoder.decode_next(f);

            if (f & Simple::kOnCurvePoint)
              appender.move_to(current_pt);
            else
              off_curve_start = appender.current_index(*out);

            if (++i >= iEnd)
              continue;

            // Initial 'MoveTo' coordinates.
            BLPoint initial_pt = current_pt;

            for (;;) {
              f = fDataPtr[i];

              BLPoint delta = vertex_decoder.decode_next(f);
              current_pt += delta;

              if ((f & kOffCurveSplineMask) != 0) {
                BL_STATIC_ASSERT(BL_PATH_CMD_QUAD - 1 == BL_PATH_CMD_ON);
                uint8_t cmd = uint8_t(BL_PATH_CMD_QUAD - (f & Simple::kOnCurvePoint));
                appender.add_vertex(cmd, current_pt);
              }
              else {
                BLPoint on_pt = current_pt - delta * 0.5;
                appender.add_vertex(BL_PATH_CMD_ON, on_pt);
                appender.add_vertex(BL_PATH_CMD_QUAD, current_pt);
              }

              if (++i >= iEnd)
                break;
            }

            if (off_curve_start != SIZE_MAX) {
              BLPathImpl* out_impl = PathInternal::get_impl(out);
              BLPoint final_pt = out_impl->vertex_data[off_curve_start];

              out_impl->command_data[off_curve_start] = BL_PATH_CMD_MOVE;

              if (!(f & Simple::kOnCurvePoint)) {
                BLPoint on_pt = (current_pt + initial_pt) * 0.5;
                appender.add_vertex(BL_PATH_CMD_ON, on_pt);
                final_pt = (initial_pt + final_pt) * 0.5;
              }

              appender.add_vertex(BL_PATH_CMD_QUAD, initial_pt);
              appender.add_vertex(BL_PATH_CMD_ON, final_pt);
            }
            else if (!(f & Simple::kOnCurvePoint)) {
              appender.add_vertex(BL_PATH_CMD_ON, initial_pt);
            }

            appender.close();
          }
          appender.done(out);
        }
      }
      else if (contour_count_signed == -1) {
        gPtr += sizeof(GlyfTable::GlyphData);
        remaining_size -= sizeof(GlyfTable::GlyphData);

        if (BL_UNLIKELY(++compound_level >= CompoundEntry::kMaxLevel))
          goto InvalidData;

        goto ContinueCompound;
      }
      else {
        // Cannot be less than -1, only -1 specifies compound glyph, lesser value is invalid according to the
        // specification.
        if (BL_UNLIKELY(contour_count_signed < -1))
          goto InvalidData;

        // Otherwise the glyph has no contours.
      }
    }

    // Compound Glyph
    // --------------

    if (compound_level) {
      while (!(compound_data[compound_level].compound_flags & Compound::kMoreComponents))
        if (--compound_level == 0)
          break;

      if (compound_level) {
        gPtr = compound_data[compound_level].gPtr;
        remaining_size = compound_data[compound_level].remaining_size;

        // The structure that we are going to read is as follows:
        //
        //   [Header]
        //     uint16_t flags;
        //     uint16_t glyph_id;
        //
        //   [Translation]
        //     a) int8_t arg1/arg2;
        //     b) int16_t arg1/arg2;
        //
        //   [Scale/Affine]
        //     a) <None>
        //     b) int16_t scale;
        //     c) int16_t scale_x, scale_y;
        //     d) int16_t m00, m01, m10, m11;

ContinueCompound:
        {
          uint32_t flags;
          int arg1, arg2;
          OverflowFlag of{};

          remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 6, &of);
          if (BL_UNLIKELY(of))
            goto InvalidData;

          flags = MemOps::readU16uBE(gPtr);
          glyph_id = MemOps::readU16uBE(gPtr + 2);
          if (BL_UNLIKELY(glyph_id >= ot_face_impl->face_info.glyph_count))
            goto InvalidData;

          arg1 = MemOps::readI8(gPtr + 4);
          arg2 = MemOps::readI8(gPtr + 5);
          gPtr += 6;

          if (flags & Compound::kArgsAreWords) {
            remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 2, &of);
            if (BL_UNLIKELY(of))
              goto InvalidData;

            arg1 = IntOps::shl(arg1, 8) | (arg2 & 0xFF);
            arg2 = MemOps::readI16uBE(gPtr);
            gPtr += 2;
          }

          if (!(flags & Compound::kArgsAreXYValues)) {
            // This makes them unsigned.
            arg1 &= 0xFFFFu;
            arg2 &= 0xFFFFu;

            // TODO: [OpenType] GLYF ArgsAreXYValues not implemented. I don't know how atm.
          }

          constexpr double kScaleF2x14 = 1.0 / 16384.0;

          BLMatrix2D& cm = compound_data[compound_level].transform;
          cm.reset(1.0, 0.0, 0.0, 1.0, double(arg1), double(arg2));

          if (flags & Compound::kAnyCompoundScale) {
            if (flags & Compound::kWeHaveScale) {
              // Simple scaling:
              //   [Sc, 0]
              //   [0, Sc]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 2, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              double scale = double(MemOps::readI16uBE(gPtr)) * kScaleF2x14;
              cm.m00 = scale;
              cm.m11 = scale;
              gPtr += 2;
            }
            else if (flags & Compound::kWeHaveScaleXY) {
              // Simple scaling:
              //   [Sx, 0]
              //   [0, Sy]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 4, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              cm.m00 = double(MemOps::readI16uBE(gPtr + 0)) * kScaleF2x14;
              cm.m11 = double(MemOps::readI16uBE(gPtr + 2)) * kScaleF2x14;
              gPtr += 4;
            }
            else {
              // Affine case:
              //   [A, B]
              //   [C, D]
              remaining_size = IntOps::sub_overflow<size_t>(remaining_size, 8, &of);
              if (BL_UNLIKELY(of))
                goto InvalidData;

              cm.m00 = double(MemOps::readI16uBE(gPtr + 0)) * kScaleF2x14;
              cm.m01 = double(MemOps::readI16uBE(gPtr + 2)) * kScaleF2x14;
              cm.m10 = double(MemOps::readI16uBE(gPtr + 4)) * kScaleF2x14;
              cm.m11 = double(MemOps::readI16uBE(gPtr + 6)) * kScaleF2x14;
              gPtr += 8;
            }

            // Translation scale should only happen when `kArgsAreXYValues` is set. The default behavior according to
            // the specification is `kUnscaledComponentOffset`, which can be overridden by `kScaledComponentOffset`.
            // However, if both or neither are set then the behavior is the same as `kUnscaledComponentOffset`.
            if ((flags & (Compound::kArgsAreXYValues | Compound::kAnyCompoundOffset    )) ==
                         (Compound::kArgsAreXYValues | Compound::kScaledComponentOffset)) {
              // This is what FreeType does and what's not 100% according to the specification. However, according to
              // FreeType this would produce much better offsets so we will match FreeType instead of following the
              // specification.
              cm.m20 *= Geometry::magnitude(BLPoint(cm.m00, cm.m01));
              cm.m21 *= Geometry::magnitude(BLPoint(cm.m10, cm.m11));
            }
          }

          compound_data[compound_level].gPtr = gPtr;
          compound_data[compound_level].remaining_size = remaining_size;
          compound_data[compound_level].compound_flags = flags;
          TransformInternal::multiply(cm, cm, compound_data[compound_level - 1].transform);
          continue;
        }
      }
    }

    break;
  }

  *contour_count_out = contour_count_total;
  return BL_SUCCESS;

InvalidData:
  *contour_count_out = 0;
  return bl_make_error(BL_ERROR_INVALID_DATA);
}

// bl::OpenType::GlyfImpl - Init
// =============================

BLResult init(OTFaceImpl* ot_face_impl, OTFaceTables& tables) noexcept {
  ot_face_impl->face_info.outline_type = BL_FONT_OUTLINE_TYPE_TRUETYPE;
  ot_face_impl->glyf.glyf_table = tables.glyf;
  ot_face_impl->glyf.loca_table = tables.loca;
  ot_face_impl->funcs.get_glyph_bounds = get_glyph_bounds;

  // Don't reference any function that won't be used when certain optimizations are enabled across the whole binary.
#if defined(BL_TARGET_OPT_AVX2)
  ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_avx2;
#elif defined(BL_TARGET_OPT_SSE4_2)
  ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_sse4_2;
#elif BL_TARGET_ARCH_ARM >= 64 && defined(BL_TARGET_OPT_ASIMD)
  ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_asimd;
#else
#if defined(BL_BUILD_OPT_AVX2)
  if (bl_runtime_has_avx2(&bl_runtime_context))
  {
    ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_avx2;
  }
  else
#endif
#if defined(BL_BUILD_OPT_SSE4_2)
  if (bl_runtime_has_sse4_2(&bl_runtime_context))
  {
    ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_sse4_2;
  }
  else
#endif
#if BL_TARGET_ARCH_ARM >= 64 && defined(BL_BUILD_OPT_ASIMD)
  if (bl_runtime_has_asimd(&bl_runtime_context))
  {
    ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines_asimd;
  }
  else
#endif
  {
    ot_face_impl->funcs.get_glyph_outlines = get_glyph_outlines;
  }
#endif

  return BL_SUCCESS;
}

} // {GlyfImpl}
} // {bl::OpenType}
