// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERCOMMANDPROCSYNC_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERCOMMANDPROCSYNC_P_H_INCLUDED

#include <blend2d/core/context_p.h>
#include <blend2d/geometry/commons_p.h>
#include <blend2d/pipeline/pipedefs_p.h>
#include <blend2d/raster/analyticrasterizer_p.h>
#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/rasterdefs_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/support/arenaallocator_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/zeroallocator_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {
namespace CommandProcSync {

static BL_INLINE BLResult fill_box_a(WorkData& work_data, const Pipeline::DispatchData& dispatch_data, uint32_t alpha, const BLBoxI& box_a, const void* fetch_data) noexcept {
  Pipeline::FillData fill_data;
  fill_data.init_box_a_8bpc(alpha, box_a.x0, box_a.y0, box_a.x1, box_a.y1);

  Pipeline::FillFunc fill_func = dispatch_data.fill_func;
  Pipeline::FetchFunc fetch_func = dispatch_data.fetch_func;

  if (fetch_func == nullptr) {
    fill_func(&work_data.ctx_data, &fill_data, fetch_data);
  }
  else {
    // TODO:
  }

  return BL_SUCCESS;
}

static BL_INLINE BLResult fill_box_u(WorkData& work_data, const Pipeline::DispatchData& dispatch_data, uint32_t alpha, const BLBoxI& box_u, const void* fetch_data) noexcept {
  Pipeline::FillData fill_data;
  Pipeline::BoxUToMaskData boxUToMaskData;

  if (!fill_data.init_box_u_8bpc_24x8(alpha, box_u.x0, box_u.y0, box_u.x1, box_u.y1, boxUToMaskData))
    return BL_SUCCESS;

  Pipeline::FillFunc fill_func = dispatch_data.fill_func;
  Pipeline::FetchFunc fetch_func = dispatch_data.fetch_func;

  if (fetch_func == nullptr) {
    fill_func(&work_data.ctx_data, &fill_data, fetch_data);
  }
  else {
    // TODO:
  }

  return BL_SUCCESS;
}

static BL_INLINE BLResult fill_box_masked_a(WorkData& work_data, const Pipeline::DispatchData& dispatch_data, uint32_t alpha, const RenderCommand::FillBoxMaskA& payload, const void* fetch_data) noexcept {
  const BLImageImpl* mask_impl = payload.mask_image_i.ptr;
  const BLPointI& mask_offset = payload.mask_offset_i;
  const uint8_t* mask_data = static_cast<const uint8_t*>(mask_impl->pixel_data) + mask_impl->stride * intptr_t(mask_offset.y) + uint32_t(mask_offset.x) * (mask_impl->depth / 8u);

  const BLBoxI& box_i = payload.box_i;

  Pipeline::MaskCommand mask_commands[2];
  Pipeline::MaskCommandType vMaskCmd = alpha >= 255 ? Pipeline::MaskCommandType::kVMaskA8WithGA : Pipeline::MaskCommandType::kVMaskA8WithoutGA;

  mask_commands[0].init_vmask(vMaskCmd, uint32_t(box_i.x0), uint32_t(box_i.x1), mask_data, mask_impl->stride);
  mask_commands[1].init_repeat();

  Pipeline::FillData fill_data;
  fill_data.init_mask_a(alpha, box_i.x0, box_i.y0, box_i.x1, box_i.y1, mask_commands);

  Pipeline::FillFunc fill_func = dispatch_data.fill_func;
  fill_func(&work_data.ctx_data, &fill_data, fetch_data);

  return BL_SUCCESS;
}

static BL_NOINLINE BLResult fill_analytic(WorkData& work_data, const Pipeline::DispatchData& dispatch_data, uint32_t alpha, const EdgeStorage<int>* edge_storage, BLFillRule fill_rule, const void* fetch_data) noexcept {
  // Rasterizer options to use - do not change unless you are improving the existing rasterizers.
  constexpr uint32_t kRasterizerOptions = AnalyticRasterizer::kOptionBandOffset | AnalyticRasterizer::kOptionRecordMinXMaxX;

  // Can only be called if there is something to fill.
  BL_ASSERT(edge_storage != nullptr);
  // Should have been verified by the caller.
  BL_ASSERT(edge_storage->bounding_box().y0 < edge_storage->bounding_box().y1);

  uint32_t band_height = edge_storage->band_height();
  uint32_t band_height_mask = band_height - 1;

  const uint32_t y_start = (uint32_t(edge_storage->bounding_box().y0)                          ) >> Pipeline::A8Info::kShift;
  const uint32_t y_end   = (uint32_t(edge_storage->bounding_box().y1) + Pipeline::A8Info::kMask) >> Pipeline::A8Info::kShift;

  size_t required_width = IntOps::align_up(uint32_t(work_data.dst_size().w) + 1u + BL_PIPE_PIXELS_PER_ONE_BIT, BL_PIPE_PIXELS_PER_ONE_BIT);
  size_t required_height = band_height;
  size_t cell_alignment = 16;

  size_t bit_stride = IntOps::word_count_from_bit_count<BLBitWord>(required_width / BL_PIPE_PIXELS_PER_ONE_BIT) * sizeof(BLBitWord);
  size_t cell_stride = required_width * sizeof(uint32_t);

  size_t bits_start = 0;
  size_t bits_size = required_height * bit_stride;

  size_t cells_start = IntOps::align_up(bits_start + bits_size, cell_alignment);
  BL_ASSERT(work_data.zero_buffer.size >= cells_start + required_height * cell_stride);

  AnalyticCellStorage cell_storage;
  cell_storage.init(
    reinterpret_cast<BLBitWord*>(work_data.zero_buffer.data + bits_start), bit_stride,
    IntOps::align_up(reinterpret_cast<uint32_t*>(work_data.zero_buffer.data + cells_start), cell_alignment), cell_stride);

  AnalyticActiveEdge<int>* active = nullptr;
  AnalyticActiveEdge<int>* pooled = nullptr;

  EdgeList<int>* band_edges = edge_storage->band_edges();
  uint32_t band_id = edge_storage->bandStartFromBBox();
  uint32_t band_end = edge_storage->bandEndFromBBox();

  uint32_t dst_width = uint32_t(work_data.dst_size().w);

  Pipeline::FillFunc fill_func = dispatch_data.fill_func;
  Pipeline::FillData fill_data;

  fill_data.init_analytic(alpha,
                        uint32_t(fill_rule),
                        cell_storage.bit_ptr_top, cell_storage.bit_stride,
                        cell_storage.cell_ptr_top, cell_storage.cell_stride);

  AnalyticRasterizer ras;
  ras.init(cell_storage.bit_ptr_top, cell_storage.bit_stride,
           cell_storage.cell_ptr_top, cell_storage.cell_stride,
           band_id * band_height, band_height);
  ras._band_offset = y_start;

  ArenaAllocator* work_zone = &work_data.work_zone;
  do {
    EdgeVector<int>* edges = band_edges[band_id].first();
    band_edges[band_id].reset();

    AnalyticActiveEdge<int>** pPrev = &active;
    AnalyticActiveEdge<int>* current = *pPrev;

    ras.reset_bounds();
    ras._band_end = bl_min((band_id + 1) * band_height, y_end) - 1;

    while (current) {
      ras.restore(current->state);
      ras.set_sign_mask_from_bit(current->sign_bit);

      for (;;) {
Rasterize:
        if (ras.template rasterize<kRasterizerOptions | AnalyticRasterizer::kOptionBandingMode>()) {
          // The edge is fully rasterized.
          const EdgePoint<int>* pts = current->cur;
          while (pts != current->end) {
            pts++;
            if (!ras.prepare(pts[-2], pts[-1]))
              continue;

            current->cur = pts;
            if (uint32_t(ras._ey0) <= ras._band_end)
              goto Rasterize;
            else
              goto SaveState;
          }

          AnalyticActiveEdge<int>* old = current;
          current = current->next;

          old->next = pooled;
          pooled = old;
          break;
        }

SaveState:
        // The edge is not fully rasterized and crosses the band.
        ras.save(current->state);

        *pPrev = current;
        pPrev = &current->next;
        current = *pPrev;
        break;
      }
    }

    if (edges) {
      if (!pooled) {
        pooled = static_cast<AnalyticActiveEdge<int>*>(work_zone->alloc(sizeof(AnalyticActiveEdge<int>)));
        if (BL_UNLIKELY(!pooled))
          return bl_make_error(BL_ERROR_OUT_OF_MEMORY);
        pooled->next = nullptr;
      }

      do {
        const EdgePoint<int>* pts = edges->pts + 1;
        const EdgePoint<int>* end = edges->pts + edges->count();

        uint32_t sign_bit = edges->sign_bit();
        ras.set_sign_mask_from_bit(sign_bit);

        edges = edges->next;
        do {
          pts++;
          if (!ras.prepare(pts[-2], pts[-1]))
            continue;

          if (uint32_t(ras._ey1) <= ras._band_end) {
            ras.template rasterize<kRasterizerOptions>();
          }
          else {
            current = pooled;
            pooled = current->next;

            current->sign_bit = sign_bit;
            current->cur = pts;
            current->end = end;
            current->next = nullptr;

            if (uint32_t(ras._ey0) <= ras._band_end)
              goto Rasterize;
            else
              goto SaveState;
          }
        } while (pts != end);
      } while (edges);
    }

    // Makes `active` or the last `AnalyticActiveEdge->next` null. It's important, because we don't unlink during
    // edge pooling as it's just faster to do it here.
    *pPrev = nullptr;

    if (ras.has_bounds()) {
      fill_data.analytic.box.x0 = int(ras._cellMinX);
      fill_data.analytic.box.x1 = int(bl_min(dst_width, IntOps::align_up(ras._cellMaxX + 1, BL_PIPE_PIXELS_PER_ONE_BIT)));
      fill_data.analytic.box.y0 = int(ras._band_offset);
      fill_data.analytic.box.y1 = int(ras._band_end) + 1;

      fill_func(&work_data.ctx_data, &fill_data, fetch_data);
    }

    ras._band_offset = (ras._band_offset + band_height) & ~band_height_mask;
  } while (++band_id < band_end);

  work_zone->clear();
  return BL_SUCCESS;
}

} // {CommandProcSync}
} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERCOMMANDPROCSYNC_P_H_INCLUDED
