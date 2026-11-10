// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERJOB_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERJOB_P_H_INCLUDED

#include <blend2d/geometry/commons_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/renderbatch_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/statedata_p.h>
#include <blend2d/raster/styledata_p.h>
#include <blend2d/support/arenaallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

enum class RenderJobType : uint8_t {
  kNone = 0,

  kFillGeometry = 1,
  kFillText = 2,

  kStrokeGeometry = 3,
  kStrokeText = 4,

  kMaxValue = 4
};

enum class RenderJobFlags : uint8_t {
  kNoFlags = 0u,

  kComputePendingFetchData = 0x01u
};
BL_DEFINE_ENUM_FLAGS(RenderJobFlags)

//! Render job.
struct RenderJob {
  //! Type of the text data stored in `RenderJob_TextOp`.
  enum TextDataType : uint8_t {
    kTextDataRawUTF8 = BL_TEXT_ENCODING_UTF8,
    kTextDataRawUTF16 = BL_TEXT_ENCODING_UTF16,
    kTextDataRawUTF32 = BL_TEXT_ENCODING_UTF32,
    kTextDataRawLatin1 = BL_TEXT_ENCODING_LATIN1,
    kTextDataGlyphRun = 0xFE,
    kTextDataGlyphBuffer = 0xFF
  };


  //! \name Job Data
  //! \{

  RenderJobType _job_type;
  RenderJobFlags _job_flags;
  uint8_t _payload_type;
  uint8_t _meta_transform_fixed_type;
  uint8_t _final_transform_fixed_type;
  uint8_t _reserved;
  uint16_t _command_index;
  BLPoint _origin_fixed;
  RenderCommandQueue* _command_queue;

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE void _init_internal(RenderJobType job_type, RenderCommandQueue* command_queue, size_t command_index) noexcept {
    _job_type = job_type;
    _job_flags = RenderJobFlags::kNoFlags;
    _payload_type = 0;
    _meta_transform_fixed_type = 0;
    _final_transform_fixed_type = 0;
    _command_index = uint16_t(command_index);
    _command_queue = command_queue;
  }

  BL_INLINE_NODEBUG void set_origin_fixed(const BLPoint& pt) noexcept { _origin_fixed = pt; }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG RenderJobType job_type() const noexcept { return _job_type; }

  BL_INLINE_NODEBUG RenderJobFlags job_flags() const noexcept { return _job_flags; }
  BL_INLINE_NODEBUG bool has_job_flag(RenderJobFlags flag) const noexcept { return bl_test_flag(_job_flags, flag); }
  BL_INLINE_NODEBUG void add_job_flags(RenderJobFlags flags) noexcept { _job_flags |= flags; }

  BL_INLINE_NODEBUG RenderCommandQueue* command_queue() const noexcept { return _command_queue; }

  BL_INLINE_NODEBUG size_t command_index() const noexcept { return _command_index; }
  BL_INLINE_NODEBUG RenderCommand& command() const noexcept { return _command_queue->at(_command_index); }
  BL_INLINE_NODEBUG const BLPoint& origin_fixed() const noexcept { return _origin_fixed;}

  //! \}
};

//! Base class for fill and stroke operations responsible for holding shared states.
struct RenderJob_BaseOp : public RenderJob {
  const SharedFillState* _shared_fill_state;
  const SharedBaseStrokeState* _shared_stroke_state;

  BL_INLINE void init_states(const SharedFillState* shared_fill_state, const SharedBaseStrokeState* shared_stroke_state = nullptr) noexcept {
    _shared_fill_state = shared_fill_state;
    _shared_stroke_state = shared_stroke_state;
  }

  BL_INLINE_NODEBUG const SharedFillState* fill_state() const noexcept { return _shared_fill_state; }
  BL_INLINE_NODEBUG const SharedBaseStrokeState* stroke_state() const noexcept { return _shared_stroke_state; }

  BL_INLINE_NODEBUG BLTransformType meta_transform_fixed_type() const noexcept { return BLTransformType(_meta_transform_fixed_type); }
  BL_INLINE_NODEBUG BLTransformType final_transform_fixed_type() const noexcept { return BLTransformType(_final_transform_fixed_type); }

  BL_INLINE_NODEBUG void set_meta_transform_fixed_type(BLTransformType type) noexcept { _meta_transform_fixed_type = uint8_t(type); }
  BL_INLINE_NODEBUG void set_final_transform_fixed_type(BLTransformType type) noexcept { _final_transform_fixed_type = uint8_t(type); }
};

