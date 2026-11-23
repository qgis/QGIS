// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_DEBUGGING_P_H_INCLUDED
#define BLEND2D_RASTER_DEBUGGING_P_H_INCLUDED

#include <blend2d/core/api-internal_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/support/traits_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {
namespace Debugging {

static void debug_edges(EdgeStorage<int>* edge_storage) noexcept {
  EdgeList<int>* edge_list = edge_storage->band_edges();
  size_t count = edge_storage->band_count();
  uint32_t band_height = edge_storage->band_height();

  int min_x = Traits::max_value<int>();
  int min_y = Traits::max_value<int>();
  int max_x = Traits::min_value<int>();
  int max_y = Traits::min_value<int>();

  const BLBoxI& bb = edge_storage->bounding_box();
  printf("EDGE STORAGE [%d.%d %d.%d %d.%d %d.%d]:\n",
         bb.x0 >> 8, bb.x0 & 0xFF,
         bb.y0 >> 8, bb.y0 & 0xFF,
         bb.x1 >> 8, bb.x1 & 0xFF,
         bb.y1 >> 8, bb.y1 & 0xFF);

  for (size_t band_id = 0; band_id < count; band_id++) {
    EdgeVector<int>* edge = edge_list[band_id].first();
    if (edge) {
      printf("BAND #%d y={%d:%d}\n", int(band_id), int(band_id * band_height), int((band_id + 1) * band_height - 1));
      while (edge) {
        printf("  EDGES {sign=%u count=%u}", unsigned(edge->sign_bit), unsigned(edge->count));

        if (edge->count <= 1)
          printf("{WRONG COUNT!}");

        EdgePoint<int>* ptr = edge->pts;
        EdgePoint<int>* ptr_start = ptr;
        EdgePoint<int>* ptr_end = edge->pts + edge->count;

        while (ptr != ptr_end) {
          min_x = bl_min(min_x, ptr[0].x);
          min_y = bl_min(min_y, ptr[0].y);
          max_x = bl_max(max_x, ptr[0].x);
          max_y = bl_max(max_y, ptr[0].y);

          printf(" [%d.%d, %d.%d]", ptr[0].x >> 8, ptr[0].x & 0xFF, ptr[0].y >> 8, ptr[0].y & 0xFF);

          if (ptr != ptr_start && ptr[-1].y > ptr[0].y)
            printf(" !INVALID! ");

          ptr++;
        }

        printf("\n");
        edge = edge->next;
      }
    }
  }

  printf("EDGE STORAGE BBOX [%d.%d, %d.%d] -> [%d.%d, %d.%d]\n\n",
    min_x >> 8, min_x & 0xFF,
    min_y >> 8, min_y & 0xFF,
    max_x >> 8, max_x & 0xFF,
    max_y >> 8, max_y & 0xFF);
}

} // {Debugging}
} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_DEBUGGING_P_H_INCLUDED
