// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERCOMMAND_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERCOMMAND_P_H_INCLUDED

#include <blend2d/geometry/commons_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/renderfetchdata_p.h>
#include <blend2d/raster/styledata_p.h>
#include <blend2d/support/arenaallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

struct RenderFetchData;

//! Source data that belongs to a \ref RenderCommand, but stored separately.
union RenderCommandSource {
  //! Solid data.
  Pipeline::FetchData::Solid solid;
  //! Fetch data.
  RenderFetchData* fetch_data;

  //! Reset all data to zero.
  BL_INLINE_NODEBUG void reset() noexcept { solid.prgb64 = 0; }
  //! Copy all data from `other` to this command source.
  BL_INLINE_NODEBUG void reset(const RenderCommandSource& other) noexcept { *this = other; }
};

//! Render command type.
enum class RenderCommandType : uint8_t {
  kNone = 0,
  kFillBoxA = 1,
  kFillBoxU = 2,
  kFillAnalytic = 3,
  kFillBoxMaskA = 4
};

//! Raster command flags.
enum class RenderCommandFlags : uint8_t {
  //! No flags specified.
  kNoFlags = 0x00u,

  //! The command holds `_source.fetch_data` (the operation is non-solid, fetch-data is valid and used).
  kHasStyleFetchData = 0x10u,

  //! The command retains `_source.fetch_data`, which must be released during batch finalization.
  kRetainsStyleFetchData = 0x20u,

  //! The command retains `_payload.mask_raw.mask_image_i`, which must be released during batch finalization.
  //!
  //! \note This flag cannot be set together with \ref kRetainsMaskImageData, one or the other.
  kRetainsMaskFetchData = 0x40u,

  //! The command retains `_payload.mask_fetch_data`, which must be released during batch finalization
  //!
  //! \note This flag cannot be set together with \ref kRetainsMaskFetchData, one or the other.
  kRetainsMaskImageData = 0x80u
};

BL_DEFINE_ENUM_FLAGS(RenderCommandFlags)

//! 64-bit pointer to unify the layout of the render command.
//!
//! The reason is that a command has a fixed size calculated to be good for 8-byte pointers (64-bit machines).
template<typename T>
struct Ptr64 {
  T* ptr;
#if BL_TARGET_ARCH_BITS < 64
  uint32_t padding;
#endif
};

//! Render command.
//!
//! Render command provides information required to render the lowest-level operation.
struct RenderCommand {
  //! \name Constants
  //! \{

  //! Maximum size of the payload embedded in the \ref RenderCommand itself.
  enum PayloadDataSize : uint32_t { kPayloadDataSize = 32 };

  //! \}

  //! \name Payload
  //! \{

  //! FillBoxA, FillBoxU, FillMaskBoxA, FillMaskBoxU payload.
  struct FillBox {
    Ptr64<RenderFetchData> mask_fetch_data;
    uint8_t reserved[8];
    BLBoxI box_i;
  };

  //! FillBoxWithMaskA payload - special case for aligned fills with aligned mask.
  //!
  //! This payload was designed to save space in command buffer as it avoids RenderFetchData.
  struct FillBoxMaskA {
    Ptr64<BLImageImpl> mask_image_i;
    BLPointI mask_offset_i;
    BLBoxI box_i;
  };

  //! FillAnalytic and FillMaskAnalytic payload, used by the asynchronous rendering context implementation.
  struct FillAnalytic {
    //! Fetch data used by mask `kTypeFillMaskAnalytic` command types.
    Ptr64<RenderFetchData> mask_fetch_data;
    //! Points to the start of the first edge. Edges that start in next bands are linked next after edges of the previous
    //! band, which makes it possible to only store the start of the list.
    Ptr64<const EdgeVector<int>> edges;
    //! Fill rule.
    uint32_t fill_rule;
    //! Topmost Y coordinate used to skip quickly the whole band if the worker is not there yet.
    int fixed_y0;
    //! Index of state slot that is used by to keep track of the command progress. The index refers to a table where
    //! a command-specific state data is stored.
    uint32_t state_slot_index;
  };

