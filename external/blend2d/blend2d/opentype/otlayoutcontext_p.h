// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_OPENTYPE_OTLAYOUTCONTEXT_P_H_INCLUDED
#define BLEND2D_OPENTYPE_OTLAYOUTCONTEXT_P_H_INCLUDED

#include <blend2d/opentype/otcore_p.h>
#include <blend2d/opentype/otlayout_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_opentype_impl
//! \{

namespace bl::OpenType {

//! A context used for OpenType glyph substitution (GSUB) processing.
struct GSubContext {
  enum class AllocMode {
    //! Allocated buffer replaces the current primary buffer (is flipped), the existing buffer would still be valid
    //! after the allocation. This mode is used by multiple substitution, which calculates the final buffer first;
    //! and then allocates the buffer and fills it with glyphs / infos.
    kCurrent = 0,

    //! Allocated buffer doesn't replace the current buffer. When multiple requests to a separate buffer are made
    //! within a single GSUB lookup, the content of the previous buffer is copied to the new one (this is to handle
    //! outputs where multiple substitution grows the buffer beyond the initial estimate).
    kSeparate = 1
  };

  struct WorkBuffer {
    BLGlyphId* glyph_data;
    BLGlyphInfo* info_data;
    size_t size;
    size_t capacity;
  };

  typedef BLResult (BL_CDECL* PrepareOutputBufferFunc)(GSubContext* self, size_t size) noexcept;

  //! \name Members
  //! \{

  WorkBuffer _work_buffer;
  DebugSink _debug_sink;
  PrepareOutputBufferFunc _prepare_output_buffer;

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE WorkBuffer& work_buffer() noexcept { return _work_buffer; }
  BL_INLINE const WorkBuffer& work_buffer() const noexcept { return _work_buffer; }

  BL_INLINE BLGlyphId* glyph_data() const noexcept { return _work_buffer.glyph_data; }
  BL_INLINE BLGlyphInfo* info_data() const noexcept { return _work_buffer.info_data; }

  BL_INLINE bool is_empty() const noexcept { return _work_buffer.size == 0u; }
  BL_INLINE size_t size() const noexcept { return _work_buffer.size; }
  BL_INLINE size_t capacity() const noexcept { return _work_buffer.capacity; }

  BL_INLINE void truncate(size_t new_size) noexcept {
    BL_ASSERT(new_size <= _work_buffer.size);
    _work_buffer.size = new_size;
  }

  BL_INLINE BLGlyphId* glyph_end_data() const noexcept { return _work_buffer.glyph_data + _work_buffer.size; }
  BL_INLINE BLGlyphInfo* info_end_data() const noexcept { return _work_buffer.info_data + _work_buffer.size; }

  //! \}

  //! \name Processing
  //! \{

  //! Used by a substitution that substitutes a single glyph or a sequence of glyphs with a longer sequence.
  BL_INLINE BLResult prepare_output_buffer(size_t size) noexcept {
    return _prepare_output_buffer(this, size);
  }

  //! Makes sure that there is at least `size` bytes in the work buffer.
  //!
  //! If the capacity of work buffer is sufficient, it's size is set to `size` and nothing else happens, however,
  //! if the capacity of work buffer is not sufficient, a new buffer is created and set as `_work_buffer`. The
  //! caller must remember the pointers to the previous buffer before the call as the buffers will be flipped.
  BL_INLINE BLResult ensure_work_buffer(size_t size) noexcept {
    if (size > _work_buffer.capacity)
      BL_PROPAGATE(prepare_output_buffer(size));

    _work_buffer.size = size;
    return BL_SUCCESS;
  }

  //! \}
};

//! A nested GSUB context that is used to process nested lookups.
//!
//! There can only be a single nested context to avoid recursion.
struct GSubContextNested : public GSubContext {
  enum : uint32_t {
    //! Maximum buffer size of a nested GSUB context.
    kNestedStorageSize = 64
  };

  //! Nested buffer having a fixed size.
  struct NestedBuffer {
    BLGlyphId glyph_data[kNestedStorageSize];
    BLGlyphInfo info_data[kNestedStorageSize];
  };

  //! \name Members
  //! \{

  //! Two nested buffers, restricted to `kNestedStorageSize`.
  NestedBuffer _nested_buffers[2];
  //! The id of the next nested buffers, for flipping - every time flip happens `_next_nested_buffer_id` is XORed by 1.
  uint32_t _next_nested_buffer_id;

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE void init(BLGlyphBufferPrivateImpl* gbd) noexcept {
    // Initialize GSubContext.
    _debug_sink.init(gbd->debug_sink, gbd->debug_sink_user_data);
    _prepare_output_buffer = prepare_output_buffer_impl;

    // Initialize GSubContextNested.
    _next_nested_buffer_id = 0;
  }

