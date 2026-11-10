// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_IMAGESCALE_P_H_INCLUDED
#define BLEND2D_IMAGESCALE_P_H_INCLUDED

#include <blend2d/core/geometry.h>
#include <blend2d/core/image.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl {

//! Low-level image scaling context.
class ImageScaleContext {
public:
  enum Dir : uint32_t {
    kDirHorz = 0,
    kDirVert = 1
  };

  struct Record {
    uint32_t pos;
    uint32_t count;
  };

  struct Data {
    int dst_size[2];
    int src_size[2];
    int kernel_size[2];
    int is_unbound[2];

    double scale[2];
    double factor[2];
    double radius[2];

    int32_t* weight_list[2];
    Record* record_list[2];
  };

  Data* data;

  BL_INLINE ImageScaleContext() noexcept
    : data(nullptr) {}

  BL_INLINE ~ImageScaleContext() noexcept { reset(); }

  BL_INLINE bool is_initialized() const noexcept { return data != nullptr; }

  BL_INLINE int dst_width() const noexcept { return data->dst_size[kDirHorz]; }
  BL_INLINE int dst_height() const noexcept { return data->dst_size[kDirVert]; }

  BL_INLINE int src_width() const noexcept { return data->src_size[kDirHorz]; }
  BL_INLINE int src_height() const noexcept { return data->src_size[kDirVert]; }

  BL_HIDDEN BLResult reset() noexcept;
  BL_HIDDEN BLResult create(const BLSizeI& to, const BLSizeI& from, uint32_t filter) noexcept;

  BL_HIDDEN BLResult process_horz_data(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride, uint32_t format) const noexcept;
  BL_HIDDEN BLResult process_vert_data(uint8_t* dst_line, intptr_t dst_stride, const uint8_t* src_line, intptr_t src_stride, uint32_t format) const noexcept;
};

} // {bl}

//! \}
//! \endcond

#endif // BLEND2D_IMAGESCALE_P_H_INCLUDED
