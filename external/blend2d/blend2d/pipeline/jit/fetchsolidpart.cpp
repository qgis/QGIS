// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#if !defined(BL_BUILD_NO_JIT)

#include <blend2d/pipeline/jit/compoppart_p.h>
#include <blend2d/pipeline/jit/fetchsolidpart_p.h>
#include <blend2d/pipeline/jit/fetchutilspixelaccess_p.h>
#include <blend2d/pipeline/jit/pipecompiler_p.h>

namespace bl::Pipeline::JIT {

#define REL_SOLID(FIELD) BL_OFFSET_OF(FetchData::Solid, FIELD)

// bl::Pipeline::JIT::FetchSolidPart - Construction & Destruction
// ==============================================================

FetchSolidPart::FetchSolidPart(PipeCompiler* pc, FormatExt format) noexcept
  : FetchPart(pc, FetchType::kSolid, format),
    _pixel("solid") {

  // Advancing has no cost.
  _part_flags |= PipePartFlags::kAdvanceXIsSimple;
  // Solid fetcher doesn't access memory, so masked access is always available.
  _part_flags |= PipePartFlags::kMaskedAccess;

  _max_pixels = kUnlimitedMaxPixels;
  _max_vec_width_supported = kMaxPlatformWidth;
  _pixel.set_count(PixelCount(1));
}

// bl::Pipeline::JIT::FetchSolidPart - Prepare
// ===========================================

void FetchSolidPart::prepare_part() noexcept {}

// bl::Pipeline::JIT::FetchSolidPart - Init & Fini
// ===============================================

void FetchSolidPart::_init_part(const PipeFunction& fn, Gp& x, Gp& y) noexcept {
  bl_unused(x, y);

  _fetch_data = fn.fetch_data();

  if (_pixel.type() != _pixel_type) {
    _pixel.set_type(_pixel_type);
  }
  else {
    // The type should never change after it's been assigned.
    BL_ASSERT(_pixel.type() == _pixel_type);
  }
}

void FetchSolidPart::_fini_part() noexcept {}

// bl::Pipeline::JIT::FetchSolidPart - InitSolidFlags
// ==================================================

void FetchSolidPart::init_solid_flags(PixelFlags flags) noexcept {
  ScopedInjector injector(cc, &_global_hook);
  Pixel& s = _pixel;

  switch (s.type()) {
    case PixelType::kA8: {
      if (bl_test_flag(flags, PixelFlags::kSA | PixelFlags::kPA_PI_UA_UI) && !s.sa.is_valid()) {
        s.sa = pc->new_gp32("solid.sa");
        pc->load_u8(s.sa, mem_ptr(_fetch_data, 3));
      }

      if (bl_test_flag(flags, PixelFlags::kPA_PI_UA_UI) && s.ua.is_empty()) {
        s.ua.init(pc->new_vec("solid.ua"));
        pc->v_broadcast_u16z(s.ua[0], s.sa);
      }
      break;
    }

    case PixelType::kRGBA32: {
      if (bl_test_flag(flags, PixelFlags::kPC_UC | PixelFlags::kPA_PI_UA_UI) && s.pc.is_empty()) {
        s.pc.init(pc->new_vec("solid.pc"));
        pc->v_broadcast_u32(s.pc[0], mem_ptr(_fetch_data));
      }
      break;
    }

    default:
      BL_NOT_REACHED();
  }

  FetchUtils::satisfy_solid_pixels(pc, s, flags);
}

// bl::Pipeline::JIT::FetchSolidPart - Fetch
// =========================================

void FetchSolidPart::fetch(Pixel& p, PixelCount n, PixelFlags flags, PixelPredicate& predicate) noexcept {
  BL_ASSERT(_pixel.type() == p.type());
  bl_unused(predicate);

  p.set_count(n);
  Pixel& s = _pixel;

  switch (p.type()) {
    case PixelType::kA8: {
      if (n == PixelCount(1)) {
        if (bl_test_flag(flags, PixelFlags::kSA)) {
          init_solid_flags(PixelFlags::kSA);

          if (bl_test_flag(flags, PixelFlags::kImmutable)) {
            if (bl_test_flag(flags, PixelFlags::kSA)) {
              p.sa = s.sa;
            }
          }
          else {
            if (bl_test_flag(flags, PixelFlags::kSA)) {
              p.sa = pc->new_gp32("%ssa", p.name());
              pc->mov(p.sa, s.sa);
            }
          }
        }
      }
      else {
        init_solid_flags(flags & (PixelFlags::kPA | PixelFlags::kUA | PixelFlags::kUI));

        VecWidth pa_vec_width = pc->vec_width_of(DataWidth::k8, n);
        VecWidth ua_vec_width = pc->vec_width_of(DataWidth::k16, n);

        size_t pa_count = pc->vec_count_of(DataWidth::k8, n);
        size_t ua_count = pc->vec_count_of(DataWidth::k16, n);

        if (bl_test_flag(flags, PixelFlags::kImmutable)) {
          if (bl_test_flag(flags, PixelFlags::kPA)) { p.pa = s.pa.clone_as(pa_vec_width); }
          if (bl_test_flag(flags, PixelFlags::kUA)) { p.ua = s.ua.clone_as(ua_vec_width); }
          if (bl_test_flag(flags, PixelFlags::kUI)) { p.ui = s.ui.clone_as(ua_vec_width); }
        }
        else {
          if (bl_test_flag(flags, PixelFlags::kPA)) {
            pc->new_vec_array(p.pa, pa_count, pa_vec_width, p.name(), "pa");
            pc->v_mov(p.pa, s.pa[0].clone_as(p.pa[0]));
          }

          if (bl_test_flag(flags, PixelFlags::kUA)) {
            pc->new_vec_array(p.ua, ua_count, ua_vec_width, p.name(), "ua");
            pc->v_mov(p.ua, s.ua[0].clone_as(p.ua[0]));
          }

          if (bl_test_flag(flags, PixelFlags::kUI)) {
            pc->new_vec_array(p.ui, ua_count, ua_vec_width, p.name(), "ui");
            pc->v_mov(p.ui, s.ui[0].clone_as(p.ui[0]));
          }
        }
      }
      break;
    }

    case PixelType::kRGBA32: {
      init_solid_flags(flags & (PixelFlags::kPC_UC | PixelFlags::kPA_PI_UA_UI));

      VecWidth pc_width = pc->vec_width_of(DataWidth::k32, n);
      VecWidth uc_width = pc->vec_width_of(DataWidth::k64, n);

      size_t pc_count = pc->vec_count_of(DataWidth::k32, n);
      size_t uc_count = pc->vec_count_of(DataWidth::k64, n);

      if (bl_test_flag(flags, PixelFlags::kImmutable)) {
        if (bl_test_flag(flags, PixelFlags::kPC)) { p.pc = s.pc.clone_as(pc_width); }
        if (bl_test_flag(flags, PixelFlags::kUC)) { p.uc = s.uc.clone_as(uc_width); }
        if (bl_test_flag(flags, PixelFlags::kUA)) { p.ua = s.ua.clone_as(uc_width); }
        if (bl_test_flag(flags, PixelFlags::kUI)) { p.ui = s.ui.clone_as(uc_width); }
      }
      else {
        if (bl_test_flag(flags, PixelFlags::kPC)) {
          pc->new_vec_array(p.pc, pc_count, pc_width, p.name(), "pc");
          pc->v_mov(p.pc, s.pc[0].clone_as(p.pc[0]));
        }

        if (bl_test_flag(flags, PixelFlags::kUC)) {
          pc->new_vec_array(p.uc, uc_count, uc_width, p.name(), "uc");
          pc->v_mov(p.uc, s.uc[0].clone_as(p.uc[0]));
        }

        if (bl_test_flag(flags, PixelFlags::kUA)) {
          pc->new_vec_array(p.ua, uc_count, uc_width, p.name(), "ua");
          pc->v_mov(p.ua, s.ua[0].clone_as(p.ua[0]));
        }

        if (bl_test_flag(flags, PixelFlags::kUI)) {
          pc->new_vec_array(p.ui, uc_count, uc_width, p.name(), "ui");
          pc->v_mov(p.ui, s.ui[0].clone_as(p.ui[0]));
        }
      }

      break;
    }

    default:
      BL_NOT_REACHED();
  }

  FetchUtils::satisfy_pixels(pc, p, flags);
}

} // {bl::Pipeline::JIT}

#endif // !BL_BUILD_NO_JIT
