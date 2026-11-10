// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_ANALYTICRASTERIZER_P_H_INCLUDED
#define BLEND2D_RASTER_ANALYTICRASTERIZER_P_H_INCLUDED

#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/edgestorage_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/ptrops_p.h>
#include <blend2d/support/traits_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {

//! Analytic rasterizer cell and bit-vector storage.
struct AnalyticCellStorage {
  //! BitWord pointer at top-left corner.
  BLBitWord* bit_ptr_top;
  //! BitWord stride [in bytes].
  size_t bit_stride;

  //! Cell pointer at top-left corner.
  uint32_t* cell_ptr_top;
  //! Cell stride [in bytes].
  size_t cell_stride;

  BL_INLINE void init(BLBitWord* bit_ptr_top_, size_t bit_stride_, uint32_t* cell_ptr_top_, size_t cell_stride_) noexcept {
    this->bit_ptr_top = bit_ptr_top_;
    this->bit_stride = bit_stride_;
    this->cell_ptr_top = cell_ptr_top_;
    this->cell_stride = cell_stride_;
  }

  BL_INLINE void reset() noexcept {
    this->bit_ptr_top = nullptr;
    this->bit_stride = 0;
    this->cell_ptr_top = nullptr;
    this->cell_stride = 0;
  }
};

//! Analytic rasterizer utilities.
namespace AnalyticUtils {

//! Apply a sign-mask to `x`.
//!
//! A sign mask must have all bits either zero (no change) or ones (inverts
//! the sign).
template<typename X, typename Y>
BL_INLINE constexpr X apply_sign_mask(const X& x, const Y& mask) noexcept {
  using U = std::make_unsigned_t<X>;
  return X((U(x) ^ U(mask)) - U(mask));
}

//! Branchless implementation of the following code:
//!
//! ```
//! iter -= step;
//! if (iter < 0) {
//!   acc++;
//!   iter += correction;
//! }
//! ```
template<typename AccT, typename IterT>
static BL_INLINE void acc_err_step(AccT& acc, IterT& iter, const IterT& step, const IterT& correction) noexcept {
  BL_STATIC_ASSERT(sizeof(AccT) == sizeof(IterT));

  iter -= step;

  // Contains all ones if the iterator has underflown (requires correction).
  IterT mask = IterT(IntOps::sar(iter, IntOps::bit_size_of<IterT>() - 1));

  acc -= AccT(mask);         // if (iter < 0) acc++;
  iter += mask & correction; // if (iter < 0) iter += correction;
}

template<typename AccT, typename IterT, typename CountT>
static BL_INLINE void acc_err_multi_step(AccT& acc, IterT& iter, const IterT& step, const IterT& correction, const CountT& count) noexcept {
  BL_STATIC_ASSERT(sizeof(AccT) == sizeof(IterT));

  using U = std::make_unsigned_t<IterT>;

  int64_t i = int64_t(U(iter));
  i -= int64_t(uint64_t(step) * uint32_t(count));

  if (i < 0) {
    int n = int(((uint64_t(-i) + U(correction) - 1u) / uint64_t(correction)));
    acc += AccT(n);
    i += int64_t(correction) * n;
  }

  iter = IterT(i);
}

} // {AnalyticUtils}

//! Analytic rasterizer state.
//!
//! This state can be used to temporarily terminate rasterization. It's used in case that the context uses banding
//! (large inputs) or asynchronous rendering possibly combined with multithreading.
struct AnalyticState {
  enum Flags : uint32_t {
    //! This flag is always set by `AnalyticRasterizer::prepare()`, however,
    //! it can be ignored completely if the line is not horizontally oriented.
    kFlagInitialScanline = 0x00000001u,

    //! Flag set if the line is strictly vertical (dy == 0) or if it fits into
    //! a single cell. These are two special cases handled differently.
    kFlagVertOrSingle = 0x00000002u,

    //! Set if the line is rasterized from right to left.
    kFlagRightToLeft = 0x00000004u
  };

  int _ex0, _ey0, _ex1, _ey1;
  int _fx0, _fy0, _fx1, _fy1;

  int _xErr, _yErr;
  int _xDlt, _yDlt;
  int _xRem, _yRem;
  int _xLift, _yLift;

  int _dx, _dy;
  int _savedFy1;
  uint32_t _flags;
};

template<typename T>
struct AnalyticActiveEdge {
  //! Rasterizer state.
  AnalyticState state;
  //! Sign bit, for making cover/area negative.
  uint32_t sign_bit;
  //! Start of point data (advanced during rasterization).
  const EdgePoint<T>* cur;
  //! End of point data.
  const EdgePoint<T>* end;
  //! Next active edge (single-linked list).
  AnalyticActiveEdge<T>* next;
};