  //! Command payload - each command type has a specific payload.
  union Payload {
    //! Payload used by FillBoxA, FillBoxU, FillMaskA, FillMaskU.
    FillBox box;
    //! Payload used by FillBoxAMaskA.
    FillBoxMaskA box_mask_a;
    //! Payload used by FillAnalytic in case of asynchronous rendering.
    FillAnalytic analytic;

    //! Mask fetch-data, which is provided by the most commands.
    Ptr64<RenderFetchData> mask_fetch_data;

    //! Payload buffer (holds RAW data).
    uint8_t buffer[kPayloadDataSize];
  };

  BL_STATIC_ASSERT(sizeof(Payload) == kPayloadDataSize);
  BL_STATIC_ASSERT(sizeof(FillBox) == kPayloadDataSize);
  BL_STATIC_ASSERT(sizeof(FillBoxMaskA) <= kPayloadDataSize);
  BL_STATIC_ASSERT(sizeof(FillAnalytic) <= kPayloadDataSize);

  //! \}

  //! \name Members
  //! \{

  //! Command payload.
  Payload _payload;

  //! Global alpha value.
  uint32_t _alpha;
  //! Command type.
  RenderCommandType _type;
  //! Command flags.
  RenderCommandFlags _flags;
  //! Reserved.
  uint16_t _reserved;

  RenderCommandSource _source;

  union {
    //! Dispatch data.
    Pipeline::DispatchData _dispatch_data;
    //! Signature, used during command construction, replaced by `_dispatch_data` when constructed.
    Pipeline::Signature _signature;
  };

  //! \}

  //! \name Command Core Initialization
  //! \{

  BL_INLINE void init_command(uint32_t alpha) noexcept {
    _alpha = alpha;
    _type = RenderCommandType::kNone;
    _flags = RenderCommandFlags::kNoFlags;
    _reserved = 0;
  }

  BL_INLINE void init_fill_box_a(const BLBoxI& box_a) noexcept {
    _payload.box.box_i = box_a;
    _type = RenderCommandType::kFillBoxA;
  }

  BL_INLINE void init_fill_box_u(const BLBoxI& box_u) noexcept {
    _payload.box.box_i = box_u;
    _type = RenderCommandType::kFillBoxU;
  }

  //! Initializes FillAnalytic command.
  //!
  //! \note `edges` may be null in case that this command requires a job to build the edges. In this case both `edges`
  //! and `fixed_y0` members will be changed when such job completes.
  BL_INLINE void init_fill_analytic(EdgeVector<int>* edges, int fixed_y0, BLFillRule fill_rule) noexcept {
    BL_ASSERT(fill_rule <= BL_FILL_RULE_MAX_VALUE);

    _payload.analytic.edges.ptr = edges;
    _payload.analytic.fixed_y0 = fixed_y0;
    _payload.analytic.fill_rule = uint32_t(fill_rule);
    _type = RenderCommandType::kFillAnalytic;
  }

  BL_INLINE void init_fill_box_mask_a(const BLBoxI& box_a, const BLImageCore* mask_image, const BLPointI& mask_offset_i) noexcept {
    _payload.box_mask_a.mask_image_i.ptr = ImageInternal::get_impl(mask_image);
    _payload.box_mask_a.mask_offset_i = mask_offset_i;
    _payload.box_mask_a.box_i = box_a;
    _type = RenderCommandType::kFillBoxMaskA;
  }

  //! Sets edges of FillAnalytic or FillMaskAnalytic command.
  BL_INLINE void set_analytic_edges(EdgeStorage<int>* edge_storage) noexcept {
    _payload.analytic.edges.ptr = edge_storage->flatten_edge_links();
    _payload.analytic.fixed_y0 = edge_storage->bounding_box().y0;
  }

