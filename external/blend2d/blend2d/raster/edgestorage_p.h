// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_EDGESTORAGE_P_H_INCLUDED
#define BLEND2D_RASTER_EDGESTORAGE_P_H_INCLUDED

#include <blend2d/support/intops_p.h>
#include <blend2d/support/math_p.h>
#include <blend2d/support/traits_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

//! Parametrized point used by edge builder that should represent either 16-bit
//! or 32-bit fixed point.
template<typename T>
struct EdgePoint {
  T x, y;

  BL_INLINE void reset(T x_, T y_) noexcept {
    this->x = x_;
    this->y = y_;
  }
};

static BL_INLINE size_t pack_count_and_sign_bit(size_t count, uint32_t sign_bit) noexcept {
  BL_ASSERT(count <= (~size_t(0) >> 1));
  BL_ASSERT(sign_bit <= 0x1u);

  return (count << 1u) | sign_bit;
}

template<typename CoordT>
struct alignas(8) EdgeVector {
  EdgeVector<CoordT>* next;
  size_t count_and_sign;
  EdgePoint<CoordT> pts[1];

  BL_INLINE size_t count() const noexcept { return count_and_sign >> 1u; }
  BL_INLINE uint32_t sign_bit() const noexcept { return uint32_t(count_and_sign & 0x1u); }

  static constexpr uint32_t min_size_of() noexcept {
    return uint32_t(sizeof(EdgeVector<CoordT>) + sizeof(EdgePoint<CoordT>));
  }
};

template<typename CoordT>
struct EdgeList {
  EdgeVector<CoordT>* _first;
  EdgeVector<CoordT>* _last;

  BL_INLINE void reset() noexcept {
    _first = nullptr;
    _last = nullptr;
  }

  BL_INLINE bool is_empty() const noexcept { return _last == nullptr; }

  BL_INLINE EdgeVector<CoordT>* first() const noexcept { return _first; }
  BL_INLINE EdgeVector<CoordT>* last() const noexcept { return _last; }

  BL_INLINE void append(EdgeVector<CoordT>* item) noexcept {
    item->next = nullptr;
    if (is_empty()) {
      _first = item;
      _last = item;
    }
    else {
      _last->next = item;
      _last = item;
    }
  }
};

template<typename CoordT>
class EdgeStorage {
public:
  //! Start edge vectors of each band.
  EdgeList<CoordT>* _band_edges;
  //! Length of `_band_edges` array.
  uint32_t _band_count;
  //! Capacity of `_band_edges` array.
  uint32_t _band_capacity;
  //! Height of a single band (in pixels).
  uint32_t _band_height;
  //! Shift to get a band_id from a fixed-point y coordinate.
  uint32_t _fixed_band_height_shift;
  //! Bounding box in fixed-point.
  BLBoxI _bounding_box;

  BL_INLINE EdgeStorage() noexcept { reset(); }
  BL_INLINE EdgeStorage(const EdgeStorage& other) noexcept = default;

  BL_INLINE void reset() noexcept {
    _band_edges = nullptr;
    _band_count = 0;
    _band_capacity = 0;
    _band_height = 0;
    _fixed_band_height_shift = 0;
    reset_bounding_box();
  }

  BL_INLINE void clear() noexcept {
    if (!is_empty()) {
      size_t band_start = bandStartFromBBox();
      size_t band_end = bandEndFromBBox();

      for (size_t i = band_start; i < band_end; i++)
        _band_edges[i].reset();

      reset_bounding_box();
    }
  }

  BL_INLINE bool is_empty() const noexcept { return _bounding_box.y0 == Traits::max_value<int>(); }

  BL_INLINE EdgeList<CoordT>* band_edges() const noexcept { return _band_edges; }
  BL_INLINE uint32_t band_count() const noexcept { return _band_count; }
  BL_INLINE uint32_t band_capacity() const noexcept { return _band_capacity; }
  BL_INLINE uint32_t band_height() const noexcept { return _band_height; }
  BL_INLINE uint32_t fixed_band_height_shift() const noexcept { return _fixed_band_height_shift; }
  BL_INLINE const BLBoxI& bounding_box() const noexcept { return _bounding_box; }

  BL_INLINE void init_data(EdgeList<CoordT>* band_edges, uint32_t band_count, uint32_t band_capacity, uint32_t band_height) noexcept {
    _band_edges = band_edges;
    _band_count = band_count;
    _band_capacity = band_capacity;
    _band_height = band_height;
    _fixed_band_height_shift = IntOps::ctz(band_height) + Pipeline::A8Info::kShift;
  }

  BL_INLINE void reset_bounding_box() noexcept {
    _bounding_box.reset(Traits::max_value<int>(), Traits::max_value<int>(), Traits::min_value<int>(), Traits::min_value<int>());
  }

  BL_INLINE uint32_t bandStartFromBBox() const noexcept {
    return unsigned(bounding_box().y0) >> fixed_band_height_shift();
  }

  BL_INLINE uint32_t bandEndFromBBox() const noexcept {
    // NOTE: Calculating `band_end` is tricky, because in some rare cases
    // the bounding box can end exactly at some band's initial coordinate.
    // In such case we don't know whether the band has data there or not,
    // so we must consider it initially.
    return bl_min((unsigned(bounding_box().y1) >> fixed_band_height_shift()) + 1, band_count());
  }

  BL_INLINE EdgeVector<CoordT>* flatten_edge_links() noexcept {
    EdgeList<int>* band_edges = this->band_edges();

    size_t band_id = bandStartFromBBox();
    size_t band_end = bandEndFromBBox();

    EdgeVector<CoordT>* first = band_edges[band_id].first();
    EdgeVector<CoordT>* current = band_edges[band_id].last();

    // The first band must always be non-null as it starts the edges.
    BL_ASSERT(first != nullptr);
    BL_ASSERT(current != nullptr);

    band_edges[band_id].reset();
    while (++band_id < band_end) {
      EdgeVector<int>* band_first = band_edges[band_id].first();
      if (!band_first)
        continue;
      current->next = band_first;
      current = band_edges[band_id].last();
      band_edges[band_id].reset();
    }

    return first;
  }
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_EDGESTORAGE_P_H_INCLUDED

