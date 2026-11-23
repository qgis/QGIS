// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_test_p.h>
#if defined(BL_TEST)

#include <blend2d/core/random.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/raster/analyticrasterizer_p.h>
#include <blend2d/support/intops_p.h>

// bl::RasterEngine - AnalyticRasterizer - Tests
// =============================================

namespace bl::RasterEngine {

static bool check_rasterizer_state(const AnalyticState& a, const AnalyticState& b) noexcept {
  int yDltMask = (a._dy >= a._dx) ? 255 : -1;

  return a._dx   == b._dx   && a._dy   == b._dy   &&
         a._ex0  == b._ex0  && a._ey0  == b._ey0  &&
         a._ex1  == b._ex1  && a._ey1  == b._ey1  &&
         a._fx0  == b._fx0  && a._fy0  == b._fy0  &&
         a._fx1  == b._fx1  && a._fy1  == b._fy1  &&
         a._xErr == b._xErr && a._yErr == b._yErr &&
         a._xDlt == b._xDlt && (a._yDlt & yDltMask) == (b._yDlt & yDltMask) &&
         a._xRem == b._xRem && a._yRem == b._yRem &&
         a._xLift == b._xLift && a._yLift == b._yLift &&
         a._savedFy1 == b._savedFy1;
}

UNIT(analytic_rasterizer, BL_TEST_GROUP_RENDERING_UTILITIES) {
  int w = 1000;
  int h = 1000;

  typedef Pipeline::A8Info A8Info;

  uint32_t max_band_height = 64;
  uint32_t edge_count = BrokenAPI::has_arg("--quick") ? 5000 : 100000;

  for (uint32_t band_height_shift = 0; band_height_shift < IntOps::ctz(max_band_height); band_height_shift++) {
    uint32_t band_height = 1 << band_height_shift;
    INFO("Testing advanceToY() correctness [band_height=%u]", band_height);

    // TODO: Wrap this logic into something, now it's duplicated 3x.
    size_t required_width = IntOps::align_up(uint32_t(w) + 1u + BL_PIPE_PIXELS_PER_ONE_BIT, BL_PIPE_PIXELS_PER_ONE_BIT);
    size_t required_height = band_height;
    size_t cell_alignment = 16;

    size_t bit_stride = IntOps::word_count_from_bit_count<BLBitWord>(required_width / BL_PIPE_PIXELS_PER_ONE_BIT) * sizeof(BLBitWord);
    size_t cell_stride = required_width * sizeof(uint32_t);

    size_t bits_start = 0;
    size_t bits_size = required_height * bit_stride;

    size_t cells_start = IntOps::align_up(bits_start + bits_size, cell_alignment);
    size_t cells_size = required_height * cell_stride;

    uint8_t* buffer = static_cast<uint8_t*>(calloc(bits_size + cells_size + cell_alignment, 1));
    EXPECT_NE(buffer, nullptr);

    AnalyticCellStorage cell_storage;
    cell_storage.init(
      reinterpret_cast<BLBitWord*>(buffer + bits_start), bit_stride,
      IntOps::align_up(reinterpret_cast<uint32_t*>(buffer + cells_start), cell_alignment), cell_stride);

    BLRandom rnd(0x1234);
    for (uint32_t i = 0; i < edge_count; i++) {
      int x0 = int(rnd.next_double() * double(w) * double(A8Info::kScale));
      int y0 = int(rnd.next_double() * double(h) * double(A8Info::kScale));
      int x1 = int(rnd.next_double() * double(w) * double(A8Info::kScale));
      int y1 = int(rnd.next_double() * double(h) * double(A8Info::kScale));

      // At least one horizontal line.
      if (i == 0)
        x1 = x0;

      if (y0 > y1) {
        BLInternal::swap(x0, x1);
        BLInternal::swap(y0, y1);
      }

      y1 += int((band_height + 1) * A8Info::kScale);
      if (y1 > (h << A8Info::kShift))
        y1 = (h << A8Info::kShift);

      uint32_t bandY0 = uint32_t(y0 >> A8Info::kShift);
      uint32_t bandY1 = bandY0 + band_height;

      AnalyticRasterizer a;
      AnalyticRasterizer b;

      // We don't really care of the cell storage here, can be the same...
      a.init(cell_storage.bit_ptr_top, cell_storage.bit_stride,
             cell_storage.cell_ptr_top, cell_storage.cell_stride, bandY0, band_height);
      b.init(cell_storage.bit_ptr_top, cell_storage.bit_stride,
             cell_storage.cell_ptr_top, cell_storage.cell_stride, bandY0, band_height);

      bool a_prepared = a.prepare_ref(EdgePoint<int>{x0, y0}, EdgePoint<int>{x1, y1});
      bool b_prepared = b.prepare(EdgePoint<int>{x0, y0}, EdgePoint<int>{x1, y1});

      bool prepare_must_match = check_rasterizer_state(a, b);
      EXPECT_TRUE(prepare_must_match)
        .message("Rasterizer preparation failed [TestId=%u]:\n"
                 "    Line: int x0=%d, y0=%d, x1=%d, y1=%d;\n"
                 "    A: x0={%d.%d} y0={%d.%d} x1={%d.%d} y1={%d.%d} err={%d|%d} dlt={%d|%d} rem={%d|%d} lift={%d|%d} dx|dy={%d|%d}\n"
                 "    B: x0={%d.%d} y0={%d.%d} x1={%d.%d} y1={%d.%d} err={%d|%d} dlt={%d|%d} rem={%d|%d} lift={%d|%d} dx|dy={%d|%d}",
                 i,
                 x0, y0, x1, y1,
                 a._ex0, a._fx0, a._ey0, a._fy0, a._ex1, a._fx1, a._ey1, a._fy1, a._xErr, a._yErr, a._xDlt, a._yDlt, a._xRem, a._yRem, a._xLift, a._yLift, a._dx, a._dy,
                 b._ex0, b._fx0, b._ey0, b._fy0, b._ex1, b._fx1, b._ey1, b._fy1, b._xErr, b._yErr, b._xDlt, b._yDlt, b._xRem, b._yRem, b._xLift, b._yLift, b._dx, b._dy);

      EXPECT_TRUE(a_prepared);
      EXPECT_TRUE(b_prepared);

      constexpr uint32_t kRasterizerOptions =
        AnalyticRasterizer::kOptionBandOffset |
        AnalyticRasterizer::kOptionBandingMode;

      uint32_t iteration = 0;
      for (;;) {
        bool is_finished = a.template rasterize<kRasterizerOptions>();

        // We cannot advance beyond the end of the edge.
        if (is_finished)
          break;

        b.advanceToY(int(bandY1));
        bool states_must_match = check_rasterizer_state(a, b);

        EXPECT_TRUE(states_must_match)
          .message("Rasterizer states are different [TestId=%u, Iteration=%u, BandY0=%u, BandY1=%u]:\n"
                   "    Line: int x0=%d, y0=%d, x1=%d, y1=%d;\n"
                   "    A: x0={%d.%d} y0={%d.%d} x1={%d.%d} y1={%d.%d} err={%d|%d} dlt={%d|%d} rem={%d|%d} lift={%d|%d} dx|dy={%d|%d}\n"
                   "    B: x0={%d.%d} y0={%d.%d} x1={%d.%d} y1={%d.%d} err={%d|%d} dlt={%d|%d} rem={%d|%d} lift={%d|%d} dx|dy={%d|%d}",
                   i, iteration, bandY0, bandY1,
                   x0, y0, x1, y1,
                   a._ex0, a._fx0, a._ey0, a._fy0, a._ex1, a._fx1, a._ey1, a._fy1, a._xErr, a._yErr, a._xDlt, a._yDlt, a._xRem, a._yRem, a._xLift, a._yLift, a._dx, a._dy,
                   b._ex0, b._fx0, b._ey0, b._fy0, b._ex1, b._fx1, b._ey1, b._fy1, b._xErr, b._yErr, b._xDlt, b._yDlt, b._xRem, b._yRem, b._xLift, b._yLift, b._dx, b._dy);

        bandY0 = bandY1;
        bandY1 += band_height;

        a._band_offset = bandY0;
        a._band_end = bl_min(bandY1 - 1, uint32_t((y1 - 1) >> A8Info::kShift));

        b._band_offset = a._band_offset;
        b._band_end = a._band_end;

        iteration++;
      }
    }

    free(buffer);
  }
}

} // {bl::RasterEngine}

#endif // BL_TEST