//! Analytic rasterizer.
//!
//! This rasterizer is designed to provide some customization through `OPTIONS`. It's well suited for both small and
//! large paths having any number of input vertices. The algorithm is based on AGG rasterizer, but was improved to
//! always render from top to bottom (to support banding) and to use dense cell representation instead of cell spans
//! or any other sparse cell representation.
//!
//! To mark cells that are non-zero (and have to be processed by the compositor) it uses a fixed bit vectors per each
//! scanline where 1 bit represents N cells (and thus N target pixels). This has a huge advantage as the compositor
//! can skip pixels in hundreds by just checking the bit vector without having to process cells that are zero.
//!
//! Since the rasterizer requires dense cell buffer and expects this buffer to be zero initialized, the compositor
//! should zero all cells and bits it processes so the buffer is ready for another rasterization. Blend2D does this
//! implicitly, so this is just a note for anyone wondering how this works.
struct AnalyticRasterizer : public AnalyticState {
  // Compile-time dispatched features the rasterizer supports.
  enum Options : uint32_t {
    //! Rasterizer uses banding technique.
    kOptionBandingMode = 0x0004u,
    //! Takes `_band_offset` into consideration.
    kOptionBandOffset = 0x0008u,
    //! BitStride is equal to sizeof(BLBitWord).
    kOptionEasyBitStride = 0x0010u,
    //! Record minimum and maximum X coordinate so the compositor can optimize
    //! bit scanning - this means it would start at BitWord that has minimum X
    //! and end at BitWord that has maximum X.
    kOptionRecordMinXMaxX = 0x0020u
  };

  //! Bit operations that the rasterizer uses.
  typedef PrivateBitWordOps BitOps;

  //! BitWords and Cells, initialized by `init()`, never modified.
  AnalyticCellStorage _cell_storage;

  //! Sign mask.
  uint32_t _sign_mask;
  //! Height of a rendering band (number of scanlines).
  uint32_t _band_height;
  //! Offset to the first scanline in the current band.
  uint32_t _band_offset;
  //! End of the current band (_band_offset + _band_height - 1).
  uint32_t _band_end;

  //! Recorded minimum X, only updated when `kOptionRecordMinXMaxX` is set.
  uint32_t _cellMinX;
  //! Recorded maximum X, only updated when `kOptionRecordMinXMaxX` is set.
  uint32_t _cellMaxX;

  typedef Pipeline::A8Info A8Info;

  //! \name Initialization
  //! \{

