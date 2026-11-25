// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/fetchutilscoverage_p.h>

namespace bl::Pipeline::JIT {

// bl::Pipeline::JIT - GlobalAlpha
// ===============================

void GlobalAlpha::_init_internal(PipeCompiler* pc) noexcept {
  _pc = pc;
  _pc->cc->comment("[[Global Alpha]]");
  _hook = pc->cc->cursor();
}

void GlobalAlpha::init_from_mem(PipeCompiler* pc, const Mem& mem) noexcept {
  // Can only be initialized once.
  BL_ASSERT(!is_initialized());

  _init_internal(pc);
  _mem = mem;
}

void GlobalAlpha::init_from_scalar(PipeCompiler* pc, const Gp& sa) noexcept {
  // Can only be initialized once.
  BL_ASSERT(!is_initialized());

  _init_internal(pc);
  _sa = sa;
}

void GlobalAlpha::init_from_packed(PipeCompiler* pc, const Vec& pa) noexcept {
  // Can only be initialized once.
  BL_ASSERT(!is_initialized());

  _init_internal(pc);
  _pa = pa;
}

void GlobalAlpha::init_from_unpacked(PipeCompiler* pc, const Vec& ua) noexcept {
  // Can only be initialized once.
  BL_ASSERT(!is_initialized());

  _init_internal(pc);
  _ua = ua;
}

const Gp& GlobalAlpha::sa() noexcept {
  BL_ASSERT(is_initialized());

  if (!_sa.is_valid()) {
    ScopedInjector injector(_pc->cc, &_hook);
    _sa = _pc->new_gp32("ga.sa");

    if (_ua.is_valid()) {
      _pc->s_extract_u16(_sa, _ua, 0u);
    }
    else if (_pa.is_valid()) {
      _pc->s_extract_u8(_sa, _ua, 0u);
    }
    else {
      _pc->load_u8(_sa, _mem);
    }
  }

  return _sa;
}

const Vec& GlobalAlpha::pa() noexcept {
  BL_ASSERT(is_initialized());

  if (!_pa.is_valid()) {
    ScopedInjector injector(_pc->cc, &_hook);
    _pa = _pc->new_vec("ga.pa");

    if (_ua.is_valid()) {
      _pc->v_packs_i16_u8(_pa, _ua, _ua);
    }
    else if (_sa.is_valid()) {
      _pc->v_broadcast_u8z(_pa, _sa);
    }
    else {
      _pc->v_broadcast_u8(_pa, _mem);
    }
  }

  return _pa;
}

const Vec& GlobalAlpha::ua() noexcept {
  BL_ASSERT(is_initialized());

  if (!_ua.is_valid()) {
    ScopedInjector injector(_pc->cc, &_hook);
    _ua = _pc->new_vec("ga.ua");

    if (_pa.is_valid()) {
      _pc->v_cvt_u8_lo_to_u16(_ua, _pa);
    }
    else if (_sa.is_valid()) {
      _pc->v_broadcast_u16z(_ua, _sa);
    }
    else {
      _pc->v_broadcast_u16(_ua, _mem);
    }
  }

  return _ua;
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
