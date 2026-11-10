// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/pipecompiler_p.h>
#include <blend2d/pipeline/jit/pipepart_p.h>
#include <blend2d/support/intops_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::PipeCompiler - Construction & Destruction
// =======================================================

PipeCompiler::PipeCompiler(BackendCompiler* cc, const CpuFeatures& cpu_features, CpuHints cpu_hints) noexcept
  : UniCompiler(cc, cpu_features, cpu_hints, VecConstTableRef{common_table, sizeof(common_table)}) {}

PipeCompiler::~PipeCompiler() noexcept {}

// bl::Pipeline::PipeCompiler - Predicate Helpers
// ==============================================

#if defined(BL_JIT_ARCH_X86)
static KReg PipeCompiler_make_mask_predicate(PipeCompiler* pc, PixelPredicate& predicate, size_t last_n, const Gp& adjusted_count) noexcept {
  BL_ASSERT(last_n <= 64);
  BL_ASSERT(IntOps::is_power_of_2(last_n));

  KReg k_pred;
  if (!pc->has_avx512())
    return k_pred;

  uint32_t materialized_count = predicate._materialized_count;
  for (uint32_t i = 0; i < materialized_count; i++) {
    const PixelPredicate::MaterializedMask& p = predicate._materialized_masks[i];
    if (p.last_n == last_n && p.element_size == 0u) {
      // If the record was created it has to provide a mask register, not any other register type.
      BL_ASSERT(p.mask.is_kreg());
      return p.mask.as<KReg>();
    }
  }

  if (materialized_count >= PixelPredicate::kMaterializedMaskCapacity)
    return k_pred;

  BackendCompiler* cc = pc->cc;
  bool use_bzhi = last_n <= 32 || pc->is_64bit();

  if (last_n <= 32)
    k_pred = cc->new_kd("@k_pred");
  else
    k_pred = cc->new_kq("@k_pred");

  PixelPredicate::MaterializedMask& p = predicate._materialized_masks[materialized_count];
  p.last_n = uint8_t(last_n);
  p.element_size = 0;
  p.mask = k_pred;

  Gp gp_count = predicate.count();

  if (adjusted_count.is_valid()) {
    gp_count = adjusted_count;
  }
  else if (last_n < predicate.size()) {
    gp_count = pc->new_gpz("@gp_count");
    pc->and_(gp_count.clone_as(predicate.count()), predicate.count(), last_n - 1);
  }

  if (use_bzhi) {
    Gp gp_pred = pc->new_gpz("@gp_pred");

    if (last_n <= 32)
      gp_pred = gp_pred.r32();

    cc->mov(gp_pred, -1);
    cc->bzhi(gp_pred, gp_pred, gp_count.clone_as(gp_pred));

    if (last_n <= 32)
      cc->kmovd(k_pred, gp_pred);
    else
      cc->kmovq(k_pred, gp_pred);
  }
  else {
    x86::Mem mem = pc->_get_mem_const(common_table.k_msk64_data);
    mem.set_index(cc->gpz(gp_count.id()));
    mem.set_shift(3);

    if (last_n <= 8)
      cc->kmovb(k_pred, mem);
    else if (last_n <= 16)
      cc->kmovw(k_pred, mem);
    else if (last_n <= 32)
      cc->kmovd(k_pred, mem);
    else
      cc->kmovq(k_pred, mem);
  }

  predicate._materialized_count++;
  return k_pred;
}

KReg PipeCompiler::make_mask_predicate(PixelPredicate& predicate, size_t last_n) noexcept {
  Gp no_adjusted_count;
  return PipeCompiler_make_mask_predicate(this, predicate, last_n, no_adjusted_count);
}

KReg PipeCompiler::make_mask_predicate(PixelPredicate& predicate, size_t last_n, const Gp& adjusted_count) noexcept {
  return PipeCompiler_make_mask_predicate(this, predicate, last_n, adjusted_count);
}

Vec PipeCompiler::make_vec_predicate32(PixelPredicate& predicate, size_t last_n) noexcept {
  Gp no_adjusted_count;
  return make_vec_predicate32(predicate, last_n, no_adjusted_count);
}

Vec PipeCompiler::make_vec_predicate32(PixelPredicate& predicate, size_t last_n, const Gp& adjusted_count) noexcept {
  BL_ASSERT(last_n <= 8);
  BL_ASSERT(IntOps::is_power_of_2(last_n));

  Vec v_pred;
  if (!has_avx())
    return v_pred;

  uint32_t materialized_count = predicate._materialized_count;
  for (uint32_t i = 0; i < materialized_count; i++) {
    const PixelPredicate::MaterializedMask& p = predicate._materialized_masks[i];
    if (p.last_n == last_n && p.element_size == 4u) {
      // If the record was created it has to provide a mask register, not any other register type.
      BL_ASSERT(p.mask.is_vec());
      return p.mask.as<Vec>();
    }
  }

  if (materialized_count >= PixelPredicate::kMaterializedMaskCapacity)
    return v_pred;

  if (last_n <= 4)
    v_pred = new_vec128("@vPred128");
  else if (last_n <= 8)
    v_pred = new_vec256("@vPred256");
  else
    BL_NOT_REACHED();

  PixelPredicate::MaterializedMask& p = predicate._materialized_masks[materialized_count];
  p.last_n = uint8_t(last_n);
  p.element_size = uint8_t(4);
  p.mask = v_pred;

  Gp gp_count = predicate.count();

  if (adjusted_count.is_valid()) {
    gp_count = adjusted_count;
  }
  else if (last_n < predicate.size()) {
    gp_count = new_gpz("@gp_count");
    and_(gp_count.clone_as(predicate.count()), predicate.count(), last_n - 1);
  }

  x86::Mem mem = _get_mem_const(common_table.loadstore16_lo8_msk8());
  mem.set_index(cc->gpz(gp_count.id()));
  mem.set_shift(3);
  cc->vpmovsxbd(v_pred, mem);

  predicate._materialized_count++;
  return v_pred;
}
#endif // !BL_BUILD_NO_JIT

} // {bl::Pipeline::JIT}

#endif
