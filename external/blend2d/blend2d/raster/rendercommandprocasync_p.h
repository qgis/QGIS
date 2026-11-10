// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_RASTER_RENDERCOMMANDPROCASYNC_P_H_INCLUDED
#define BLEND2D_RASTER_RENDERCOMMANDPROCASYNC_P_H_INCLUDED

#include <blend2d/raster/edgebuilder_p.h>
#include <blend2d/raster/renderbatch_p.h>
#include <blend2d/raster/rendercommandprocsync_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_raster_engine_impl
//! \{

namespace bl::RasterEngine {
namespace CommandProcAsync {

enum class CommandStatus : uint32_t {
  kContinue = 0,
  kDone = 1
};

struct SlotData {
  struct Analytic {
    const EdgeVector<int>* edges;
    AnalyticActiveEdge<int>* active;
  };

  union {
    Analytic analytic;
  };
};

class ProcData {
public:
  typedef PrivateBitWordOps BitOps;

  //! \name Members
  //! \{

  WorkData* _work_data;
  RenderBatch* _batch;

  uint32_t _bandY0;
  uint32_t _bandY1;
  uint32_t _bandFixedY0;
  uint32_t _bandFixedY1;

  SlotData* _state_slot_data;
  size_t _state_slot_count;

  BLBitWord* _pending_command_bit_set_data;
  size_t _pending_command_bit_set_size;
  BLBitWord _pending_command_bit_set_mask;

  AnalyticActiveEdge<int>* _pooled_edges;

  //! \}

  //! \name Construction & Destruction
  //! \{

  BL_INLINE ProcData(WorkData* work_data, RenderBatch* batch) noexcept
    : _work_data(work_data),
      _batch(batch),
      _bandY0(0),
      _bandY1(0),
      _bandFixedY0(0),
      _bandFixedY1(0),
      _state_slot_data(nullptr),
      _state_slot_count(0),
      _pending_command_bit_set_data(nullptr),
      _pending_command_bit_set_size(0),
      _pending_command_bit_set_mask(0),
      _pooled_edges(nullptr) {}

  //! \}

  //! \name Initialization
  //! \{

  BL_INLINE BLResult init_proc_data() noexcept {
    size_t command_count = _batch->command_count();
    size_t state_slot_count = _batch->state_slot_count();

    size_t bit_word_count = IntOps::word_count_from_bit_count<BLBitWord>(command_count);
    size_t remaining_bits = command_count & (IntOps::bit_size_of<BLBitWord>() - 1);

    _state_slot_data = _work_data->work_zone.allocT<SlotData>(state_slot_count * sizeof(SlotData));
    _pending_command_bit_set_data = _work_data->work_zone.allocT<BLBitWord>(bit_word_count * sizeof(BLBitWord), sizeof(BLBitWord));

    if (!_state_slot_data || !_pending_command_bit_set_data)
      return bl_make_error(BL_ERROR_OUT_OF_MEMORY);

    _state_slot_count = state_slot_count;
    _pending_command_bit_set_size = bit_word_count;

    // Initialize the last BitWord as it can have bits that are outside of the command count. We rely on these bits,
    // they cannot be wrong...
    if (remaining_bits)
      _pending_command_bit_set_data[bit_word_count - 1] = BitOps::non_zero_start_mask(remaining_bits);
    else
      _pending_command_bit_set_data[bit_word_count - 1] = BitOps::ones();

    if (bit_word_count > 1)
      _pending_command_bit_set_mask = IntOps::all_ones<BLBitWord>();
    else
      _pending_command_bit_set_mask = 0;

    return BL_SUCCESS;
  }

  BL_INLINE void init_band(uint32_t band_id, uint32_t band_height, uint32_t fp_scale) noexcept {
    _bandY0 = band_id * band_height;
    _bandY1 = _bandY0 + band_height;
    _bandFixedY0 = _bandY0 * fp_scale;
    _bandFixedY1 = _bandY1 * fp_scale;
  }

  //! \}

  //! \name Accessors
  //! \{

  BL_INLINE WorkData* work_data() const noexcept { return _work_data; }
  BL_INLINE RenderBatch* batch() const noexcept { return _batch; }