struct RenderJob_GeometryOp : public RenderJob_BaseOp {
  BL_INLINE void init_fill_job(RenderCommandQueue* command_queue, size_t command_index) noexcept {
    _init_internal(RenderJobType::kFillGeometry, command_queue, command_index);
  }

  BL_INLINE void init_stroke_job(RenderCommandQueue* command_queue, size_t command_index) noexcept {
    _init_internal(RenderJobType::kStrokeGeometry, command_queue, command_index);
  }

  BL_INLINE_NODEBUG BLGeometryType geometry_type() const noexcept { return (BLGeometryType)_payload_type; }

  BL_INLINE void set_geometry_with_path(const BLPathCore* path) noexcept {
    _payload_type = uint8_t(BL_GEOMETRY_TYPE_PATH);
    bl_object_private_init_weak_tagged(geometry_data<BLPathCore>(), path);
  }

  BL_INLINE void set_geometry_with_shape(BLGeometryType geometry_type, const void* src_data_ptr, size_t src_data_size) noexcept {
    _payload_type = uint8_t(geometry_type);
    memcpy(geometry_data<void>(), src_data_ptr, src_data_size);
  }

  BL_INLINE void set_geometry(BLGeometryType geometry_type, const void* src_data_ptr, size_t src_data_size) noexcept {
    switch (geometry_type) {
      case BL_GEOMETRY_TYPE_PATH:
        set_geometry_with_path(static_cast<const BLPathCore*>(src_data_ptr));
        break;

      default:
        set_geometry_with_shape(geometry_type, src_data_ptr, src_data_size);
        break;
    }
  }

  template<typename T>
  BL_INLINE_NODEBUG T* geometry_data() noexcept { return reinterpret_cast<T*>(this + 1); }

  template<typename T>
  BL_INLINE_NODEBUG const T* geometry_data() const noexcept { return reinterpret_cast<const T*>(this + 1); }
};

struct RenderJob_TextOp : public RenderJob_BaseOp {
  BLFontCore _font;

  union {
    BLArrayView<uint8_t> _text_data;
    BLGlyphRun _glyph_run;
    BLGlyphBufferCore _glyph_buffer;
  };

  BL_INLINE void init_fill_job(RenderCommandQueue* command_queue, size_t command_index) noexcept {
    _init_internal(RenderJobType::kFillText, command_queue, command_index);
  }

  BL_INLINE void init_stroke_job(RenderCommandQueue* command_queue, size_t command_index) noexcept {
    _init_internal(RenderJobType::kStrokeText, command_queue, command_index);
  }

  BL_INLINE void destroy() noexcept {
    _font.dcast().~BLFont();
    if (_payload_type == kTextDataGlyphBuffer)
      _glyph_buffer.dcast().~BLGlyphBuffer();
  }

  BL_INLINE void init_font(const BLFontCore& font) noexcept {
    bl_object_private_init_weak_tagged(&_font, &font);
  }

  BL_INLINE void init_text_data(const void* text, size_t size, BLTextEncoding encoding) noexcept {
    _payload_type = uint8_t(encoding);
    _text_data.reset(static_cast<const uint8_t*>(text), size);
  }

  BL_INLINE void init_glyph_run(void* glyph_data, void* placement_data, size_t size, uint32_t placement_type, uint32_t flags) noexcept {
    _payload_type = kTextDataGlyphRun;
    _glyph_run.glyph_data = glyph_data;
    _glyph_run.placement_data = placement_data;
    _glyph_run.size = size;
    _glyph_run.reserved = uint8_t(0);
    _glyph_run.placement_type = uint8_t(placement_type);
    _glyph_run.glyph_advance = uint8_t(4);
    _glyph_run.placement_advance = uint8_t(16);
    _glyph_run.flags = flags;
  }

  BL_INLINE void init_glyph_buffer(BLGlyphBufferImpl* gb_impl) noexcept {
    _payload_type = kTextDataGlyphBuffer;
    _glyph_buffer.impl = gb_impl;
  }

  BL_INLINE_NODEBUG uint32_t text_data_type() const noexcept { return _payload_type; }
  BL_INLINE_NODEBUG const void* text_data() const noexcept { return _text_data.data; }
  BL_INLINE_NODEBUG size_t text_size() const noexcept { return _text_data.size; }
  BL_INLINE_NODEBUG const BLGlyphBuffer& glyph_buffer() const noexcept { return _glyph_buffer.dcast(); }
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERJOB_P_H_INCLUDED
