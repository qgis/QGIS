// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_STYLEDATA_P_H_INCLUDED
#define BLEND2D_RASTER_STYLEDATA_P_H_INCLUDED

#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/renderfetchdata_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

//! Style data holds a copy of user-provided style with additional members that allow to create a `RenderFetchData`
//! from it. When a style is assigned to the rendering context it has to calculate the style transformation matrix
//! and a few other things that could degrade the style into a solid fill.
struct StyleData {
  //! \name Members
  //! \{

  //! Pointer to a fetch data - it points to either a separate `RenderFetchData` data or to `&solid` data in this
  //! struct. Use `has_implicit_fetch_data()` to check whether fetch data points to external fetch data or to &solid.
  RenderFetchDataHeader* fetch_data;

  struct SolidData : public RenderFetchDataSolid {
    //! The original color passed to set_style() API.
    union {
      //! Solid color as passed to frontend (non-premultiplied RGBA float components).
      BLRgba rgba;
      //! Solid color as passed to frontend (non-premultiplied RGBA32 integer components).
      BLRgba32 rgba32;
      //! Solid color as passed to frontend (non-premultiplied RGBA64 integer components).
      BLRgba64 rgba64;
    } original;
  };

  struct NonSolidData {
    //! Style transformation matrix combined with the rendering context transformation matrix.
    BLMatrix2D adjusted_transform;
  };

  union {
    SolidData solid;
    NonSolidData non_solid;
  };

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE_NODEBUG void make_fetch_data_implicit() noexcept { fetch_data = &solid; }
  BL_INLINE_NODEBUG bool has_implicit_fetch_data() const noexcept { return fetch_data == static_cast<const void*>(&solid); }

  BL_INLINE_NODEBUG bool has_fetch_data() const noexcept { return !has_implicit_fetch_data(); }
  BL_INLINE_NODEBUG RenderFetchData* get_render_fetch_data() const noexcept { return static_cast<RenderFetchData*>(fetch_data); }

  //! \}

  //! \name Memory Operations
  //! \{

  BL_INLINE void swap(StyleData& other) noexcept {
    bool this_implicit = has_implicit_fetch_data();
    bool other_implicit = other.has_implicit_fetch_data();

    BLInternal::swap(*this, other);

    if (this_implicit)
      other.make_fetch_data_implicit();

    if (other_implicit)
      make_fetch_data_implicit();
  }

  BL_INLINE void copy_from(const StyleData& other) noexcept {
    memcpy(this, &other, sizeof(StyleData));
    if (other.has_implicit_fetch_data())
      make_fetch_data_implicit();
  }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_STYLEDATA_P_H_INCLUDED