  BL_INLINE uint32_t bandY0() const noexcept { return _bandY0; }
  BL_INLINE uint32_t bandY1() const noexcept { return _bandY1; }
  BL_INLINE uint32_t bandFixedY0() const noexcept { return _bandFixedY0; }
  BL_INLINE uint32_t bandFixedY1() const noexcept { return _bandFixedY1; }

  BL_INLINE BLBitWord* pending_command_bit_set_data() const noexcept { return _pending_command_bit_set_data; }
  BL_INLINE BLBitWord* pending_command_bit_set_end() const noexcept { return _pending_command_bit_set_data + _pending_command_bit_set_size; }
  BL_INLINE size_t pending_command_bit_set_size() const noexcept { return _pending_command_bit_set_size; }

  BL_INLINE BLBitWord pending_command_bit_set_mask() const noexcept { return _pending_command_bit_set_mask; }
  BL_INLINE void clear_pending_command_bit_set_mask() noexcept { _pending_command_bit_set_mask = 0; }

  BL_INLINE SlotData& state_data_at(size_t index) noexcept {
    BL_ASSERT(index < _state_slot_count);
    return _state_slot_data[index];
  }

  //! \}
};

static BL_INLINE CommandStatus fill_box_a(ProcData& proc_data, const RenderCommand& command) noexcept {
  int y0 = bl_max(command.box_i().y0, int(proc_data.bandY0()));
  int y1 = bl_min(command.box_i().y1, int(proc_data.bandY1()));

  if (y0 < y1) {
    Pipeline::FillData fill_data;
    fill_data.init_box_a_8bpc(command.alpha(), command.box_i().x0, y0, command.box_i().x1, y1);

    Pipeline::FillFunc fill_func = command.pipe_dispatch_data()->fill_func;
    Pipeline::FetchFunc fetch_func = command.pipe_dispatch_data()->fetch_func;
    const void* fetch_data = command.get_pipe_fetch_data();

    if (fetch_func == nullptr) {
      fill_func(&proc_data.work_data()->ctx_data, &fill_data, fetch_data);
    }
    else {
      // TODO:
    }
  }

  return CommandStatus(command.box_i().y1 <= int(proc_data.bandY1()));
}

static BL_INLINE CommandStatus fill_box_u(ProcData& proc_data, const RenderCommand& command) noexcept {
  int y0 = bl_max(command.box_i().y0, int(proc_data.bandFixedY0()));
  int y1 = bl_min(command.box_i().y1, int(proc_data.bandFixedY1()));

  if (y0 < y1) {
    Pipeline::FillData fill_data;
    Pipeline::BoxUToMaskData boxUToMaskData;

    if (fill_data.init_box_u_8bpc_24x8(command.alpha(), command.box_i().x0, y0, command.box_i().x1, y1, boxUToMaskData)) {
      Pipeline::FillFunc fill_func = command.pipe_dispatch_data()->fill_func;
      Pipeline::FetchFunc fetch_func = command.pipe_dispatch_data()->fetch_func;
      const void* fetch_data = command.get_pipe_fetch_data();

      if (fetch_func == nullptr) {
        fill_func(&proc_data.work_data()->ctx_data, &fill_data, fetch_data);
      }
      else {
        // TODO:
      }
    }
  }

  return CommandStatus(command.box_i().y1 <= int(proc_data.bandFixedY1()));
}

static CommandStatus fillBoxMaskA(ProcData& proc_data, const RenderCommand& command) noexcept {
  const RenderCommand::FillBoxMaskA& payload = command._payload.box_mask_a;
  const BLBoxI& box_i = payload.box_i;

  int y0 = bl_max(box_i.y0, int(proc_data.bandY0()));
  int y1 = bl_min(box_i.y1, int(proc_data.bandY1()));

  if (y0 < y1) {
    uint32_t maskX = uint32_t(payload.mask_offset_i.x);
    uint32_t maskY = uint32_t(payload.mask_offset_i.y) + uint32_t(y0 - box_i.y0);

    const BLImageImpl* mask_impl = payload.mask_image_i.ptr;
    const uint8_t* mask_data = (static_cast<const uint8_t*>(mask_impl->pixel_data) + intptr_t(maskY) * mask_impl->stride) + maskX * (mask_impl->depth / 8u);

    Pipeline::MaskCommand mask_commands[2];
    Pipeline::MaskCommandType vMaskCmd = command.alpha() >= 255 ? Pipeline::MaskCommandType::kVMaskA8WithoutGA : Pipeline::MaskCommandType::kVMaskA8WithGA;

    mask_commands[0].init_vmask(vMaskCmd, uint32_t(box_i.x0), uint32_t(box_i.x1), mask_data, mask_impl->stride);
    mask_commands[1].init_repeat();

    Pipeline::FillData fill_data;
    fill_data.init_mask_a(command.alpha(), box_i.x0, y0, box_i.x1, y1, mask_commands);

    Pipeline::FillFunc fill_func = command.pipe_dispatch_data()->fill_func;
    Pipeline::FetchFunc fetch_func = command.pipe_dispatch_data()->fetch_func;
    const void* fetch_data = command.get_pipe_fetch_data();

    if (fetch_func == nullptr) {
      fill_func(&proc_data.work_data()->ctx_data, &fill_data, fetch_data);
    }
    else {
      // TODO:
    }
  }

  return CommandStatus(box_i.y1 <= int(proc_data.bandY1()));
}

static CommandStatus fill_analytic(ProcData& proc_data, const RenderCommand& command, int32_t prevBandFy1, int32_t nextBandFy0) noexcept {
  // Rasterizer options to use - do not change unless you are improving the existing rasterizers.
  constexpr uint32_t kRasterizerOptions =
    AnalyticRasterizer::kOptionBandOffset     |
    AnalyticRasterizer::kOptionRecordMinXMaxX ;

  WorkData& work_data = *proc_data.work_data();
  SlotData::Analytic& proc_state = proc_data.state_data_at(command._payload.analytic.state_slot_index).analytic;

  uint32_t bandFixedY0 = proc_data.bandFixedY0();
  uint32_t bandFixedY1 = proc_data.bandFixedY1();

  const EdgeVector<int>* edges;
  AnalyticActiveEdge<int>* active;

  // TODO:
  bl_unused(nextBandFy0);

  {
    int32_t cmdFy0 = command._payload.analytic.fixed_y0;
    bool is_first_band = prevBandFy1 < cmdFy0;

    if (is_first_band) {
      // If it's the first band we have to initialize the state. This must be done only once per command.
      edges = command.analytic_edges();
      active = nullptr;

      proc_state.edges = edges;
      proc_state.active = active;

      // Everything clipped out, or all lines horizontal, etc...
      if (!edges)
        return CommandStatus::kDone;
    }
    else {
      // If the state has been already initialized, we have to take the remaining `edges` and `active` ones from it.
      edges = proc_state.edges;
      active = proc_state.active;
    }

    // Don't do anything if we haven't advanced enough.
    if (uint32_t(cmdFy0) >= bandFixedY1) {
      return CommandStatus::kContinue;
    }
  }

  uint32_t bandY0 = proc_data.bandY0();
  uint32_t bandY1 = proc_data.bandY1();
  uint32_t band_height = work_data.band_height();

  // TODO:
  /*
  if (BL_UNLIKELY(edge_storage->bounding_box().y0 >= edge_storage->bounding_box().y1))
    return BL_SUCCESS;
  */

  uint32_t dst_width = uint32_t(work_data.dst_size().w);
  size_t required_width = IntOps::align_up(dst_width + 1u + BL_PIPE_PIXELS_PER_ONE_BIT, BL_PIPE_PIXELS_PER_ONE_BIT);
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

  AnalyticActiveEdge<int>* pooled = proc_data._pooled_edges;

  Pipeline::FillData fill_data;
  fill_data.init_analytic(command.alpha(),
                        command.analytic_fill_rule(),
                        cell_storage.bit_ptr_top, cell_storage.bit_stride,
                        cell_storage.cell_ptr_top, cell_storage.cell_stride);

  AnalyticRasterizer ras;
  ras.init(cell_storage.bit_ptr_top, cell_storage.bit_stride,
           cell_storage.cell_ptr_top, cell_storage.cell_stride,
           bandY0, band_height);

  ArenaAllocator* work_zone = &work_data.work_zone;

  AnalyticActiveEdge<int>** pPrev = &active;
  AnalyticActiveEdge<int>* current = *pPrev;

  ras.reset_bounds();
  ras._band_end = bandY1 - 1;

  while (current) {
    // Skipped.
    ras.set_sign_mask_from_bit(current->sign_bit);
    if (current->state._ey1 < int(bandY0))
      goto EdgeDone;
    ras.restore(current->state);

    // Important - since we only process a single band here we have to skip into the correct band as it's not
    // guaranteed that the next band would be consecutive.
AdvanceY:
    ras.advanceToY(int(bandY0));

    for (;;) {
Rasterize:
      if (ras.template rasterize<kRasterizerOptions | AnalyticRasterizer::kOptionBandingMode>()) {
        // The edge is fully rasterized.
EdgeDone:
        const EdgePoint<int>* pts = current->cur;
        while (pts != current->end) {
          pts++;
          if (pts[-1].y <= int(bandFixedY0) || !ras.prepare(pts[-2], pts[-1]))
            continue;

          current->cur = pts;
          if (uint32_t(ras._ey0) > ras._band_end)
            goto SaveState;

          if (ras._ey0 < int(bandY0))
            goto AdvanceY;

          goto Rasterize;
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
      if (BL_UNLIKELY(!pooled)) {
        // Failed to allocate memory for the current edge.
        proc_data.work_data()->accumulate_error_flag(BL_CONTEXT_ERROR_FLAG_OUT_OF_MEMORY);
        return CommandStatus::kDone;
      }
      pooled->next = nullptr;
    }

    do {
      const EdgePoint<int>* pts = edges->pts + 1;
      const EdgePoint<int>* end = edges->pts + edges->count();

      if (pts[-1].y >= int(bandFixedY1))
        break;

      uint32_t sign_bit = edges->sign_bit();
      ras.set_sign_mask_from_bit(sign_bit);

      edges = edges->next;
      if (end[-1].y <= int(bandFixedY0))
        continue;

      do {
        pts++;
        if (pts[-1].y <= int(bandFixedY0) || !ras.prepare(pts[-2], pts[-1]))
          continue;

        ras.advanceToY(int(bandY0));
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

          if (uint32_t(ras._ey0) > ras._band_end)
            goto SaveState;

          if (ras._ey0 < int(bandY0))
            goto AdvanceY;

          goto Rasterize;
        }
      } while (pts != end);
    } while (edges);
  }

  // Makes `active` or the last `AnalyticActiveEdge->next` null. It's important, because we don't unlink during
  // edge pooling as it's just faster to do it here.
  *pPrev = nullptr;

  // Pooled active edges can be reused, we cannot return them to the allocator.
  proc_data._pooled_edges = pooled;
  proc_state.edges = edges;
  proc_state.active = active;

  if (ras.has_bounds()) {
    Pipeline::FillFunc fill_func = command.pipe_dispatch_data()->fill_func;
    Pipeline::FetchFunc fetch_func = command.pipe_dispatch_data()->fetch_func;
    const void* fetch_data = command.get_pipe_fetch_data();

    fill_data.analytic.box.x0 = int(ras._cellMinX);
    fill_data.analytic.box.x1 = int(bl_min(dst_width, IntOps::align_up(ras._cellMaxX + 1, BL_PIPE_PIXELS_PER_ONE_BIT)));
    fill_data.analytic.box.y0 = int(ras._band_offset);
    fill_data.analytic.box.y1 = int(ras._band_end) + 1;

    if (fetch_func == nullptr) {
      fill_func(&work_data.ctx_data, &fill_data, fetch_data);
    }
    else {
      // TODO:
    }
  }

  return CommandStatus(!edges && !active);
}

static CommandStatus process_command(ProcData& proc_data, const RenderCommand& command, int32_t prevBandFy1, int32_t nextBandFy0) noexcept {
  switch (command.type()) {
    case RenderCommandType::kFillBoxA:
      return fill_box_a(proc_data, command);

    case RenderCommandType::kFillBoxU:
      return fill_box_u(proc_data, command);

    case RenderCommandType::kFillAnalytic:
      return fill_analytic(proc_data, command, prevBandFy1, nextBandFy0);

    case RenderCommandType::kFillBoxMaskA:
      return fillBoxMaskA(proc_data, command);

    default:
      return CommandStatus::kDone;
  }
}

} // {CommandProcAsync}
} // {bl::RasterEngine}

//! \}
//! \endcond

#endif // BLEND2D_RASTER_RENDERCOMMANDPROCASYNC_P_H_INCLUDED
