// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_GLYPHBUFFER_P_H_INCLUDED
#define BLEND2D_GLYPHBUFFER_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/core/font.h>
#include <blend2d/core/glyphbuffer.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

enum BLGlyphBufferFlags : uint32_t {
  //! Glyph-buffer already contains glyph advances.
  BL_GLYPH_BUFFER_GLYPH_ADVANCES = 0x00000001u,
  //! Glyph-buffer has a calculated bounding box.
  BL_GLYPH_BUFFER_BOUNDING_BOX   = 0x00000002u
};

enum BLGlyphBufferEnums : uint32_t {
  //! Size of either GlyphIdData+GlyphItemData or PlacementData.
  BL_GLYPH_BUFFER_ANY_ITEM_SIZE = 16,

  BL_GLYPH_BUFFER_INITIAL_CAPACITY = 256,
  BL_GLYPH_BUFFER_AGGRESIVE_GROWTH = BL_ALLOC_GROW_LIMIT / BL_GLYPH_BUFFER_ANY_ITEM_SIZE,
};

struct BLGlyphBufferPrivateImpl : public BLGlyphBufferImpl {
  uint8_t* buffer[2];
  size_t capacity[2];

  BLDebugMessageSinkFunc debug_sink;
  void* debug_sink_user_data;

  // Default-constructed data should not be initialized.
  BL_INLINE constexpr BLGlyphBufferPrivateImpl() noexcept
    : BLGlyphBufferImpl {},
      buffer { nullptr, nullptr },
      capacity { 0, 0 },
      debug_sink(nullptr),
      debug_sink_user_data(nullptr) {}

  static BLGlyphBufferPrivateImpl* create() noexcept {
    BLGlyphBufferPrivateImpl* d = (BLGlyphBufferPrivateImpl*)malloc(sizeof(BLGlyphBufferPrivateImpl));
    if (BL_UNLIKELY(!d))
      return nullptr;

    d->content = nullptr;
    d->placement_data = nullptr;
    d->size = 0;
    d->glyph_run.reserved = uint8_t(0);
    d->glyph_run.placement_type = BL_GLYPH_PLACEMENT_TYPE_NONE;
    d->glyph_run.glyph_advance = int8_t(sizeof(uint32_t));
    d->glyph_run.placement_advance = int8_t(sizeof(BLGlyphPlacement));
    d->flags = 0;

    d->info_data = nullptr;
    d->buffer[0] = nullptr;
    d->buffer[1] = nullptr;
    d->capacity[0] = 0;
    d->capacity[1] = 0;
    d->debug_sink = nullptr;
    d->debug_sink_user_data = nullptr;

    return d;
  }

  BL_INLINE void destroy() noexcept {
    reset_buffers();
    free(this);
  }

  BL_INLINE void reset_buffers() noexcept {
    free(buffer[0]);
    free(buffer[1]);

    buffer[0] = nullptr;
    buffer[1] = nullptr;
  }

  BL_INLINE void clear() noexcept {
    size = 0;
    glyph_run.placement_type = BL_GLYPH_PLACEMENT_TYPE_NONE;
    glyph_run.flags = 0;
    placement_data = nullptr;
    get_glyph_data_ptrs(0, &content, &info_data);
  }

  BL_HIDDEN BLResult ensure_buffer(size_t buffer_id, size_t copy_size, size_t min_capacity) noexcept;

  BL_INLINE BLResult ensure_placement() noexcept {
    BL_PROPAGATE(ensure_buffer(1, 0, size));
    placement_data = reinterpret_cast<BLGlyphPlacement*>(buffer[1]);
    return BL_SUCCESS;
  }

  BL_INLINE void flip() noexcept {
    BLInternal::swap(buffer[0], buffer[1]);
    BLInternal::swap(capacity[0], capacity[1]);
  }

  BL_INLINE void get_glyph_data_ptrs(size_t buffer_id, uint32_t** glyph_data_out, BLGlyphInfo** info_data_out) noexcept {
    *glyph_data_out = reinterpret_cast<uint32_t*>(buffer[buffer_id]);
    *info_data_out = reinterpret_cast<BLGlyphInfo*>(buffer[buffer_id] + capacity[buffer_id] * sizeof(uint32_t));
  }
};

static BL_INLINE BLGlyphBufferPrivateImpl* bl_glyph_buffer_get_impl(const BLGlyphBufferCore* self) noexcept {
  return static_cast<BLGlyphBufferPrivateImpl*>(self->impl);
}

static BL_INLINE void bl_copy_glyph_data(uint32_t* glyph_dst, BLGlyphInfo* info_dst, const uint32_t* glyph_src, const BLGlyphInfo* info_src, size_t n) noexcept {
  for (size_t i = 0; i < n; i++) {
    glyph_dst[i] = glyph_src[i];
    info_dst[i] = info_src[i];
  }
}

//! \}
//! \endcond

#endif // BLEND2D_GLYPHBUFFER_P_H_INCLUDED
