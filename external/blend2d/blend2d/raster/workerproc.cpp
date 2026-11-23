// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/core/api-internal_p.h>
#include <blend2d/raster/rastercontext_p.h>
#include <blend2d/raster/rendercommand_p.h>
#include <blend2d/raster/rendercommandprocasync_p.h>
#include <blend2d/raster/renderjob_p.h>
#include <blend2d/raster/renderjobproc_p.h>
#include <blend2d/raster/workdata_p.h>
#include <blend2d/raster/workerproc_p.h>
#include <blend2d/raster/workersynchronization_p.h>
#include <blend2d/simd/simd_p.h>
#include <blend2d/support/bitops_p.h>
#include <blend2d/support/intops_p.h>

namespace bl::RasterEngine {
namespace WorkerProc {

// TODO: [Rendering Context] HARDCODED.
static const uint32_t fp_scale = 256;

// bl::RasterEngine::WorkerProc - ProcessJobs
// ==========================================

static BL_NOINLINE void process_jobs(WorkData* work_data, RenderBatch* batch) noexcept {
  size_t job_count = batch->job_count();

  if (!job_count) {
    work_data->synchronization->no_jobs_to_wait_for();
    return;
  }

  const RenderJobQueue* queue = batch->job_list().first();
  BL_ASSERT(queue != nullptr);

  size_t queue_index = 0;
  size_t queue_end = queue_index + queue->size();

  for (;;) {
    size_t job_index = batch->next_job_index();
    if (job_index >= job_count)
      break;

    while (job_index >= queue_end) {
      queue = queue->next();
      BL_ASSERT(queue != nullptr);

      queue_index = queue_end;
      queue_end = queue_index + queue->size();
    }

    RenderJob* job = queue->at(job_index - queue_index);
    BL_ASSERT(job != nullptr);

    JobProc::process_job(work_data, job);
  }

  work_data->avoid_cache_line_sharing();
  work_data->synchronization->wait_for_jobs_to_finish();
}

// bl::RasterEngine::WorkerProc - ProcessBand
// ==========================================

namespace {

#if BL_TARGET_ARCH_X86 && BL_SIMD_WIDTH_I

struct CommandMatcher {
#if BL_SIMD_WIDTH_I >= 256
  SIMD::Vec32xU8 _vqy;

  BL_INLINE explicit CommandMatcher(uint8_t qy) noexcept {
    _vqy = SIMD::make256_u8(qy);
  }

  BL_INLINE BLBitWord match(const uint8_t* bandQy0) noexcept {
    using namespace SIMD;

    Vec32xU8 q0 = cmp_ge_u8(_vqy, loadu<Vec32xU8>(bandQy0));

#if BL_TARGET_ARCH_BITS == 32
    return extract_mask_bits_i8(q0);
#else
    Vec32xU8 q1 = cmp_ge_u8(_vqy, loadu<Vec32xU8>(bandQy0 + 32));
    return extract_mask_bits_i8(q0, q1);
#endif
  }
#else
  SIMD::Vec16xU8 _vqy;

  BL_INLINE explicit CommandMatcher(uint8_t qy) noexcept {
    _vqy = SIMD::make128_u8(qy);
  }

  BL_INLINE BLBitWord match(const uint8_t* bandQy0) noexcept {
    using namespace SIMD;

    Vec16xU8 q0 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 +  0));
    Vec16xU8 q1 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 16));

#if BL_TARGET_ARCH_BITS == 32
    return extract_mask_bits_i8(q0, q1);
#else
    Vec16xU8 q2 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 32));
    Vec16xU8 q3 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 48));
    return extract_mask_bits_i8(q0, q1, q2, q3);
#endif
  }
#endif
};

#elif BL_TARGET_ARCH_ARM && BL_SIMD_WIDTH_I

// NOTE: We cannot use `extract_mask_bits_i8()` as it returns a LSB bit-mask, but we need a MSB one in this case.
struct CommandMatcher {
  SIMD::Vec16xU8 _vqy;
  SIMD::Vec16xU8 _vbm;

  BL_INLINE explicit CommandMatcher(uint8_t qy) noexcept {
    _vqy = SIMD::make128_u8(qy);
    _vbm = SIMD::make128_u8(0x01u, 0x02u, 0x04u, 0x08u, 0x10u, 0x20u, 0x40u, 0x80u);
  }

  BL_INLINE BLBitWord match(const uint8_t* bandQy0) noexcept {
    using namespace SIMD;

    Vec16xU8 q0 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 +  0));
    Vec16xU8 q1 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 16));
    Vec16xU8 m0 = and_(vec_cast<Vec16xU8>(q0), _vbm);
    Vec16xU8 m1 = and_(vec_cast<Vec16xU8>(q1), _vbm);