  BL_INLINE void init(BLBitWord* bit_ptr_top, size_t bit_stride, uint32_t* cell_ptr_top, size_t cell_stride, uint32_t band_offset, uint32_t band_height) noexcept {
    // Reset most members so the compiler doesn't think some of them are used
    // uninitialized in case we save state of a vertical only line, etc...
    //
    // We don't reset coords & dx/dy as they are always properly set by `prepare()`.
    _xErr = 0;
    _yErr = 0;
    _xDlt = 0;
    _yDlt = 0;
    _xRem = 0;
    _yRem = 0;
    _xLift = 0;
    _yLift = 0;
    _flags = 0;

    // AnalyticRasterizer members.
    _cell_storage.init(bit_ptr_top, bit_stride, cell_ptr_top, cell_stride);
    _sign_mask = 0;
    _band_height = band_height;
    _band_offset = band_offset;
    _band_end = band_offset + band_height - 1;

    reset_bounds();
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE BLBitWord* bit_ptr_top() const noexcept {
    return _cell_storage.bit_ptr_top;
  }

  //! Returns the current `bit_stride`.
  //!
  //! This function returns `sizeof(BLBitWord)` in case we are generating an
  //! optimized rasterizer for small-art where the number of bits that
  //! represent pixels including padding doesn't exceed a single BitWord.
  template<uint32_t OPTIONS>
  BL_INLINE size_t bit_stride() const noexcept {
    if (OPTIONS & kOptionEasyBitStride)
      return sizeof(BLBitWord);
    else
      return _cell_storage.bit_stride;
  }

  BL_INLINE uint32_t* cell_ptr_top() const noexcept { return _cell_storage.cell_ptr_top; }
  BL_INLINE size_t cell_stride() const noexcept { return _cell_storage.cell_stride; }

  BL_INLINE uint32_t sign_mask() const noexcept { return _sign_mask; }
  BL_INLINE void set_sign_mask(uint32_t sign_mask) noexcept { _sign_mask = sign_mask; }
  BL_INLINE void set_sign_mask_from_bit(uint32_t sign_bit) noexcept { _sign_mask = IntOps::negate(sign_bit); }

  //! \}

  //! \name Global Bounds
  //! \{

  BL_INLINE bool has_bounds() const noexcept {
    return _cellMinX <= _cellMaxX;
  }

  BL_INLINE void reset_bounds() noexcept {
    _cellMinX = Traits::max_value<uint32_t>();
    _cellMaxX = 0;
  }

  //! \}

  //! \name Save & Restore
  //! \{

  BL_INLINE void save(AnalyticState& state) const noexcept {
    state = *static_cast<const AnalyticState*>(this);
  }

  BL_INLINE void restore(AnalyticState& state) noexcept {
    *static_cast<AnalyticState*>(this) = state;
  }

  //! \}

  //! \name Prepare
  //! \{

  BL_INLINE bool prepare_ref(const EdgePoint<int>& p0, const EdgePoint<int>& p1) noexcept {
    using AnalyticUtils::acc_err_step;

    // Line should be already reversed in case it has a negative sign.
    BL_ASSERT(p0.y <= p1.y);

    // Should not happen regularly, but in some edge cases this can happen in cases where a curve was flattened
    // into line segments that don't change vertically or produced by `EdgeBuilderFromSource` that doesn't
    // eliminate strictly horizontal edges.
    if (p0.y == p1.y)
      return false;

    _dx = p1.x - p0.x;
    _dy = p1.y - p0.y;
    _flags = kFlagInitialScanline;

    if (_dx < 0) {
      _flags |= kFlagRightToLeft;
      _dx = -_dx;
    }

    _ex0 = (p0.x    ) >> A8Info::kShift;
    _ey0 = (p0.y    ) >> A8Info::kShift;
    _ex1 = (p1.x    ) >> A8Info::kShift;
    _ey1 = (p1.y - 1) >> A8Info::kShift;

    _fx0 = ((p0.x    ) & int(A8Info::kMask));
    _fy0 = ((p0.y    ) & int(A8Info::kMask));
    _fx1 = ((p1.x    ) & int(A8Info::kMask));
    _fy1 = ((p1.y - 1) & int(A8Info::kMask)) + 1;

    _savedFy1 = _fy1;
    if (_ey0 != _ey1)
      _fy1 = int(A8Info::kScale);

    if (_ex0 == _ex1 && (_ey0 == _ey1 || _dx == 0)) {
      _flags |= kFlagVertOrSingle;
      return true;
    }

    uint64_t x_base = uint64_t(uint32_t(_dx)) * A8Info::kScale;
    uint64_t y_base = uint64_t(uint32_t(_dy)) * A8Info::kScale;

    _xLift = int(x_base / unsigned(_dy));
    _xRem  = int(x_base % unsigned(_dy));

    _yLift = int(y_base / unsigned(_dx));
    _yRem  = int(y_base % unsigned(_dx));

    _xDlt = _dx;
    _yDlt = _dy;

    _xErr = (_dy >> 1) - 1;
    _yErr = (_dx >> 1) - 1;

    if (_ey0 != _ey1) {
      uint64_t p = uint64_t(A8Info::kScale - uint32_t(_fy0)) * uint32_t(_dx);
      _xDlt  = int(p / unsigned(_dy));
      _xErr -= int(p % unsigned(_dy));
      acc_err_step(_xDlt, _xErr, 0, _dy);
    }

    if (_ex0 != _ex1) {
      uint64_t p = uint64_t((_flags & kFlagRightToLeft) ? uint32_t(_fx0) : A8Info::kScale - uint32_t(_fx0)) * uint32_t(_dy);
      _yDlt  = int(p / unsigned(_dx));
      _yErr -= int(p % unsigned(_dx));
      acc_err_step(_yDlt, _yErr, 0, _dx);
    }

    _yDlt += _fy0;
    return true;
  }

  BL_INLINE bool prepare(const EdgePoint<int>& p0, const EdgePoint<int>& p1) noexcept {
    return prepare_ref(p0, p1);
  }

  //! \}

  //! \name Advance
  //! \{

  BL_INLINE void advanceToY(int y_target) noexcept {
    using AnalyticUtils::acc_err_step;
    using AnalyticUtils::acc_err_multi_step;

    if (y_target <= _ey0)
      return;
    BL_ASSERT(y_target <= _ey1);

    if (!(_flags & kFlagVertOrSingle)) {
      int ny = y_target - _ey0;

      _xDlt += _xLift * (ny - 1);
      acc_err_multi_step(_xDlt, _xErr, _xRem, _dy, ny - 1);

      if (_flags & kFlagRightToLeft) {
        _fx0 -= _xDlt;
        if (_fx0 < 0) {
          int nx = -(_fx0 >> A8Info::kShift);
          BL_ASSERT(nx <= _ex0 - _ex1);
          _ex0 -= nx;
          _fx0 &= A8Info::kMask;

          acc_err_multi_step(_yDlt, _yErr, _yRem, _dx, nx);
          _yDlt += _yLift * nx;
        }

        if (!(_dy >= _dx)) {
          if (!_fx0) {
            _fx0 = A8Info::kScale;
            _ex0--;

            acc_err_step(_yDlt, _yErr, _yRem, _dx);
            _yDlt += _yLift;
          }
        }

        if (y_target == _ey1 && _dy >= _dx) {
          _fy1 = _savedFy1;
          _xDlt = ((_ex0 - _ex1) << A8Info::kShift) + _fx0 - _fx1;
          BL_ASSERT(_xDlt >= 0);
        }
        else {
          _xDlt = _xLift;
          acc_err_step(_xDlt, _xErr, _xRem, _dy);
        }
      }
      else {
        _fx0 += _xDlt;
        if (_fx0 >= int(A8Info::kScale)) {
          int nx = (_fx0 >> A8Info::kShift);
          BL_ASSERT(nx <= _ex1 - _ex0);
          _ex0 += nx;
          _fx0 &= A8Info::kMask;

          acc_err_multi_step(_yDlt, _yErr, _yRem, _dx, nx);
          _yDlt += _yLift * nx;
        }

        if (y_target == _ey1 && _dy >= _dx) {
          _fy1 = _savedFy1;
          _xDlt = ((_ex1 - _ex0) << A8Info::kShift) + _fx1 - _fx0;
          BL_ASSERT(_xDlt >= 0);
        }
        else {
          _xDlt = _xLift;
          acc_err_step(_xDlt, _xErr, _xRem, _dy);
        }
      }

      if (_dy >= _dx) {
        _yDlt &= A8Info::kMask;
      }
      else {
        int y = ny;
        if (_flags & kFlagInitialScanline)
          y--;
        _yDlt -= y * int(A8Info::kScale);
        BL_ASSERT(_yDlt >= 0);
      }
    }
    else {
      if (y_target == _ey1)
        _fy1 = _savedFy1;
    }

    _fy0 = 0;
    _ey0 = y_target;
    _flags &= ~kFlagInitialScanline;
  }

  //! \}

  //! \name Rasterize
  //! \{

  template<uint32_t OPTIONS>
  BL_INLINE bool rasterize() noexcept {
    BL_ASSERT(uint32_t(_ey0) >= _band_offset);

    using AnalyticUtils::acc_err_step;

    // Adjust `_ey1_end` in case the line crosses the current band and banding is enabled.
    int _ey1_end = _ey1;
    if (OPTIONS & kOptionBandingMode)
      _ey1_end = bl_min(_ey1_end, int(_band_end));

    // Number of scanlines to rasterize excluding the first one.
    size_t i = unsigned(_ey1_end) - unsigned(_ey0);
    uint32_t y_offset = unsigned(_ey0);

    if (OPTIONS & kOptionBandOffset)
      y_offset -= _band_offset;

    BLBitWord* bit_ptr = PtrOps::offset(bit_ptr_top(), y_offset * bit_stride<OPTIONS>());
    uint32_t* cell_ptr = PtrOps::offset(cell_ptr_top(), y_offset * cell_stride());

    if (OPTIONS & kOptionBandingMode) {
      // Advance `_ey0` so it's valid for a next band if it crosses the current one.
      _ey0 += int(i) + 1;
    }

    const uint32_t full_cover = apply_sign_mask(A8Info::kScale);
    if (_flags & kFlagVertOrSingle) {
      // ....x....    .........
      // ....x....    .........
      // ....x.... or ....x....
      // ....x....    .........
      // ....x....    .........
      uint32_t cover;
      uint32_t area = uint32_t(_fx0) + uint32_t(_fx1);

      updateMinX<OPTIONS>(_ex0);
      updateMaxX<OPTIONS>(_ex0);

      size_t bit_index = unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT;
      BLBitWord bit_mask = BitOps::index_as_mask(bit_index % IntOps::bit_size_of<BLBitWord>());

      bit_ptr += (bit_index / IntOps::bit_size_of<BLBitWord>());
      cell_ptr += unsigned(_ex0);

      // First scanline or a line that occupies a single cell only. In case of banding support this code
      // can run multiple times, but it's safe as we adjust both `_fy0` and `_fy1` accordingly.
      cover = apply_sign_mask(uint32_t(_fy1 - _fy0));

      cell_merge(cell_ptr, 0, cover, cover * area);
      bit_ptr[0] |= bit_mask;

      if (OPTIONS & kOptionBandingMode) {
        if (!i) {
          // Single cell line.
          if (_ey0 > _ey1)
            return true;

          // Border case: If the next scanline is end-of-line we must update both `_fy0` and `_fy1` as we
          // will only go through the same code again.
          _fy0 = 0;
          _fy1 = _ey0 == _ey1 ? _savedFy1 : int(A8Info::kScale);
          return false;
        }
      }
      else {
        // Single cell line.
        if (!i)
          return true;
      }

      // All scanlines between [_ey0:_ey1], exclusive.
      bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
      cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

      cover = full_cover;
      while (--i) {
        cell_merge(cell_ptr, 0, cover, cover * area);
        cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

        bit_ptr[0] |= bit_mask;
        bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
      }

      if (OPTIONS & kOptionBandingMode) {
        if (_ey0 <= _ey1) {
          // Handle end-of-band case - renders the last scanline.
          cell_merge(cell_ptr, 0, cover, cover * area);
          bit_ptr[0] |= bit_mask;

          // Border case: If the next scanline is end-of-line we must update `_fy1` as we will only go through
          // the initial cell next time.
          _fy0 = 0;
          _fy1 = _ey0 == _ey1 ? _savedFy1 : int(A8Info::kScale);
          return false;
        }
      }

      // Special case - last scanline of the line.
      cover = apply_sign_mask(uint32_t(_savedFy1));
      cell_merge(cell_ptr, 0, cover, cover * area);
      bit_ptr[0] |= bit_mask;

      // Line ends within this band.
      return true;
    }
    else if (_dy >= _dx) {
      if (OPTIONS & kOptionBandingMode)
        i += (_ey0 <= _ey1);

      if (_flags & kFlagRightToLeft) {
        // ......x..
        // .....xx..
        // ....xx...
        // ...xx....
        // ...x.....
        updateMaxX<OPTIONS>(_ex0);

        for (;;) {
          // First and/or last scanline is a special-case that must consider `_fy0` and `_fy1`. If this is
          // a rasterizer that uses banding then this case will also be executed as a start of each band, which
          // is fine as it can handle all cases by design.
          uint32_t area = uint32_t(_fx0);
          uint32_t cov0, cov1;

          _fx0 -= _xDlt;
          if (_fx0 < 0) {
            _ex0--;
            _fx0 += A8Info::kScale;
            _yDlt &= A8Info::kMask;

            if (!area) {
              area = A8Info::kScale;
              acc_err_step(_yDlt, _yErr, _yRem, _dx);
              _yDlt += _yLift;
              goto VertRightToLeftSingleFirstOrLast;
            }

            bit_set<OPTIONS>(bit_ptr, unsigned(_ex0 + 0) / BL_PIPE_PIXELS_PER_ONE_BIT);
            bit_set<OPTIONS>(bit_ptr, unsigned(_ex0 + 1) / BL_PIPE_PIXELS_PER_ONE_BIT);
            cov0 = apply_sign_mask(uint32_t(_yDlt - _fy0));
            area = cov0 * area;
            cell_merge(cell_ptr, _ex0 + 1, cov0, area);

            cov0 = apply_sign_mask(uint32_t(_fy1 - _yDlt));
            area = cov0 * (uint32_t(_fx0) + A8Info::kScale);
            cell_merge(cell_ptr, _ex0, cov0, area);

            acc_err_step(_yDlt, _yErr, _yRem, _dx);
            _yDlt += _yLift;
          }
          else {
VertRightToLeftSingleFirstOrLast:
            bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);
            cov0 = apply_sign_mask(uint32_t(_fy1 - _fy0));
            area = cov0 * (area + uint32_t(_fx0));
            cell_merge(cell_ptr, _ex0, cov0, area);
          }

          _fy0 = 0;
          bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
          cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

          if (!i) {
            updateMinX<OPTIONS>(_ex0);
            if (OPTIONS & kOptionBandingMode) {
              if (_ey0 > _ey1)
                return true;

              _xDlt = _xLift;
              acc_err_step(_xDlt, _xErr, _xRem, _dy);
              return false;
            }
            else {
              return true;
            }
          }

          // All scanlines between [_ey0:_ey1], exclusive.
          while (--i) {
            _xDlt = _xLift;
            acc_err_step(_xDlt, _xErr, _xRem, _dy);

            area = uint32_t(_fx0);
            _fx0 -= _xDlt;

            if (_fx0 < 0) {
              _ex0--;
              _fx0 += A8Info::kScale;
              _yDlt &= A8Info::kMask;

              if (!area) {
                area = A8Info::kScale;
                acc_err_step(_yDlt, _yErr, _yRem, _dx);
                _yDlt += _yLift;
                goto VertRightToLeftSingleInLoop;
              }

              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0 + 0) / BL_PIPE_PIXELS_PER_ONE_BIT);
              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0 + 1) / BL_PIPE_PIXELS_PER_ONE_BIT);
              bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());

              cov1 = apply_sign_mask(uint32_t(_yDlt));
              area = cov1 * area;
              cell_add(cell_ptr, _ex0 + 2, area);

              cov0 = full_cover - cov1;
              cov1 = IntOps::shl(cov1, 9) - area;
              area = cov0 * (uint32_t(_fx0) + A8Info::kScale);

              cov0 = IntOps::shl(cov0, 9) - area;
              cov1 = cov1 + area;

              cell_add(cell_ptr, _ex0 + 0, cov0);
              cell_add(cell_ptr, _ex0 + 1, cov1);
              cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

              acc_err_step(_yDlt, _yErr, _yRem, _dx);
              _yDlt += _yLift;
            }
            else {
VertRightToLeftSingleInLoop:
              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);
              bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
              area = full_cover * (area + uint32_t(_fx0));