  BL_INLINE void init_nested(BLGlyphId* glyph_data, BLGlyphInfo* info_data, size_t size) noexcept {
    _work_buffer.glyph_data = glyph_data;
    _work_buffer.info_data = info_data;
    _work_buffer.size = size;
    _work_buffer.capacity = size;
  }

  //! \}

  //! \name Implementation
  //! \{

  static BLResult BL_CDECL prepare_output_buffer_impl(GSubContext* self_, size_t size) noexcept {
    GSubContextNested* self = static_cast<GSubContextNested*>(self_);
    if (size > kNestedStorageSize)
      return bl_make_error(BL_ERROR_GLYPH_SUBSTITUTION_TOO_LARGE);

    NestedBuffer& nested = self->_nested_buffers[self->_next_nested_buffer_id];
    self->_work_buffer.glyph_data = nested.glyph_data;
    self->_work_buffer.info_data = nested.info_data;
    self->_work_buffer.size = size;
    self->_work_buffer.capacity = kNestedStorageSize;
    self->_next_nested_buffer_id ^= 1u;

    return BL_SUCCESS;
  }

  //! \}
};

//! A primary GSUB context that is used to process top-level lookups.
struct GSubContextPrimary : public GSubContext {
  //! \name Members
  //! \{

  BLGlyphBufferPrivateImpl* _gbd;
  GSubContextNested _nested;

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE void init(BLGlyphBufferPrivateImpl* gbd) noexcept {
    // Initialize GSubContext.
    _debug_sink.init(gbd->debug_sink, gbd->debug_sink_user_data);
    _prepare_output_buffer = prepare_output_buffer_impl;

    _work_buffer.glyph_data = gbd->content;
    _work_buffer.info_data = gbd->info_data;
    _work_buffer.size = gbd->size;
    _work_buffer.capacity = gbd->capacity[0];

    // Initialize GSubContextPrimary.
    _gbd = gbd;
    _nested.init(gbd);
  }

  BL_INLINE void done() noexcept {
    _gbd->size = _work_buffer.size;
  }

  //! \}

  //! \name Implementation
  //! \{

  static BLResult BL_CDECL prepare_output_buffer_impl(GSubContext* self_, size_t size) noexcept {
    GSubContextPrimary* self = static_cast<GSubContextPrimary*>(self_);
    BLGlyphBufferPrivateImpl* gbd = self->_gbd;

    BL_PROPAGATE(gbd->ensure_buffer(1, 0, size));
    gbd->get_glyph_data_ptrs(1, &self->_work_buffer.glyph_data, &self->_work_buffer.info_data);
    self->_work_buffer.size = size;
    self->_work_buffer.capacity = gbd->capacity[1];
    gbd->flip();

    return BL_SUCCESS;
  }

  //! \}
};

//! A context used for OpenType glyph positioning.
struct GPosContext {
  struct WorkBuffer {
    BLGlyphId* glyph_data;
    BLGlyphInfo* info_data;
    BLGlyphPlacement* placement_data;
    size_t size;
  };

  WorkBuffer _work_buffer;
  DebugSink _debug_sink;
  BLGlyphBufferPrivateImpl* _gbd;

  BL_INLINE void init(BLGlyphBufferPrivateImpl* gbd) noexcept {
    _gbd = gbd;
    _debug_sink.init(gbd->debug_sink, gbd->debug_sink_user_data);

    _work_buffer.glyph_data = gbd->content;
    _work_buffer.info_data = gbd->info_data;
    _work_buffer.placement_data = gbd->placement_data;
    _work_buffer.size = gbd->size;
  }

  BL_INLINE void done() noexcept {}

  BL_INLINE WorkBuffer& work_buffer() noexcept { return _work_buffer; }
  BL_INLINE const WorkBuffer& work_buffer() const noexcept { return _work_buffer; }

  BL_INLINE BLGlyphId* glyph_data() const noexcept { return _work_buffer.glyph_data; }
  BL_INLINE BLGlyphInfo* info_data() const noexcept { return _work_buffer.info_data; }
  BL_INLINE BLGlyphPlacement* placement_data() const noexcept { return _work_buffer.placement_data; }

  BL_INLINE bool is_empty() const noexcept { return _work_buffer.size == 0u; }
  BL_INLINE size_t size() const noexcept { return _work_buffer.size; }
};

} // {bl::OpenType}

//! \}
//! \endcond

#endif // BLEND2D_OPENTYPE_OTLAYOUTCONTEXT_P_H_INCLUDED