#if BL_TARGET_ARCH_BITS == 32
    uint8x8_t acc0 = vpadd_u8(vget_low_u8(m0.v), vget_high_u8(m0.v));
    uint8x8_t acc1 = vpadd_u8(vget_low_u8(m1.v), vget_high_u8(m1.v));

    acc0 = vpadd_u8(acc0, acc1);
    acc0 = vpadd_u8(acc0, acc0);

    return IntOps::byteSwap32(vget_lane_u32(vreinterpret_u32_u8(acc0), 0));
#else
    Vec16xU8 q2 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 32));
    Vec16xU8 q3 = cmp_ge_u8(_vqy, loadu<Vec16xU8>(bandQy0 + 48));
    Vec16xU8 m2 = and_(vec_cast<Vec16xU8>(q2), _vbm);
    Vec16xU8 m3 = and_(vec_cast<Vec16xU8>(q3), _vbm);

    uint8x16_t acc0 = vpaddq_u8(m0.v, m1.v);
    uint8x16_t acc1 = vpaddq_u8(m2.v, m3.v);

    acc0 = vpaddq_u8(acc0, acc1);
    acc0 = vpaddq_u8(acc0, acc0);

    return IntOps::byteSwap64(vgetq_lane_u64(vreinterpretq_u64_u8(acc0), 0));
#endif
  }
};

#endif

}

static void process_band(CommandProcAsync::ProcData& proc_data, uint32_t current_band_id, uint32_t prev_band_id, uint32_t next_band_id) noexcept {
  // Should not happen.
  if (!proc_data.pending_command_bit_set_size())
    return;

  typedef PrivateBitWordOps BitOps;

  RenderBatch* batch = proc_data.batch();
  WorkData* work_data = proc_data.work_data();

  // Initialize the `proc_data` with the current band.
  proc_data.init_band(current_band_id, work_data->band_height(), fp_scale);

  BLBitWord* bit_set_ptr = proc_data.pending_command_bit_set_data();
  BLBitWord* bitSetEndMinus1 = proc_data.pending_command_bit_set_end() - 1;
  BLBitWord pending_global_mask = proc_data.pending_command_bit_set_mask();

  const RenderCommandQueue* command_queue = batch->_command_list.first();
  const RenderCommand* command_data = command_queue->data();
  const RenderCommand* command_data_end = command_queue->end();
  const uint8_t* commandQuantizedY0 = command_queue->_quantizedY0;

  int32_t prevBandFy1 = int32_t(prev_band_id + 1u) * int32_t(work_data->band_height_fixed()) - 1;
  int32_t nextBandFy0 = int32_t(next_band_id     ) * int32_t(work_data->band_height_fixed());

  if (current_band_id == prev_band_id) {
    prevBandFy1 = -1;
  }

  uint32_t bandQy0 = uint8_t(proc_data.bandY0() >> work_data->command_quantization_shift_aa());
#if (BL_TARGET_ARCH_X86 || BL_TARGET_ARCH_ARM) && BL_SIMD_WIDTH_I
  CommandMatcher matcher(static_cast<uint8_t>(bandQy0));
#endif

  for (;;) {
#ifdef __SANITIZE_ADDRESS__
    // We know it's uninitialized, that's why we use the mask, which is either all ones or all zeros.
    BLBitWord pending_mask = !pending_global_mask ? *bit_set_ptr : pending_global_mask;
#else
    BLBitWord pending_mask = pending_global_mask | *bit_set_ptr;
#endif

    if (pending_mask) {
#if (BL_TARGET_ARCH_X86 || BL_TARGET_ARCH_ARM) && BL_SIMD_WIDTH_I
      BLBitWord process_mask = pending_mask & matcher.match(commandQuantizedY0);
      BitOps::BitIterator it(process_mask);

      while (it.has_next()) {
        uint32_t bit_index = it.next();
        const RenderCommand& command = command_data[bit_index];

        CommandProcAsync::CommandStatus status = CommandProcAsync::process_command(proc_data, command, prevBandFy1, nextBandFy0);
        pending_mask ^= BitOps::index_as_mask(bit_index, status);
      }
#else
      BitOps::BitIterator it(pending_mask);

      while (it.has_next()) {
        uint32_t bit_index = it.next();
        if (bandQy0 >= commandQuantizedY0[bit_index]) {
          const RenderCommand& command = command_data[bit_index];
          CommandProcAsync::CommandStatus status = CommandProcAsync::process_command(proc_data, command, prevBandFy1, nextBandFy0);
          pending_mask ^= BitOps::index_as_mask(bit_index, status);
        }
      }
#endif
      *bit_set_ptr = pending_mask;
    }

    if (++bit_set_ptr >= bitSetEndMinus1) {
      pending_global_mask = 0;
      if (bit_set_ptr > bitSetEndMinus1)
        break;
    }

    command_data += IntOps::bit_size_of<BLBitWord>();
    commandQuantizedY0 += IntOps::bit_size_of<BLBitWord>();

    if (command_data == command_data_end) {
      command_queue = command_queue->next();
      BL_ASSERT(command_queue != nullptr);

      command_data = command_queue->data();
      command_data_end = command_queue->end();
      commandQuantizedY0 = command_queue->_quantizedY0;
    }
  }

  proc_data.clear_pending_command_bit_set_mask();
}