  BL_INLINE void mark_fetch_data() noexcept { add_flags(RenderCommandFlags::kHasStyleFetchData); }

  //! \}

  //! \name Command Source and Mask Initialization
  //! \{

  BL_INLINE void init_mask_fetch_data(RenderFetchData* mask_fetch_data) noexcept {
    _payload.mask_fetch_data.ptr = mask_fetch_data;
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG RenderCommandType type() const noexcept { return RenderCommandType(_type); }

  BL_INLINE_NODEBUG bool is_fill_box_a() const noexcept { return _type == RenderCommandType::kFillBoxA; }
  BL_INLINE_NODEBUG bool is_fill_box_u() const noexcept { return _type == RenderCommandType::kFillBoxU; }
  BL_INLINE_NODEBUG bool is_fill_analytic() const noexcept { return _type == RenderCommandType::kFillAnalytic; }
  BL_INLINE_NODEBUG bool is_fill_box_mask_a() const noexcept { return _type == RenderCommandType::kFillBoxMaskA; }

  BL_INLINE_NODEBUG RenderCommandFlags flags() const noexcept { return RenderCommandFlags(_flags); }
  BL_INLINE_NODEBUG bool has_flag(RenderCommandFlags flag) const noexcept { return uint32_t(_flags & flag) != 0; }
  BL_INLINE_NODEBUG void add_flags(RenderCommandFlags flags) noexcept { _flags |= flags; }

  BL_INLINE_NODEBUG bool has_style_fetch_data() const noexcept { return has_flag(RenderCommandFlags::kHasStyleFetchData); }
  BL_INLINE_NODEBUG bool retains_style_fetch_data() const noexcept { return has_flag(RenderCommandFlags::kRetainsStyleFetchData); }
  BL_INLINE_NODEBUG bool retains_mask() const noexcept { return has_flag(RenderCommandFlags::kRetainsMaskImageData | RenderCommandFlags::kRetainsMaskFetchData); }
  BL_INLINE_NODEBUG bool retains_mask_image_data() const noexcept { return has_flag(RenderCommandFlags::kRetainsMaskImageData); }
  BL_INLINE_NODEBUG bool retains_mask_fetch_data() const noexcept { return has_flag(RenderCommandFlags::kRetainsMaskFetchData); }

  BL_INLINE_NODEBUG uint32_t alpha() const noexcept { return _alpha; }
  BL_INLINE_NODEBUG const BLBoxI& box_i() const noexcept { return _payload.box.box_i; }

  BL_INLINE uint32_t analytic_fill_rule() const noexcept {
    BL_ASSERT(is_fill_analytic());
    return _payload.analytic.fill_rule;
  }

  BL_INLINE const EdgeVector<int>* analytic_edges() const noexcept {
    BL_ASSERT(is_fill_analytic());
    return _payload.analytic.edges.ptr;
  }

  //! Returns a pointer to `Pipeline::FillData` that is only valid when the command type is `kTypeFillBoxA`. It casts
  //! `_rect` member to the requested data type as it's  compatible. This trick cannot be used for any other command types.
  BL_INLINE const void* get_pipe_fill_data_of_box_a() const noexcept {
    BL_ASSERT(is_fill_box_a());
    return &_payload.box.box_i;
  }

  //! Returns `_solid_data` or `_fetch_data` casted properly to `Pipeline::FetchData` type.
  BL_INLINE const void* get_pipe_fetch_data() const noexcept {
    const void* data = &_source.solid;
    if (has_style_fetch_data())
      data = &_source.fetch_data->pipeline_data;
    return data;
  }

  BL_INLINE_NODEBUG Pipeline::DispatchData* pipe_dispatch_data() noexcept { return &_dispatch_data; }
  BL_INLINE_NODEBUG const Pipeline::DispatchData* pipe_dispatch_data() const noexcept { return &_dispatch_data; }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERCOMMAND_P_H_INCLUDED