              cell_merge(cell_ptr, _ex0, full_cover, area);
              cell_ptr = PtrOps::offset(cell_ptr, cell_stride());
            }
          }

          if (OPTIONS & kOptionBandingMode) {
            if (_ey0 >= _ey1) {
              // Last scanline, we will do it either now or in the next band (border-case).
              _fy1 = _savedFy1;
              _xDlt = IntOps::shl(_ex0 - _ex1, A8Info::kShift) + _fx0 - _fx1;
              BL_ASSERT(_xDlt >= 0);

              // Border case, last scanline is the first scanline in the next band.
              if (_ey0 == _ey1) {
                updateMinX<OPTIONS>(_ex0);
                return false;
              }
            }
            else {
              updateMinX<OPTIONS>(_ex0);
              _xDlt = _xLift;
              acc_err_step(_xDlt, _xErr, _xRem, _dy);
              return false;
            }
          }
          else {
            // Prepare the last scanline.
            _fy1 = _savedFy1;
            _xDlt = IntOps::shl(_ex0 - _ex1, A8Info::kShift) + _fx0 - _fx1;
            BL_ASSERT(_xDlt >= 0);
          }
        }
      }
      else {
        // ..x......
        // ..xx.....
        // ...xx....
        // ....xx...
        // .....x...
        updateMinX<OPTIONS>(_ex0);

        for (;;) {
          // First and/or last scanline is a special-case that must consider `_fy0` and `_fy1`. If this is
          // a rasterizer that uses banding then this case will also be executed as a start of each band,
          // which is fine as it can handle all cases by design.
          uint32_t area = uint32_t(_fx0);
          uint32_t cov0, cov1;

          _fx0 += _xDlt;
          bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

          if (_fx0 <= int(A8Info::kScale)) {
            cov0 = apply_sign_mask(uint32_t(_fy1 - _fy0));
            area = cov0 * (area + uint32_t(_fx0));
            cell_merge(cell_ptr, _ex0, cov0, area);

            if (_fx0 == int(A8Info::kScale)) {
              _ex0++;
              _fx0 = 0;
              _yDlt += _yLift;
              acc_err_step(_yDlt, _yErr, _yRem, _dx);
            }
          }
          else {
            _ex0++;
            _fx0 &= A8Info::kMask;
            _yDlt &= A8Info::kMask;
            bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

            cov0 = apply_sign_mask(uint32_t(_yDlt - _fy0));
            area = cov0 * (area + A8Info::kScale);
            cell_merge(cell_ptr, _ex0 - 1, cov0, area);

            cov0 = apply_sign_mask(uint32_t(_fy1 - _yDlt));
            area = cov0 * uint32_t(_fx0);
            cell_merge(cell_ptr, _ex0, cov0, area);

            _yDlt += _yLift;
            acc_err_step(_yDlt, _yErr, _yRem, _dx);
          }

          _fy0 = 0;
          bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
          cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

          if (!i) {
            updateMaxX<OPTIONS>(_ex0);

            if (OPTIONS & kOptionBandingMode) {
              if (_ey0 > _ey1)
                return true;
              _yDlt += _yLift;
              acc_err_step(_yDlt, _yErr, _yRem, _dx);
              return false;
            }
            else {
              // If this was the only scanline (first and last at the same time) it would end here.
              return true;
            }
          }

          // All scanlines between [_ey0:_ey1], exclusive.
          while (--i) {
            _xDlt = _xLift;
            acc_err_step(_xDlt, _xErr, _xRem, _dy);

            area = uint32_t(_fx0);
            _fx0 += _xDlt;
            bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

            if (_fx0 <= int(A8Info::kScale)) {
              bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
              area = full_cover * (area + uint32_t(_fx0));

              cell_merge(cell_ptr, _ex0, full_cover, area);
              cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

              if (_fx0 < int(A8Info::kScale))
                continue;

              _ex0++;
              _fx0 = 0;
            }
            else {
              _fx0 &= A8Info::kMask;
              _yDlt &= A8Info::kMask;

              cov0 = apply_sign_mask(uint32_t(_yDlt));
              cov1 = cov0 * (area + A8Info::kScale);

              cov0 = IntOps::shl(cov0, 9) - cov1;
              cell_add(cell_ptr, _ex0 + 0, cov0);
              _ex0++;

              cov0 = apply_sign_mask(A8Info::kScale - uint32_t(_yDlt));
              area = cov0 * uint32_t(_fx0);

              cov0 = IntOps::shl(cov0, 9) - area + cov1;
              cell_add(cell_ptr, _ex0 + 0, cov0);
              cell_add(cell_ptr, _ex0 + 1, area);
              cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);
              bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
            }

            _yDlt += _yLift;
            acc_err_step(_yDlt, _yErr, _yRem, _dx);
          }

          if (OPTIONS & kOptionBandingMode) {
            // Last scanline, we will do it either now or in the next band (border-case).
            if (_ey0 >= _ey1) {
              _fy1 = _savedFy1;
              _xDlt = ((_ex1 - _ex0) << A8Info::kShift) + _fx1 - _fx0;
              BL_ASSERT(_xDlt >= 0);

              // Border case, last scanline is the first scanline in the next band.
              if (_ey0 == _ey1) {
                updateMaxX<OPTIONS>(_ex0);
                return false;
              }
            }
            else {
              updateMaxX<OPTIONS>(_ex0);
              _xDlt = _xLift;
              acc_err_step(_xDlt, _xErr, _xRem, _dy);
              return false;
            }
          }
          else {
            // Prepare the last scanline.
            _fy1 = _savedFy1;
            _xDlt = ((_ex1 - _ex0) << A8Info::kShift) + _fx1 - _fx0;
          }
        }
      }
    }
    else {
      // Since both first and last scanlines are special we set `i` to one and then repeatedly to number of scanlines
      // in the middle, and then to `1` again for the last one. Since this is a horizontally oriented line this
      // overhead is fine and makes the rasterizer cleaner.
      size_t j = 1;
      int x_local = (_ex0 << A8Info::kShift) + _fx0;

      uint32_t cover;
      uint32_t area;

      if (_flags & kFlagRightToLeft) {
        // .........
        // ......xxx
        // ..xxxxx..
        // xxx......
        // .........
        updateMaxX<OPTIONS>(_ex0);

        if (_flags & kFlagInitialScanline) {
          _flags &= ~kFlagInitialScanline;

          j = i;
          i = 1;

          cover = apply_sign_mask(uint32_t(_yDlt - _fy0));
          BL_ASSERT(int32_t(cover) >= -int32_t(A8Info::kScale) && int32_t(cover) <= int32_t(A8Info::kScale));

          if (_fx0 - _xDlt < 0)
            goto HorzRightToLeftInside;

          x_local -= _xDlt;
          bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

          cover = apply_sign_mask(uint32_t(_fy1 - _fy0));
          area = cover * uint32_t(_fx0 * 2 - _xDlt);
          cell_merge(cell_ptr, _ex0, cover, area);

          if ((x_local & int(A8Info::kMask)) == 0) {
            _yDlt += _yLift;
            acc_err_step(_yDlt, _yErr, _yRem, _dx);
          }

          _xDlt = _xLift;
          acc_err_step(_xDlt, _xErr, _xRem, _dy);

          bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
          cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

          i--;
        }

        for (;;) {
          while (i) {
            _ex0 = ((x_local - 1) >> A8Info::kShift);
            _fx0 = ((x_local - 1) & int(A8Info::kMask)) + 1;

HorzRightToLeftSkip:
            _yDlt -= int(A8Info::kScale);
            cover = apply_sign_mask(uint32_t(_yDlt));
            BL_ASSERT(int32_t(cover) >= -int32_t(A8Info::kScale) && int32_t(cover) <= int32_t(A8Info::kScale));

HorzRightToLeftInside:
            x_local -= _xDlt;
            {
              int ex_local = x_local >> A8Info::kShift;
              int fx_local = x_local & int(A8Info::kMask);

              bit_fill<OPTIONS>(bit_ptr, unsigned(ex_local) / BL_PIPE_PIXELS_PER_ONE_BIT, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);
              area = cover * uint32_t(_fx0);

              while (_ex0 != ex_local) {
                cell_merge(cell_ptr, _ex0, cover, area);

                cover = uint32_t(_yLift);
                acc_err_step(cover, _yErr, _yRem, _dx);
                _yDlt += int32_t(cover);

                cover = apply_sign_mask(cover);
                area = cover * A8Info::kScale;

                _ex0--;
              }

              cover += apply_sign_mask(uint32_t(_fy1 - _yDlt));
              area = cover * (uint32_t(fx_local) + A8Info::kScale);
              cell_merge(cell_ptr, _ex0, cover, area);

              if (fx_local == 0) {
                _yDlt += _yLift;
                acc_err_step(_yDlt, _yErr, _yRem, _dx);
              }
            }

            _xDlt = _xLift;
            acc_err_step(_xDlt, _xErr, _xRem, _dy);

            bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
            cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

            i--;
          }

          _fy0 = 0;
          _fy1 = A8Info::kScale;

          if (OPTIONS & kOptionBandingMode) {
            if (!j) {
              updateMinX<OPTIONS>(_ex0);
              _ex0 = ((x_local - 1) >> A8Info::kShift);
              _fx0 = ((x_local - 1) & int(A8Info::kMask)) + 1;
              return _ey0 > _ey1;
            }
          }
          else {
            if (!j) {
              updateMinX<OPTIONS>(_ex0);
              return true;
            }
          }

          i = j - 1;
          j = 1;

          if (!i) {
            i = 1;
            j = 0;

            bool is_last = (OPTIONS & kOptionBandingMode) ? _ey0 > _ey1 : true;
            if (!is_last) continue;

            _xDlt = x_local - ((_ex1 << A8Info::kShift) + _fx1);
            _fy1 = _savedFy1;

            _ex0 = ((x_local - 1) >> A8Info::kShift);
            _fx0 = ((x_local - 1) & int(A8Info::kMask)) + 1;

            if (_fx0 - _xDlt >= 0) {
              cover = apply_sign_mask(uint32_t(_fy1));
              area = cover * uint32_t(_fx0 * 2 - _xDlt);

              cell_merge(cell_ptr, _ex0, cover, area);
              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

              updateMinX<OPTIONS>(_ex0);
              return true;
            }

            goto HorzRightToLeftSkip;
          }
        }
      }
      else {
        // .........
        // xxx......
        // ..xxxxx..
        // ......xxx
        // .........
        updateMinX<OPTIONS>(_ex0);

        if (_flags & kFlagInitialScanline) {
          _flags &= ~kFlagInitialScanline;

          j = i;
          i = 1;

          cover = apply_sign_mask(uint32_t(_yDlt - _fy0));
          if (_fx0 + _xDlt > int(A8Info::kScale))
            goto HorzLeftToRightInside;

          x_local += _xDlt;
          bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

          // First scanline is only a single pixel, we deal with it here as it's a special case.
          cover = apply_sign_mask(uint32_t(_fy1 - _fy0));
          area = cover * (uint32_t(_fx0) * 2 + uint32_t(_xDlt));
          cell_merge(cell_ptr, _ex0, cover, area);

          if (_fx0 + _xDlt == int(A8Info::kScale)) {
            _yDlt += _yLift;
            acc_err_step(_yDlt, _yErr, _yRem, _dx);
          }

          _xDlt = _xLift;
          acc_err_step(_xDlt, _xErr, _xRem, _dy);

          bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
          cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

          i--;
        }

        for (;;) {
          while (i) {
            _ex0 = x_local >> A8Info::kShift;
            _fx0 = x_local & int(A8Info::kMask);

HorzLeftToRightSkip:
            //_yDlt &= int(A8Info::kMask);
            _yDlt -= int(A8Info::kScale);
            cover = apply_sign_mask(uint32_t(_yDlt));
            BL_ASSERT(int32_t(cover) >= -int32_t(A8Info::kScale) && int32_t(cover) <= int32_t(A8Info::kScale));

HorzLeftToRightInside:
            x_local += _xDlt;
            {
              BL_ASSERT(_ex0 != (x_local >> A8Info::kShift));

              int ex_local = (x_local - 1) >> A8Info::kShift;
              int fx_local = ((x_local - 1) & int(A8Info::kMask)) + 1;

              bit_fill<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT, unsigned(ex_local) / BL_PIPE_PIXELS_PER_ONE_BIT);
              area = cover * (uint32_t(_fx0) + A8Info::kScale);

              while (_ex0 != ex_local) {
                cell_merge(cell_ptr, _ex0, cover, area);

                cover = uint32_t(_yLift);
                acc_err_step(cover, _yErr, _yRem, _dx);
                _yDlt += int32_t(cover);

                cover = apply_sign_mask(cover);
                area = cover * A8Info::kScale;

                _ex0++;
              }

              cover += apply_sign_mask(uint32_t(_fy1 - _yDlt));
              area = cover * uint32_t(fx_local);
              cell_merge(cell_ptr, _ex0, cover, area);

              if (fx_local == A8Info::kScale) {
                _yDlt += _yLift;
                acc_err_step(_yDlt, _yErr, _yRem, _dx);
              }
            }

            _xDlt = _xLift;
            acc_err_step(_xDlt, _xErr, _xRem, _dy);

            bit_ptr = PtrOps::offset(bit_ptr, bit_stride<OPTIONS>());
            cell_ptr = PtrOps::offset(cell_ptr, cell_stride());

            i--;
          }

          _fy0 = 0;
          _fy1 = A8Info::kScale;

          if (OPTIONS & kOptionBandingMode) {
            if (!j) {
              updateMaxX<OPTIONS>(_ex0);
              _ex0 = x_local >> A8Info::kShift;
              _fx0 = x_local & int(A8Info::kMask);
              return _ey0 > _ey1;
            }
          }
          else {
            if (!j) {
              updateMaxX<OPTIONS>(_ex0);
              return true;
            }
          }

          i = j - 1;
          j = 1;

          if (!i) {
            i = 1;
            j = 0;

            bool is_last = (OPTIONS & kOptionBandingMode) ? _ey0 > _ey1 : true;
            if (!is_last) continue;

            _xDlt = ((_ex1 << A8Info::kShift) + _fx1) - x_local;
            _fy1 = _savedFy1;

            _ex0 = x_local >> A8Info::kShift;
            _fx0 = x_local & int(A8Info::kMask);

            if (_fx0 + _xDlt <= int(A8Info::kScale)) {
              cover = apply_sign_mask(uint32_t(_fy1));
              area = cover * (uint32_t(_fx0) * 2 + uint32_t(_xDlt));

              cell_merge(cell_ptr, _ex0, cover, area);
              bit_set<OPTIONS>(bit_ptr, unsigned(_ex0) / BL_PIPE_PIXELS_PER_ONE_BIT);

              updateMaxX<OPTIONS>(_ex0);
              return true;
            }

            goto HorzLeftToRightSkip;
          }
        }
      }
    }
  }

  //! \}

  //! \name Min/Max Helpers
  //! \{

  template<uint32_t OPTIONS, typename T>
  BL_INLINE void updateMinX(const T& x) noexcept {
    if (OPTIONS & kOptionRecordMinXMaxX)
      _cellMinX = bl_min(_cellMinX, unsigned(x));
  }

  template<uint32_t OPTIONS, typename T>
  BL_INLINE void updateMaxX(const T& x) noexcept {
    if (OPTIONS & kOptionRecordMinXMaxX)
      _cellMaxX = bl_max(_cellMaxX, unsigned(x));
  }

  //! \}

  //! \name Cell Helpers
  //! \{

  template<typename T>
  BL_INLINE T apply_sign_mask(T cover) const noexcept {
    return AnalyticUtils::apply_sign_mask(cover, _sign_mask);
  }

  template<typename X>
  BL_INLINE void cell_add(uint32_t* cell_ptr, X x, uint32_t value) const noexcept {
    BL_ASSERT(x >= 0);

    using U = std::make_unsigned_t<X>;
    cell_ptr[size_t(U(x))] += value;
  }

  template<typename X>
  BL_INLINE void cell_merge(uint32_t* cell_ptr, X x, uint32_t cover, uint32_t area) const noexcept {
    BL_ASSERT(x >= 0);

    using U = std::make_unsigned_t<X>;

    cell_ptr[size_t(U(x)) + 0] += IntOps::shl(cover, 9) - area;
    cell_ptr[size_t(U(x)) + 1] += area;
  }

  //! \}

  //! \name Shadow Bit-Array Helpers
  //! \{

  //! Set bit `x` to 1 in a bit-vector starting at `bit_ptr`.
  template<uint32_t OPTIONS, typename X>
  BL_INLINE void bit_set(BLBitWord* bit_ptr, X x) const noexcept {
    using U = std::make_unsigned_t<X>;

    if (OPTIONS & kOptionEasyBitStride)
      bit_ptr[0] |= BitOps::index_as_mask(U(x));
    else
      BitOps::bit_array_set_bit(bit_ptr, U(x));
  }

  //! Fill bits between `first` and `last` (inclusive) in a bit-vector starting at `bit_ptr`.
  template<uint32_t OPTIONS, typename X>
  BL_INLINE BL_FLATTEN void bit_fill(BLBitWord* bit_ptr, X first, X last) const noexcept {
    BL_ASSERT(first <= last);

    using U = std::make_unsigned_t<X>;

    if (OPTIONS & kOptionEasyBitStride) {
      BL_ASSERT(first < BitOps::kNumBits);
      BL_ASSERT(last < BitOps::kNumBits);

      bit_ptr[0] |= BitOps::shift_to_end(BitOps::ones(), U(first)) ^
                   BitOps::shift_to_end(BitOps::ones() ^ BitOps::index_as_mask(0), U(last));
    }
    else {
      size_t idx_cur = U(first) / BitOps::kNumBits;
      size_t idx_end = U(last) / BitOps::kNumBits;

      BLBitWord mask = BitOps::shift_to_end(BitOps::ones(), U(first) % BitOps::kNumBits);
      if (idx_cur != idx_end) {
        bit_ptr[idx_cur] |= mask;
        mask = BitOps::ones();

        BL_NOUNROLL
        while (++idx_cur != idx_end)
          bit_ptr[idx_cur] = mask;
      }

      mask ^= BitOps::shift_to_end(BitOps::ones() ^ BitOps::index_as_mask(0), U(last) % BitOps::kNumBits);
      bit_ptr[idx_cur] |= mask;
    }
  }

  //! \}
};

} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_ANALYTICRASTERIZER_P_H_INCLUDED