// bl::RasterEngine::WorkerProc - ProcessCommands
// ==============================================

static void process_commands(WorkData* work_data, RenderBatch* batch) noexcept {
  ArenaAllocator::StatePtr zone_state = work_data->work_zone.save_state();
  CommandProcAsync::ProcData proc_data(work_data, batch);

  BLResult result = proc_data.init_proc_data();
  if (result != BL_SUCCESS) {
    work_data->accumulate_error(result);
    return;
  }

  uint32_t worker_count = batch->worker_count();
  uint32_t band_count = batch->band_count();

  // We can process several consecutive bands at once when there is enough of bands for all the threads.
  //
  // TODO: [Rendering Context] At the moment this feature is not used as it regressed bl_bench using 4+ threads.
  uint32_t consecutive_band_count = 1;

  uint32_t band_id = work_data->worker_id() * consecutive_band_count;
  uint32_t consecutive_index = 0;

  uint32_t current_band_id = band_id + consecutive_index;
  uint32_t prev_band_id = current_band_id;

  while (current_band_id < band_count) {
    // Calculate the next band so we can pass it to `process_band()`.
    if (++consecutive_index == consecutive_band_count) {
      consecutive_index = 0;
      band_id += worker_count * consecutive_band_count;
    }

    uint32_t next_band_id = band_id + consecutive_index;
    process_band(proc_data, current_band_id, prev_band_id, next_band_id);

    prev_band_id = current_band_id;
    current_band_id = next_band_id;
  }

  work_data->work_zone.restore_state(zone_state);
}

// bl::RasterEngine::WorkerProc - Finished
// =======================================

static void finished(WorkData* work_data, RenderBatch* batch) noexcept {
  work_data->reset_batch();

  if (work_data->is_sync())
    return;

  uint32_t accumulated_error_flags = work_data->accumulated_error_flags();
  if (!accumulated_error_flags)
    return;

  batch->accumulate_error_flags(accumulated_error_flags);
  work_data->clean_accumulated_error_flags();
}

// bl::RasterEngine::WorkerProc - ProcessWorkData
// ==============================================

// Can be also called by the rendering context from user thread.
void process_work_data(WorkData* work_data, RenderBatch* batch) noexcept {
  // NOTE: The zone must be cleared when the worker thread starts processing jobs and commands. The reason is that
  // once we finish job processing other threads can still use data produced by such job, so even when we are done
  // we cannot really clear the allocator, we must wait until all threads are done with the current batch, and that
  // is basically only guaranteed when we enter the proc again (or by the rendering context once it finishes).
  if (!work_data->is_sync())
    work_data->start_over();

  // Fix the alignment of the arena allocator in case it's currently not aligned - this prevents possible sharing of
  // a cache line that was used for something that could be used by all worker threads with a possible allocation
  // that is only intended to be used by the worker - for a memory region that the worker can write to frequently
  // (like active edges during rasterization).
  work_data->avoid_cache_line_sharing();

  // Pass 1 - Process jobs.
  //
  // Once the thread acquires a job to process no other thread can have that job. Jobs can be processed in any order,
  // however, we just use atomics to increment the job counter and each thread acquires the next in the queue.
  process_jobs(work_data, batch);

  // Pass 2 - Process commands.
  //
  // Commands are processed after the last job finishes. Command are processed multiple times per each band. Threads
  // process all commands in a band and then move to the next available band. This ensures that even when there is
  // something more complicated in one band than in all other bands the distribution of threads should be fair as
  // other threads won't wait for a particular band to be rendered.
  process_commands(work_data, batch);

  // Propagates accumulated error flags into the batch.
  finished(work_data, batch);
}

// bl::RasterEngine::WorkerProc - WorkerThreadEntry
// ================================================

void worker_thread_entry(BLThread* thread, void* data) noexcept {
  bl_unused(thread);

  WorkData* work_data = static_cast<WorkData*>(data);
  WorkerSynchronization* synchronization = work_data->synchronization;

  synchronization->thread_started();
  process_work_data(work_data, work_data->acquire_batch());
  synchronization->thread_done();
}

} // {WorkerProc}
} // {bl::RasterEngine}
