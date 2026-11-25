// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/ujit/ujitbase.h>

#if defined(ASMJIT_UJIT_X86)

#include <asmjit/ujit/unicompiler.h>
#include <asmjit/ujit/unicompiler_utils_p.h>
#include <asmjit/ujit/unicondition.h>

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

using GPExt = UniCompiler::GPExt;
using SSEExt = UniCompiler::SSEExt;
using AVXExt = UniCompiler::AVXExt;
namespace Inst { using namespace x86::Inst; }

// ujit::UniCompiler - Constants
// =============================

static constexpr OperandSignature signature_of_xmm_ymm_zmm[] = {
  OperandSignature{RegTraits<RegType::kVec128>::kSignature},
  OperandSignature{RegTraits<RegType::kVec256>::kSignature},
  OperandSignature{RegTraits<RegType::kVec512>::kSignature}
};

static ASMJIT_INLINE RegType vec_reg_type_from_width(VecWidth vw) noexcept {
  return RegType(uint32_t(RegType::kVec128) + uint32_t(vw));
}

// ujit::UniCompiler - Construction & Destruction
// ==============================================

UniCompiler::UniCompiler(BackendCompiler* cc, const CpuFeatures& features, CpuHints cpu_hints, VecConstTableRef ct_ref) noexcept
  : cc(cc),
    _ct_ref(ct_ref),
    _features(features),
    _cpu_hints(cpu_hints),
    _vec_reg_count(16),
    _common_table_offset(128) {

  _scalar_op_behavior = ScalarOpBehavior::kPreservingVec128;
  _fmin_fmax_op_behavior = FMinFMaxOpBehavior::kTernaryLogic;
  _fmadd_op_behavior = FMAddOpBehavior::kNoFMA; // Will be changed by _init_extensions() if supported.
  _float_to_int_outside_range_behavior = FloatToIntOutsideRangeBehavior::kSmallestValue;

  _init_extensions(features);
}

UniCompiler::~UniCompiler() noexcept {}

// ujit::UniCompiler - CPU Architecture, Features and Optimization Options
// =======================================================================

void UniCompiler::_init_extensions(const CpuFeatures& features) noexcept {
  uint32_t gp_ext_mask = 0;
  uint32_t sse_ext_mask = 0;
  uint64_t avx_ext_mask = 0;

  if (features.x86().has_adx()) gp_ext_mask |= 1u << uint32_t(GPExt::kADX);
  if (features.x86().has_bmi()) gp_ext_mask |= 1u << uint32_t(GPExt::kBMI);
  if (features.x86().has_bmi2()) gp_ext_mask |= 1u << uint32_t(GPExt::kBMI2);
  if (features.x86().has_lzcnt()) gp_ext_mask |= 1u << uint32_t(GPExt::kLZCNT);
  if (features.x86().has_movbe()) gp_ext_mask |= 1u << uint32_t(GPExt::kMOVBE);
  if (features.x86().has_popcnt()) gp_ext_mask |= 1u << uint32_t(GPExt::kPOPCNT);

  sse_ext_mask |= 1u << uint32_t(SSEExt::kSSE2);
  if (features.x86().has_sse3()) sse_ext_mask |= 1u << uint32_t(SSEExt::kSSE3);
  if (features.x86().has_ssse3()) sse_ext_mask |= 1u << uint32_t(SSEExt::kSSSE3);
  if (features.x86().has_sse4_1()) sse_ext_mask |= 1u << uint32_t(SSEExt::kSSE4_1);
  if (features.x86().has_sse4_2()) sse_ext_mask |= 1u << uint32_t(SSEExt::kSSE4_2);
  if (features.x86().has_pclmulqdq()) sse_ext_mask |= 1u << uint32_t(SSEExt::kPCLMULQDQ);

  if (features.x86().has_avx()) {
    avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX);
    if (features.x86().has_avx2()            ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX2);
    if (features.x86().has_f16c()            ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kF16C);
    if (features.x86().has_fma()             ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kFMA);
    if (features.x86().has_gfni()            ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kGFNI);
    if (features.x86().has_vaes()            ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kVAES);
    if (features.x86().has_vpclmulqdq()      ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kVPCLMULQDQ);
    if (features.x86().has_avx_ifma()        ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX_IFMA);
    if (features.x86().has_avx_ne_convert()  ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX_NE_CONVERT);
    if (features.x86().has_avx_vnni()        ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX_VNNI);
    if (features.x86().has_avx_vnni_int8()   ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX_VNNI_INT8);
    if (features.x86().has_avx_vnni_int16()  ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX_VNNI_INT16);
  }

  if (features.x86().has_avx2()      && features.x86().has_avx512_f() &&
      features.x86().has_avx512_cd() && features.x86().has_avx512_bw() &&
      features.x86().has_avx512_dq() && features.x86().has_avx512_vl()) {
    _vec_reg_count = 32;
    avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512);
    if (features.x86().has_avx512_bf16()     ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_BF16);
    if (features.x86().has_avx512_bitalg()   ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_BITALG);
    if (features.x86().has_avx512_fp16()     ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_FP16);
    if (features.x86().has_avx512_ifma()     ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_IFMA);
    if (features.x86().has_avx512_vbmi()     ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_VBMI);
    if (features.x86().has_avx512_vbmi2()    ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_VBMI2);
    if (features.x86().has_avx512_vnni()     ) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_VNNI);
    if (features.x86().has_avx512_vpopcntdq()) avx_ext_mask |= uint64_t(1) << uint32_t(AVXExt::kAVX512_VPOPCNTDQ);
  }

  _gp_ext_mask = gp_ext_mask;
  _sse_ext_mask = sse_ext_mask;
  _avx_ext_mask = avx_ext_mask;

  if (has_fma()) {
    _fmadd_op_behavior = FMAddOpBehavior::kFMAStoreToAny;
  }
}

VecWidth UniCompiler::max_vec_width_from_cpu_features() noexcept {
  // Use 512-bit SIMD width if AVX512 is available and the target is 64-bit. We never use 512-bit SIMD in 32-bit mode
  // as it doesn't have enough registers to hold 512-bit constants and we don't store 512-bit constants in memory
  // (they must be broadcasted to full width).
  if (has_avx512() && is_64bit())
    return VecWidth::k512;

  // Use 256-bit SIMD width if AVX2 is available.
  if (has_avx2())
    return VecWidth::k256;

  return VecWidth::k128;
}

void UniCompiler::init_vec_width(VecWidth vw) noexcept {
  _vec_width = vw;
  _vec_reg_type = vec_reg_type_from_width(vw);
  _vec_type_id = RegUtils::type_id_of(_vec_reg_type);
  _vec_multiplier = uint8_t(1u << (uint32_t(_vec_reg_type) - uint32_t(RegType::kVec128)));
}

bool UniCompiler::has_masked_access_of(uint32_t data_size) const noexcept {
  switch (data_size) {
    case 1: return has_cpu_hint(CpuHints::kVecMaskedOps8);
    case 2: return has_cpu_hint(CpuHints::kVecMaskedOps16);
    case 4: return has_cpu_hint(CpuHints::kVecMaskedOps32);
    case 8: return has_cpu_hint(CpuHints::kVecMaskedOps64);

    default:
      return false;
  }
}

// ujit::UniCompiler - Embed
// =========================

void UniCompiler::embed_jump_table(Span<const Label> jump_table, const Label& jump_table_base, uint32_t entry_size) {
  static const uint8_t zeros[8] {};

  for (const Label& label : jump_table) {
    if (label.is_valid()) {
      cc->embed_label_delta(label, jump_table_base, entry_size);
    }
    else {
      cc->embed(zeros, entry_size);
    }
  }
}

// ujit::UniCompiler - Function
// ============================

void UniCompiler::hook_func() noexcept {
  FuncNode* func = cc->func();
  _func_init_hook = func;

  if (func && has_avx()) {
    func->frame().set_avx_enabled();
    func->frame().set_avx_auto_cleanup();

    if (has_avx512()) {
      func->frame().set_avx512_enabled();
    }
  }
}

void UniCompiler::unhook_func() noexcept {
  _func_init_hook = nullptr;
}

// ujit::UniCompiler - Constants
// =============================

void UniCompiler::_init_vec_const_table_ptr() {
  const void* ct_addr = ct_ptr<void>();

  if (!_common_table_ptr.is_valid()) {
    ScopedInjector injector(cc, &_func_init_hook);
    _common_table_ptr = new_gpz("common_table_ptr");
    cc->mov(_common_table_ptr, (int64_t)ct_addr + _common_table_offset);
  }
}

x86::KReg UniCompiler::k_const(uint64_t value) {
  uint32_t slot;
  for (slot = 0; slot < kMaxKRegConstCount; slot++)
    if (_k_reg[slot].is_valid() && _k_imm[slot] == value)
      return _k_reg[slot];

  BaseNode* prev_node = nullptr;
  Gp tmp;
  x86::KReg kReg;

  if (slot < kMaxKRegConstCount) {
    prev_node = cc->set_cursor(_func_init_hook);
  }

  if (value & 0xFFFFFFFF00000000u) {
    tmp = new_gp64("kTmp");
    kReg = cc->new_kq("k0x%016llX", (unsigned long long)value);
    cc->mov(tmp, value);
    cc->kmovq(kReg, tmp);
  }
  else {
    tmp = new_gp32("kTmp");
    kReg = cc->new_kd("k0x%08llX", (unsigned long long)value);
    cc->mov(tmp, value);
    cc->kmovd(kReg, tmp);
  }

  if (slot < kMaxKRegConstCount) {
    _k_reg[slot] = kReg;
    _func_init_hook = cc->set_cursor(prev_node);
  }

  return kReg;
}

Operand UniCompiler::simd_const(const void* c, Bcst bcst_width, VecWidth const_width) {
  size_t const_count = _vec_consts.size();

  for (size_t i = 0; i < const_count; i++) {
    if (_vec_consts[i].ptr == c) {
      return Vec(signature_of_xmm_ymm_zmm[size_t(const_width)], _vec_consts[i].virt_reg_id);
    }
  }

  // We don't use memory constants when compiling for AVX-512, because we don't store 64-byte constants and AVX-512
  // has enough registers to hold all the constants that we need. However, in SSE/AVX2 case, we don't want so many
  // constants in registers as that could limit registers that we need during fetching and composition.
  if (!has_avx512()) {
    bool use_vreg = (c == &ct().p_0000000000000000); // Required if the CPU doesn't have SSE4.1.
    if (!use_vreg) {
      return simd_mem_const(c, bcst_width, const_width);
    }
  }

  return Vec(signature_of_xmm_ymm_zmm[size_t(const_width)], _new_vec_const(c, bcst_width == Bcst::kNA_Unique).id());
}

Operand UniCompiler::simd_const(const void* c, Bcst bcst_width, const Vec& similar_to) {
  VecWidth const_width = VecWidth(uint32_t(similar_to.reg_type()) - uint32_t(RegType::kVec128));
  return simd_const(c, bcst_width, const_width);
}

Operand UniCompiler::simd_const(const void* c, Bcst bcst_width, const VecArray& similar_to) {
  ASMJIT_ASSERT(!similar_to.is_empty());

  VecWidth const_width = VecWidth(uint32_t(similar_to[0].reg_type()) - uint32_t(RegType::kVec128));
  return simd_const(c, bcst_width, const_width);
}

Vec UniCompiler::simd_vec_const(const void* c, Bcst bcst_width, VecWidth const_width) {
  size_t const_count = _vec_consts.size();

  for (size_t i = 0; i < const_count; i++)
    if (_vec_consts[i].ptr == c)
      return Vec(signature_of_xmm_ymm_zmm[size_t(const_width)], _vec_consts[i].virt_reg_id);

  return Vec(signature_of_xmm_ymm_zmm[size_t(const_width)], _new_vec_const(c, bcst_width == Bcst::kNA_Unique).id());
}

Vec UniCompiler::simd_vec_const(const void* c, Bcst bcst_width, const Vec& similar_to) {
  VecWidth const_width = VecWidth(uint32_t(similar_to.reg_type()) - uint32_t(RegType::kVec128));
  return simd_vec_const(c, bcst_width, const_width);
}

Vec UniCompiler::simd_vec_const(const void* c, Bcst bcst_width, const VecArray& similar_to) {
  ASMJIT_ASSERT(!similar_to.is_empty());

  VecWidth const_width = VecWidth(uint32_t(similar_to[0].reg_type()) - uint32_t(RegType::kVec128));
  return simd_vec_const(c, bcst_width, const_width);
}

x86::Mem UniCompiler::simd_mem_const(const void* c, Bcst bcst_width, VecWidth const_width) {
  x86::Mem m = _get_mem_const(c);
  if (const_width != VecWidth::k512)
    return m;

  x86::Mem::Broadcast bcst = x86::Mem::Broadcast::kNone;
  switch (bcst_width) {
    case Bcst::k8: bcst = x86::Mem::Broadcast::k1To64; break;
    case Bcst::k16: bcst = x86::Mem::Broadcast::k1To32; break;
    case Bcst::k32: bcst = x86::Mem::Broadcast::k1To16; break;
    case Bcst::k64: bcst = x86::Mem::Broadcast::k1To8; break;
    default: bcst = x86::Mem::Broadcast::kNone; break;
  }

  m.set_broadcast(bcst);
  return m;
}

x86::Mem UniCompiler::simd_mem_const(const void* c, Bcst bcst_width, const Vec& similar_to) {
  VecWidth const_width = VecWidth(uint32_t(similar_to.reg_type()) - uint32_t(RegType::kVec128));
  return simd_mem_const(c, bcst_width, const_width);
}

x86::Mem UniCompiler::simd_mem_const(const void* c, Bcst bcst_width, const VecArray& similar_to) {
  ASMJIT_ASSERT(!similar_to.is_empty());

  VecWidth const_width = VecWidth(uint32_t(similar_to[0].reg_type()) - uint32_t(RegType::kVec128));
  return simd_mem_const(c, bcst_width, const_width);
}

x86::Mem UniCompiler::_get_mem_const(const void* c) {
  // Make sure we are addressing a constant from the `commonTable` constant pool.
  const void* ct_addr = ct_ptr<void>();
  ASMJIT_ASSERT((uintptr_t)c >= (uintptr_t)ct_addr &&
                (uintptr_t)c <  (uintptr_t)ct_addr + _ct_ref.size);

  if (is_32bit()) {
    // 32-bit mode - These constants will never move in memory so the absolute addressing is a win/win as we can save
    // one GP register that can be used for something else.
    return x86::ptr((uint64_t)c);
  }
  else {
    // 64-bit mode - One GP register is sacrificed to hold the pointer to the `ct`. This is probably the safest
    // approach as relying on absolute addressing or anything else could lead to problems or performance issues.
    _init_vec_const_table_ptr();

    int32_t disp = int32_t((intptr_t)c - (intptr_t)ct_addr);
    return x86::ptr(_common_table_ptr, disp - _common_table_offset);
  }
}

Vec UniCompiler::_new_vec_const(const void* c, bool is_unique_const) {
  Vec vec;
  const char* special_const_name = nullptr;

  if (special_const_name) {
    vec = new_vec_with_width(vec_width(), special_const_name);
  }
  else {
    uint64_t u0 = static_cast<const uint64_t*>(c)[0];
    uint64_t u1 = static_cast<const uint64_t*>(c)[1];

    if (u0 != u1)
      vec = new_vec_with_width(vec_width(), "c_0x%016llX%016llX", (unsigned long long)u1, (unsigned long long)u0);
    else if ((u0 >> 32) != (u0 & 0xFFFFFFFFu))
      vec = new_vec_with_width(vec_width(), "c_0x%016llX", (unsigned long long)u0);
    else if (((u0 >> 16) & 0xFFFFu) != (u0 & 0xFFFFu))
      vec = new_vec_with_width(vec_width(), "c_0x%08X", (unsigned)(u0 & 0xFFFFFFFFu));
    else
      vec = new_vec_with_width(vec_width(), "c_0x%04X", (unsigned)(u0 & 0xFFFFu));
  }

  VecConstData const_data;
  const_data.ptr = c;
  const_data.virt_reg_id = vec.id();
  _vec_consts.append(arena(), const_data);

  if (c == &ct().p_0000000000000000) {
    ScopedInjector inject(cc, &_func_init_hook);
    v_zero_i(vec.xmm());
  }
  else {
    // NOTE: _get_mem_const() must be outside of injected code as it uses injection too.
    Mem m = _get_mem_const(c);

    ScopedInjector inject(cc, &_func_init_hook);
    if (has_avx512() && !vec.is_vec128() && !is_unique_const)
      cc->vbroadcasti32x4(vec, m);
    else if (has_avx2() && vec.is_vec256() && !is_unique_const)
      cc->vbroadcasti128(vec, m);
    else if (has_avx512())
      cc->vmovdqa32(vec, m); // EVEX prefix has a compressed displacement, which is smaller.
    else
      v_loadavec(vec, m);
  }
  return vec;
}

// ujit::UniCompiler - Stack
// =========================

x86::Mem UniCompiler::tmp_stack(StackId id, uint32_t size) {
  ASMJIT_ASSERT(Support::is_power_of_2(size));
  ASMJIT_ASSERT(size <= 64);

  // Only used by asserts.
  Support::maybe_unused(size);

  Mem& stack = _tmp_stack[size_t(id)];
  if (!stack.base_id())
    stack = cc->new_stack(64, 16, "tmp_stack");
  return stack;
}

// ujit::UniCompiler - General Purpose Instructions - Conditions
// =============================================================

static constexpr InstId condition_to_inst_id[size_t(UniOpCond::kMaxValue) + 1] = {
  Inst::kIdAnd,  // UniOpCond::kAssignAnd
  Inst::kIdOr,   // UniOpCond::kAssignOr
  Inst::kIdXor,  // UniOpCond::kAssignXor
  Inst::kIdAdd,  // UniOpCond::kAssignAdd
  Inst::kIdSub,  // UniOpCond::kAssignSub
  Inst::kIdShr,  // UniOpCond::kAssignShr
  Inst::kIdTest, // UniOpCond::kTest
  Inst::kIdBt,   // UniOpCond::kBitTest
  Inst::kIdCmp   // UniOpCond::kCompare
};

class ConditionApplier : public UniCondition {
public:
  ASMJIT_INLINE ConditionApplier(const UniCondition& condition) noexcept : UniCondition(condition) {
    // The first operand must always be a register.
    ASMJIT_ASSERT(a.is_gp());
  }

  ASMJIT_NOINLINE void optimize(UniCompiler& uc) noexcept {
    switch (op) {
      case UniOpCond::kAssignShr:
        if (b.is_imm() && b.as<Imm>().value() == 0) {
          if (a.is_gp32()) {
            // Shifting by 0 would not set the flags...
            op = UniOpCond::kAssignAnd;
            b = a;
          }
          else {
            op = UniOpCond::kTest;
            b = a;
          }
        }
        break;

      case UniOpCond::kCompare:
        if (b.is_imm() && b.as<Imm>().value() == 0 && (cond == CondCode::kEqual || cond == CondCode::kNotEqual)) {
          op = UniOpCond::kTest;
          b = a;
          reverse();
        }
        break;

      case UniOpCond::kBitTest: {
        if (b.is_imm()) {
          uint64_t bit_index = b.as<Imm>().value_as<uint64_t>();

          // NOTE: AMD has no performance difference between 'test' and 'bt' instructions, however, Intel can execute less
          // 'bt' instructions per cycle than 'test's, so we prefer 'test' if bit_index is low. Additionally, we only use
          // test on 64-bit hardware as it's guaranteed that any register index is encodable. On 32-bit hardware only the
          // first 4 registers can be used, which could mean that the register would have to be moved just to be tested,
          // which is something we would like to avoid.
          if (uc.is_64bit() && bit_index < 8) {
            op = UniOpCond::kTest;
            b = Imm(1u << bit_index);
            cond = cond == CondCode::kC ? CondCode::kNZ : CondCode::kZ;
          }
        }
        break;
      }

      default:
        break;
    }
  }

  ASMJIT_INLINE void reverse() noexcept {
    cond = x86::reverse_cond(cond);
  }

  ASMJIT_NOINLINE void emit(UniCompiler& uc) {
    BackendCompiler* cc = uc.cc;
    InstId inst_id = condition_to_inst_id[size_t(op)];

    if (inst_id == Inst::kIdTest && cc->is_64bit()) {
      if (b.is_imm() && b.as<Imm>().value_as<uint64_t>() <= 255u) {
        // Emit 8-bit operation if targeting 64-bit mode and the immediate fits 8 bits.
        cc->test(a.as<Gp>().r8(), b.as<Imm>());
        return;
      }
      else if (a.as<Gp>().size() > 4 && b.is_imm() && uint64_t(b.as<Imm>().value()) <= 0xFFFFFFFFu) {
        // Emit 32-bit operation if targeting 64-bit mode and the immediate is lesser than UINT32_MAX.
        // This possibly saves a REX prefix required to promote the instruction to a 64-bit operation.
        cc->test(a.as<Gp>().r32(), b.as<Imm>());
        return;
      }
    }

    if (inst_id == Inst::kIdShr && b.is_reg()) {
      cc->emit(inst_id, a, b.as<Gp>().r8());
      return;
    }


    cc->emit(inst_id, a, b);
  }
};

// ujit::UniCompiler - General Purpose Instructions - Emit
// =======================================================

void UniCompiler::emit_mov(const Gp& dst, const Operand_& src) {
  if (src.is_imm() && src.as<Imm>().value() == 0) {
    Gp r(dst);
    if (r.is_gp64())
      r = r.r32();
    cc->xor_(r, r);
  }
  else {
    cc->emit(Inst::kIdMov, dst, src);
  }
}

void UniCompiler::emit_m(UniOpM op, const Mem& m_) {
  static constexpr uint8_t size_table[] = {
    1, // Prefetch
    0, // kStoreZeroReg
    1, // kStoreZeroU8
    2, // kStoreZeroU16
    4, // kStoreZeroU32
    8  // kStoreZeroU64
  };

  if (op == UniOpM::kPrefetch) {
    cc->prefetcht0(m_);
  }
  else {
    Mem m(m_);
    uint32_t size = size_table[size_t(op)];
    if (size == 0)
      size = cc->register_size();

    m.set_size(size);
    cc->mov(m, 0);
  }
}

void UniCompiler::emit_rm(UniOpRM op, const Gp& dst, const Mem& src) {
  static constexpr uint8_t size_table[] = {
    0, // kLoadReg
    1, // kLoadI8
    1, // kLoadU8
    2, // kLoadI16
    2, // kLoadU16
    4, // kLoadI32
    4, // kLoadU32
    8, // kLoadI64
    8, // kLoadU64
    1, // kLoadMergeU8
    1, // kLoadShiftU8
    2, // kLoadMergeU16
    2  // kLoadShiftU16
  };

  Gp r(dst);
  Mem m(src);

  InstId inst_id = Inst::kIdMov;
  uint32_t size = size_table[size_t(op)];

  switch (op) {
    case UniOpRM::kLoadReg:
      size = dst.size();
      break;

    case UniOpRM::kLoadU8:
    case UniOpRM::kLoadU16:
    case UniOpRM::kLoadU32:
      r.set_signature(RegTraits<RegType::kGp32>::kSignature);
      if (size < 4)
        inst_id = Inst::kIdMovzx;
      break;

    case UniOpRM::kLoadI8:
    case UniOpRM::kLoadI16:
      inst_id = Inst::kIdMovsx;
      break;

    case UniOpRM::kLoadI32:
      inst_id = dst.is_gp64() ? Inst::kIdMovsxd : Inst::kIdMov;
      break;

    case UniOpRM::kLoadI64:
    case UniOpRM::kLoadU64:
      ASMJIT_ASSERT(dst.is_gp64());
      m.set_size(8);
      break;

    case UniOpRM::kLoadShiftU8:
      cc->shl(r, 8);
      [[fallthrough]];

    case UniOpRM::kLoadMergeU8:
      r = r.r8();
      break;

    case UniOpRM::kLoadShiftU16:
      cc->shl(r, 16);
      [[fallthrough]];

    case UniOpRM::kLoadMergeU16:
      r = r.r16();
      break;

    default:
      ASMJIT_NOT_REACHED();
  }

  m.set_size(size);
  cc->emit(inst_id, r, m);
}

struct UniOpMRInfo {
  uint16_t inst_id;
  uint16_t size;
};

void UniCompiler::emit_mr(UniOpMR op, const Mem& dst, const Gp& src) {
  static constexpr UniOpMRInfo op_info_table[] = {
    { Inst::kIdMov, 0 }, // kStoreReg
    { Inst::kIdMov, 1 }, // kStoreU8
    { Inst::kIdMov, 2 }, // kStoreU16
    { Inst::kIdMov, 4 }, // kStoreU32
    { Inst::kIdMov, 8 }, // kStoreU64
    { Inst::kIdAdd, 0 }, // kAddReg,
    { Inst::kIdAdd, 1 }, // kAddU8,
    { Inst::kIdAdd, 2 }, // kAddU16,
    { Inst::kIdAdd, 4 }, // kAddU32,
    { Inst::kIdAdd, 8 }  // kAddU64
  };

  Mem m(dst);
  Gp r = src;

  const UniOpMRInfo& op_info = op_info_table[size_t(op)];

  uint32_t size = op_info.size;
  switch (size) {
    case 0: size = src.size(); break;
    case 1: r = src.r8(); break;
    case 2: r = src.r16(); break;
    case 4: r = src.r32(); break;
    case 8: r = src.r64(); break;

    default:
      ASMJIT_NOT_REACHED();
  }

  m.set_size(size);
  cc->emit(op_info.inst_id, m, r);
}

void UniCompiler::emit_cmov(const Gp& dst, const Operand_& sel, const UniCondition& condition) {
  ConditionApplier ca(condition);
  ca.optimize(*this);
  ca.emit(*this);
  cc->emit(Inst::cmovcc_from_cond(ca.cond), dst, sel);
}

void UniCompiler::emit_select(const Gp& dst, const Operand_& sel1_, const Operand_& sel2_, const UniCondition& condition) {
  ConditionApplier ca(condition);
  ca.optimize(*this);

  bool dst_is_a = ca.a.is_reg() && dst.id() == ca.a.as<Reg>().id();
  bool dst_is_b = ca.b.is_reg() && dst.id() == ca.b.as<Reg>().id();

  Operand sel1(sel1_);
  Operand sel2(sel2_);

  // Reverse the condition if we can place the immediate value first or if `dst == sel2`.
  if ((!sel1.is_imm() && sel2.is_imm()) || (sel2.is_reg() && dst.id() == sel2.id())) {
    ca.reverse();
    std::swap(sel1, sel2);
  }

  bool dst_is_sel = sel1.is_reg() && dst.id() == sel1.id();
  if (sel1 == sel2) {
    if (!dst_is_sel)
      cc->emit(Inst::kIdMov, dst, sel1);
    return;
  }

  if (sel1.is_imm() && sel1.as<Imm>().value() == 0 && !dst_is_a && !dst_is_b && !dst_is_sel) {
    cc->xor_(dst, dst);
    ca.emit(*this);
  }
  else {
    ca.emit(*this);
    if (!dst_is_sel)
      cc->emit(Inst::kIdMov, dst, sel1);
  }

  if (sel2.is_imm()) {
    int64_t value = sel2.as<Imm>().value();
    Mem sel2_mem = cc->new_const(ConstPoolScope::kLocal, &value, dst.size());
    sel2 = sel2_mem;
  }

  cc->emit(Inst::cmovcc_from_cond(x86::negate_cond(ca.cond)), dst, sel2);
}

void UniCompiler::emit_2i(UniOpRR op, const Gp& dst, const Operand_& src_) {
  Operand src(src_);

  // Notes
  //
  //   - CTZ:
  //     - INTEL - No difference, `bsf` and `tzcnt` both have latency ~2.5 cycles.
  //     - AMD   - Big difference, `tzcnt` has only ~1.5 cycle latency while `bsf` has ~2.5 cycles.

  // ArithOp Reg, Any
  // ----------------

  if (src.is_reg_or_mem()) {
    switch (op) {
      case UniOpRR::kCLZ: {
        if (has_lzcnt()) {
          cc->emit(Inst::kIdLzcnt, dst, src);
        }
        else {
          uint32_t msk = (dst.size() * 8u) - 1u;
          cc->emit(Inst::kIdBsr, dst, src);
          cc->xor_(dst, msk);
        }
        return;
      }

      case UniOpRR::kCTZ: {
        cc->emit(has_bmi() ? Inst::kIdTzcnt : Inst::kIdBsf, dst, src);
        return;
      }

      case UniOpRR::kReflect: {
        int nBits = int(dst.size()) * 8 - 1;

        if (src.is_reg() && dst.id() == src.as<Reg>().id()) {
          ASMJIT_ASSERT(dst.size() == src.as<Reg>().size());
          Gp copy = new_similar_reg(dst, "@copy");

          cc->mov(copy, dst);
          cc->sar(copy, nBits);
          cc->xor_(dst, copy);
        }
        else {
          cc->emit(Inst::kIdMov, dst, src);
          cc->sar(dst, nBits);
          cc->emit(Inst::kIdXor, dst, src);
        }
        return;
      }

      default:
        break;
    }
  }

  // ArithOp Reg, Mem
  // ----------------

  if (src.is_mem()) {
    switch (op) {
      case UniOpRR::kBSwap: {
        if (has_movbe()) {
          cc->movbe(dst, src.as<Mem>());
        }
        else {
          cc->mov(dst, src.as<Mem>());
          cc->bswap(dst);
        }
        return;
      }

      default:
        break;
    }

    Gp src_gp = new_similar_reg(dst, "@src");
    cc->mov(src_gp, src.as<Mem>());
    src = src_gp;
  }

  // ArithOp Reg, Reg
  // ----------------

  if (src.is_reg()) {
    const Gp& src_gp = src.as<Gp>();
    bool dst_is_src = dst.id() == src_gp.id();

    switch (op) {
      case UniOpRR::kAbs: {
        if (dst_is_src) {
          Gp tmp = new_similar_reg(dst, "@tmp");
          cc->mov(tmp, dst);
          cc->neg(dst);
          cc->cmovs(dst, tmp);
        }
        else {
          cc->mov(dst, src_gp);
          cc->neg(dst);
          cc->cmovs(dst, src_gp);
        }
        return;
      }

      case UniOpRR::kBSwap: {
        if (!dst_is_src)
          cc->mov(dst, src_gp);
        cc->bswap(dst);
        return;
      }

      case UniOpRR::kNeg:
      case UniOpRR::kNot: {
        if (!dst_is_src)
          cc->mov(dst, src_gp);
        cc->emit(op == UniOpRR::kNeg ? Inst::kIdNeg : Inst::kIdNot, dst);
        return;
      }

      default:
        break;
    }
  }

  // Everything should be handled, so this should never be reached!
  ASMJIT_NOT_REACHED();
}

static constexpr uint64_t kOp3ICommutativeMask =
  (uint64_t(1) << unsigned(UniOpRRR::kAnd )) |
  (uint64_t(1) << unsigned(UniOpRRR::kOr  )) |
  (uint64_t(1) << unsigned(UniOpRRR::kXor )) |
  (uint64_t(1) << unsigned(UniOpRRR::kAdd )) |
  (uint64_t(1) << unsigned(UniOpRRR::kMul )) |
  (uint64_t(1) << unsigned(UniOpRRR::kSMin)) |
  (uint64_t(1) << unsigned(UniOpRRR::kSMax)) |
  (uint64_t(1) << unsigned(UniOpRRR::kUMin)) |
  (uint64_t(1) << unsigned(UniOpRRR::kUMax)) ;

static ASMJIT_INLINE_NODEBUG bool is_op3i_commutative(UniOpRRR op) {
  return (kOp3ICommutativeMask & (uint64_t(1) << unsigned(op))) != 0;
}

struct UniOpRRRMinMaxCMovInst { InstId a, b; };

void UniCompiler::emit_3i(UniOpRRR op, const Gp& dst, const Operand_& src1_, const Operand_& src2_) {
  Operand src1(src1_);
  Operand src2(src2_);

  static constexpr UniOpRRRMinMaxCMovInst arith_min_max_cmov_inst_table[4] = {
    { Inst::kIdCmovl, Inst::kIdCmovg }, // MinI
    { Inst::kIdCmovg, Inst::kIdCmovl }, // MaxI
    { Inst::kIdCmovb, Inst::kIdCmova }, // MinU
    { Inst::kIdCmova, Inst::kIdCmovb }  // MaxU
  };

  static constexpr InstId legacy_shift_inst_table[5] = {
    Inst::kIdShl,  // SHL
    Inst::kIdShr,  // SHR
    Inst::kIdSar,  // SAR
    Inst::kIdRol,  // ROL
    Inst::kIdRor   // ROR
  };

  static constexpr InstId legacy_logical_inst_table[3] = {
    Inst::kIdAnd,  // AND
    Inst::kIdOr,   // OR
    Inst::kIdXor   // XOR
  };

  static constexpr InstId bmi2_shift_inst_table[5] = {
    Inst::kIdShlx, // SHL
    Inst::kIdShrx, // SHR
    Inst::kIdSarx, // SAR
    Inst::kIdNone, // ROL (doesn't exist).
    Inst::kIdNone  // ROR (can only be used with immediate, special handling).
  };

  // ArithOp Reg, Mem, Imm
  // ---------------------

  if (src1.is_mem() && src2.is_imm()) {
    const Mem& a = src1.as<Mem>();
    const Imm& b = src2.as<Imm>();

    switch (op) {
      case UniOpRRR::kMul:
        cc->imul(dst, a, b);
        return;

      default:
        break;
    }

    cc->mov(dst, a);
    src1 = dst;
  }

  if (!src1.is_reg() && is_op3i_commutative(op)) {
    std::swap(src1, src2);
  }

  // ArithOp Reg, Reg, Imm
  // ---------------------

  if (src1.is_reg() && src2.is_imm()) {
    const Gp& a = src1.as<Gp>();
    const Imm& b = src2.as<Imm>();

    bool dst_is_a = dst.id() == a.id();
    ASMJIT_ASSERT(dst.size() == a.size());

    switch (op) {
      case UniOpRRR::kAnd:
      case UniOpRRR::kOr:
      case UniOpRRR::kXor: {
        InstId inst_id = legacy_logical_inst_table[size_t(op) - size_t(UniOpRRR::kAnd)];
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->emit(inst_id, dst, b);
        return;
      }

      case UniOpRRR::kBic: {
        if (!dst_is_a)
          cc->mov(dst, a);

        Imm nImm(~b.value());
        if (dst.size() <= 4)
          nImm.sign_extend_int32();
        cc->and_(dst, nImm);
        return;
      }

      case UniOpRRR::kAdd: {
        if (!dst_is_a && b.is_int32()) {
          lea(dst, x86::ptr(a, b.value_as<int32_t>()));
        }
        else {
          if (!dst_is_a)
            cc->mov(dst, a);

          if (b.value() == 128) {
            cc->sub(dst, -128);
          }
          else {
            cc->add(dst, b);
          }
        }
        return;
      }

      case UniOpRRR::kSub: {
        if (!dst_is_a) {
          lea(dst, x86::ptr(a, int32_t(0u - b.value_as<uint32_t>())));
        }
        else {
          cc->sub(dst, b);
        }
        return;
      }

      case UniOpRRR::kMul: {
        int64_t val = b.value();
        if (dst_is_a && Support::is_power_of_2(uint64_t(val))) {
          cc->shl(dst, Support::ctz(val));
          return;
        }

        switch (b.value()) {
          case 0:
            cc->xor_(dst, dst);
            return;

          case 1:
            if (!dst_is_a)
              cc->mov(dst, a);
            return;

          case 2:
            lea(dst, x86::ptr(a, a));
            return;

          case 3:
            lea(dst, x86::ptr(a, a, 1));
            return;

          case 5:
            lea(dst, x86::ptr(a, a, 2));
            return;

          case 9:
            lea(dst, x86::ptr(a, a, 3));
            return;

          default:
            break;
        }

        cc->imul(dst, a, b);
        return;
      }

      case UniOpRRR::kSMin:
      case UniOpRRR::kSMax:
      case UniOpRRR::kUMin:
      case UniOpRRR::kUMax: {
        const UniOpRRRMinMaxCMovInst& cmov_inst = arith_min_max_cmov_inst_table[size_t(op) - size_t(UniOpRRR::kSMin)];

        if (dst_is_a) {
          Gp tmp = new_similar_reg(dst, "@tmp");
          cc->mov(tmp, b);
          cc->cmp(dst, tmp);
          cc->emit(cmov_inst.b, dst, tmp);
        }
        else {
          cc->mov(dst, b);
          cc->cmp(dst, a);
          cc->emit(cmov_inst.b, dst, a); // cmov_inst.b is correct, we have reversed the comparison in this case.
        }
        return;
      }

      case UniOpRRR::kSll:
        // Optimize `dst = dst << 1`.
        if (b.value() == 1) {
          if (dst_is_a) {
            // `dst = dst + dst`.
            cc->add(dst, dst);
          }
          else if (is_64bit()) {
            // `dst = a + a` (using a 64-bit address saves address-override prefix).
            cc->lea(dst, x86::ptr(a.r64(), a.r64()));
          }
          else {
            // `dst = a + a`.
            cc->lea(dst, x86::ptr(a, a));
          }
          return;
        }
        [[fallthrough]];

      case UniOpRRR::kSrl:
      case UniOpRRR::kSra: {
        InstId legacy_inst_id = legacy_shift_inst_table[size_t(op) - size_t(UniOpRRR::kSll)];

        if (!dst_is_a)
          cc->mov(dst, a);
        cc->emit(legacy_inst_id, dst, b);
        return;
      }

      case UniOpRRR::kRol: {
        if (has_bmi2()) {
          uint32_t reg_size = dst.size() * 8u;
          uint32_t imm = (reg_size - b.value_as<uint32_t>()) & Support::lsb_mask<uint32_t>(reg_size);
          cc->rorx(dst, a, imm);
        }
        else {
          if (!dst_is_a)
            cc->mov(dst, a);
          cc->rol(dst, b);
        }
        return;
      }

      case UniOpRRR::kRor: {
        if (has_bmi2()) {
          cc->rorx(dst, a, b);
        }
        else {
          if (!dst_is_a)
            cc->mov(dst, a);
          cc->ror(dst, b);
        }
        return;
      }

      default:
        break;
    }

    Gp bTmp = new_similar_reg(dst, "@bImm");
    cc->mov(bTmp, b);
    src2 = bTmp;
  }

  // ArithOp Reg, Mem, Reg
  // ---------------------

  if (src1.is_mem() && src2.is_reg()) {
    const Mem& a = src1.as<Mem>();
    const Gp& b = src2.as<Gp>();

    bool dst_is_b = dst.id() == b.id();

    switch (op) {
      case UniOpRRR::kAnd:
      case UniOpRRR::kOr:
      case UniOpRRR::kXor:
      case UniOpRRR::kAdd:
      case UniOpRRR::kMul:
      case UniOpRRR::kSMin:
      case UniOpRRR::kSMax:
      case UniOpRRR::kUMin:
      case UniOpRRR::kUMax:
        // These are commutative, so this should never happen as these should have been corrected to `Reg, Reg, Mem`.
        ASMJIT_NOT_REACHED();

      case UniOpRRR::kSub: {
        ASMJIT_ASSERT(dst.size() == b.size());

        if (dst_is_b) {
          cc->neg(dst);
          cc->add(dst, a);
          return;
        }

        // Bail to `Reg, Reg, Reg` form.
        break;
      }

      case UniOpRRR::kSll:
      case UniOpRRR::kSrl:
      case UniOpRRR::kSra: {
        // Prefer BMI2 variants: SHLX, SHRX, SARX, and RORX.
        if (has_bmi2()) {
          InstId bmi2_inst_id = bmi2_shift_inst_table[size_t(op) - size_t(UniOpRRR::kSll)];
          cc->emit(bmi2_inst_id, dst, a, b.clone_as(dst));
          return;
        }

        // Bail to `Reg, Reg, Reg` form if BMI2 is not available.
        break;
      }

      default:
        break;
    }

    if (!dst_is_b) {
      cc->mov(dst, a);
      src1 = dst;
    }
    else {
      Gp aTmp = new_similar_reg(dst, "@aTmp");
      cc->mov(aTmp, a);
      src1 = aTmp;
    }
  }

  // ArithOp Reg, Reg, Mem
  // ---------------------

  if (src1.is_reg() && src2.is_mem()) {
    const Gp& a = src1.as<Gp>();
    const Mem& b = src2.as<Mem>();

    bool dst_is_a = dst.id() == a.id();
    ASMJIT_ASSERT(dst.size() == a.size());

    switch (op) {
      case UniOpRRR::kAnd:
      case UniOpRRR::kOr:
      case UniOpRRR::kXor: {
        InstId inst_id = legacy_logical_inst_table[size_t(op) - size_t(UniOpRRR::kAnd)];
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->emit(inst_id, dst, b);
        return;
      }

      case UniOpRRR::kBic: {
        Gp tmp = new_similar_reg(dst);
        cc->mov(tmp, b);
        cc->not_(tmp);
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->and_(dst, tmp);
        return;
      }

      case UniOpRRR::kAdd: {
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->add(dst, b);
        return;
      }

      case UniOpRRR::kSub: {
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->sub(dst, b);
        return;
      }

      case UniOpRRR::kMul: {
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->imul(dst, b);
        return;
      }

      case UniOpRRR::kUDiv: {
        Gp tmp1 = new_similar_reg(dst, "@tmp1");
        cc->xor_(tmp1, tmp1);

        if (dst_is_a) {
          cc->div(tmp1, dst, b);
        }
        else {
          cc->mov(dst, a);
          cc->div(tmp1, dst, b);
        }
        return;
      }

      case UniOpRRR::kUMod: {
        Gp tmp1 = new_similar_reg(dst, "@tmp1");
        cc->xor_(tmp1, tmp1);

        if (dst_is_a) {
          cc->div(tmp1, dst, b);
          cc->mov(dst, tmp1);
        }
        else {
          Gp tmp2 = new_similar_reg(dst, "@tmp2");
          cc->mov(tmp2, a);
          cc->div(tmp1, tmp2, b);
          cc->mov(dst, tmp1);
        }
        return;
      }

      case UniOpRRR::kSMin:
      case UniOpRRR::kSMax:
      case UniOpRRR::kUMin:
      case UniOpRRR::kUMax: {
        const UniOpRRRMinMaxCMovInst& cmov_inst = arith_min_max_cmov_inst_table[size_t(op) - size_t(UniOpRRR::kSMin)];

        if (dst_is_a) {
          cc->cmp(dst, b);
          cc->emit(cmov_inst.b, dst, b);
        }
        else {
          cc->mov(dst, b);
          cc->cmp(dst, a);
          cc->emit(cmov_inst.b, dst, a); // cmov_inst.b is correct, we have reversed the comparison in this case.
        }
        return;
      }

      case UniOpRRR::kSBound: {
        cc->xor_(dst, dst);
        cc->cmp(a, b);
        cc->cmovbe(dst, a);
        cc->cmovg(dst, b);
        return;
      }

      default:
        break;
    }

    Gp bTmp = new_similar_reg(dst, "@bTmp");
    cc->mov(bTmp, b);
    src2 = bTmp;
  }

  // ArithOp Reg, Reg, Reg
  // ---------------------

  if (src1.is_reg() && src2.is_reg()) {
    const Gp& a = src1.as<Gp>();
    const Gp& b = src2.as<Gp>();

    bool aIsB = a.id() == b.id();
    bool dst_is_a = dst.id() == a.id();
    bool dst_is_b = dst.id() == b.id();

    ASMJIT_ASSERT(dst.size() == a.size());

    switch (op) {
      case UniOpRRR::kAnd:
      case UniOpRRR::kOr:
      case UniOpRRR::kXor: {
        ASMJIT_ASSERT(dst.size() == b.size());

        InstId inst_id = legacy_logical_inst_table[size_t(op) - size_t(UniOpRRR::kAnd)];
        if (!dst_is_a)
          cc->mov(dst, a);
        cc->emit(inst_id, dst, b);
        return;
      }

      case UniOpRRR::kBic: {
        ASMJIT_ASSERT(dst.size() == b.size());

        if (has_bmi()) {
          cc->andn(dst, b, a);
        }
        else if (dst_is_b) {
          if (dst_is_a) {
            cc->mov(dst, 0);
            return;
          }
          cc->not_(dst);
          cc->and_(dst, a);
        }
        else {
          Gp tmp = new_similar_reg(dst, "@tmp");
          cc->mov(tmp, b);
          cc->not_(tmp);
          if (!dst_is_a)
            cc->mov(dst, a);
          cc->and_(dst, tmp);
        }
        return;
      }

      case UniOpRRR::kAdd: {
        ASMJIT_ASSERT(dst.size() == b.size());

        if (dst_is_a || dst_is_b) {
          cc->add(dst, dst_is_b ? a : b);
        }
        else if (dst.size() >= 4) {
          if (is_64bit())
            lea(dst, x86::ptr(a.r64(), b.r64()));
          else
            lea(dst, x86::ptr(a, b));
        }
        else {
          cc->mov(dst, a);
          cc->add(dst, b);
        }
        return;
      }

      case UniOpRRR::kSub: {
        ASMJIT_ASSERT(dst.size() == b.size());

        if (aIsB) {
          cc->xor_(dst, dst);
        }
        else if (dst_is_a) {
          cc->sub(dst, b);
        }
        else if (dst_is_b) {
          cc->neg(dst);
          cc->add(dst, a);
        }
        else {
          cc->mov(dst, a);
          cc->sub(dst, b);
        }
        return;
      }

      case UniOpRRR::kMul: {
        ASMJIT_ASSERT(dst.size() == b.size());

        if (!dst_is_a && !dst_is_b)
          cc->mov(dst, a);
        cc->imul(dst, dst_is_b ? a : b);
        return;
      }

      case UniOpRRR::kUDiv: {
        ASMJIT_ASSERT(dst.size() == b.size());

        Gp tmp1 = new_similar_reg(dst, "@tmp1");
        cc->xor_(tmp1, tmp1);

        if (dst_is_a) {
          cc->div(tmp1, dst, b);
        }
        else if (dst_is_b) {
          Gp tmp2 = new_similar_reg(dst, "@tmp2");
          cc->mov(tmp2, a);
          cc->div(tmp1, tmp2, b);
          cc->mov(dst, tmp2);
        }
        else {
          cc->mov(dst, a);
          cc->div(tmp1, dst, b);
        }
        return;
      }

      case UniOpRRR::kUMod: {
        ASMJIT_ASSERT(dst.size() == b.size());

        Gp tmp1 = new_similar_reg(dst, "@tmp1");
        cc->xor_(tmp1, tmp1);

        if (dst_is_a) {
          cc->div(tmp1, dst, b);
          cc->mov(dst, tmp1);
        }
        else {
          Gp tmp2 = new_similar_reg(dst, "@tmp2");
          cc->mov(tmp2, a);
          cc->div(tmp1, tmp2, b);
          cc->mov(dst, tmp1);
        }
        return;
      }

      case UniOpRRR::kSMin:
      case UniOpRRR::kSMax:
      case UniOpRRR::kUMin:
      case UniOpRRR::kUMax: {
        ASMJIT_ASSERT(dst.size() == b.size());
        const UniOpRRRMinMaxCMovInst& cmov_inst = arith_min_max_cmov_inst_table[size_t(op) - size_t(UniOpRRR::kSMin)];

        cc->cmp(a, b);
        if (dst_is_b) {
          cc->emit(cmov_inst.a, dst, a);
        }
        else {
          if (!dst_is_a)
            cc->mov(dst, a);
          cc->emit(cmov_inst.b, dst, b);
        }
        return;
      }

      case UniOpRRR::kSll:
      case UniOpRRR::kSrl:
      case UniOpRRR::kSra:
      case UniOpRRR::kRol:
      case UniOpRRR::kRor: {
        // Prefer BMI2 variants: SHLX, SHRX, SARX, and RORX.
        if (has_bmi2()) {
          InstId bmi2_inst_id = bmi2_shift_inst_table[size_t(op) - size_t(UniOpRRR::kSll)];
          if (bmi2_inst_id != Inst::kIdNone) {
            cc->emit(bmi2_inst_id, dst, a, b.clone_as(dst));
            return;
          }
        }

        InstId legacy_inst_id = legacy_shift_inst_table[size_t(op) - size_t(UniOpRRR::kSll)];
        if (dst_is_a) {
          cc->emit(legacy_inst_id, dst, b.r8());
          return;
        }
        else if (dst_is_b) {
          Gp tmp = new_gp32("@tmp");
          if (!dst_is_a)
            cc->mov(dst, a);
          cc->mov(tmp, b.r32());
          cc->emit(legacy_inst_id, dst, tmp.r8());
        }
        else {
          cc->mov(dst, a);
          cc->emit(legacy_inst_id, dst, b.r8());
        }
        return;
      }

      case UniOpRRR::kSBound: {
        if (dst.id() == a.id()) {
          Gp zero = new_similar_reg(dst, "@zero");

          cc->xor_(zero, zero);
          cc->cmp(dst, b);
          cc->cmova(dst, zero);
          cc->cmovg(dst, b);
        }
        else {
          cc->xor_(dst, dst);
          cc->cmp(a, b);
          cc->cmovbe(dst, a);
          cc->cmovg(dst, b);
        }
        return;
      }
    }
  }

  // Everything should be handled, so this should never be reached!
  ASMJIT_NOT_REACHED();
}

void UniCompiler::emit_j(const Operand_& target) {
  cc->emit(Inst::kIdJmp, target);
}

void UniCompiler::emit_j_if(const Label& target, const UniCondition& condition) {
  ConditionApplier ca(condition);
  ca.optimize(*this);
  ca.emit(*this);
  cc->j(ca.cond, target);
}

void UniCompiler::adds_u8(const Gp& dst, const Gp& src1, const Gp& src2) {
  ASMJIT_ASSERT(dst.size() == src1.size());
  ASMJIT_ASSERT(dst.size() == src2.size());

  if (dst.id() == src1.id()) {
    cc->add(dst.r8(), src2.r8());
  }
  else if (dst.id() == src2.id()) {
    cc->add(dst.r8(), src1.r8());
  }
  else {
    cc->mov(dst, src1);
    cc->add(dst, src2);
  }

  Gp u8_msk = new_gp32("@u8_msk");
  cc->sbb(u8_msk, u8_msk);
  cc->or_(dst.r8(), u8_msk.r8());
}

void UniCompiler::inv_u8(const Gp& dst, const Gp& src) {
  if (dst.id() != src.id())
    cc->mov(dst, src);
  cc->xor_(dst.r8(), 0xFF);
}

void UniCompiler::div_255_u32(const Gp& dst, const Gp& src) {
  ASMJIT_ASSERT(dst.size() == src.size());

  if (dst.id() == src.id()) {
    // tmp = src + 128;
    // dst = (tmp + (tmp >> 8)) >> 8
    Gp tmp = new_similar_reg(dst, "@tmp");
    cc->sub(dst, -128);
    cc->mov(tmp, dst);
    cc->shr(tmp, 8);
    cc->add(dst, tmp);
    cc->shr(dst, 8);
  }
  else {
    // dst = (src + 128 + ((src + 128) >> 8)) >> 8
    lea(dst, x86::ptr(src, 128));
    cc->shr(dst, 8);
    lea(dst, x86::ptr(dst, src, 0, 128));
    cc->shr(dst, 8);
  }
}

void UniCompiler::mul_257_hu16(const Gp& dst, const Gp& src) {
  ASMJIT_ASSERT(dst.size() == src.size());
  cc->imul(dst, src, 257);
  cc->shr(dst, 16);
}

void UniCompiler::add_scaled(const Gp& dst, const Gp& a, int b) {
  switch (b) {
    case 1:
      cc->add(dst, a);
      return;

    case 2:
    case 4:
    case 8: {
      uint32_t shift = b == 2 ? 1 :
                       b == 4 ? 2 : 3;
      lea(dst, x86::ptr(dst, a, shift));
      return;
    }

    default: {
      Gp tmp = new_similar_reg(dst, "@tmp");
      cc->imul(tmp, a, b);
      cc->add(dst, tmp);
      return;
    }
  }
}

void UniCompiler::add_ext(const Gp& dst, const Gp& src_, const Gp& idx_, uint32_t scale, int32_t disp) {
  ASMJIT_ASSERT(scale != 0u);

  Gp src = src_.clone_as(dst);
  Gp idx = idx_.clone_as(dst);

  switch (scale) {
    case 1:
      if (dst.id() == src.id() && disp == 0) {
        cc->add(dst, idx);
        return;
      }
      [[fallthrough]];

    case 2:
    case 4:
    case 8:
      lea(dst, x86::ptr(src, idx, Support::ctz(scale), disp));
      return;

    default:
      break;
  }

  if (src.id() == idx.id()) {
    cc->imul(dst, src, scale + 1);
    return;
  }

  if (dst.id() != idx.id() && scale == 3) {
    lea(dst, x86::ptr(src, idx, 1, disp));
    cc->add(dst, idx);
    return;
  }

  Gp tmp = new_similar_reg(dst);
  cc->imul(tmp, idx, scale);
  cc->lea(dst, x86::ptr(src, tmp));
}

void UniCompiler::lea(const Gp& dst, const Mem& src) {
  Mem m(src);

  if (is_64bit() && dst.size() == 4) {
    if (m.base_type() == RegType::kGp32) {
      m.set_base_type(RegType::kGp64);
    }

    if (m.index_type() == RegType::kGp32) {
      m.set_index_type(RegType::kGp64);
    }
  }

  cc->lea(dst, m);
}

// ujit::UniCompiler - Vector Instructions - Constants
// ===================================================

//! Floating point mode is used in places that are generic and implement various functionality that needs more
//! than a single instruction. Typically implementing either higher level concepts or missing functionality.
enum FloatMode : uint8_t {
  //! Scalar 32-bit floating point operation.
  kF32S = 0,
  //! Scalar 64-bit floating point operation.
  kF64S = 1,
  //! Vector 32-bit floating point operation.
  kF32V = 2,
  //! Vector 64-bit floating point operation.
  kF64V = 3,

  //! Used by non-floating point instructions.
  kNone = 4
};

enum class ElementSize : uint8_t {
  k8,
  k16,
  k32,
  k64
};

enum class SameVecOp : uint8_t {
  kNone = 0,
  kZero = 1,
  kOnes = 2,
  kSrc = 3
};

enum class VecPart : uint8_t {
  kNA = 0,
  kLo = 0,
  kHi = 1
};

enum class WideningOp : uint32_t  {
  kNone,
  kI8ToI16,
  kU8ToU16,
  kI8ToI32,
  kU8ToU32,
  kU8ToU64,
  kI16ToI32,
  kU16ToU32,
  kI32ToI64,
  kU32ToU64
};

enum class NarrowingOp : uint32_t  {
  kNone,
  kI16ToI8,
  kI16ToU8,
  kU16ToU8,
  kI32ToI16,
  kI32ToU16,
  kU32ToU16,
  kI64ToI32,
  kI64ToU32,
  kU64ToU32
};

enum class NarrowingMode : uint32_t {
  kTruncate,
  kSaturateSToU,
  kSaturateSToS,
  kSaturateUToU
};

[[maybe_unused]]
static ASMJIT_INLINE bool is_scalar_fp_op(FloatMode fm) noexcept { return fm <= kF64S; }

[[maybe_unused]]
static ASMJIT_INLINE bool is_f32_op(FloatMode fm) noexcept { return fm == kF32S || fm == kF32V; }

[[maybe_unused]]
static ASMJIT_INLINE bool is_f64_op(FloatMode fm) noexcept { return fm == kF64S || fm == kF64V; }

// ujit::UniCompiler - Vector Instructions - Broadcast / Shuffle Data
// ==================================================================

static constexpr uint16_t avx512_vinsert_128[] = {
  Inst::kIdVinserti32x4,
  Inst::kIdVinserti64x2,
  Inst::kIdVinsertf32x4,
  Inst::kIdVinsertf64x2
};

static constexpr uint16_t avx512_vshuf_128[] = {
  Inst::kIdVshufi32x4,
  Inst::kIdVshufi64x2,
  Inst::kIdVshuff32x4,
  Inst::kIdVshuff64x2
};

// ujit::UniCompiler - Vector Instructions - Integer Cmp/Min/Max Data
// ==================================================================

struct CmpMinMaxInst {
  uint16_t peq;
  uint16_t pgt;
  uint16_t pmin;
  uint16_t pmax;
};

static constexpr CmpMinMaxInst sse_cmp_min_max[] = {
  { Inst::kIdPcmpeqb, Inst::kIdPcmpgtb, Inst::kIdPminsb, Inst::kIdPmaxsb },
  { Inst::kIdPcmpeqb, Inst::kIdPcmpgtb, Inst::kIdPminub, Inst::kIdPmaxub },
  { Inst::kIdPcmpeqw, Inst::kIdPcmpgtw, Inst::kIdPminsw, Inst::kIdPmaxsw },
  { Inst::kIdPcmpeqw, Inst::kIdPcmpgtw, Inst::kIdPminuw, Inst::kIdPmaxuw },
  { Inst::kIdPcmpeqd, Inst::kIdPcmpgtd, Inst::kIdPminsd, Inst::kIdPmaxsd },
  { Inst::kIdPcmpeqd, Inst::kIdPcmpgtd, Inst::kIdPminud, Inst::kIdPmaxud },
  { Inst::kIdPcmpeqq, Inst::kIdPcmpgtq, Inst::kIdNone  , Inst::kIdNone   },
  { Inst::kIdPcmpeqq, Inst::kIdPcmpgtq, Inst::kIdNone  , Inst::kIdNone   },
};

static constexpr CmpMinMaxInst avx_cmp_min_max[] = {
  { Inst::kIdVpcmpeqb, Inst::kIdVpcmpgtb, Inst::kIdVpminsb, Inst::kIdVpmaxsb },
  { Inst::kIdVpcmpeqb, Inst::kIdVpcmpgtb, Inst::kIdVpminub, Inst::kIdVpmaxub },
  { Inst::kIdVpcmpeqw, Inst::kIdVpcmpgtw, Inst::kIdVpminsw, Inst::kIdVpmaxsw },
  { Inst::kIdVpcmpeqw, Inst::kIdVpcmpgtw, Inst::kIdVpminuw, Inst::kIdVpmaxuw },
  { Inst::kIdVpcmpeqd, Inst::kIdVpcmpgtd, Inst::kIdVpminsd, Inst::kIdVpmaxsd },
  { Inst::kIdVpcmpeqd, Inst::kIdVpcmpgtd, Inst::kIdVpminud, Inst::kIdVpmaxud },
  { Inst::kIdVpcmpeqq, Inst::kIdVpcmpgtq, Inst::kIdVpminsq, Inst::kIdVpmaxsq },
  { Inst::kIdVpcmpeqq, Inst::kIdVpcmpgtq, Inst::kIdVpminuq, Inst::kIdVpmaxuq },
};

// ujit::UniCompiler - Vector Instructions - Integer Conversion Data
// =================================================================

struct WideningOpInfo {
  uint32_t mov          : 16;
  uint32_t unpack_lo    : 16;
  uint32_t unpack_hi    : 13;
  uint32_t sign_extends : 1;
  uint32_t reserved     : 5;
};

struct NarrowingOpInfo {
  uint32_t mov          : 13;
  uint32_t pack         : 13;
  uint32_t sign         : 1;
  uint32_t mode         : 2;
  uint32_t reserved     : 19;
};

static constexpr WideningOpInfo sse_int_widening_op_info[] = {
  { Inst::kIdNone     , Inst::kIdNone      , Inst::kIdNone      , 0, 0 }, // kNone.
  { Inst::kIdPmovsxbw , Inst::kIdPunpcklbw , Inst::kIdPunpckhbw , 1, 0 }, // kI8ToI16.
  { Inst::kIdPmovzxbw , Inst::kIdPunpcklbw , Inst::kIdPunpckhbw , 0, 0 }, // kU8ToU16.
  { Inst::kIdPmovsxbd , Inst::kIdNone      , Inst::kIdNone      , 1, 0 }, // kI8ToI32.
  { Inst::kIdPmovzxbd , Inst::kIdNone      , Inst::kIdNone      , 0, 0 }, // kU8ToU32.
  { Inst::kIdPmovzxbq , Inst::kIdNone      , Inst::kIdNone      , 0, 0 }, // kU8ToU64.
  { Inst::kIdPmovsxwd , Inst::kIdPunpcklwd , Inst::kIdPunpckhwd , 1, 0 }, // kI16ToI32.
  { Inst::kIdPmovzxwd , Inst::kIdPunpcklwd , Inst::kIdPunpckhwd , 0, 0 }, // kU16ToU32.
  { Inst::kIdPmovsxdq , Inst::kIdPunpckldq , Inst::kIdPunpckhdq , 1, 0 }, // kI32ToI64.
  { Inst::kIdPmovzxdq , Inst::kIdPunpckldq , Inst::kIdPunpckhdq , 0, 0 }  // kU32ToU64.
};

// ujit::UniCompiler - Vector Instructions - Float Instruction Data
// ================================================================

struct FloatInst {
  uint16_t fmovs;
  uint16_t fmova;
  uint16_t fmovu;
  uint16_t fand;
  uint16_t for_;
  uint16_t fxor;
  uint16_t fandn;
  uint16_t fadd;
  uint16_t fsub;
  uint16_t fmul;
  uint16_t fdiv;
  uint16_t fmin;
  uint16_t fmax;
  uint16_t fcmp;
  uint16_t fround;
  uint16_t frndscale;
  uint16_t psrl;
  uint16_t psll;
};

static constexpr FloatInst sse_float_inst[4] = {
  {
    Inst::kIdMovss,
    Inst::kIdMovaps,
    Inst::kIdMovups,
    Inst::kIdAndps,
    Inst::kIdOrps,
    Inst::kIdXorps,
    Inst::kIdAndnps,
    Inst::kIdAddss,
    Inst::kIdSubss,
    Inst::kIdMulss,
    Inst::kIdDivss,
    Inst::kIdMinss,
    Inst::kIdMaxss,
    Inst::kIdCmpss,
    Inst::kIdRoundss,
    Inst::kIdNone,
    Inst::kIdPsrld,
    Inst::kIdPslld
  },
  {
    Inst::kIdMovsd,
    Inst::kIdMovaps,
    Inst::kIdMovups,
    Inst::kIdAndpd,
    Inst::kIdOrpd,
    Inst::kIdXorpd,
    Inst::kIdAndnpd,
    Inst::kIdAddsd,
    Inst::kIdSubsd,
    Inst::kIdMulsd,
    Inst::kIdDivsd,
    Inst::kIdMinsd,
    Inst::kIdMaxsd,
    Inst::kIdCmpsd,
    Inst::kIdRoundsd,
    Inst::kIdNone,
    Inst::kIdPsrlq,
    Inst::kIdPsllq
  },
  {
    Inst::kIdMovaps,
    Inst::kIdMovaps,
    Inst::kIdMovups,
    Inst::kIdAndps,
    Inst::kIdOrps,
    Inst::kIdXorps,
    Inst::kIdAndnps,
    Inst::kIdAddps,
    Inst::kIdSubps,
    Inst::kIdMulps,
    Inst::kIdDivps,
    Inst::kIdMinps,
    Inst::kIdMaxps,
    Inst::kIdCmpps,
    Inst::kIdRoundps,
    Inst::kIdNone,
    Inst::kIdPsrld,
    Inst::kIdPslld
  },
  {
    Inst::kIdMovaps,
    Inst::kIdMovaps,
    Inst::kIdMovups,
    Inst::kIdAndpd,
    Inst::kIdOrpd,
    Inst::kIdXorpd,
    Inst::kIdAndnpd,
    Inst::kIdAddpd,
    Inst::kIdSubpd,
    Inst::kIdMulpd,
    Inst::kIdDivpd,
    Inst::kIdMinpd,
    Inst::kIdMaxpd,
    Inst::kIdCmppd,
    Inst::kIdRoundpd,
    Inst::kIdNone,
    Inst::kIdPsrlq,
    Inst::kIdPsllq
  }
};

static constexpr FloatInst avx_float_inst[4] = {
  {
    Inst::kIdVmovss,
    Inst::kIdVmovaps,
    Inst::kIdVmovups,
    Inst::kIdVandps,
    Inst::kIdVorps,
    Inst::kIdVxorps,
    Inst::kIdVandnps,
    Inst::kIdVaddss,
    Inst::kIdVsubss,
    Inst::kIdVmulss,
    Inst::kIdVdivss,
    Inst::kIdVminss,
    Inst::kIdVmaxss,
    Inst::kIdVcmpss,
    Inst::kIdVroundss,
    Inst::kIdVrndscaless,
    Inst::kIdVpsrld,
    Inst::kIdVpslld
  },
  {
    Inst::kIdVmovsd,
    Inst::kIdVmovaps,
    Inst::kIdVmovups,
    Inst::kIdVandpd,
    Inst::kIdVorpd,
    Inst::kIdVxorpd,
    Inst::kIdVandnpd,
    Inst::kIdVaddsd,
    Inst::kIdVsubsd,
    Inst::kIdVmulsd,
    Inst::kIdVdivsd,
    Inst::kIdVminsd,
    Inst::kIdVmaxsd,
    Inst::kIdVcmpsd,
    Inst::kIdVroundsd,
    Inst::kIdVrndscalesd,
    Inst::kIdVpsrlq,
    Inst::kIdVpsllq
  },
  {
    Inst::kIdVmovaps,
    Inst::kIdVmovaps,
    Inst::kIdVmovups,
    Inst::kIdVandps,
    Inst::kIdVorps,
    Inst::kIdVxorps,
    Inst::kIdVandnps,
    Inst::kIdVaddps,
    Inst::kIdVsubps,
    Inst::kIdVmulps,
    Inst::kIdVdivps,
    Inst::kIdVminps,
    Inst::kIdVmaxps,
    Inst::kIdVcmpps,
    Inst::kIdVroundps,
    Inst::kIdVrndscaleps,
    Inst::kIdVpsrld,
    Inst::kIdVpslld
  },
  {
    Inst::kIdVmovaps,
    Inst::kIdVmovaps,
    Inst::kIdVmovups,
    Inst::kIdVandpd,
    Inst::kIdVorpd,
    Inst::kIdVxorpd,
    Inst::kIdVandnpd,
    Inst::kIdVaddpd,
    Inst::kIdVsubpd,
    Inst::kIdVmulpd,
    Inst::kIdVdivpd,
    Inst::kIdVminpd,
    Inst::kIdVmaxpd,
    Inst::kIdVcmppd,
    Inst::kIdVroundpd,
    Inst::kIdVrndscalepd,
    Inst::kIdVpsrlq,
    Inst::kIdVpsllq
  }
};

// ujit::UniCompiler - Vector Instructions - UniOp Information
// ===========================================================

struct UniOpVInfo {
  //! \name Members
  //! \{

  uint32_t sse_inst_id    : 13;
  uint32_t sse_op_count   : 3;
  uint32_t sse_ext        : 3;
  uint32_t avx_inst_id    : 13;
  uint32_t avx_ext        : 6;
  uint32_t commutative    : 1;
  uint32_t comparison     : 1;
  uint32_t same_vec_op    : 3;
  uint32_t use_imm        : 1;
  uint32_t imm            : 8;
  uint32_t float_mode     : 3;
  uint32_t element_size   : 2;
  uint32_t broadcast_size : 4;
  uint32_t hi             : 1;
  uint32_t reserved       : 3;

  //! \}
};

#define DEFINE_OP(sse_inst_id, sse_op_count, sse_ext, avx_inst_id, avx_ext, commutative, comparison, same_vec_op, use_imm, imm, float_mode, element_size, broadcast_size, vec_part) \
  UniOpVInfo {                           \
    Inst::sse_inst_id,                   \
    sse_op_count,                        \
    uint8_t(SSEExt::sse_ext),            \
    Inst::avx_inst_id,                   \
    uint8_t(AVXExt::avx_ext),            \
    commutative,                         \
    comparison,                          \
    uint8_t(SameVecOp::same_vec_op),     \
    use_imm,                             \
    imm,                                 \
    uint8_t(FloatMode::float_mode),      \
    uint8_t(ElementSize::element_size),  \
    broadcast_size,                      \
    uint8_t(VecPart::vec_part),          \
    0                                    \
  }

static constexpr UniOpVInfo opcode_info_2v[size_t(UniOpVV::kMaxValue) + 1] = {
  DEFINE_OP(kIdMovaps     , 0, kIntrin, kIdVmovaps        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kMov.
  DEFINE_OP(kIdMovq       , 0, kIntrin, kIdVmovq          , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kMovU64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastb   , kIntrin     , 0, 0, kNone, 0, 0x01u, kNone, k8 , 0, kNA), // kBroadcastU8Z.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastw   , kIntrin     , 0, 0, kNone, 0, 0x01u, kNone, k16, 0, kNA), // kBroadcastU16Z.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastb   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kBroadcastU8.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastw   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kBroadcastU16.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastd   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpbroadcastq   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastU64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcastss   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcastsd   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti32x4, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastV128_U32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti64x2, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastV128_U64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcastf32x4, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastV128_F32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcastf64x2, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastV128_F64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti32x8, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastV256_U32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti64x4, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastV256_U64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti32x8, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kBroadcastV256_F32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVbroadcasti64x4, kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kBroadcastV256_F64.
  DEFINE_OP(kIdPabsb      , 2, kSSSE3 , kIdVpabsb         , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kAbsI8.
  DEFINE_OP(kIdPabsw      , 2, kSSSE3 , kIdVpabsw         , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kAbsI16.
  DEFINE_OP(kIdPabsd      , 2, kSSSE3 , kIdVpabsd         , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kAbsI32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpabsq         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kAbsI64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kNotU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kNotU64.
  DEFINE_OP(kIdPmovsxbw   , 0, kIntrin, kIdVpmovsxbw      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kCvtI8LoToI16.
  DEFINE_OP(kIdPmovsxbw   , 0, kIntrin, kIdVpmovsxbw      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kCvtI8HiToI16.
  DEFINE_OP(kIdPmovzxbw   , 0, kIntrin, kIdVpmovzxbw      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kCvtU8LoToU16.
  DEFINE_OP(kIdPmovzxbw   , 0, kIntrin, kIdVpmovzxbw      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kCvtU8HiToU16.
  DEFINE_OP(kIdPmovsxbd   , 0, kIntrin, kIdVpmovsxbd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtI8ToI32.
  DEFINE_OP(kIdPmovzxbd   , 0, kIntrin, kIdVpmovzxbd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtU8ToU32.
  DEFINE_OP(kIdPmovsxwd   , 0, kIntrin, kIdVpmovsxwd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtI16LoToI32.
  DEFINE_OP(kIdPmovsxwd   , 0, kIntrin, kIdVpmovsxwd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtI16HiToI32.
  DEFINE_OP(kIdPmovzxwd   , 0, kIntrin, kIdVpmovzxwd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtU16LoToU32.
  DEFINE_OP(kIdPmovzxwd   , 0, kIntrin, kIdVpmovzxwd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtU16HiToU32.
  DEFINE_OP(kIdPmovsxdq   , 0, kIntrin, kIdVpmovsxdq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtI32LoToI64.
  DEFINE_OP(kIdPmovsxdq   , 0, kIntrin, kIdVpmovsxdq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtI32HiToI64.
  DEFINE_OP(kIdPmovzxdq   , 0, kIntrin, kIdVpmovzxdq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtU32LoToU64.
  DEFINE_OP(kIdPmovzxdq   , 0, kIntrin, kIdVpmovzxdq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtU32HiToU64.
  DEFINE_OP(kIdAndps      , 0, kIntrin, kIdVandps         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kAbsF32S.
  DEFINE_OP(kIdAndpd      , 0, kIntrin, kIdVandpd         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kAbsF64S.
  DEFINE_OP(kIdAndps      , 0, kIntrin, kIdVandps         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kAbsF32.
  DEFINE_OP(kIdAndpd      , 0, kIntrin, kIdVandpd         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kAbsF64.
  DEFINE_OP(kIdXorps      , 0, kIntrin, kIdVxorps         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kNegF32S.
  DEFINE_OP(kIdXorpd      , 0, kIntrin, kIdVxorpd         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kNegF64S.
  DEFINE_OP(kIdXorps      , 0, kIntrin, kIdVxorps         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kNegF32.
  DEFINE_OP(kIdXorpd      , 0, kIntrin, kIdVxorpd         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kNegF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kAbsU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kAbsU64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF32S, k32, 4, kNA), // kTruncF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF64S, k64, 8, kNA), // kTruncF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF32V, k32, 4, kNA), // kTruncF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF64V, k64, 8, kNA), // kTruncF64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF32S, k32, 4, kNA), // kFloorF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF64S, k64, 8, kNA), // kFloorF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF32V, k32, 4, kNA), // kFloorF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF64V, k64, 8, kNA), // kFloorF64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x0Au, kF32S, k32, 4, kNA), // kCeilF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x0Au, kF64S, k64, 8, kNA), // kCeilF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x0Au, kF32V, k32, 4, kNA), // kCeilF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x0Au, kF64V, k64, 8, kNA), // kCeilF64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x08u, kF32S, k32, 4, kNA), // kRoundEvenF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x08u, kF64S, k64, 8, kNA), // kRoundEvenF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x08u, kF32V, k32, 4, kNA), // kRoundEvenF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x08u, kF64V, k64, 8, kNA), // kRoundEvenF64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF32S, k32, 4, kNA), // kRoundHalfAwayF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF64S, k64, 8, kNA), // kRoundHalfAwayF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF32V, k32, 4, kNA), // kRoundHalfAwayF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x0Bu, kF64V, k64, 8, kNA), // kRoundHalfAwayF64.
  DEFINE_OP(kIdRoundss    , 2, kIntrin, kIdVroundss       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF32S, k32, 4, kNA), // kRoundHalfUpF32S.
  DEFINE_OP(kIdRoundsd    , 2, kIntrin, kIdVroundsd       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF64S, k64, 8, kNA), // kRoundHalfUpF64S.
  DEFINE_OP(kIdRoundps    , 2, kIntrin, kIdVroundps       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF32V, k32, 4, kNA), // kRoundHalfUpF32.
  DEFINE_OP(kIdRoundpd    , 2, kIntrin, kIdVroundpd       , kIntrin     , 0, 0, kNone, 1, 0x09u, kF64V, k64, 8, kNA), // kRoundHalfUpF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kRcpF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kRcpF64.
  DEFINE_OP(kIdSqrtss     , 2, kIntrin, kIdVsqrtss        , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kSqrtF32S.
  DEFINE_OP(kIdSqrtsd     , 2, kIntrin, kIdVsqrtsd        , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kSqrtF64S.
  DEFINE_OP(kIdSqrtps     , 2, kSSE2  , kIdVsqrtps        , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kSqrtF32.
  DEFINE_OP(kIdSqrtpd     , 2, kSSE2  , kIdVsqrtpd        , kAVX        , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kSqrtF64.
  DEFINE_OP(kIdCvtss2sd   , 2, kIntrin, kIdVcvtss2sd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 0, kNA), // kCvtF32ToF64S.
  DEFINE_OP(kIdCvtsd2ss   , 2, kIntrin, kIdVcvtsd2ss      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k32, 0, kNA), // kCvtF64ToF32S.
  DEFINE_OP(kIdCvtdq2ps   , 2, kSSE2  , kIdVcvtdq2ps      , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kCvtI32ToF32.
  DEFINE_OP(kIdCvtps2pd   , 2, kSSE2  , kIdVcvtps2pd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k64, 4, kLo), // kCvtF32LoToF64.
  DEFINE_OP(kIdCvtps2pd   , 2, kIntrin, kIdVcvtps2pd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k64, 4, kHi), // kCvtF32HiToF64.
  DEFINE_OP(kIdCvtpd2ps   , 2, kSSE2  , kIdVcvtpd2ps      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kLo), // kCvtF64ToF32Lo.
  DEFINE_OP(kIdCvtpd2ps   , 2, kIntrin, kIdVcvtpd2ps      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kHi), // kCvtF64ToF32Hi.
  DEFINE_OP(kIdCvtdq2pd   , 2, kSSE2  , kIdVcvtdq2pd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 4, kLo), // kCvtI32LoToF64.
  DEFINE_OP(kIdCvtdq2pd   , 2, kIntrin, kIdVcvtdq2pd      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 4, kHi), // kCvtI32HiToF64.
  DEFINE_OP(kIdCvttps2dq  , 2, kSSE2  , kIdVcvttps2dq     , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kCvtTruncF32ToI32.
  DEFINE_OP(kIdCvttpd2dq  , 2, kSSE2  , kIdVcvttpd2dq     , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kLo), // kCvtTruncF64ToI32Lo.
  DEFINE_OP(kIdCvttpd2dq  , 2, kIntrin, kIdVcvttpd2dq     , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kHi), // kCvtTruncF64ToI32Hi.
  DEFINE_OP(kIdCvtps2dq   , 2, kSSE2  , kIdVcvtps2dq      , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kCvtRoundF32ToI32.
  DEFINE_OP(kIdCvtpd2dq   , 2, kSSE2  , kIdVcvtpd2dq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kLo), // kCvtRoundF64ToI32Lo.
  DEFINE_OP(kIdCvtpd2dq   , 2, kIntrin, kIdVcvtpd2dq      , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k32, 4, kHi)  // kCvtRoundF64ToI32Hi.
};

static constexpr UniOpVInfo opcode_info_2vs[size_t(UniOpVR::kMaxValue) + 1] = {
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kMov.
  DEFINE_OP(kIdMovd       , 0, kSSE2  , kIdVmovd          , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kMovU32.
  DEFINE_OP(kIdMovq       , 0, kSSE2  , kIdVmovq          , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kMovU64.
  DEFINE_OP(kIdPinsrb     , 0, kSSE4_1, kIdVpinsrb        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertU8.
  DEFINE_OP(kIdPinsrw     , 0, kSSE2  , kIdVpinsrw        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kInsertU16.
  DEFINE_OP(kIdPinsrd     , 0, kSSE4_1, kIdVpinsrd        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kInsertU32.
  DEFINE_OP(kIdPinsrq     , 0, kSSE4_1, kIdVpinsrq        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kInsertU64.
  DEFINE_OP(kIdPextrb     , 0, kSSE4_1, kIdVpextrb        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kExtractU8.
  DEFINE_OP(kIdPextrw     , 0, kSSE2  , kIdVpextrw        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kExtractU16.
  DEFINE_OP(kIdPextrd     , 0, kSSE4_1, kIdVpextrd        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kExtractU32.
  DEFINE_OP(kIdPextrq     , 0, kSSE4_1, kIdVpextrq        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kExtractU64.
  DEFINE_OP(kIdCvtsi2ss   , 0, kSSE2  , kIdVcvtsi2ss      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtIntToF32.
  DEFINE_OP(kIdCvtsi2sd   , 0, kSSE2  , kIdVcvtsi2sd      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtIntToF64.
  DEFINE_OP(kIdCvttss2si  , 0, kSSE2  , kIdVcvttss2si     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtTruncF32ToInt.
  DEFINE_OP(kIdCvtss2si   , 0, kSSE2  , kIdVcvtss2si      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kCvtRoundF32ToInt.
  DEFINE_OP(kIdCvttsd2si  , 0, kSSE2  , kIdVcvttsd2si     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCvtTruncF64ToInt.
  DEFINE_OP(kIdCvtsd2si   , 0, kSSE2  , kIdVcvtsd2si      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA)  // kCvtRoundF64ToInt.
};

static constexpr UniOpVInfo opcode_info_2vi[size_t(UniOpVVI::kMaxValue) + 1] = {
  DEFINE_OP(kIdPsllw      , 2, kSSE2  , kIdVpsllw         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSllU16.
  DEFINE_OP(kIdPslld      , 2, kSSE2  , kIdVpslld         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kSllU32.
  DEFINE_OP(kIdPsllq      , 2, kSSE2  , kIdVpsllq         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kSllU64.
  DEFINE_OP(kIdPsrlw      , 2, kSSE2  , kIdVpsrlw         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSrlU16.
  DEFINE_OP(kIdPsrld      , 2, kSSE2  , kIdVpsrld         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kSrlU32.
  DEFINE_OP(kIdPsrlq      , 2, kSSE2  , kIdVpsrlq         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kSrlU64.
  DEFINE_OP(kIdPsraw      , 2, kSSE2  , kIdVpsraw         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSraI16.
  DEFINE_OP(kIdPsrad      , 2, kSSE2  , kIdVpsrad         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kSraI32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpsraq         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kSraI64.
  DEFINE_OP(kIdPslldq     , 2, kSSE2  , kIdVpslldq        , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kSllbU128.
  DEFINE_OP(kIdPsrldq     , 2, kSSE2  , kIdVpsrldq        , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kSrlbU128.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSwizzleU16x4 (intrin).
  DEFINE_OP(kIdPshuflw    , 3, kIntrin, kIdVpshuflw       , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSwizzleLoU16x4.
  DEFINE_OP(kIdPshufhw    , 3, kIntrin, kIdVpshufhw       , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSwizzleHiU16x4.
  DEFINE_OP(kIdPshufd     , 3, kIntrin, kIdVpshufd        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kSwizzleU32x4.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kSwizzleU64x2 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k32, 0, kNA), // kSwizzleF32x4 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 0, kNA), // kSwizzleF64x2 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermq         , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kSwizzleU64x4 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermq         , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 0, kNA), // kSwizzleF64x4 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kExtractV128_I32 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kExtractV128_I64 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k64, 0, kNA), // kExtractV128_F32 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 0, kNA), // kExtractV128_F64 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kExtractV256_I32 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kExtractV256_I64 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k64, 0, kNA), // kExtractV256_F32 (intrin).
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 0, kNA)  // kExtractV256_F64 (intrin).
};

static constexpr UniOpVInfo opcode_info_3v[size_t(UniOpVVV::kMaxValue) + 1] = {
  DEFINE_OP(kIdPand       , 2, kSSE2  , kIdVpandd         , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kAndU32.
  DEFINE_OP(kIdPand       , 2, kSSE2  , kIdVpandq         , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kAndU64.
  DEFINE_OP(kIdPor        , 2, kSSE2  , kIdVpord          , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kOrU32.
  DEFINE_OP(kIdPor        , 2, kSSE2  , kIdVporq          , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kOrU64.
  DEFINE_OP(kIdPxor       , 2, kSSE2  , kIdVpxord         , kAVX        , 1, 0, kZero, 0, 0x00u, kNone, k32, 4, kNA), // kXorU32.
  DEFINE_OP(kIdPxor       , 2, kSSE2  , kIdVpxorq         , kAVX        , 1, 0, kZero, 0, 0x00u, kNone, k64, 8, kNA), // kXorU64.
  DEFINE_OP(kIdPandn      , 2, kSSE2  , kIdVpandnd        , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k32, 4, kNA), // kAndnU32.
  DEFINE_OP(kIdPandn      , 2, kSSE2  , kIdVpandnq        , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k64, 8, kNA), // kAndnU64.
  DEFINE_OP(kIdPandn      , 0, kIntrin, kIdVpandnd        , kIntrin     , 0, 0, kZero, 0, 0x00u, kNone, k32, 4, kNA), // kBicU32.
  DEFINE_OP(kIdPandn      , 0, kIntrin, kIdVpandnq        , kIntrin     , 0, 0, kZero, 0, 0x00u, kNone, k64, 8, kNA), // kBicU64.
  DEFINE_OP(kIdPavgb      , 2, kSSE2  , kIdVpavgb         , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k8 , 0, kNA), // kAvgrU8.
  DEFINE_OP(kIdPavgw      , 2, kSSE2  , kIdVpavgw         , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k16, 0, kNA), // kAvgrU16.
  DEFINE_OP(kIdPaddb      , 2, kSSE2  , kIdVpaddb         , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kAddU8.
  DEFINE_OP(kIdPaddw      , 2, kSSE2  , kIdVpaddw         , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kAddU16.
  DEFINE_OP(kIdPaddd      , 2, kSSE2  , kIdVpaddd         , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kAddU32.
  DEFINE_OP(kIdPaddq      , 2, kSSE2  , kIdVpaddq         , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kAddU64.
  DEFINE_OP(kIdPsubb      , 2, kSSE2  , kIdVpsubb         , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k8 , 0, kNA), // kSubU8.
  DEFINE_OP(kIdPsubw      , 2, kSSE2  , kIdVpsubw         , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k16, 0, kNA), // kSubU16.
  DEFINE_OP(kIdPsubd      , 2, kSSE2  , kIdVpsubd         , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k32, 4, kNA), // kSubU32.
  DEFINE_OP(kIdPsubq      , 2, kSSE2  , kIdVpsubq         , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k64, 8, kNA), // kSubU64.
  DEFINE_OP(kIdPaddsb     , 2, kSSE2  , kIdVpaddsb        , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kAddsI8.
  DEFINE_OP(kIdPaddusb    , 2, kSSE2  , kIdVpaddusb       , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kAddsU8.
  DEFINE_OP(kIdPaddsw     , 2, kSSE2  , kIdVpaddsw        , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kAddsI16.
  DEFINE_OP(kIdPaddusw    , 2, kSSE2  , kIdVpaddusw       , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kAddsU16.
  DEFINE_OP(kIdPsubsb     , 2, kSSE2  , kIdVpsubsb        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kSubsI8.
  DEFINE_OP(kIdPsubusb    , 2, kSSE2  , kIdVpsubusb       , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k8 , 0, kNA), // kSubsU8.
  DEFINE_OP(kIdPsubsw     , 2, kSSE2  , kIdVpsubsw        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kSubsI16.
  DEFINE_OP(kIdPsubusw    , 2, kSSE2  , kIdVpsubusw       , kAVX        , 0, 0, kZero, 0, 0x00u, kNone, k16, 0, kNA), // kSubsU16.
  DEFINE_OP(kIdPmullw     , 2, kSSE2  , kIdVpmullw        , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMulU16.
  DEFINE_OP(kIdPmulld     , 2, kSSE4_1, kIdVpmulld        , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kMulU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpmullq        , kAVX512     , 1, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kMulU64.
  DEFINE_OP(kIdPmulhw     , 2, kSSE2  , kIdVpmulhw        , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMulhI16.
  DEFINE_OP(kIdPmulhuw    , 2, kSSE2  , kIdVpmulhuw       , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMulhU16.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMulU64_LoU32.
  DEFINE_OP(kIdPmaddwd    , 2, kSSE2  , kIdVpmaddwd       , kAVX        , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMHAddI16_I32.
  DEFINE_OP(kIdPminsb     , 2, kSSE4_1, kIdVpminsb        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k8 , 0, kNA), // kMinI8.
  DEFINE_OP(kIdPminub     , 2, kSSE2  , kIdVpminub        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k8 , 0, kNA), // kMinU8.
  DEFINE_OP(kIdPminsw     , 2, kSSE2  , kIdVpminsw        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k16, 0, kNA), // kMinI16.
  DEFINE_OP(kIdPminuw     , 2, kSSE4_1, kIdVpminuw        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k16, 0, kNA), // kMinU16.
  DEFINE_OP(kIdPminsd     , 2, kSSE4_1, kIdVpminsd        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kMinI32.
  DEFINE_OP(kIdPminud     , 2, kSSE4_1, kIdVpminud        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kMinU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpminsq        , kAVX512     , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kMinI64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpminuq        , kAVX512     , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kMinU64.
  DEFINE_OP(kIdPmaxsb     , 2, kSSE4_1, kIdVpmaxsb        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k8 , 0, kNA), // kMaxI8.
  DEFINE_OP(kIdPmaxub     , 2, kSSE2  , kIdVpmaxub        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k8 , 0, kNA), // kMaxU8.
  DEFINE_OP(kIdPmaxsw     , 2, kSSE2  , kIdVpmaxsw        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k16, 0, kNA), // kMaxI16.
  DEFINE_OP(kIdPmaxuw     , 2, kSSE4_1, kIdVpmaxuw        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k16, 0, kNA), // kMaxU16.
  DEFINE_OP(kIdPmaxsd     , 2, kSSE4_1, kIdVpmaxsd        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kMaxI32.
  DEFINE_OP(kIdPmaxud     , 2, kSSE4_1, kIdVpmaxud        , kAVX        , 1, 0, kSrc , 0, 0x00u, kNone, k32, 4, kNA), // kMaxU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpmaxsq        , kAVX512     , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kMaxI64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpmaxuq        , kAVX512     , 1, 0, kSrc , 0, 0x00u, kNone, k64, 8, kNA), // kMaxU64.
  DEFINE_OP(kIdPcmpeqb    , 2, kSSE2  , kIdVpcmpeqb       , kAVX        , 1, 1, kOnes, 0, 0x00u, kNone, k8 , 0, kNA), // kCmpEqU8.
  DEFINE_OP(kIdPcmpeqw    , 2, kSSE2  , kIdVpcmpeqw       , kAVX        , 1, 1, kOnes, 0, 0x00u, kNone, k16, 0, kNA), // kCmpEqU16.
  DEFINE_OP(kIdPcmpeqd    , 2, kSSE2  , kIdVpcmpeqd       , kAVX        , 1, 1, kOnes, 0, 0x00u, kNone, k32, 4, kNA), // kCmpEqU32.
  DEFINE_OP(kIdPcmpeqq    , 2, kSSE4_1, kIdVpcmpeqq       , kAVX        , 1, 1, kOnes, 0, 0x00u, kNone, k64, 8, kNA), // kCmpEqU64.
  DEFINE_OP(kIdPcmpgtb    , 2, kSSE2  , kIdVpcmpgtb       , kAVX        , 0, 1, kZero, 0, 0x00u, kNone, k8 , 0, kNA), // kCmpGtI8.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpub        , kAVX512     , 0, 1, kZero, 1, 0x06u, kNone, k8 , 0, kNA), // kCmpGtU8.
  DEFINE_OP(kIdPcmpgtw    , 2, kSSE2  , kIdVpcmpgtw       , kAVX        , 0, 1, kZero, 0, 0x00u, kNone, k16, 0, kNA), // kCmpGtI16.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpuw        , kAVX512     , 0, 1, kZero, 1, 0x06u, kNone, k16, 0, kNA), // kCmpGtU16.
  DEFINE_OP(kIdPcmpgtd    , 2, kSSE2  , kIdVpcmpgtd       , kAVX        , 0, 1, kZero, 0, 0x00u, kNone, k32, 4, kNA), // kCmpGtI32.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpud        , kAVX512     , 0, 1, kZero, 1, 0x06u, kNone, k32, 4, kNA), // kCmpGtU32.
  DEFINE_OP(kIdPcmpgtq    , 2, kSSE4_2, kIdVpcmpgtq       , kAVX        , 0, 1, kZero, 0, 0x00u, kNone, k64, 8, kNA), // kCmpGtI64.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpuq        , kAVX512     , 0, 1, kZero, 1, 0x06u, kNone, k64, 8, kNA), // kCmpGtU64.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpb         , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k8 , 0, kNA), // kCmpGeI8.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpub        , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k8 , 0, kNA), // kCmpGeU8.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpw         , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k16, 0, kNA), // kCmpGeI16.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpuw        , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k16, 0, kNA), // kCmpGeU16.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpd         , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k32, 4, kNA), // kCmpGeI32.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpud        , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k32, 4, kNA), // kCmpGeU32.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpq         , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k64, 8, kNA), // kCmpGeI64.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpuq        , kAVX512     , 0, 1, kOnes, 1, 0x05u, kNone, k64, 8, kNA), // kCmpGeU64.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpb         , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k8 , 0, kNA), // kCmpLtI8.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpub        , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k8 , 0, kNA), // kCmpLtU8.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpw         , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k16, 0, kNA), // kCmpLtI16.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpuw        , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k16, 0, kNA), // kCmpLtU16.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpd         , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k32, 4, kNA), // kCmpLtI32.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpud        , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k32, 4, kNA), // kCmpLtU32.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpq         , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k64, 8, kNA), // kCmpLtI64.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpuq        , kAVX512     , 0, 1, kZero, 1, 0x01u, kNone, k64, 8, kNA), // kCmpLtU64.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpb         , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k8 , 0, kNA), // kCmpLeI8.
  DEFINE_OP(kIdPcmpgtb    , 0, kIntrin, kIdVpcmpub        , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k8 , 0, kNA), // kCmpLeU8.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpw         , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k16, 0, kNA), // kCmpLeI16.
  DEFINE_OP(kIdPcmpgtw    , 0, kIntrin, kIdVpcmpuw        , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k16, 0, kNA), // kCmpLeU16.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpd         , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k32, 4, kNA), // kCmpLeI32.
  DEFINE_OP(kIdPcmpgtd    , 0, kIntrin, kIdVpcmpud        , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k32, 4, kNA), // kCmpLeU32.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpq         , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k64, 8, kNA), // kCmpLeI64.
  DEFINE_OP(kIdPcmpgtq    , 0, kIntrin, kIdVpcmpuq        , kAVX512     , 0, 1, kOnes, 1, 0x02u, kNone, k64, 8, kNA), // kCmpLeU64.
  DEFINE_OP(kIdAndps      , 2, kSSE2  , kIdVandps         , kAVX        , 1, 0, kSrc , 0, 0x00u, kF32V, k32, 4, kNA), // kAndF32.
  DEFINE_OP(kIdAndpd      , 2, kSSE2  , kIdVandpd         , kAVX        , 1, 0, kSrc , 0, 0x00u, kF64V, k64, 8, kNA), // kAndF64.
  DEFINE_OP(kIdOrps       , 2, kSSE2  , kIdVorps          , kAVX        , 1, 0, kSrc , 0, 0x00u, kF32V, k32, 4, kNA), // kOrF32.
  DEFINE_OP(kIdOrpd       , 2, kSSE2  , kIdVorpd          , kAVX        , 1, 0, kSrc , 0, 0x00u, kF64V, k64, 8, kNA), // kOrF64.
  DEFINE_OP(kIdXorps      , 2, kSSE2  , kIdVxorps         , kAVX        , 1, 0, kZero, 0, 0x00u, kF32V, k32, 4, kNA), // kXorF32.
  DEFINE_OP(kIdXorpd      , 2, kSSE2  , kIdVxorpd         , kAVX        , 1, 0, kZero, 0, 0x00u, kF64V, k64, 8, kNA), // kXorF64.
  DEFINE_OP(kIdAndnps     , 2, kSSE2  , kIdVandnps        , kAVX        , 0, 0, kZero, 0, 0x00u, kF32V, k32, 4, kNA), // kAndnF32.
  DEFINE_OP(kIdAndnpd     , 2, kSSE2  , kIdVandnpd        , kAVX        , 0, 0, kZero, 0, 0x00u, kF64V, k64, 8, kNA), // kAndnF64.
  DEFINE_OP(kIdAndnps     , 0, kIntrin, kIdVandnps        , kIntrin     , 0, 0, kZero, 0, 0x00u, kF32V, k32, 4, kNA), // kBicF32.
  DEFINE_OP(kIdAndnpd     , 0, kIntrin, kIdVandnpd        , kIntrin     , 0, 0, kZero, 0, 0x00u, kF64V, k64, 8, kNA), // kBicF64.
  DEFINE_OP(kIdAddss      , 2, kSSE2  , kIdVaddss         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kAddF32S.
  DEFINE_OP(kIdAddsd      , 2, kSSE2  , kIdVaddsd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kAddF64S.
  DEFINE_OP(kIdAddps      , 2, kSSE2  , kIdVaddps         , kAVX        , 1, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kAddF32.
  DEFINE_OP(kIdAddpd      , 2, kSSE2  , kIdVaddpd         , kAVX        , 1, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kAddF64.
  DEFINE_OP(kIdSubss      , 2, kSSE2  , kIdVsubss         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kSubF32S.
  DEFINE_OP(kIdSubsd      , 2, kSSE2  , kIdVsubsd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kSubF64S.
  DEFINE_OP(kIdSubps      , 2, kSSE2  , kIdVsubps         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kSubF32.
  DEFINE_OP(kIdSubpd      , 2, kSSE2  , kIdVsubpd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kSubF64.
  DEFINE_OP(kIdMulss      , 2, kSSE2  , kIdVmulss         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kMulF32S.
  DEFINE_OP(kIdMulsd      , 2, kSSE2  , kIdVmulsd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kMulF64S.
  DEFINE_OP(kIdMulps      , 2, kSSE2  , kIdVmulps         , kAVX        , 1, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kMulF32.
  DEFINE_OP(kIdMulpd      , 2, kSSE2  , kIdVmulpd         , kAVX        , 1, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kMulF64.
  DEFINE_OP(kIdDivss      , 2, kSSE2  , kIdVdivss         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kDivF32S.
  DEFINE_OP(kIdDivsd      , 2, kSSE2  , kIdVdivsd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kDivF64S.
  DEFINE_OP(kIdDivps      , 2, kSSE2  , kIdVdivps         , kAVX        , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kDivF32.
  DEFINE_OP(kIdDivpd      , 2, kSSE2  , kIdVdivpd         , kAVX        , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kDivF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kModF32S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kModF64S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kModF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kModF64.
  DEFINE_OP(kIdMinss      , 2, kSSE2  , kIdVminss         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF32S, k32, 4, kNA), // kMinF32S.
  DEFINE_OP(kIdMinsd      , 2, kSSE2  , kIdVminsd         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF64S, k64, 8, kNA), // kMinF64S.
  DEFINE_OP(kIdMinps      , 2, kSSE2  , kIdVminps         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF32V, k32, 4, kNA), // kMinF32.
  DEFINE_OP(kIdMinpd      , 2, kSSE2  , kIdVminpd         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF64V, k64, 8, kNA), // kMinF64.
  DEFINE_OP(kIdMaxss      , 2, kSSE2  , kIdVmaxss         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF32S, k32, 4, kNA), // kMaxF32S.
  DEFINE_OP(kIdMaxsd      , 2, kSSE2  , kIdVmaxsd         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF64S, k64, 8, kNA), // kMaxF64S.
  DEFINE_OP(kIdMaxps      , 2, kSSE2  , kIdVmaxps         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF32V, k32, 4, kNA), // kMaxF32.
  DEFINE_OP(kIdMaxpd      , 2, kSSE2  , kIdVmaxpd         , kAVX        , 0, 0, kSrc , 0, 0x00u, kF64V, k64, 8, kNA), // kMaxF64.
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 1, 1, kNone, 1, 0x00u, kF32S, k32, 4, kNA), // kCmpEqF32S    (eq ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 1, 1, kNone, 1, 0x00u, kF64S, k64, 8, kNA), // kCmpEqF64S    (eq ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 1, 1, kNone, 1, 0x00u, kF32V, k32, 4, kNA), // kCmpEqF32     (eq ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 1, 1, kNone, 1, 0x00u, kF64V, k64, 8, kNA), // kCmpEqF64     (eq ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 1, 1, kNone, 1, 0x04u, kF32S, k32, 4, kNA), // kCmpNeF32S    (ne unordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 1, 1, kNone, 1, 0x04u, kF64S, k64, 8, kNA), // kCmpNeF64S    (ne unordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 1, 1, kNone, 1, 0x04u, kF32V, k32, 4, kNA), // kCmpNeF32     (ne unordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 1, 1, kNone, 1, 0x04u, kF64V, k64, 8, kNA), // kCmpNeF64     (ne unordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 0, 1, kNone, 1, 0x1Eu, kF32S, k32, 4, kNA), // kCmpGtF32S    (gt ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 0, 1, kNone, 1, 0x1Eu, kF64S, k64, 8, kNA), // kCmpGtF64S    (gt ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 0, 1, kNone, 1, 0x1Eu, kF32V, k32, 4, kNA), // kCmpGtF32     (gt ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 0, 1, kNone, 1, 0x1Eu, kF64V, k64, 8, kNA), // kCmpGtF64     (gt ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 0, 1, kNone, 1, 0x1Du, kF32S, k32, 4, kNA), // kCmpGeF32S    (ge ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 0, 1, kNone, 1, 0x1Du, kF64S, k64, 8, kNA), // kCmpGeF64S    (ge ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 0, 1, kNone, 1, 0x1Du, kF32V, k32, 4, kNA), // kCmpGeF32     (ge ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 0, 1, kNone, 1, 0x1Du, kF64V, k64, 8, kNA), // kCmpGeF64     (ge ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 0, 1, kNone, 1, 0x11u, kF32S, k32, 4, kNA), // kCmpLtF32S    (lt ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 0, 1, kNone, 1, 0x11u, kF64S, k64, 8, kNA), // kCmpLtF64S    (lt ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 0, 1, kNone, 1, 0x11u, kF32V, k32, 4, kNA), // kCmpLtF32     (lt ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 0, 1, kNone, 1, 0x11u, kF64V, k64, 8, kNA), // kCmpLtF64     (lt ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 0, 1, kNone, 1, 0x12u, kF32S, k32, 4, kNA), // kCmpLeF32S    (le ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 0, 1, kNone, 1, 0x12u, kF64S, k64, 8, kNA), // kCmpLeF64S    (le ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 0, 1, kNone, 1, 0x12u, kF32V, k32, 4, kNA), // kCmpLeF32     (le ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 0, 1, kNone, 1, 0x12u, kF64V, k64, 8, kNA), // kCmpLeF64     (le ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 1, 1, kNone, 1, 0x07u, kF32S, k32, 4, kNA), // kCmpOrdF32S   (ordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 1, 1, kNone, 1, 0x07u, kF64S, k64, 8, kNA), // kCmpOrdF64S   (ordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 1, 1, kNone, 1, 0x07u, kF32V, k32, 4, kNA), // kCmpOrdF32    (ordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 1, 1, kNone, 1, 0x07u, kF64V, k64, 8, kNA), // kCmpOrdF64    (ordered quiet).
  DEFINE_OP(kIdCmpss      , 2, kIntrin, kIdVcmpss         , kAVX        , 1, 1, kNone, 1, 0x03u, kF32S, k32, 4, kNA), // kCmpUnordF32S (unordered quiet).
  DEFINE_OP(kIdCmpsd      , 2, kIntrin, kIdVcmpsd         , kAVX        , 1, 1, kNone, 1, 0x03u, kF64S, k64, 8, kNA), // kCmpUnordF64S (unordered quiet).
  DEFINE_OP(kIdCmpps      , 2, kIntrin, kIdVcmpps         , kAVX        , 1, 1, kNone, 1, 0x03u, kF32V, k32, 4, kNA), // kCmpUnordF32  (unordered quiet).
  DEFINE_OP(kIdCmppd      , 2, kIntrin, kIdVcmppd         , kAVX        , 1, 1, kNone, 1, 0x03u, kF64V, k64, 8, kNA), // kCmpUnordF64  (unordered quiet).
  DEFINE_OP(kIdHaddpd     , 2, kSSE3  , kIdVhaddpd        , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 0, kNA), // kHAddF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCombineLoHiU64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA), // kCombineLoHiF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kSrc , 0, 0x00u, kNone, k64, 0, kNA), // kCombineHiLoU64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kSrc , 0, 0x00u, kNone, k64, 0, kNA), // kCombineHiLoF64.
  DEFINE_OP(kIdPunpcklbw  , 2, kSSE2  , kIdVpunpcklbw     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kLo), // kInterleaveLoU8.
  DEFINE_OP(kIdPunpckhbw  , 2, kSSE2  , kIdVpunpckhbw     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kHi), // kInterleaveHiU8.
  DEFINE_OP(kIdPunpcklwd  , 2, kSSE2  , kIdVpunpcklwd     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kLo), // kInterleaveLoU16.
  DEFINE_OP(kIdPunpckhwd  , 2, kSSE2  , kIdVpunpckhwd     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kHi), // kInterleaveHiU16.
  DEFINE_OP(kIdPunpckldq  , 2, kSSE2  , kIdVpunpckldq     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kLo), // kInterleaveLoU32.
  DEFINE_OP(kIdPunpckhdq  , 2, kSSE2  , kIdVpunpckhdq     , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kHi), // kInterleaveHiU32.
  DEFINE_OP(kIdPunpcklqdq , 2, kSSE2  , kIdVpunpcklqdq    , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kLo), // kInterleaveLoU64.
  DEFINE_OP(kIdPunpckhqdq , 2, kSSE2  , kIdVpunpckhqdq    , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kHi), // kInterleaveHiU64.
  DEFINE_OP(kIdUnpcklps   , 2, kSSE2  , kIdVunpcklps      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kLo), // kInterleaveLoF32.
  DEFINE_OP(kIdUnpckhps   , 2, kSSE2  , kIdVunpckhps      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kHi), // kInterleaveHiF32.
  DEFINE_OP(kIdUnpcklpd   , 2, kSSE2  , kIdVunpcklpd      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kLo), // kInterleaveLoF64.
  DEFINE_OP(kIdUnpckhpd   , 2, kSSE2  , kIdVunpckhpd      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kHi), // kInterleaveHiF64.
  DEFINE_OP(kIdPacksswb   , 2, kSSE2  , kIdVpacksswb      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kPacksI16_I8.
  DEFINE_OP(kIdPackuswb   , 2, kSSE2  , kIdVpackuswb      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kPacksI16_U8.
  DEFINE_OP(kIdPackssdw   , 2, kSSE2  , kIdVpackssdw      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kPacksI32_I16.
  DEFINE_OP(kIdPackusdw   , 2, kSSE4_1, kIdVpackusdw      , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kPacksI32_U16.
  DEFINE_OP(kIdPshufb     , 2, kSSSE3 , kIdVpshufb        , kAVX        , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kSwizzlev_U8.

  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermb         , kAVX512_VBMI, 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kPermuteU8.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermw         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kPermuteU16.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermd         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k32, 0, kNA), // kPermuteU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVpermq         , kAVX512     , 0, 0, kNone, 0, 0x00u, kNone, k64, 0, kNA)  // kPermuteU64.
};

static constexpr UniOpVInfo opcode_info_3vi[size_t(UniOpVVVI::kMaxValue) + 1] = {
  DEFINE_OP(kIdPalignr    , 2, kIntrin, kIdVpalignr       , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kAlignr_U128.
  DEFINE_OP(kIdShufps     , 2, kIntrin, kIdVshufps        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kInterleaveShuffleU32x4.
  DEFINE_OP(kIdShufpd     , 2, kIntrin, kIdVshufpd        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kInterleaveShuffleU64x2.
  DEFINE_OP(kIdShufps     , 2, kIntrin, kIdVshufps        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kInterleaveShuffleF32x4.
  DEFINE_OP(kIdShufpd     , 2, kIntrin, kIdVshufpd        , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k64, 8, kNA), // kInterleaveShuffleF64x2.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinserti32x4   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV128_U32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinserti64x2   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV128_F32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinsertf32x4   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV128_U64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinsertf64x2   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV128_F64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinserti32x8   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV256_U32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinsertf32x8   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV256_F32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinserti64x4   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kInsertV256_U64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdVinsertf64x4   , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA)  // kInsertV256_F64.
};

static constexpr UniOpVInfo opcode_info_4v[size_t(UniOpVVV::kMaxValue) + 1] = {
  DEFINE_OP(kIdPblendvb   , 0, kIntrin, kIdVpblendvb      , kIntrin     , 0, 0, kNone, 0, 0x00u, kNone, k8 , 0, kNA), // kBlendV_U8.
  DEFINE_OP(kIdPmullw     , 0, kIntrin, kIdVpmullw        , kIntrin     , 1, 0, kNone, 0, 0x00u, kNone, k16, 0, kNA), // kMAddU16.
  DEFINE_OP(kIdPmulld     , 0, kIntrin, kIdVpmulld        , kIntrin     , 1, 0, kNone, 0, 0x00u, kNone, k32, 4, kNA), // kMAddU32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32S, k32, 4, kNA), // kMAddF32S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64S, k64, 8, kNA), // kMAddF64S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF32V, k32, 4, kNA), // kMAddF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x00u, kF64V, k64, 8, kNA), // kMAddF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x01u, kF32S, k32, 4, kNA), // kMSubF32S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x01u, kF64S, k64, 8, kNA), // kMSubF64S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x01u, kF32V, k32, 4, kNA), // kMSubF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x01u, kF64V, k64, 8, kNA), // kMSubF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x02u, kF32S, k32, 4, kNA), // kNMAddF32S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x02u, kF64S, k64, 8, kNA), // kNMAddF64S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x02u, kF32V, k32, 4, kNA), // kNMAddF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x02u, kF64V, k64, 8, kNA), // kNMAddF64.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x03u, kF32S, k32, 4, kNA), // kNMSubF32S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x03u, kF64S, k64, 8, kNA), // kNMSubF64S.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x03u, kF32V, k32, 4, kNA), // kNMSubF32.
  DEFINE_OP(kIdNone       , 0, kIntrin, kIdNone           , kIntrin     , 0, 0, kNone, 0, 0x03u, kF64V, k64, 8, kNA)  // kNMSubF64.
};

#undef DEFINE_OP

struct UniOpVMInfo {
  //! \name Members
  //! \{

  uint32_t sse_inst_id    : 13;
  uint32_t avx_inst_id    : 13;
  uint32_t reserved1      : 6;
  uint32_t cvt            : 5;
  uint32_t mem_size       : 8;
  uint32_t mem_size_shift : 3;
  uint32_t reserved2      : 3;

  //! \}
};

#define DEFINE_OP(sse_inst_id, avx_inst_id, cvt, mem_size, mem_size_shift) \
  UniOpVMInfo {                                                            \
    Inst::sse_inst_id,                                                     \
    Inst::avx_inst_id,                                                     \
    0,                                                                     \
    uint8_t(WideningOp::cvt),                                              \
    mem_size,                                                              \
    mem_size_shift,                                                        \
    0                                                                      \
  }

static constexpr UniOpVMInfo opcode_info_2vm[size_t(UniOpVM::kMaxValue) + 1] = {
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  1, 0), // kLoad8.
  DEFINE_OP(kIdNone          , kIdVmovsh         , kNone    ,  2, 0), // kLoad16_U16.
  DEFINE_OP(kIdMovd          , kIdVmovd          , kNone    ,  4, 0), // kLoad32_U32.
  DEFINE_OP(kIdMovss         , kIdVmovss         , kNone    ,  4, 0), // kLoad32_F32.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kLoad64_U32.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kLoad64_U64.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kLoad64_F32.
  DEFINE_OP(kIdMovsd         , kIdVmovsd         , kNone    ,  8, 0), // kLoad64_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kLoad128_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kLoad128_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kLoad128_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kLoad128_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kLoad256_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kLoad256_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kLoad256_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kLoad256_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kLoad512_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kLoad512_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kLoad512_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kLoad512_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kLoadN_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kLoadN_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kLoadN_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kLoadN_F64.
  DEFINE_OP(kIdPmovzxbq      , kIdVpmovzxbq      , kU8ToU64 ,  2, 3), // kLoadCvt16_U8ToU64.
  DEFINE_OP(kIdPmovzxbq      , kIdVpmovzxbq      , kU8ToU64 ,  4, 3), // kLoadCvt32_U8ToU64.
  DEFINE_OP(kIdPmovzxbq      , kIdVpmovzxbq      , kU8ToU64 ,  8, 3), // kLoadCvt64_U8ToU64.
  DEFINE_OP(kIdPmovsxbw      , kIdVpmovsxbw      , kI8ToI16 ,  4, 1), // kLoadCvt32_I8ToI16.
  DEFINE_OP(kIdPmovzxbw      , kIdVpmovzxbw      , kU8ToU16 ,  4, 1), // kLoadCvt32_U8ToU16.
  DEFINE_OP(kIdPmovsxbd      , kIdVpmovsxbd      , kI8ToI32 ,  4, 2), // kLoadCvt32_I8ToI32.
  DEFINE_OP(kIdPmovzxbd      , kIdVpmovzxbd      , kU8ToU32 ,  4, 2), // kLoadCvt32_U8ToU32.
  DEFINE_OP(kIdPmovsxwd      , kIdVpmovsxwd      , kI16ToI32,  4, 1), // kLoadCvt32_I16ToI32.
  DEFINE_OP(kIdPmovzxwd      , kIdVpmovzxwd      , kU16ToU32,  4, 1), // kLoadCvt32_U16ToU32.
  DEFINE_OP(kIdPmovsxdq      , kIdVpmovsxdq      , kI32ToI64,  4, 1), // kLoadCvt32_I32ToI64.
  DEFINE_OP(kIdPmovzxdq      , kIdVpmovzxdq      , kU32ToU64,  4, 1), // kLoadCvt32_U32ToU64.
  DEFINE_OP(kIdPmovsxbw      , kIdVpmovsxbw      , kI8ToI16 ,  8, 1), // kLoadCvt64_I8ToI16.
  DEFINE_OP(kIdPmovzxbw      , kIdVpmovzxbw      , kU8ToU16 ,  8, 1), // kLoadCvt64_U8ToU16.
  DEFINE_OP(kIdPmovsxbd      , kIdVpmovsxbd      , kI8ToI32 ,  8, 2), // kLoadCvt64_I8ToI32.
  DEFINE_OP(kIdPmovzxbd      , kIdVpmovzxbd      , kU8ToU32 ,  8, 2), // kLoadCvt64_U8ToU32.
  DEFINE_OP(kIdPmovsxwd      , kIdVpmovsxwd      , kI16ToI32,  8, 1), // kLoadCvt64_I16ToI32.
  DEFINE_OP(kIdPmovzxwd      , kIdVpmovzxwd      , kU16ToU32,  8, 1), // kLoadCvt64_U16ToU32.
  DEFINE_OP(kIdPmovsxdq      , kIdVpmovsxdq      , kI32ToI64,  8, 1), // kLoadCvt64_I32ToI64.
  DEFINE_OP(kIdPmovzxdq      , kIdVpmovzxdq      , kU32ToU64,  8, 1), // kLoadCvt64_U32ToU64.
  DEFINE_OP(kIdNone          , kIdVpmovsxbw      , kI8ToI16 , 16, 3), // kLoadCvt128_I8ToI16.
  DEFINE_OP(kIdNone          , kIdVpmovzxbw      , kU8ToU16 , 16, 3), // kLoadCvt128_U8ToU16.
  DEFINE_OP(kIdNone          , kIdVpmovsxbd      , kI8ToI32 , 16, 2), // kLoadCvt128_I8ToI32.
  DEFINE_OP(kIdNone          , kIdVpmovzxbd      , kU8ToU32 , 16, 2), // kLoadCvt128_U8ToU32.
  DEFINE_OP(kIdNone          , kIdVpmovsxwd      , kI16ToI32, 16, 1), // kLoadCvt128_I16ToI32.
  DEFINE_OP(kIdNone          , kIdVpmovzxwd      , kU16ToU32, 16, 1), // kLoadCvt128_U16ToU32.
  DEFINE_OP(kIdNone          , kIdVpmovsxdq      , kI32ToI64, 16, 1), // kLoadCvt128_I32ToI64.
  DEFINE_OP(kIdNone          , kIdVpmovzxdq      , kU32ToU64, 16, 1), // kLoadCvt128_U32ToU64.
  DEFINE_OP(kIdNone          , kIdVpmovsxbw      , kI8ToI16 , 32, 1), // kLoadCvt256_I8ToI16.
  DEFINE_OP(kIdNone          , kIdVpmovzxbw      , kU8ToU16 , 32, 1), // kLoadCvt256_U8ToU16.
  DEFINE_OP(kIdNone          , kIdVpmovsxwd      , kI16ToI32, 32, 1), // kLoadCvt256_I16ToI32.
  DEFINE_OP(kIdNone          , kIdVpmovzxwd      , kU16ToU32, 32, 1), // kLoadCvt256_U16ToU32.
  DEFINE_OP(kIdNone          , kIdVpmovsxdq      , kI32ToI64, 32, 1), // kLoadCvt256_I32ToI64.
  DEFINE_OP(kIdNone          , kIdVpmovzxdq      , kU32ToU64, 32, 1), // kLoadCvt256_U32ToU64.
  DEFINE_OP(kIdPmovzxbq      , kIdVpmovzxbq      , kU8ToU64 ,  0, 3), // kLoadCvtN_U8ToU64.
  DEFINE_OP(kIdPmovsxbw      , kIdVpmovsxbw      , kI8ToI16 ,  0, 1), // kLoadCvtN_I8ToI16.
  DEFINE_OP(kIdPmovzxbw      , kIdVpmovzxbw      , kU8ToU16 ,  0, 1), // kLoadCvtN_U8ToU16.
  DEFINE_OP(kIdPmovsxbd      , kIdVpmovsxbd      , kI8ToI32 ,  0, 2), // kLoadCvtN_I8ToI32.
  DEFINE_OP(kIdPmovzxbd      , kIdVpmovzxbd      , kU8ToU32 ,  0, 2), // kLoadCvtN_U8ToU32.
  DEFINE_OP(kIdPmovsxwd      , kIdVpmovsxwd      , kI16ToI32,  0, 1), // kLoadCvtN_I16ToI32.
  DEFINE_OP(kIdPmovzxwd      , kIdVpmovzxwd      , kU16ToU32,  0, 1), // kLoadCvtN_U16ToU32.
  DEFINE_OP(kIdPmovsxdq      , kIdVpmovsxdq      , kI32ToI64,  0, 1), // kLoadCvtN_I32ToI64.
  DEFINE_OP(kIdPmovzxdq      , kIdVpmovzxdq      , kU32ToU64,  0, 1), // kLoadCvtN_U32ToU64.
  DEFINE_OP(kIdPinsrb        , kIdVpinsrb        , kNone    ,  1, 0), // kLoadInsertU8.
  DEFINE_OP(kIdPinsrw        , kIdVpinsrw        , kNone    ,  2, 0), // kLoadInsertU16.
  DEFINE_OP(kIdPinsrd        , kIdVpinsrd        , kNone    ,  4, 0), // kLoadInsertU32.
  DEFINE_OP(kIdPinsrq        , kIdVpinsrq        , kNone    ,  8, 0), // kLoadInsertU64.
  DEFINE_OP(kIdInsertps      , kIdVinsertps      , kNone    ,  4, 0), // kLoadInsertF32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  8, 0), // kLoadInsertF32x2.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  8, 0)  // kLoadInsertF64.
};

#undef DEFINE_OP

#define DEFINE_OP(sse_inst_id, avx_inst_id, cvt, mem_size, mem_size_shift) \
  UniOpVMInfo {                                                    \
    Inst::sse_inst_id,                                                \
    Inst::avx_inst_id,                                                \
    0,                                                              \
    uint8_t(NarrowingOp::cvt),                                      \
    mem_size,                                                        \
    mem_size_shift,                                                   \
    0                                                               \
  }

static constexpr UniOpVMInfo opcode_info_2mv[size_t(UniOpMV::kMaxValue) + 1] = {
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  1, 0), // kStore8.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  2, 0), // kStore16_U16.
  DEFINE_OP(kIdMovd          , kIdVmovd          , kNone    ,  4, 0), // kStore32_U32.
  DEFINE_OP(kIdMovss         , kIdVmovss         , kNone    ,  4, 0), // kStore32_F32.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kStore64_U32.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kStore64_U64.
  DEFINE_OP(kIdMovq          , kIdVmovq          , kNone    ,  8, 0), // kStore64_F32.
  DEFINE_OP(kIdMovsd         , kIdVmovsd         , kNone    ,  8, 0), // kStore64_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kStore128_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kStore128_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kStore128_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 16, 0), // kStore128_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kStore256_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kStore256_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kStore256_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 32, 0), // kStore256_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kStore512_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kStore512_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kStore512_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    , 64, 0), // kStore512_F64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kStoreN_U32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kStoreN_U64.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kStoreN_F32.
  DEFINE_OP(kIdNone          , kIdNone           , kNone    ,  0, 0), // kStoreN_F64.
  /*
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 ,  8, 1), // kStoreCvtz64_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16,  8, 1), // kStoreCvtz64_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32,  8, 1), // kStoreCvtz64_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToI8 ,  8, 1), // kStoreCvts64_I16ToI8.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToU8 ,  8, 1), // kStoreCvts64_I16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 ,  8, 1), // kStoreCvts64_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kI32ToI16,  8, 1), // kStoreCvts64_I32ToI16.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16,  8, 1), // kStoreCvts64_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kI64ToI32,  8, 1), // kStoreCvts64_I64ToI32.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32,  8, 1), // kStoreCvts64_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 , 16, 1), // kStoreCvtz128_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16, 16, 1), // kStoreCvtz128_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32, 16, 1), // kStoreCvtz128_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToI8 , 16, 1), // kStoreCvts128_I16ToI8.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToU8 , 16, 1), // kStoreCvts128_I16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 , 16, 1), // kStoreCvts128_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kI32ToI16, 16, 1), // kStoreCvts128_I32ToI16.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16, 16, 1), // kStoreCvts128_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kI64ToI32, 16, 1), // kStoreCvts128_I64ToI32.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32, 16, 1), // kStoreCvts128_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 , 32, 1), // kStoreCvtz256_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16, 32, 1), // kStoreCvtz256_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32, 32, 1), // kStoreCvtz256_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToI8 , 32, 1), // kStoreCvts256_I16ToI8.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToU8 , 32, 1), // kStoreCvts256_I16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 , 32, 1), // kStoreCvts256_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kI32ToI16, 32, 1), // kStoreCvts256_I32ToI16.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16, 32, 1), // kStoreCvts256_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kI64ToI32, 32, 1), // kStoreCvts256_I64ToI32.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32, 32, 1), // kStoreCvts256_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 ,  0, 1), // kStoreCvtzN_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16,  0, 1), // kStoreCvtzN_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32,  0, 1), // kStoreCvtzN_U64ToU32.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToI8 ,  0, 1), // kStoreCvtsN_I16ToI8.
  DEFINE_OP(kIdNone          , kIdNone           , kI16ToU8 ,  0, 1), // kStoreCvtsN_I16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kU16ToU8 ,  0, 1), // kStoreCvtsN_U16ToU8.
  DEFINE_OP(kIdNone          , kIdNone           , kI32ToI16,  0, 1), // kStoreCvtsN_I32ToI16.
  DEFINE_OP(kIdNone          , kIdNone           , kU32ToU16,  0, 1), // kStoreCvtsN_U32ToU16.
  DEFINE_OP(kIdNone          , kIdNone           , kI64ToI32,  0, 1), // kStoreCvtsN_I64ToI32.
  DEFINE_OP(kIdNone          , kIdNone           , kU64ToU32,  0, 1)  // kStoreCvtsN_U64ToU32.
  */
  DEFINE_OP(kIdPextrw        , kIdVpextrw        , kNone    ,  2, 0), // kStoreExtractU16.
  DEFINE_OP(kIdPextrd        , kIdVpextrd        , kNone    ,  4, 0), // kStoreExtractU32.
  DEFINE_OP(kIdPextrq        , kIdVpextrq        , kNone    ,  8, 0)  // kStoreExtractU64.
};

#undef DEFINE_OP

// ujit::UniCompiler - Vector Instructions - Utility Functions
// ===========================================================

static ASMJIT_NOINLINE void UniCompiler_load_into(UniCompiler& uc, const Vec& vec, const Mem& mem, uint32_t broadcast_size = 0) {
  BackendCompiler* cc = uc.cc;
  Mem m(mem);

  if (mem.has_broadcast() && broadcast_size) {
    m.reset_broadcast();
    switch (broadcast_size) {
      case 1: cc->vpbroadcastb(vec, m); break;
      case 2: cc->vpbroadcastw(vec, m); break;
      case 4: cc->vpbroadcastd(vec, m); break;
      case 8: cc->vpbroadcastq(vec, m); break;
      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    m.set_size(vec.size());
    if (vec.is_vec512())
      cc->vmovdqu32(vec, m);
    else if (uc.has_avx())
      cc->vmovdqu(vec, m);
    else
      cc->movdqu(vec, m);
  }
}

// TODO: Unused for now...
[[maybe_unused]]
static ASMJIT_NOINLINE void UniCompiler_move_to_dst(UniCompiler& uc, const Vec& dst, const Operand_& src, uint32_t broadcast_size = 0) {
  if (src.is_reg()) {
    ASMJIT_ASSERT(src.is_vec());
    if (dst.id() != src.as<Reg>().id()) {
      uc.v_mov(dst, src);
    }
  }
  else if (src.is_mem()) {
    UniCompiler_load_into(uc, dst, src.as<Mem>(), broadcast_size);
  }
  else {
    ASMJIT_NOT_REACHED();
  }
}

static ASMJIT_NOINLINE Vec UniCompiler_load_new(UniCompiler& uc, const Vec& ref, const Mem& mem, uint32_t broadcast_size = 0) {
  Vec vec = uc.new_similar_reg(ref, "@vec_m");
  UniCompiler_load_into(uc, vec, mem, broadcast_size);
  return vec;
}

static ASMJIT_INLINE bool is_same_vec(const Vec& a, const Operand_& b) noexcept {
  return b.is_reg() && a.id() == b.as<Reg>().id();
}

static ASMJIT_INLINE Operand get_fop_one(UniCompiler& uc, const Vec& dst, FloatMode fm) {
  Operand op;
  if (is_f32_op(fm))
    op = uc.simd_const(&uc.ct().f32_1, Bcst::k32, dst);
  else
    op = uc.simd_const(&uc.ct().f64_1, Bcst::k64, dst);
  return op;
}

static ASMJIT_INLINE Operand get_fop_half_minus_1ulp(UniCompiler& uc, const Vec& dst, FloatMode fm) {
  Operand op;
  if (is_f32_op(fm))
    op = uc.simd_const(&uc.ct().f32_0_5_minus_1ulp, Bcst::k32, dst);
  else
    op = uc.simd_const(&uc.ct().f64_0_5_minus_1ulp, Bcst::k64, dst);
  return op;
}

static ASMJIT_INLINE Operand get_fop_round_magic(UniCompiler& uc, const Vec& dst, FloatMode fm) {
  Operand op;
  if (is_f32_op(fm))
    op = uc.simd_const(&uc.ct().f32_round_magic, Bcst::k32, dst);
  else
    op = uc.simd_const(&uc.ct().f64_round_magic, Bcst::k64, dst);
  return op;
}

static ASMJIT_INLINE Operand get_fop_msb_bit(UniCompiler& uc, const Vec& dst, FloatMode fm) {
  Operand op;
  if (is_f32_op(fm))
    op = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst);
  else
    op = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst);
  return op;
}

static ASMJIT_NOINLINE void sse_mov(UniCompiler& uc, const Vec& dst, const Operand_& src) {
  BackendCompiler* cc = uc.cc;
  if (src.is_mem())
    cc->emit(Inst::kIdMovups, dst, src);
  else if (dst.id() != src.id())
    cc->emit(Inst::kIdMovaps, dst, src);
}

static ASMJIT_NOINLINE void sse_fmov(UniCompiler& uc, const Vec& dst, const Operand_& src, FloatMode fm) {
  BackendCompiler* cc = uc.cc;
  if (src.is_reg()) {
    if (dst.id() != src.id()) {
      cc->emit(Inst::kIdMovaps, dst, src);
    }
  }
  else if (is_scalar_fp_op(fm)) {
    cc->emit(sse_float_inst[size_t(fm)].fmovs, dst, src);
  }
  else {
    cc->emit(sse_float_inst[size_t(fm)].fmovu, dst, src);
  }
}

static ASMJIT_NOINLINE Vec sse_copy(UniCompiler& uc, const Vec& vec, const char* name) {
  Vec copy = uc.new_similar_reg(vec, name);
  uc.cc->emit(Inst::kIdMovaps, copy, vec);
  return copy;
}

static ASMJIT_NOINLINE void sse_make_vec(UniCompiler& uc, Operand_& op, const char* name) {
  if (op.is_mem()) {
    Vec tmp = uc.new_vec128(name);
    sse_mov(uc, tmp, op);
    op = tmp;
  }
}

static ASMJIT_INLINE uint32_t shuf_imm2_from_swizzle(Swizzle2 s) noexcept {
  return x86::shuffle_imm((s.value >> 8) & 0x1, s.value & 0x1);
}

static ASMJIT_INLINE uint32_t shuf_imm2_from_swizzle_with_width(Swizzle2 s, VecWidth w) noexcept {
  static constexpr uint32_t multipliers[] = { 0x1, 0x5, 0x55 };
  return shuf_imm2_from_swizzle(s) * multipliers[size_t(w)];
}

static ASMJIT_INLINE uint32_t shuf_imm4_from_swizzle(Swizzle4 s) noexcept {
  return x86::shuffle_imm((s.value >> 24 & 0x3), (s.value >> 16) & 0x3, (s.value >> 8) & 0x3, s.value & 0x3);
}

static ASMJIT_INLINE uint32_t shuf_imm4_from_swizzle(Swizzle2 s) noexcept {
  uint32_t imm0 = uint32_t(s.value     ) & 1u;
  uint32_t imm1 = uint32_t(s.value >> 8) & 1u;
  return x86::shuffle_imm(imm1 * 2u + 1u, imm1 * 2u, imm0 * 2u + 1u, imm0 * 2u);
}

static ASMJIT_NOINLINE void sse_bit_not(UniCompiler& uc, const Vec& dst, const Operand_& src) {
  BackendCompiler* cc = uc.cc;

  sse_mov(uc, dst, src);
  Operand ones = uc.simd_const(&uc.ct().p_FFFFFFFFFFFFFFFF, Bcst::k32, dst);
  cc->emit(Inst::kIdPxor, dst, ones);
}

static ASMJIT_NOINLINE void sse_msb_flip(UniCompiler& uc, const Vec& dst, const Operand_& src, ElementSize sz) {
  BackendCompiler* cc = uc.cc;
  const void* msk_data {};

  switch (sz) {
    case ElementSize::k8 : msk_data = &uc.ct().p_8080808080808080; break;
    case ElementSize::k16: msk_data = &uc.ct().p_8000800080008000; break;
    case ElementSize::k32: msk_data = &uc.ct().p_8000000080000000; break;
    case ElementSize::k64: msk_data = &uc.ct().p_8000000000000000; break;

    default:
      ASMJIT_NOT_REACHED();
  }

  Operand msk = uc.simd_const(msk_data, Bcst::kNA, dst);
  sse_mov(uc, dst, src);
  cc->emit(Inst::kIdPxor, dst, msk);
}

static ASMJIT_NOINLINE void sse_fsign_flip(UniCompiler& uc, const Vec& dst, const Operand_& src, FloatMode fm) {
  BackendCompiler* cc = uc.cc;

  const FloatInst& fi = sse_float_inst[size_t(fm)];
  Operand msk;

  switch (fm) {
    case FloatMode::kF32S: msk = uc.simd_const(&uc.ct().sign32_scalar, Bcst::k32, dst); break;
    case FloatMode::kF64S: msk = uc.simd_const(&uc.ct().sign64_scalar, Bcst::k64, dst); break;
    case FloatMode::kF32V: msk = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst); break;
    case FloatMode::kF64V: msk = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst); break;

    default:
      ASMJIT_NOT_REACHED();
  }

  sse_fmov(uc, dst, src, fm);
  cc->emit(fi.fxor, dst, msk);
}

// Possibly the best solution:
//   https://stackoverflow.com/questions/65166174/how-to-simulate-pcmpgtq-on-sse2
static ASMJIT_NOINLINE void sse_cmp_gt_i64(UniCompiler& uc, const Vec& dst, const Operand_& a, const Operand_& b) {
  BackendCompiler* cc = uc.cc;

  if (uc.has_sse4_2()) {
    if (is_same_vec(dst, a)) {
      cc->emit(Inst::kIdPcmpgtq, dst, b);
    }
    else {
      Operand_ second = b;
      if (is_same_vec(dst, b)) {
        second = cc->new_similar_reg(dst, "@tmp");
        sse_mov(uc, second.as<Vec>(), b);
      }
      sse_mov(uc, dst, a);
      cc->emit(Inst::kIdPcmpgtq, dst, second);
    }
  }
  else {
    Vec tmp1 = cc->new_similar_reg(dst, "@tmp1");
    Vec tmp2 = cc->new_similar_reg(dst, "@tmp2");

    cc->emit(Inst::kIdMovdqa, tmp1, a);
    cc->emit(Inst::kIdMovdqa, tmp2, b);
    cc->emit(Inst::kIdPcmpeqd, tmp1, tmp2);
    cc->emit(Inst::kIdPsubq, tmp2, a);
    cc->emit(Inst::kIdPand, tmp1, tmp2);

    if (!is_same_vec(dst, b)) {
      sse_mov(uc, dst, a);
      cc->emit(Inst::kIdPcmpgtd, dst, b);
      cc->emit(Inst::kIdPor, dst, tmp1);
      cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(3, 3, 1, 1));
    }
    else {
      sse_mov(uc, tmp2, a);
      cc->emit(Inst::kIdPcmpgtd, tmp2, b);
      cc->emit(Inst::kIdPor, tmp2, tmp1);
      cc->emit(Inst::kIdPshufd, dst, tmp2, x86::shuffle_imm(3, 3, 1, 1));
    }
  }
}

// Possibly the best solution:
//   https://stackoverflow.com/questions/65441496/what-is-the-most-efficient-way-to-do-unsigned-64-bit-comparison-on-sse2
static ASMJIT_NOINLINE void sse_cmp_gt_u64(UniCompiler& uc, const Vec& dst, const Operand_& a, const Operand_& b) {
  BackendCompiler* cc = uc.cc;

  if (uc.has_sse4_2()) {
    Operand msk = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst);
    Vec tmp = cc->new_similar_reg(dst, "@tmp");

    if (is_same_vec(dst, a)) {
      sse_mov(uc, tmp, msk);
      cc->emit(Inst::kIdPxor, dst, tmp);
      cc->emit(Inst::kIdPxor, tmp, b);
      cc->emit(Inst::kIdPcmpgtq, dst, tmp);
    }
    else {
      sse_mov(uc, tmp, b);
      sse_mov(uc, dst, a);
      cc->emit(Inst::kIdPxor, dst, msk);
      cc->emit(Inst::kIdPxor, tmp, msk);
      cc->emit(Inst::kIdPcmpgtq, dst, tmp);
    }
  }
  else {
    Vec tmp1 = cc->new_similar_reg(dst, "@tmp1");
    Vec tmp2 = cc->new_similar_reg(dst, "@tmp2");
    Vec tmp3 = cc->new_similar_reg(dst, "@tmp3");

    sse_mov(uc, tmp1, b);                  // tmp1 = b;
    sse_mov(uc, tmp2, a);                  // tmp2 = a;
    cc->emit(Inst::kIdMovaps, tmp3, tmp1); // tmp3 = b;
    cc->emit(Inst::kIdPsubq, tmp3, tmp2);  // tmp3 = b - a
    cc->emit(Inst::kIdPxor, tmp2, tmp1);   // tmp2 = b ^ a
    cc->emit(Inst::kIdPandn, tmp1, a);     // tmp1 =~b & a
    cc->emit(Inst::kIdPandn, tmp2, tmp3);  // tmp2 =~(b ^ a) & (b - a)
    cc->emit(Inst::kIdPor, tmp1, tmp2);    // tmp2 =~(b ^ a) & (b - a) | (~b & a)
    cc->emit(Inst::kIdPsrad, tmp1, 31);    // tmp1 =~(b ^ a) & (b - a) | (~b & a) - repeated MSB bits in 32-bit lanes
    cc->emit(Inst::kIdPshufd, dst, tmp1, x86::shuffle_imm(3, 3, 1, 1));
  }
}

static ASMJIT_NOINLINE void sse_select(UniCompiler& uc, const Vec& dst, const Vec& a, const Operand_& b, const Vec& msk) {
  BackendCompiler* cc = uc.cc;
  sse_mov(uc, dst, a);
  cc->emit(Inst::kIdPand, dst, msk);
  cc->emit(Inst::kIdPandn, msk, b);
  cc->emit(Inst::kIdPor, dst, msk);
}

static ASMJIT_NOINLINE void sse_int_widen(UniCompiler& uc, const Vec& dst, const Vec& src, WideningOp cvt) {
  BackendCompiler* cc = uc.cc;
  WideningOpInfo cvt_info = sse_int_widening_op_info[size_t(cvt)];

  if (uc.has_sse4_1()) {
    cc->emit(cvt_info.mov, dst, src);
    return;
  }

  if (!cvt_info.sign_extends && cvt_info.unpack_lo != Inst::kIdNone) {
    Operand zero = uc.simd_const(&uc.ct().p_0000000000000000, Bcst::kNA, dst);
    sse_mov(uc, dst, src);
    cc->emit(cvt_info.unpack_lo, dst, zero);
    return;
  }

  switch (cvt) {
    case WideningOp::kI8ToI16: {
      cc->overwrite().emit(cvt_info.unpack_lo, dst, src);
      cc->psraw(dst, 8);
      return;
    }

    case WideningOp::kI8ToI32: {
      cc->overwrite().emit(Inst::kIdPunpcklbw, dst, src);
      cc->punpcklwd(dst, dst);
      cc->psrad(dst, 24);
      return;
    }

    case WideningOp::kU8ToU32: {
      Operand zero = uc.simd_const(&uc.ct().p_0000000000000000, Bcst::kNA, dst);
      sse_mov(uc, dst, src);

      cc->emit(Inst::kIdPunpcklbw, dst, zero);
      cc->emit(Inst::kIdPunpcklwd, dst, zero);
      return;
    }

    case WideningOp::kU8ToU64: {
      Operand zero = uc.simd_const(&uc.ct().p_0000000000000000, Bcst::kNA, dst);
      sse_mov(uc, dst, src);

      cc->emit(Inst::kIdPunpcklbw, dst, zero);
      cc->emit(Inst::kIdPunpcklwd, dst, zero);
      cc->emit(Inst::kIdPunpckldq, dst, zero);
      return;
    }

    case WideningOp::kI16ToI32: {
      cc->overwrite().emit(cvt_info.unpack_lo, dst, src);
      cc->psrad(dst, 16);
      return;
    }

    case WideningOp::kI32ToI64: {
      Vec tmp = uc.new_similar_reg(dst, "@tmp");
      sse_mov(uc, tmp, src);
      sse_mov(uc, dst, src);
      cc->psrad(tmp, 31);
      cc->punpckldq(dst, tmp);
      return;
    }

    default:
      ASMJIT_NOT_REACHED();
  }
}

static ASMJIT_NOINLINE void sse_round(UniCompiler& uc, const Vec& dst, const Operand& src, FloatMode fm, x86::RoundImm round_mode) {
  BackendCompiler* cc = uc.cc;

  uint32_t is_f32 = fm == FloatMode::kF32S || fm == FloatMode::kF32V;
  const FloatInst& fi = sse_float_inst[size_t(fm)];

  // NOTE: This may be dead code as the compiler handles this case well, however, if this function is
  // called as a helper we don't want to emit a longer sequence if we can just use a single instruction.
  if (uc.has_sse4_1()) {
    cc->emit(fi.fround, dst, src, round_mode | x86::RoundImm::kSuppress);
    return;
  }

  // round_max (f32) == 0x4B000000
  // round_max (f64) == 0x4330000000000000
  Operand maxn = get_fop_round_magic(uc, dst, fm);

  Vec t1 = uc.new_similar_reg(dst, "@t1");
  Vec t2 = uc.new_similar_reg(dst, "@t2");
  Vec t3 = uc.new_similar_reg(dst, "@t3");

  if (round_mode == x86::RoundImm::kTrunc) {
    if (fm == FloatMode::kF32S || (fm == FloatMode::kF64S && cc->is_64bit())) {
      Gp r;
      Operand msb;

      if (fm == FloatMode::kF32S) {
        r = uc.new_gp32("@gp_tmp");
        msb = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst);
      }
      else {
        r = uc.new_gp64("@gp_tmp");
        msb = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst);
      }

      sse_fmov(uc, dst, src, fm);

      if (fm == FloatMode::kF32S)
        cc->cvttss2si(r, dst);
      else
        cc->cvttsd2si(r, dst);

      cc->emit(fi.fmova, t2, msb);
      cc->emit(fi.fandn, t2, dst);
      cc->emit(fi.fxor, t1, t1);

      if (fm == FloatMode::kF32S)
        cc->cvtsi2ss(t1, r);
      else
        cc->cvtsi2sd(t1, r);

      cc->emit(fi.fcmp, t2, maxn, x86::CmpImm::kLT);
      cc->emit(fi.fand, t1, t2);
      cc->emit(fi.fandn, t2, dst);
      cc->emit(fi.for_, t2, t1);
      cc->emit(fi.fmovs, dst, t2);
      return;
    }
  }

  if (round_mode == x86::RoundImm::kNearest) {
    // Pure SSE2 round-to-even implementation:
    //
    //   float round_even(float x) {
    //     float magic = x >= 0 ? pow(2, 22) : pow(2, 22) + pow(2, 21);
    //     return x >= magic ? x : x + magic - magic;
    //   }
    //
    //   double round_even(double x) {
    //     double magic = x >= 0 ? pow(2, 52) : pow(2, 52) + pow(2, 51);
    //     return x >= magic ? x : x + magic - magic;
    //   }
    sse_fmov(uc, dst, src, fm);
    cc->emit(fi.fmova, t3, dst);
    // cc->emit(fi.psrl, t3, Imm(is_f32 ? 31 : 63));
    // cc->emit(fi.psll, t3, Imm(is_f32 ? 23 : 51));
    // cc->emit(fi.for_, t3, maxn);

    cc->emit(fi.psrl, t3, Imm(is_f32 ? 31 : 63));
    cc->emit(fi.psll, t3, Imm(is_f32 ? 23 : 52));
    cc->emit(is_f32 ? Inst::kIdPaddd : Inst::kIdPaddq, t3, maxn);

    cc->emit(fi.fmova, t1, dst);
    cc->emit(fi.fcmp, t1, t3, x86::CmpImm::kLT);
    cc->emit(fi.fand, t1, t3);

    cc->emit(fi.fadd, dst, t1);
    cc->emit(fi.fsub, dst, t1);
    return;
  }

  Operand one = get_fop_one(uc, dst, fm);

  if (round_mode == x86::RoundImm::kTrunc) {
    // Should be handled earlier.
    ASMJIT_ASSERT(fm != FloatMode::kF32S);

    Operand msb;

    if (fm == FloatMode::kF32V) {
      msb = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst);
      sse_fmov(uc, dst, src, fm);

      cc->cvttps2dq(t1, dst);
      cc->emit(fi.fmova, t2, msb);
      cc->emit(fi.fandn, t2, dst);
      cc->cvtdq2ps(t1, t1);

      cc->emit(fi.fcmp, t2, maxn, x86::CmpImm::kLT);
      cc->emit(fi.fand, t1, t2);
      cc->emit(fi.fandn, t2, dst);
      cc->emit(fi.for_, t2, t1);
      cc->emit(fi.fmova, dst, t2);
    }
    else {
      msb = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst);

      sse_fmov(uc, dst, src, fm);
      cc->emit(fi.fmova, t3, msb);
      cc->emit(fi.fandn, t3, dst);
      cc->emit(fi.fmova, t2, t3);
      cc->emit(fi.fcmp, t2, maxn, x86::CmpImm::kLT);
      cc->emit(fi.fand, t2, maxn);
      cc->emit(fi.fmova, t1, t3);
      cc->emit(fi.fadd, t1, t2);
      cc->emit(fi.fsub, t1, t2);
      cc->emit(fi.fcmp, t3, t1, x86::CmpImm::kLT);
      cc->emit(fi.fand, t3, one);
      cc->emit(fi.fsub, t1, t3);

      cc->emit(fi.fand, dst, msb);
      cc->emit(fi.for_, dst, t1);
      return;
    }
    return;
  }

  // Round up & down needs a correction as adding and subtracting magic number rounds to nearest.
  if (round_mode == x86::RoundImm::kDown || round_mode == x86::RoundImm::kUp) {
    InstId correction_inst_id = round_mode == x86::RoundImm::kDown ? fi.fsub : fi.fadd;
    x86::CmpImm correction_predicate = round_mode == x86::RoundImm::kDown ? x86::CmpImm::kLT : x86::CmpImm::kNLE;

    sse_fmov(uc, dst, src, fm);

    // maxn (f32) == 0x4B000000 (f64) == 0x4330000000000000
    // t3   (f32) == 0x00800000 (f64) == 0x0008000000000000

    cc->emit(fi.fmova, t3, dst);
    cc->emit(fi.psrl, t3, Imm(is_f32 ? 31 : 63));
    cc->emit(fi.psll, t3, Imm(is_f32 ? 23 : 52));
    cc->emit(is_f32 ? Inst::kIdPaddd : Inst::kIdPaddq, t3, maxn);

    cc->emit(fi.fmova, t1, dst);
    cc->emit(fi.fmova, t2, dst);
    cc->emit(fi.fadd, t2, t3);
    cc->emit(fi.fsub, t2, t3);

    cc->emit(fi.fcmp, t1, t3, x86::CmpImm::kNLT);
    cc->emit(fi.fmova, t3, dst);
    cc->emit(fi.fcmp, t3, t2, correction_predicate);
    cc->emit(fi.fand, t3, one);

    cc->emit(fi.fand, dst, t1);
    cc->emit(correction_inst_id, t2, t3);

    cc->emit(fi.fandn, t1, t2);
    cc->emit(fi.for_, dst, t1);
    return;
  }

  ASMJIT_NOT_REACHED();
}

static ASMJIT_NOINLINE void avx_mov(UniCompiler& uc, const Vec& dst, const Operand_& src) {
  BackendCompiler* cc = uc.cc;
  InstId inst_id = 0;

  if (dst.is_vec512()) {
    inst_id = src.is_mem() ? Inst::kIdVmovdqu32 : Inst::kIdVmovdqa32;
  }
  else {
    inst_id = src.is_mem() ? Inst::kIdVmovdqu : Inst::kIdVmovdqa;
  }

  cc->emit(inst_id, dst, src);
}

static ASMJIT_NOINLINE void avx_fmov(UniCompiler& uc, const Vec& dst, const Operand_& src, FloatMode fm) {
  BackendCompiler* cc = uc.cc;
  if (src.is_reg()) {
    if (dst.id() != src.id()) {
      if (fm <= FloatMode::kF64S)
        cc->emit(Inst::kIdVmovaps, dst.xmm(), src);
      else
        cc->emit(Inst::kIdVmovaps, dst, src);
    }
  }
  else if (is_scalar_fp_op(fm)) {
    cc->emit(avx_float_inst[size_t(fm)].fmovs, dst, src);
  }
  else {
    cc->emit(avx_float_inst[size_t(fm)].fmovu, dst, src);
  }
}

static ASMJIT_NOINLINE void avx_make_vec(UniCompiler& uc, Operand_& op, const Vec& ref, const char* name) {
  if (op.is_mem()) {
    Vec tmp = uc.new_similar_reg(ref, name);
    avx_mov(uc, tmp, op);
    op = tmp;
  }
}

static ASMJIT_NOINLINE void avx_zero(UniCompiler& uc, const Vec& dst) {
  BackendCompiler* cc = uc.cc;
  Vec x = dst.xmm();
  cc->vpxor(x, x, x);
  return;
}

static ASMJIT_NOINLINE void avx_ones(UniCompiler& uc, const Vec& dst) {
  BackendCompiler* cc = uc.cc;
  if (uc.has_avx512())
    cc->emit(Inst::kIdVpternlogd, dst, dst, dst, 0xFF);
  else
    cc->emit(Inst::kIdVpcmpeqb, dst, dst, dst);
}

static ASMJIT_NOINLINE void avx_bit_not(UniCompiler& uc, const Vec& dst, const Operand_& src) {
  BackendCompiler* cc = uc.cc;

  if (uc.has_avx512()) {
    if (src.is_reg())
      cc->overwrite().emit(Inst::kIdVpternlogd, dst, src, src, 0x55);
    else
      cc->overwrite().emit(Inst::kIdVpternlogd, dst, dst, src, 0x55);
    return;
  }

  Operand ones = uc.simd_const(&uc.ct().p_FFFFFFFFFFFFFFFF, Bcst::k32, dst);
  if (!src.is_reg()) {
    if (ones.is_reg()) {
      cc->emit(Inst::kIdVpxor, dst, ones, src);
    }
    else {
      avx_mov(uc, dst, src);
      cc->emit(Inst::kIdVpxor, dst, dst, ones);
    }
  }
  else {
    cc->emit(Inst::kIdVpxor, dst, src, ones);
  }
}

static ASMJIT_NOINLINE void avx_isign_flip(UniCompiler& uc, const Vec& dst, const Operand_& src, ElementSize sz) {
  BackendCompiler* cc = uc.cc;
  Operand msk;

  InstId xor_ = (uc.has_avx512() && dst.is_vec512()) ? Inst::kIdVpxord : Inst::kIdVpxor;

  switch (sz) {
    case ElementSize::k8: msk = uc.simd_const(&uc.ct().p_8080808080808080, Bcst::kNA, dst); break;
    case ElementSize::k16: msk = uc.simd_const(&uc.ct().p_8000800080008000, Bcst::kNA, dst); break;
    case ElementSize::k32: msk = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst); break;
    case ElementSize::k64: msk = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst); break;
  }

  if (src.is_reg()) {
    cc->emit(xor_, dst, src, msk);
  }
  else if (msk.is_reg()) {
    cc->emit(xor_, dst, msk, src);
  }
  else {
    avx_mov(uc, dst, src);
    cc->emit(xor_, dst, dst, msk);
  }
}

static ASMJIT_NOINLINE void avx_fsign_flip(UniCompiler& uc, const Vec& dst, const Operand_& src, FloatMode fm) {
  BackendCompiler* cc = uc.cc;

  const FloatInst& fi = avx_float_inst[size_t(fm)];
  Operand msk;

  switch (fm) {
    case FloatMode::kF32S: msk = uc.simd_const(&uc.ct().sign32_scalar, Bcst::kNA, dst); break;
    case FloatMode::kF64S: msk = uc.simd_const(&uc.ct().sign64_scalar, Bcst::kNA, dst); break;
    case FloatMode::kF32V: msk = uc.simd_const(&uc.ct().p_8000000080000000, Bcst::k32, dst); break;
    case FloatMode::kF64V: msk = uc.simd_const(&uc.ct().p_8000000000000000, Bcst::k64, dst); break;

    default:
      ASMJIT_NOT_REACHED();
  }

  if (src.is_reg()) {
    cc->emit(fi.fxor, dst, src, msk);
  }
  else if (msk.is_reg() && fm >= FloatMode::kF32V) {
    cc->emit(fi.fxor, dst, msk, src);
  }
  else {
    avx_fmov(uc, dst, src, fm);
    cc->emit(fi.fxor, dst, dst, msk);
  }
}

// ujit::UniCompiler - Vector Instructions - OpArray Iterator
// ==========================================================

template<typename T>
class OpArrayIter {
public:
  const T& _op;

  ASMJIT_INLINE_NODEBUG OpArrayIter(const T& op) noexcept : _op(op) {}
  ASMJIT_INLINE_NODEBUG const T& op() const noexcept { return _op; }
  ASMJIT_INLINE_NODEBUG void next() noexcept {}
};

template<>
class OpArrayIter<OpArray> {
public:
  const OpArray& _opArray;
  size_t _i {};
  size_t _n {};

  ASMJIT_INLINE_NODEBUG OpArrayIter(const OpArray& op_array) noexcept : _opArray(op_array), _i(0), _n(op_array.size()) {}
  ASMJIT_INLINE_NODEBUG const Operand_& op() const noexcept { return _opArray[_i]; }
  ASMJIT_INLINE_NODEBUG void next() noexcept { if (++_i >= _n) _i = 0; }
};

template<typename Src>
static ASMJIT_INLINE void emit_2v_t(UniCompiler& uc, UniOpVV op, const OpArray& dst_, const Src& src_) {
  size_t n = dst_.size();
  OpArrayIter<Src> src(src_);

  for (size_t i = 0; i < n; i++) {
    uc.emit_2v(op, dst_[i], src.op());
    src.next();
  }
}

template<typename Src>
static ASMJIT_INLINE void emit_2vi_t(UniCompiler& uc, UniOpVVI op, const OpArray& dst_, const Src& src_, uint32_t imm) {
  size_t n = dst_.size();
  OpArrayIter<Src> src(src_);

  for (size_t i = 0; i < n; i++) {
    uc.emit_2vi(op, dst_[i], src.op(), imm);
    src.next();
  }
}

template<typename Src1, typename Src2>
static ASMJIT_INLINE void emit_3v_t(UniCompiler& uc, UniOpVVV op, const OpArray& dst_, const Src1& src1_, const Src2& src2_) {
  size_t n = dst_.size();
  OpArrayIter<Src1> src1(src1_);
  OpArrayIter<Src2> src2(src2_);

  for (size_t i = 0; i < n; i++) {
    uc.emit_3v(op, dst_[i], src1.op(), src2.op());
    src1.next();
    src2.next();
  }
}

template<typename Src1, typename Src2>
static ASMJIT_INLINE void emit_3vi_t(UniCompiler& uc, UniOpVVVI op, const OpArray& dst_, const Src1& src1_, const Src2& src2_, uint32_t imm) {
  size_t n = dst_.size();
  OpArrayIter<Src1> src1(src1_);
  OpArrayIter<Src2> src2(src2_);

  for (size_t i = 0; i < n; i++) {
    uc.emit_3vi(op, dst_[i], src1.op(), src2.op(), imm);
    src1.next();
    src2.next();
  }
}

template<typename Src1, typename Src2, typename Src3>
static ASMJIT_INLINE void emit_4v_t(UniCompiler& uc, UniOpVVVV op, const OpArray& dst_, const Src1& src1_, const Src2& src2_, const Src3& src3_) {
  size_t n = dst_.size();
  OpArrayIter<Src1> src1(src1_);
  OpArrayIter<Src2> src2(src2_);
  OpArrayIter<Src3> src3(src3_);

  for (size_t i = 0; i < n; i++) {
    uc.emit_4v(op, dst_[i], src1.op(), src2.op(), src3.op());
    src1.next();
    src2.next();
    src3.next();
  }
}

// ujit::UniCompiler - Vector Instructions - Emit 2V
// =================================================

void UniCompiler::emit_2v(UniOpVV op, const Operand_& dst_, const Operand_& src_) {
  ASMJIT_ASSERT(dst_.is_vec());

  Vec dst(dst_.as<Vec>());
  Operand src(src_);
  UniOpVInfo op_info = opcode_info_2v[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    InstId inst_id = op_info.avx_inst_id;

    if (has_avx_ext(AVXExt(op_info.avx_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      if (op_info.use_imm)
        cc->emit(inst_id, dst, src, Imm(op_info.imm));
      else
        cc->emit(inst_id, dst, src);
      return;
    }

    switch (op) {
      case UniOpVV::kMov: {
        cc->emit(Inst::kIdVmovaps, dst, src);
        return;
      }

      case UniOpVV::kMovU64: {
        if (src.is_vec())
          src = src.as<Vec>().xmm();
        cc->emit(Inst::kIdVmovq, dst.xmm(), src);
        return;
      }

      case UniOpVV::kBroadcastU8Z:
      case UniOpVV::kBroadcastU16Z:
      case UniOpVV::kBroadcastU8:
      case UniOpVV::kBroadcastU16:
      case UniOpVV::kBroadcastU32:
      case UniOpVV::kBroadcastU64:
      case UniOpVV::kBroadcastF32:
      case UniOpVV::kBroadcastF64: {
        // Intrinsic - 32/64-bit broadcasts require AVX, 8/16-bit broadcasts require AVX2/AVX512.
        ASMJIT_ASSERT(src.is_reg() || src.is_mem());
        ElementSize element_size = ElementSize(op_info.element_size);

        if (src.is_gp()) {
          Gp src_gp = src.as<Gp>();
          if (element_size <= ElementSize::k32)
            src_gp = src_gp.r32();
          else
            src_gp = src_gp.r64();

          // AVX512 provides broadcast instructions for both GP, XMM, and memory sources, however, from GP register
          // only VP instructions are available, so we have to convert VBROADCAST[SS|SD] to VPBROADCAST[D|Q].
          if (has_avx512()) {
            if (op == UniOpVV::kBroadcastF32) inst_id = Inst::kIdVpbroadcastd;
            if (op == UniOpVV::kBroadcastF64) inst_id = Inst::kIdVpbroadcastq;
            cc->emit(inst_id, dst, src_gp);
            return;
          }

          // We can handle BroadcastU[8|16]Z differently when AVX2 is not present. Since the opcode has guaranteed
          // source, which has zerod the rest of the register, we are going to multiply with a constant to extend
          // the data into 32 bits, and then we can just use VBROADCASTSS, which would do the rest.
          if (!has_avx2() && element_size <= ElementSize::k16 && op_info.imm == 0x01u) {
            Gp expanded = new_gp32("@expanded");
            cc->imul(expanded, src_gp, element_size == ElementSize::k8 ? 0x01010101u : 0x00010001u);
            cc->vmovd(dst.xmm(), expanded);
            cc->vpshufd(dst.xmm(), dst.xmm(), x86::shuffle_imm(0, 0, 0, 0));

            if (!dst.is_vec128())
              cc->emit(Inst::kIdVinsertf128, dst, dst, dst.xmm(), 0);
            return;
          }

          // AVX/AVX2 doesn't provide broadcast from GP to XMM, we have to move to XMM first.
          InstId mov = element_size <= ElementSize::k32 ? Inst::kIdVmovd : Inst::kIdVmovq;
          cc->emit(mov, dst.xmm(), src_gp);
          src = dst.xmm();
        }

        // We have ether a broadcast from memory or an XMM register - AVX2 requires special handling from here...
        if (!has_avx2()) {
          Vec dst_xmm = dst.xmm();

          if (element_size <= ElementSize::k16) {
            // AVX doesn't provide 8-bit and 16-bit broadcasts - the simplest way is to just use VPSHUFB to repeat the byte.
            InstId insert_inst_id = element_size == ElementSize::k8 ? Inst::kIdVpinsrb : Inst::kIdVpinsrw;

            const void* pred_data = element_size == ElementSize::k8 ? static_cast<const void*>(&ct().p_0000000000000000)
                                                                    : static_cast<const void*>(&ct().p_0100010001000100);
            Vec pred = simd_vec_const(pred_data, Bcst::k32, dst_xmm);

            if (src.is_mem()) {
              cc->emit(insert_inst_id, dst_xmm, pred, src, 0);
              cc->vpshufb(dst_xmm, dst_xmm, pred);
            }
            else {
              cc->vpshufb(dst_xmm, src.as<Vec>().xmm(), pred);
            }
          }
          else {
            // AVX doesn't have VPBROADCAST[D|Q], but it has VBROADCAST[SS|SD], which do the same. However,
            // these cannot be used when the source is a register - initially these instructions only allowed
            // broadcasting from memory, then with AVX2 a version that broadcasts from a register was added.
            if (src.is_mem()) {
              InstId bcst_inst_id = (element_size == ElementSize::k32) ? Inst::kIdVbroadcastss : Inst::kIdVbroadcastsd;
              if (dst.is_vec128() && bcst_inst_id == Inst::kIdVbroadcastsd)
                bcst_inst_id = Inst::kIdVmovddup;
              cc->emit(bcst_inst_id, dst, src.as<Mem>());
              return;
            }

            Vec src_xmm = src.as<Vec>().xmm();
            if (element_size == ElementSize::k32)
              cc->vpshufd(dst_xmm, src_xmm, x86::shuffle_imm(0, 0, 0, 0));
            else
              cc->vmovddup(dst_xmm, src_xmm);
          }

          if (!dst.is_vec128())
            cc->emit(Inst::kIdVinsertf128, dst, dst, dst_xmm, 0);
          return;
        }

        // VBROADCASTSD cannot be used when XMM is a destination, in that case we must use VMOVDDUP.
        if (dst.is_vec128() && inst_id == Inst::kIdVbroadcastsd)
          inst_id = Inst::kIdVmovddup;

        if (src.is_mem()) {
          Mem m = src.as<Mem>();
          m.set_size(1u << op_info.element_size);
          cc->emit(inst_id, dst, m);
        }
        else {
          cc->emit(inst_id, dst, src.as<Vec>().xmm());
        }
        return;
      }

      case UniOpVV::kBroadcastV128_U32:
      case UniOpVV::kBroadcastV128_U64:
      case UniOpVV::kBroadcastV128_F32:
      case UniOpVV::kBroadcastV128_F64: {
        if (src.is_reg()) {
          ASMJIT_ASSERT(src.is_vec());
          src = src.as<Vec>().xmm();
        }

        // 128-bit broadcast is like 128-bit mov in this case as we don't have a wider destination.
        if (dst.is_vec128()) {
          avx_mov(*this, dst, src);
          return;
        }

        // Broadcast instructions only work when the source is a memory operand.
        if (src.is_mem()) {
          if (!has_avx512()) {
            ASMJIT_ASSERT(dst.is_vec256());
            inst_id = (op >= UniOpVV::kBroadcastV128_F32 || !has_avx2()) ? Inst::kIdVbroadcastf128 : Inst::kIdVbroadcasti128;
          }

          cc->emit(inst_id, dst, src);
          return;
        }

        // Broadcast with a register source operand is implemented via insert in AVX/AVX2 case.
        if (dst.is_vec256()) {
          if (!has_avx512())
            inst_id = (op >= UniOpVV::kBroadcastV128_F32 || !has_avx2()) ? Inst::kIdVinsertf128 : Inst::kIdVinserti128;
          else
            inst_id = avx512_vinsert_128[size_t(op) - size_t(UniOpVV::kBroadcastV128_U32)];

          cc->emit(inst_id, dst, src.as<Vec>().ymm(), src, 1);
          return;
        }

        // Broadcast with a register to 512-bits is implemented via 128-bit shuffle.
        ASMJIT_ASSERT(dst.is_vec512());

        inst_id = avx512_vshuf_128[size_t(op) - size_t(UniOpVV::kBroadcastV128_U32)];
        src = src.as<Vec>().zmm();
        cc->emit(inst_id, dst, src, src, x86::shuffle_imm(0, 0, 0, 0));
        return;
      }

      case UniOpVV::kBroadcastV256_U32:
      case UniOpVV::kBroadcastV256_U64:
      case UniOpVV::kBroadcastV256_F32:
      case UniOpVV::kBroadcastV256_F64: {
        if (src.is_reg()) {
          ASMJIT_ASSERT(src.is_vec());
          src = src.as<Vec>().ymm();
        }

        // Cannot broadcast 256-bit vector to a 128-bit or 256-bit vector...
        if (!dst.is_vec512()) {
          avx_mov(*this, dst.ymm(), src);
          return;
        }

        if (src.is_mem()) {
          cc->emit(inst_id, dst, src);
          return;
        }

        inst_id = avx512_vshuf_128[size_t(op) - size_t(UniOpVV::kBroadcastV256_U32)];
        src = src.as<Vec>().zmm();
        cc->emit(inst_id, dst, src, src, x86::shuffle_imm(1, 0, 1, 0));
        return;
      }

      case UniOpVV::kAbsI64: {
        // Native operation requires AVX512, which is not supported by the target.
        Vec tmp = new_similar_reg(dst, "@tmp");
        cc->vpxor(tmp, tmp, tmp);
        cc->emit(Inst::kIdVpsubq, tmp, tmp, src);
        cc->emit(Inst::kIdVblendvpd, dst, tmp, src, tmp);
        return;
      }

      case UniOpVV::kNotU32:
      case UniOpVV::kNotU64:
      case UniOpVV::kNotF32:
      case UniOpVV::kNotF64: {
        avx_bit_not(*this, dst, src);
        return;
      }

      case UniOpVV::kCvtI8ToI32:
      case UniOpVV::kCvtU8ToU32: {
        if (src.is_reg())
          src.as<Vec>().set_signature(signature_of_xmm_ymm_zmm[0]);
        else
          src.as<Mem>().set_size(dst.size() / 4u);

        cc->emit(inst_id, dst, src);
        return;
      }

      case UniOpVV::kCvtI8HiToI16:
      case UniOpVV::kCvtU8HiToU16:
      case UniOpVV::kCvtI16HiToI32:
      case UniOpVV::kCvtU16HiToU32:
      case UniOpVV::kCvtI32HiToI64:
      case UniOpVV::kCvtU32HiToU64:
        if (src.is_vec()) {
          if (dst.is_vec128()) {
            Vec tmp = new_vec128("@tmp");
            cc->vpshufd(tmp, src.as<Vec>(), x86::shuffle_imm(3, 2, 3, 2));
            src = tmp;
          }
          else if (dst.is_vec256()) {
            Vec tmp = new_vec128("@tmp");
            cc->vextractf128(tmp, src.as<Vec>().ymm(), 1u);
            src = tmp;
          }
          else if (dst.is_vec512()) {
            Vec tmp = new_vec256("@tmp");
            cc->vextracti32x8(tmp, src.as<Vec>().zmm(), 1u);
            src = tmp;
          }
          else {
            ASMJIT_NOT_REACHED();
          }
        }
        else if (src.is_mem()) {
          src.as<Mem>().add_offset(dst.size() / 2u);
        }
        else {
          ASMJIT_NOT_REACHED();
        }
        [[fallthrough]];

      case UniOpVV::kCvtI8LoToI16:
      case UniOpVV::kCvtU8LoToU16:
      case UniOpVV::kCvtI16LoToI32:
      case UniOpVV::kCvtU16LoToU32:
      case UniOpVV::kCvtI32LoToI64:
      case UniOpVV::kCvtU32LoToU64: {
        if (src.is_reg())
          src.as<Vec>().set_signature(signature_of_xmm_ymm_zmm[dst.size() >> 6]);
        else
          src.as<Mem>().set_size(dst.size() / 2u);

        cc->emit(inst_id, dst, src);
        return;
      }

      case UniOpVV::kAbsF32S:
      case UniOpVV::kAbsF64S:
      case UniOpVV::kAbsF32:
      case UniOpVV::kAbsF64:
      case UniOpVV::kNegF32S:
      case UniOpVV::kNegF64S:
      case UniOpVV::kNegF32:
      case UniOpVV::kNegF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);

        const void* msk_data =
          op == UniOpVV::kAbsF32 || op == UniOpVV::kAbsF32S ? static_cast<const void*>(&ct().p_7FFFFFFF7FFFFFFF) :
          op == UniOpVV::kAbsF64 || op == UniOpVV::kAbsF64S ? static_cast<const void*>(&ct().p_7FFFFFFFFFFFFFFF) :
          op == UniOpVV::kNegF32 || op == UniOpVV::kNegF32S ? static_cast<const void*>(&ct().p_8000000080000000) :
                                                              static_cast<const void*>(&ct().p_8000000000000000);
        Operand msk = simd_const(msk_data, Bcst(op_info.broadcast_size), dst);

        if (src.is_mem() && is_scalar_fp_op(fm)) {
          avx_fmov(*this, dst, src, fm);
          cc->emit(inst_id, dst, dst, msk);
        }
        else if (src.is_mem() && msk.is_mem()) {
          avx_fmov(*this, dst, msk, fm);
          cc->emit(inst_id, dst, dst, src);
        }
        else if (src.is_mem()) {
          cc->emit(inst_id, dst, msk, src);
        }
        else {
          cc->emit(inst_id, dst, src, msk);
        }
        return;
      }

      case UniOpVV::kRcpF32: {
        // Intrinsic.
        Vec one = simd_vec_const(&ct().f32_1, Bcst::k32, dst);
        cc->emit(Inst::kIdVdivps, dst, one, src);
        return;
      }

      case UniOpVV::kRcpF64: {
        // Intrinsic.
        Vec one = simd_vec_const(&ct().f64_1, Bcst::k32, dst);
        cc->emit(Inst::kIdVdivpd, dst, one, src);
        return;
      }

      case UniOpVV::kTruncF32S:
      case UniOpVV::kTruncF64S:
      case UniOpVV::kTruncF32:
      case UniOpVV::kTruncF64:
      case UniOpVV::kFloorF32S:
      case UniOpVV::kFloorF64S:
      case UniOpVV::kFloorF32:
      case UniOpVV::kFloorF64:
      case UniOpVV::kCeilF32S:
      case UniOpVV::kCeilF64S:
      case UniOpVV::kCeilF32:
      case UniOpVV::kCeilF64:
      case UniOpVV::kRoundEvenF32S:
      case UniOpVV::kRoundEvenF64S:
      case UniOpVV::kRoundEvenF32:
      case UniOpVV::kRoundEvenF64: {
        FloatMode fm = FloatMode(op_info.float_mode);

        if (is_scalar_fp_op(fm)) {
          dst = dst.xmm();
        }

        if (has_avx512() && dst.is_vec512()) {
          // AVX512 uses a different name.
          constexpr uint16_t avx512_rndscale[4] = {
            Inst::kIdVrndscaless,
            Inst::kIdVrndscalesd,
            Inst::kIdVrndscaleps,
            Inst::kIdVrndscalepd
          };
          inst_id = avx512_rndscale[(size_t(op) - size_t(UniOpVV::kTruncF32S)) & 0x3];
        }

        if (is_scalar_fp_op(fm)) {
          // These instructions use 3 operand form for historical reasons.
          if (src.is_mem()) {
            cc->emit(avx_float_inst[size_t(op_info.float_mode)].fmovs, dst, src);
            cc->emit(inst_id, dst, dst, dst, uint32_t(op_info.imm));
          }
          else {
            src = src.as<Vec>().xmm();
            cc->emit(inst_id, dst, src, src, uint32_t(op_info.imm));
          }
        }
        else {
          cc->emit(inst_id, dst, src, uint32_t(op_info.imm));
        }
        return;
      }

      case UniOpVV::kRoundHalfAwayF32S:
      case UniOpVV::kRoundHalfAwayF64S:
      case UniOpVV::kRoundHalfAwayF32:
      case UniOpVV::kRoundHalfAwayF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);
        const FloatInst& fi = avx_float_inst[fm];

        if (is_scalar_fp_op(fm)) {
          dst = dst.xmm();
          if (src.is_vec()) {
            src = src.as<Vec>().clone_as(dst);
          }
        }

        if (src.is_mem()) {
          avx_fmov(*this, dst, src, fm);
          src = dst;
        }

        Operand half = get_fop_half_minus_1ulp(*this, dst, fm);
        Operand msb = get_fop_msb_bit(*this, dst, fm);
        Vec tmp = new_similar_reg(dst, "@tmp");

        if (has_avx512()) {
          cc->emit(fi.fmova, tmp, msb);
          cc->emit(Inst::kIdVpternlogd, tmp, src, half, 0xEAu); // tmp = (msb & src) | half
        }
        else {
          cc->emit(fi.fand, tmp, src, msb);
          cc->emit(fi.for_, tmp, tmp, half);
        }

        cc->emit(fi.fadd, dst, src, tmp);

        if (is_scalar_fp_op(fm)) {
          cc->emit(fi.fround, dst, dst, dst, x86::RoundImm::kTrunc | x86::RoundImm::kSuppress);
        }
        else {
          InstId round_inst = dst.is_vec512() ? fi.frndscale : fi.fround;
          cc->emit(round_inst, dst, dst, x86::RoundImm::kTrunc | x86::RoundImm::kSuppress);
        }
        return;
      }

      case UniOpVV::kRoundHalfUpF32S:
      case UniOpVV::kRoundHalfUpF64S:
      case UniOpVV::kRoundHalfUpF32:
      case UniOpVV::kRoundHalfUpF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);

        if (is_scalar_fp_op(fm)) {
          dst = dst.xmm();
        }

        Operand half = get_fop_half_minus_1ulp(*this, dst, fm);
        UniOpVVV add_op = translate_op(op, UniOpVV::kRoundHalfUpF32S, UniOpVVV::kAddF32S);
        UniOpVV floor_op = translate_op(op, UniOpVV::kRoundHalfUpF32S, UniOpVV::kFloorF32S);

        if (src.is_mem()) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          avx_fmov(*this, tmp, src, fm);
          emit_3v(add_op, tmp, tmp, half);
          emit_2v(floor_op, dst, tmp);
        }
        else {
          emit_3v(add_op, dst, src.as<Vec>().clone_as(dst), half);
          emit_2v(floor_op, dst, dst);
        }
        return;
      }

      case UniOpVV::kSqrtF32S:
      case UniOpVV::kSqrtF64S: {
        dst = dst.xmm();

        // Intrinsic - these instructions use 3 operand form for historical reasons.
        if (src.is_mem()) {
          avx_fmov(*this, dst, src, FloatMode(op_info.float_mode));
          cc->emit(inst_id, dst, dst, dst);
        }
        else {
          src = src.as<Vec>().xmm();
          cc->emit(inst_id, dst, src, src);
        }
        return;
      }

      case UniOpVV::kCvtF32ToF64S:
      case UniOpVV::kCvtF64ToF32S: {
        dst = dst.xmm();
        if (src.is_vec())
          src = src.as<Vec>().xmm();

        // Intrinsic - these instructions use 3 operand form for historical reasons.
        Vec zeros = simd_vec_const(&ct().p_0000000000000000, Bcst::k32, dst);
        cc->emit(inst_id, dst, zeros, src);
        return;
      }

      case UniOpVV::kCvtF32LoToF64:
      case UniOpVV::kCvtI32LoToF64: {
        // Intrinsic - widening conversion - low part conversions are native, high part emulated.
        if (src.is_reg()) {
          uint32_t w = dst.size() >> 6;
          src.set_signature(signature_of_xmm_ymm_zmm[w]);
        }
        else {
          uint32_t w = dst.size() >> 4;
          src.as<Mem>().set_size(w * 8u);
        }

        cc->emit(inst_id, dst, src);
        return;
      }

      case UniOpVV::kCvtF32HiToF64:
      case UniOpVV::kCvtI32HiToF64: {
        if (src.is_reg()) {
          uint32_t w = dst.size() >> 6;
          Vec tmp = new_vec_with_width(VecWidth(w), "@tmp");

          src.set_signature(signature_of_xmm_ymm_zmm[w]);
          if (dst.is_vec512()) {
            cc->vextracti32x8(tmp, src.as<Vec>().zmm(), 1u);
            cc->emit(inst_id, dst, tmp);
          }
          else if (dst.is_vec256()) {
            if (has_avx512())
              cc->vextracti32x4(tmp, src.as<Vec>().ymm(), 1u);
            else
              cc->vextracti128(tmp, src.as<Vec>().ymm(), 1u);
            cc->emit(inst_id, dst, tmp);
          }
          else {
            cc->vpshufd(tmp, src.as<Vec>(), x86::shuffle_imm(3, 2, 3, 2));
            cc->emit(inst_id, dst, tmp);
          }
        }
        else {
          uint32_t w = dst.size() >> 4;
          src.as<Mem>().set_size(w * 8u);
          src.as<Mem>().add_offset(w * 8u);
          cc->emit(inst_id, dst, src);
        }

        return;
      }

      case UniOpVV::kCvtF64ToF32Lo:
      case UniOpVV::kCvtTruncF64ToI32Lo:
      case UniOpVV::kCvtRoundF64ToI32Lo: {
        // Intrinsic - narrowing conversion - low part conversions are native, high part emulated.
        uint32_t dst_size = Support::max(dst.size() / 2u, src.x86_rm_size());
        uint32_t w = dst_size >> 5;

        dst.set_signature(signature_of_xmm_ymm_zmm[w ? w - 1u : 0u]);

        if (src.is_reg())
          src.set_signature(signature_of_xmm_ymm_zmm[w]);
        else if (src.x86_rm_size() == 0)
          src.as<Mem>().set_size(w * 32u);

        cc->emit(inst_id, dst, src);
        return;
      }

      case UniOpVV::kCvtF64ToF32Hi:
      case UniOpVV::kCvtTruncF64ToI32Hi:
      case UniOpVV::kCvtRoundF64ToI32Hi: {
        uint32_t w = dst.size() >> 6;
        Vec tmp = new_vec_with_width(VecWidth(w), "@tmp");

        if (src.is_mem())
          src.as<Mem>().set_size(dst.size());

        cc->emit(inst_id, tmp, src);

        if (dst.is_vec512())
          cc->vinserti32x8(dst, dst, tmp.ymm(), 1);
        else if (dst.is_vec256())
          cc->vinserti128(dst, dst, tmp.xmm(), 1);
        else
          cc->vunpcklpd(dst, dst, tmp);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    InstId inst_id = op_info.sse_inst_id;

    if (has_sse_ext(SSEExt(op_info.sse_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      if (op_info.use_imm)
        cc->emit(inst_id, dst, src, Imm(op_info.imm));
      else
        cc->emit(inst_id, dst, src);
      return;
    }

    switch (op) {
      case UniOpVV::kMov: {
        cc->emit(Inst::kIdMovaps, dst, src);
        return;
      }

      case UniOpVV::kMovU64: {
        cc->emit(Inst::kIdMovq, dst, src);
        return;
      }

      case UniOpVV::kBroadcastU8Z:
      case UniOpVV::kBroadcastU16Z:
      case UniOpVV::kBroadcastU8:
      case UniOpVV::kBroadcastU16: {
        // Intrinsic - 8/16-bit broadcasts are generally not available in SSE mode - we have to emulate.
        ASMJIT_ASSERT(src.is_reg() || src.is_mem());
        ElementSize element_size = ElementSize(op_info.element_size);

        if (src.is_mem() || src.is_gp()) {
          Gp tmp = new_gp32("@tmp");
          uint32_t mul_by = element_size == ElementSize::k8 ? 0x01010101u : 0x00010001u;

          if (src.is_mem()) {
            src.as<Mem>().set_size(element_size == ElementSize::k8 ? 1 : 2);
            cc->movzx(tmp, src.as<Mem>());
            cc->imul(tmp, tmp, mul_by);
          }
          else if (op_info.imm == 0x01) {
            // OPTIMIZATION: If it's guaranteed that the unused part of the register is zero, we can imul without zero extending.
            cc->imul(tmp, src.as<Gp>().r32(), mul_by);
          }
          else {
            OperandSignature src_signature = OperandSignature{
              element_size == ElementSize::k8 ? RegTraits<RegType::kGp8Lo>::kSignature : RegTraits<RegType::kGp16>::kSignature};
            src.as<Gp>().set_signature(src_signature);
            cc->movzx(tmp, src.as<Gp>());
            cc->imul(tmp, tmp, mul_by);
          }

          cc->emit(Inst::kIdMovd, dst, tmp);
          cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(0, 0, 0, 0));
          return;
        }

        ASMJIT_ASSERT(src.is_vec());

        if (has_ssse3()) {
          if (element_size == ElementSize::k8 || (element_size == ElementSize::k16 && is_same_vec(dst, src))) {
            Operand predicate = element_size == ElementSize::k8 ? simd_const(&ct().p_0000000000000000, Bcst::kNA, dst.as<Vec>())
                                                                : simd_const(&ct().p_0100010001000100, Bcst::kNA, dst.as<Vec>());
            sse_mov(*this, dst, src);
            cc->emit(Inst::kIdPshufb, dst, predicate);
            return;
          }
        }

        if (element_size == ElementSize::k8) {
          sse_mov(*this, dst, src);
          cc->emit(Inst::kIdPunpcklbw, dst, dst);
          src = dst;
        }

        cc->emit(Inst::kIdPshuflw, dst, src, x86::shuffle_imm(0, 0, 0, 0));
        cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(0, 0, 0, 0));
        return;
      }

      case UniOpVV::kBroadcastU32:
      case UniOpVV::kBroadcastF32: {
        // Intrinsic - 32-bit broadcast is generally not available in SSE mode - we have to emulate.
        ASMJIT_ASSERT(src.is_reg() || src.is_mem());

        if (src.is_gp()) {
          cc->emit(Inst::kIdMovd, dst, src.as<Gp>().r32());
          src = dst;
        }

        if (src.is_reg()) {
          cc->emit(Inst::kIdPshufd, dst, src, x86::shuffle_imm(0, 0, 0, 0));
        }
        else {
          cc->emit(Inst::kIdMovd, dst, src);
          cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(0, 0, 0, 0));
        }

        return;
      }

      case UniOpVV::kBroadcastU64:
      case UniOpVV::kBroadcastF64: {
        // Intrinsic - 64-bit broadcast is generally not available in SSE mode - we have to emulate.
        ASMJIT_ASSERT(src.is_reg() || src.is_mem());

        if (src.is_gp()) {
          cc->emit(Inst::kIdMovq, dst, src.as<Gp>().r64());
          src = dst;
        }

        if (has_sse3()) {
          cc->emit(Inst::kIdMovddup, dst, src);
        }
        else if (src.is_reg()) {
          cc->emit(Inst::kIdPshufd, dst, src, x86::shuffle_imm(1, 0, 1, 0));
        }
        else {
          cc->emit(Inst::kIdMovq, dst, src);
          cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(1, 0, 1, 0));
        }

        return;
      }

      case UniOpVV::kBroadcastV128_U32:
      case UniOpVV::kBroadcastV128_U64:
      case UniOpVV::kBroadcastV128_F32:
      case UniOpVV::kBroadcastV128_F64: {
        // 128-bit broadcast is like 128-bit mov in this case as we don't have wider vectors.
        sse_mov(*this, dst, src);
        return;
      }

      case UniOpVV::kAbsI8: {
        // Native operation requires SSSE3, which is not supported by the target.
        if (is_same_vec(dst, src)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          cc->emit(Inst::kIdPxor, tmp, tmp);
          cc->emit(Inst::kIdPsubb, tmp, dst);
          cc->emit(Inst::kIdPminub, dst, tmp);
        }
        else {
          cc->emit(Inst::kIdPxor, dst, dst);
          cc->emit(Inst::kIdPsubb, dst, src);
          cc->emit(Inst::kIdPminub, dst, src);
        }
        return;
      }

      case UniOpVV::kAbsI16: {
        // Native operation requires SSSE3, which is not supported by the target.
        if (is_same_vec(dst, src)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          cc->emit(Inst::kIdPxor, tmp, tmp);
          cc->emit(Inst::kIdPsubw, tmp, dst);
          cc->emit(Inst::kIdPmaxsw, dst, tmp);
        }
        else {
          cc->emit(Inst::kIdPxor, dst, dst);
          cc->emit(Inst::kIdPsubw, dst, src);
          cc->emit(Inst::kIdPmaxsw, dst, src);
        }
        return;
      }

      case UniOpVV::kAbsI32: {
        // Native operation requires SSSE3, which is not supported by the target.
        Vec tmp = new_similar_reg(dst, "@tmp");
        cc->emit(Inst::kIdMovaps, tmp, src);
        cc->emit(Inst::kIdPsrad, tmp, 31);
        sse_mov(*this, dst, src);
        cc->emit(Inst::kIdPxor, dst, tmp);
        cc->emit(Inst::kIdPsubd, dst, tmp);
        return;
      }

      case UniOpVV::kAbsI64: {
        // Native operation requires AVX512, which is not supported by the target.
        Vec tmp = new_similar_reg(dst, "@tmp");
        cc->emit(Inst::kIdPshufd, tmp, src, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdPsrad, tmp, 31);
        sse_mov(*this, dst, src);
        cc->emit(Inst::kIdPxor, dst, tmp);
        cc->emit(Inst::kIdPsubq, dst, tmp);
        return;
      }

      case UniOpVV::kNotU32:
      case UniOpVV::kNotU64:
      case UniOpVV::kNotF32:
      case UniOpVV::kNotF64: {
        sse_bit_not(*this, dst, src);
        return;
      }

      case UniOpVV::kCvtI8ToI32:
      case UniOpVV::kCvtU8ToU32: {
        if (src.is_mem())
          src.as<Mem>().set_size(4u);

        if (has_sse4_1()) {
          cc->emit(inst_id, dst, src);
          return;
        }

        if (src.is_mem()) {
          cc->movd(dst, src.as<x86::Mem>());
          src = dst;
        }

        WideningOp cvt = (op == UniOpVV::kCvtI8ToI32) ? WideningOp::kI8ToI32 : WideningOp::kU8ToU32;
        sse_int_widen(*this, dst, src.as<Vec>(), cvt);
        return;
      }

      case UniOpVV::kCvtU8HiToU16:
      case UniOpVV::kCvtU16HiToU32:
      case UniOpVV::kCvtU32HiToU64:
        if (src.is_vec() && dst.id() != src.id() && has_sse4_1()) {
          cc->pshufd(dst, src.as<Vec>(), x86::shuffle_imm(3, 2, 3, 2));
          cc->emit(inst_id, dst, dst);
          return;
        }
        [[fallthrough]];

      case UniOpVV::kCvtI8HiToI16:
      case UniOpVV::kCvtI16HiToI32:
      case UniOpVV::kCvtI32HiToI64:
        if (src.is_vec()) {
          sse_mov(*this, dst, src);

          switch (op) {
            case UniOpVV::kCvtI8HiToI16: {
              cc->punpckhbw(dst, dst);
              cc->psraw(dst, 8);
              break;
            }

            case UniOpVV::kCvtU8HiToU16: {
              cc->emit(Inst::kIdPunpckhbw, dst, simd_const(&ct().p_0000000000000000, Bcst::kNA, dst));
              break;
            }

            case UniOpVV::kCvtI16HiToI32: {
              cc->punpckhwd(dst, dst);
              cc->psrad(dst, 16);
              break;
            }

            case UniOpVV::kCvtU16HiToU32: {
              cc->emit(Inst::kIdPunpckhwd, dst, simd_const(&ct().p_0000000000000000, Bcst::kNA, dst));
              break;
            }

            case UniOpVV::kCvtI32HiToI64: {
              Vec tmp = new_vec128("@tmp");
              sse_mov(*this, tmp, dst);
              cc->psrad(tmp, 31);
              cc->punpckhdq(dst, tmp);
              break;
            }

            case UniOpVV::kCvtU32HiToU64: {
              cc->emit(Inst::kIdPunpckhdq, dst, simd_const(&ct().p_0000000000000000, Bcst::kNA, dst));
              break;
            }

            default:
              ASMJIT_NOT_REACHED();
          }
          return;
        }
        else if (src.is_mem()) {
          src.as<Mem>().add_offset(8u);
          op = UniOpVV(uint32_t(op) - 1);
        }
        else {
          ASMJIT_NOT_REACHED();
        }
        [[fallthrough]];

      case UniOpVV::kCvtI8LoToI16:
      case UniOpVV::kCvtU8LoToU16:
      case UniOpVV::kCvtI16LoToI32:
      case UniOpVV::kCvtU16LoToU32:
      case UniOpVV::kCvtI32LoToI64:
      case UniOpVV::kCvtU32LoToU64: {
        if (src.is_mem())
          src.as<Mem>().set_size(8u);

        if (has_sse4_1()) {
          cc->emit(inst_id, dst, src);
          return;
        }

        if (src.is_mem()) {
          cc->movq(dst, src.as<x86::Mem>());
          src = dst;
        }

        WideningOp cvt {};
        switch (op) {
          case UniOpVV::kCvtI8LoToI16 : cvt = WideningOp::kI8ToI16; break;
          case UniOpVV::kCvtU8LoToU16 : cvt = WideningOp::kU8ToU16; break;
          case UniOpVV::kCvtI16LoToI32: cvt = WideningOp::kI16ToI32; break;
          case UniOpVV::kCvtU16LoToU32: cvt = WideningOp::kU16ToU32; break;
          case UniOpVV::kCvtI32LoToI64: cvt = WideningOp::kI32ToI64; break;
          case UniOpVV::kCvtU32LoToU64: cvt = WideningOp::kU32ToU64; break;
          default:
            ASMJIT_NOT_REACHED();
        }

        sse_int_widen(*this, dst, src.as<Vec>(), cvt);
        return;
      }

      case UniOpVV::kTruncF32:
      case UniOpVV::kTruncF64:
      case UniOpVV::kFloorF32:
      case UniOpVV::kFloorF64:
      case UniOpVV::kCeilF32:
      case UniOpVV::kCeilF64:
      case UniOpVV::kRoundEvenF32:
      case UniOpVV::kRoundEvenF64:
        // Native operation requires SSE4.1.
        if (has_sse4_1()) {
          cc->emit(inst_id, dst, src, Imm(op_info.imm));
          return;
        }
        [[fallthrough]];

      case UniOpVV::kTruncF32S:
      case UniOpVV::kTruncF64S:
      case UniOpVV::kFloorF32S:
      case UniOpVV::kFloorF64S:
      case UniOpVV::kCeilF32S:
      case UniOpVV::kCeilF64S:
      case UniOpVV::kRoundEvenF32S:
      case UniOpVV::kRoundEvenF64S: {
        // Native operation requires SSE4.1.
        if (has_sse4_1()) {
          sse_fmov(*this, dst, src, FloatMode(op_info.float_mode));
          cc->emit(inst_id, dst, dst, Imm(op_info.imm));
          return;
        }

        sse_round(*this, dst, src, FloatMode(op_info.float_mode), x86::RoundImm(op_info.imm & 0x7));
        return;
      }

      case UniOpVV::kRoundHalfAwayF32S:
      case UniOpVV::kRoundHalfAwayF64S:
      case UniOpVV::kRoundHalfAwayF32:
      case UniOpVV::kRoundHalfAwayF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);
        const FloatInst& fi = sse_float_inst[fm];

        Operand half = get_fop_half_minus_1ulp(*this, dst, fm);
        Operand msb = get_fop_msb_bit(*this, dst, fm);

        Vec tmp = new_similar_reg(dst, "@tmp");

        sse_fmov(*this, dst, src, fm);
        sse_mov(*this, tmp, msb);
        cc->emit(fi.fand, tmp, dst);
        cc->emit(fi.for_, tmp, half);
        cc->emit(fi.fadd, dst, tmp);

        sse_round(*this, dst, dst, fm, x86::RoundImm(op_info.imm & 0x7));
        return;
      }

      case UniOpVV::kRoundHalfUpF32S:
      case UniOpVV::kRoundHalfUpF64S:
      case UniOpVV::kRoundHalfUpF32:
      case UniOpVV::kRoundHalfUpF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);
        const FloatInst& fi = sse_float_inst[fm];

        Operand half = get_fop_half_minus_1ulp(*this, dst, fm);
        sse_fmov(*this, dst, src, fm);
        cc->emit(fi.fadd, dst, half);
        sse_round(*this, dst, dst, fm, x86::RoundImm(op_info.imm & 0x7));
        return;
      }

      case UniOpVV::kAbsF32S:
      case UniOpVV::kAbsF64S:
      case UniOpVV::kAbsF32:
      case UniOpVV::kAbsF64:
      case UniOpVV::kNegF32S:
      case UniOpVV::kNegF64S:
      case UniOpVV::kNegF32:
      case UniOpVV::kNegF64: {
        // Intrinsic.
        FloatMode fm = FloatMode(op_info.float_mode);

        const void* msk_data =
          op == UniOpVV::kAbsF32 || op == UniOpVV::kAbsF32S ? static_cast<const void*>(&ct().p_7FFFFFFF7FFFFFFF) :
          op == UniOpVV::kAbsF64 || op == UniOpVV::kAbsF64S ? static_cast<const void*>(&ct().p_7FFFFFFFFFFFFFFF) :
          op == UniOpVV::kNegF32 || op == UniOpVV::kNegF32S ? static_cast<const void*>(&ct().p_8000000080000000) :
                                                              static_cast<const void*>(&ct().p_8000000000000000);
        Operand msk = simd_const(msk_data, Bcst(op_info.broadcast_size), dst);

        sse_fmov(*this, dst, src, fm);
        cc->emit(inst_id, dst, msk);
        return;
      }

      case UniOpVV::kRcpF32: {
        Operand one = simd_const(&ct().f32_1, Bcst::k32, dst);
        if (is_same_vec(dst, src)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, one);
          cc->emit(Inst::kIdDivps, tmp, src);
          sse_mov(*this, dst, tmp);
        }
        else {
          sse_mov(*this, dst, one);
          cc->emit(Inst::kIdDivps, dst, src);
        }
        return;
      }

      case UniOpVV::kRcpF64: {
        Operand one = simd_const(&ct().f64_1, Bcst::k64, dst);
        if (is_same_vec(dst, src)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, one);
          cc->emit(Inst::kIdDivpd, tmp, src);
          sse_mov(*this, dst, tmp);
        }
        else {
          sse_mov(*this, dst, one);
          cc->emit(Inst::kIdDivpd, dst, src);
        }
        return;
      }

      case UniOpVV::kSqrtF32S:
      case UniOpVV::kSqrtF64S: {
        sse_mov(*this, dst, src);
        cc->emit(inst_id, dst, dst);
        return;
      }

      case UniOpVV::kCvtF32ToF64S:
      case UniOpVV::kCvtF64ToF32S: {
        if (is_same_vec(dst, src)) {
          cc->emit(inst_id, dst, src);
        }
        else {
          cc->emit(Inst::kIdXorps, dst, dst);
          cc->emit(inst_id, dst, src);
        }
        return;
      }

      case UniOpVV::kCvtF32HiToF64:
      case UniOpVV::kCvtI32HiToF64: {
        if (src.is_mem()) {
          Mem mem(src.as<Mem>());
          mem.add_offset(8);
          cc->emit(inst_id, dst, mem);
        }
        else {
          if (is_same_vec(dst, src))
            cc->emit(Inst::kIdMovhlps, dst, src);
          else
            cc->emit(Inst::kIdPshufd, dst, src, x86::shuffle_imm(3, 2, 3, 2));
          cc->emit(inst_id, dst, dst);
        }
        return;
      }

      case UniOpVV::kCvtF64ToF32Hi:
      case UniOpVV::kCvtTruncF64ToI32Hi:
      case UniOpVV::kCvtRoundF64ToI32Hi: {
        Vec tmp = new_vec128("@tmp");

        if (src.is_mem())
          src.as<Mem>().set_size(dst.size());

        cc->emit(inst_id, tmp, src);
        cc->emit(Inst::kIdUnpcklpd, dst, tmp);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_2v(UniOpVV op, const OpArray& dst_, const Operand_& src_) { emit_2v_t(*this, op, dst_, src_); }
void UniCompiler::emit_2v(UniOpVV op, const OpArray& dst_, const OpArray& src_) { emit_2v_t(*this, op, dst_, src_); }

// ujit::UniCompiler - Vector Instructions - Emit 2VI
// ==================================================

void UniCompiler::emit_2vi(UniOpVVI op, const Operand_& dst_, const Operand_& src_, uint32_t imm) {
  ASMJIT_ASSERT(dst_.is_vec());

  Vec dst(dst_.as<Vec>());
  Operand src(src_);
  UniOpVInfo op_info = opcode_info_2vi[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    InstId inst_id = op_info.avx_inst_id;

    if (has_avx_ext(AVXExt(op_info.avx_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      cc->emit(inst_id, dst, src, Imm(imm));
      return;
    }

    switch (op) {
      case UniOpVVI::kSllU16:
      case UniOpVVI::kSllU32:
      case UniOpVVI::kSllU64:
      case UniOpVVI::kSrlU16:
      case UniOpVVI::kSrlU32:
      case UniOpVVI::kSrlU64:
      case UniOpVVI::kSraI16:
      case UniOpVVI::kSraI32:
      case UniOpVVI::kSllbU128:
      case UniOpVVI::kSrlbU128: {
        // This instruction requires AVX-512 if the source is a memory operand.
        if (src.is_mem()) {
          avx_mov(*this, dst, src);
          cc->emit(inst_id, dst, dst, imm);
        }
        else {
          cc->emit(inst_id, dst, src, imm);
        }
        return;
      }

      case UniOpVVI::kSraI64: {
        // Native operation requires AVX-512, which is not supported by the target.
        if (imm == 0) {
          avx_mov(*this, dst, src);
          return;
        }

        if (imm == 63) {
          cc->emit(Inst::kIdVpshufd, dst, src, x86::shuffle_imm(3, 3, 1, 1));
          cc->emit(Inst::kIdVpsrad, dst, dst, 31);
          return;
        }

        Vec tmp = new_similar_reg(dst, "@tmp");

        if (src.is_mem()) {
          avx_mov(*this, dst, src);
          src = dst;
        }

        if (imm <= 32) {
          cc->emit(Inst::kIdVpsrad, tmp, src, Support::min<uint32_t>(imm, 31u));
          cc->emit(Inst::kIdVpsrlq, dst, src, imm);
          cc->emit(Inst::kIdVpblendw, dst, dst, tmp, 0xCC);
          return;
        }

        cc->emit(Inst::kIdVpshufd, tmp, src, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdVpsrad, tmp, tmp, 31);
        cc->emit(Inst::kIdVpsrlq, dst, src, imm);
        cc->emit(Inst::kIdVpsllq, tmp, tmp, 64u - imm);
        cc->emit(Inst::kIdVpor, dst, dst, tmp);
        return;
      }

      case UniOpVVI::kSwizzleU16x4: {
        // Intrinsic.

        // TODO: [JIT] OPTIMIZATION: Use VPSHUFB instead where appropriate.
        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        cc->emit(Inst::kIdVpshuflw, dst, src, shuf_imm);
        cc->emit(Inst::kIdVpshufhw, dst, dst, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleLoU16x4:
      case UniOpVVI::kSwizzleHiU16x4:
      case UniOpVVI::kSwizzleU32x4: {
        // Intrinsic (AVX | AVX512).
        ASMJIT_ASSERT(inst_id != Inst::kIdNone);

        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        cc->emit(inst_id, dst, src, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleU64x2: {
        // Intrinsic (AVX | AVX512).
        if (Swizzle2{imm} == swizzle(0, 0)) {
          cc->emit(Inst::kIdVmovddup, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && src.is_reg()) {
          cc->emit(Inst::kIdVpunpcklqdq, dst, src, src);
        }
        else if (Swizzle2{imm} == swizzle(1, 1) && src.is_reg()) {
          cc->emit(Inst::kIdVpunpckhqdq, dst, src, src);
        }
        else {
          uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle2{imm});
          cc->emit(Inst::kIdVpshufd, dst, src, shuf_imm);
        }
        return;
      }

      case UniOpVVI::kSwizzleF32x4: {
        // Intrinsic (AVX | AVX512).
        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        if (src.is_reg())
          cc->emit(Inst::kIdVshufps, dst, src, src, shuf_imm);
        else
          cc->emit(Inst::kIdVpshufd, dst, src, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleF64x2: {
        // Intrinsic (AVX | AVX512).
        if (Swizzle2{imm} == swizzle(0, 0) && !dst.is_vec512()) {
          cc->emit(Inst::kIdVmovddup, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && src.is_reg()) {
          cc->emit(Inst::kIdVunpcklpd, dst, src, src);
        }
        else if (Swizzle2{imm} == swizzle(1, 1) && src.is_reg()) {
          cc->emit(Inst::kIdVunpckhpd, dst, src, src);
        }
        else if (src.is_reg()) {
          uint32_t shuf_imm = shuf_imm2_from_swizzle_with_width(Swizzle2{imm}, VecWidthUtils::vec_width_of(dst));
          cc->emit(Inst::kIdVshufpd, dst, src, src, shuf_imm);
        }
        else {
          uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle2{imm});
          cc->emit(Inst::kIdVpshufd, dst, src, shuf_imm);
        }
        return;
      }

      case UniOpVVI::kSwizzleF64x4:
      case UniOpVVI::kSwizzleU64x4: {
        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        cc->emit(op_info.avx_inst_id, dst, src, shuf_imm);
        return;
      }

      case UniOpVVI::kExtractV128_I32:
      case UniOpVVI::kExtractV128_I64:
      case UniOpVVI::kExtractV128_F32:
      case UniOpVVI::kExtractV128_F64: {
        // Intrinsic (AVX | AVX512).
        ASMJIT_ASSERT(imm < 4);
        dst.set_signature(signature_of_xmm_ymm_zmm[0]);

        if (src.is_mem()) {
          src.as<Mem>().add_offset(imm * 16u);
          v_loadu128(dst, src.as<x86::Mem>());
          return;
        }

        if (src.as<Vec>().is_vec512()) {
          ASMJIT_ASSERT(imm < 4);
          cc->vextracti32x4(dst, src.as<Vec>(), imm);
        }
        else if (src.as<Vec>().is_vec256()) {
          ASMJIT_ASSERT(imm < 2);
          cc->vextractf128(dst, src.as<Vec>(), imm);
        }
        else {
          ASMJIT_NOT_REACHED();
        }

        return;
      }

      case UniOpVVI::kExtractV256_I32:
      case UniOpVVI::kExtractV256_I64:
      case UniOpVVI::kExtractV256_F32:
      case UniOpVVI::kExtractV256_F64: {
        // Intrinsic (AVX | AVX512).
        ASMJIT_ASSERT(imm < 2);
        dst.set_signature(signature_of_xmm_ymm_zmm[1]);

        if (src.is_mem()) {
          src.as<Mem>().add_offset(imm * 32u);
          v_loadu256(dst, src.as<Mem>());
          return;
        }

        ASMJIT_ASSERT(src.as<Vec>().is_vec512());
        cc->vextracti32x8(dst, src.as<Vec>(), imm);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    InstId inst_id = op_info.sse_inst_id;

    if (has_sse_ext(SSEExt(op_info.sse_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      if (op_info.sse_op_count == 2) {
        sse_mov(*this, dst, src);
        cc->emit(inst_id, dst, imm);
        return;
      }
      else if (op_info.sse_op_count == 3) {
        cc->emit(inst_id, dst, src, imm);
        return;
      }

      ASMJIT_NOT_REACHED();
    }

    switch (op) {
      case UniOpVVI::kSraI64: {
        // Intrinsic (SSE2).
        if (imm == 0) {
          sse_mov(*this, dst, src);
          return;
        }

        if (imm == 63) {
          cc->emit(Inst::kIdPshufd, dst, src, x86::shuffle_imm(3, 3, 1, 1));
          cc->emit(Inst::kIdPsrad, dst, 31);
          return;
        }

        Vec tmp = new_similar_reg(dst, "@tmp");

        if (imm <= 32 && has_sse4_1()) {
          sse_mov(*this, dst, src);
          sse_mov(*this, tmp, src.is_reg() ? src.as<Vec>() : dst);
          cc->emit(Inst::kIdPsrad, tmp, Support::min<uint32_t>(imm, 31u));
          cc->emit(Inst::kIdPsrlq, dst, imm);
          cc->emit(Inst::kIdPblendw, dst, tmp, 0xCC);
          return;
        }

        sse_mov(*this, dst, src);
        cc->emit(Inst::kIdPshufd, tmp, src.is_reg() ? src.as<Vec>() : dst, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdPsrad, tmp, 31);
        cc->emit(Inst::kIdPsrlq, dst, imm);
        cc->emit(Inst::kIdPsllq, tmp, 64u - imm);
        cc->emit(Inst::kIdPor, dst, tmp);
        return;
      }

      case UniOpVVI::kSwizzleU16x4: {
        // Intrinsic (SSE2).

        // TODO: [JIT] OPTIMIZATION: Use VPSHUFB instead where appropriate.
        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        cc->emit(Inst::kIdPshuflw, dst, src, shuf_imm);
        cc->emit(Inst::kIdPshufhw, dst, dst, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleLoU16x4:
      case UniOpVVI::kSwizzleHiU16x4:
      case UniOpVVI::kSwizzleU32x4: {
        // Intrinsic (SSE2).
        ASMJIT_ASSERT(inst_id != Inst::kIdNone);

        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        cc->emit(inst_id, dst, src, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleU64x2: {
        // Intrinsic (SSE2 | SSE3).
        if (Swizzle2{imm} == swizzle(1, 0)) {
          sse_mov(*this, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && has_sse3()) {
          cc->emit(Inst::kIdMovddup, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && is_same_vec(dst, src)) {
          cc->emit(Inst::kIdPunpcklqdq, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(1, 1) && is_same_vec(dst, src)) {
          cc->emit(Inst::kIdPunpckhqdq, dst, src);
        }
        else {
          uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle2{imm});
          cc->emit(Inst::kIdPshufd, dst, src, shuf_imm);
        }
        return;
      }

      case UniOpVVI::kSwizzleF32x4: {
        // Intrinsic (SSE2).
        uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        if (is_same_vec(dst, src))
          cc->emit(Inst::kIdShufps, dst, dst, shuf_imm);
        else
          cc->emit(Inst::kIdPshufd, dst, src, shuf_imm);
        return;
      }

      case UniOpVVI::kSwizzleF64x2: {
        // Intrinsic (SSE2 | SSE3).
        if (Swizzle2{imm} == swizzle(1, 0)) {
          sse_mov(*this, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && has_sse3()) {
          cc->emit(Inst::kIdMovddup, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(0, 0) && is_same_vec(dst, src)) {
          cc->emit(Inst::kIdUnpcklpd, dst, src);
        }
        else if (Swizzle2{imm} == swizzle(1, 1) && is_same_vec(dst, src)) {
          cc->emit(Inst::kIdUnpckhpd, dst, src);
        }
        else if (is_same_vec(dst, src)) {
          uint32_t shuf_imm = shuf_imm2_from_swizzle(Swizzle2{imm});
          cc->emit(Inst::kIdShufpd, dst, dst, shuf_imm);
        }
        else {
          uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle2{imm});
          cc->emit(Inst::kIdPshufd, dst, src, shuf_imm);
        }
        return;
      }

      case UniOpVVI::kSwizzleF64x4:
      case UniOpVVI::kSwizzleU64x4:
      case UniOpVVI::kExtractV128_I32:
      case UniOpVVI::kExtractV128_I64:
      case UniOpVVI::kExtractV128_F32:
      case UniOpVVI::kExtractV128_F64:
      case UniOpVVI::kExtractV256_I32:
      case UniOpVVI::kExtractV256_I64:
      case UniOpVVI::kExtractV256_F32:
      case UniOpVVI::kExtractV256_F64:
        // Not supported in SSE mode.
        ASMJIT_NOT_REACHED();

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_2vi(UniOpVVI op, const OpArray& dst_, const Operand_& src_, uint32_t imm) { emit_2vi_t(*this, op, dst_, src_, imm); }
void UniCompiler::emit_2vi(UniOpVVI op, const OpArray& dst_, const OpArray& src_, uint32_t imm) { emit_2vi_t(*this, op, dst_, src_, imm); }

// ujit::UniCompiler - Vector Instructions - Emit 2VS
// ==================================================

void UniCompiler::emit_2vs(UniOpVR op, const Operand_& dst_, const Operand_& src_, uint32_t idx) {
  UniOpVInfo op_info = opcode_info_2vs[size_t(op)];

  Operand src(src_);
  Operand dst(dst_);

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    switch (op) {
      case UniOpVR::kMov: {
        ASMJIT_ASSERT(dst.is_reg());
        ASMJIT_ASSERT(src.is_reg());

        if (dst.is_gp() && src.is_vec()) {
          if (dst.as<Reg>().size() <= 4)
            cc->emit(Inst::kIdVmovd, dst.as<Gp>().r32(), src.as<Vec>().xmm());
          else
            cc->emit(Inst::kIdVmovq, dst.as<Gp>().r64(), src.as<Vec>().xmm());
          return;
        }

        if (dst.is_vec() && src.is_gp()) {
          if (src.as<Reg>().size() <= 4)
            cc->emit(Inst::kIdVmovd, dst.as<Vec>().xmm(), src.as<Gp>().r32());
          else
            cc->emit(Inst::kIdVmovq, dst.as<Vec>().xmm(), src.as<Gp>().r64());
          return;
        }

        ASMJIT_NOT_REACHED();
      }

      case UniOpVR::kMovU32:
      case UniOpVR::kMovU64: {
        ASMJIT_ASSERT(dst.is_reg());
        ASMJIT_ASSERT(src.is_reg());

        if (dst.is_gp() && src.is_vec()) {
          if (op == UniOpVR::kMovU32)
            cc->emit(Inst::kIdVmovd, dst.as<Gp>().r32(), src.as<Vec>().xmm());
          else
            cc->emit(Inst::kIdVmovq, dst.as<Gp>().r64(), src.as<Vec>().xmm());
          return;
        }

        if (dst.is_vec() && src.is_gp()) {
          if (op == UniOpVR::kMovU32)
            cc->emit(Inst::kIdVmovd, dst.as<Vec>().xmm(), src.as<Gp>().r32());
          else
            cc->emit(Inst::kIdVmovq, dst.as<Vec>().xmm(), src.as<Gp>().r64());
          return;
        }

        ASMJIT_NOT_REACHED();
      }

      case UniOpVR::kInsertU8:
      case UniOpVR::kInsertU16:
      case UniOpVR::kInsertU32:
      case UniOpVR::kInsertU64: {
        ASMJIT_ASSERT(dst.is_vec());
        ASMJIT_ASSERT(src.is_gp());

        dst = dst.as<Vec>().xmm();

        if (op != UniOpVR::kInsertU64)
          src = src.as<Gp>().r32();

        cc->emit(op_info.avx_inst_id, dst, dst, src, idx);
        return;
      }

      case UniOpVR::kExtractU8:
      case UniOpVR::kExtractU16:
      case UniOpVR::kExtractU32:
      case UniOpVR::kExtractU64: {
        ASMJIT_ASSERT(dst.is_gp());
        ASMJIT_ASSERT(src.is_vec());

        src = src.as<Vec>().xmm();

        if (op != UniOpVR::kExtractU64)
          dst = dst.as<Gp>().r32();

        if (op == UniOpVR::kExtractU32 && idx == 0) {
          cc->vmovd(dst.as<Gp>(), src.as<Vec>());
          return;
        }

        if (op == UniOpVR::kExtractU64) {
          cc->vmovq(dst.as<Gp>(), src.as<Vec>());
          return;
        }

        cc->emit(op_info.avx_inst_id, dst, src, idx);
        return;
      }

      case UniOpVR::kCvtIntToF32:
      case UniOpVR::kCvtIntToF64: {
        dst = dst.as<Vec>().xmm();
        cc->emit(Inst::kIdVpxor, dst, dst, dst);
        cc->emit(op_info.avx_inst_id, dst, dst, src);
        return;
      }

      case UniOpVR::kCvtTruncF32ToInt:
      case UniOpVR::kCvtRoundF32ToInt:
      case UniOpVR::kCvtTruncF64ToInt:
      case UniOpVR::kCvtRoundF64ToInt: {
        if (src.is_vec())
          src = src.as<Vec>().xmm();

        cc->emit(op_info.avx_inst_id, dst, src);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    switch (op) {
      case UniOpVR::kMov: {
        ASMJIT_ASSERT(dst.is_reg());
        ASMJIT_ASSERT(src.is_reg());

        if (dst.is_gp() && src.is_vec()) {
          if (dst.as<Reg>().size() <= 4)
            cc->emit(Inst::kIdMovd, dst.as<Gp>().r32(), src.as<Vec>().xmm());
          else
            cc->emit(Inst::kIdMovq, dst.as<Gp>().r64(), src.as<Vec>().xmm());
          return;
        }

        if (dst.is_vec() && src.is_gp()) {
          if (src.as<Reg>().size() <= 4)
            cc->emit(Inst::kIdMovd, dst.as<Vec>().xmm(), src.as<Gp>().r32());
          else
            cc->emit(Inst::kIdMovq, dst.as<Vec>().xmm(), src.as<Gp>().r64());
          return;
        }

        ASMJIT_NOT_REACHED();
      }

      case UniOpVR::kMovU32:
      case UniOpVR::kMovU64: {
        ASMJIT_ASSERT(dst.is_reg());
        ASMJIT_ASSERT(src.is_reg());

        if (dst.is_gp() && src.is_vec()) {
          if (op == UniOpVR::kMovU32)
            cc->emit(Inst::kIdMovd, dst.as<Gp>().r32(), src.as<Vec>().xmm());
          else
            cc->emit(Inst::kIdMovq, dst.as<Gp>().r64(), src.as<Vec>().xmm());
          return;
        }

        if (dst.is_vec() && src.is_gp()) {
          if (op == UniOpVR::kMovU32)
            cc->emit(Inst::kIdMovd, dst.as<Vec>().xmm(), src.as<Gp>().r32());
          else
            cc->emit(Inst::kIdMovq, dst.as<Vec>().xmm(), src.as<Gp>().r64());
          return;
        }

        ASMJIT_NOT_REACHED();
      }

      case UniOpVR::kInsertU8:
      case UniOpVR::kInsertU16:
      case UniOpVR::kInsertU32:
      case UniOpVR::kInsertU64: {
        ASMJIT_ASSERT(dst.is_vec());
        ASMJIT_ASSERT(src.is_gp());

        if (op != UniOpVR::kInsertU64)
          src = src.as<Gp>().r32();

        if (has_sse_ext(SSEExt(op_info.sse_ext))) {
          cc->emit(op_info.sse_inst_id, dst, src, idx);
        }
        else if (op == UniOpVR::kInsertU8) {
          Gp tmp = new_gp32("@tmp");
          cc->pextrw(tmp, dst.as<Vec>(), idx / 2u);
          if (idx & 1)
            cc->mov(tmp.r8_hi(), src.as<Gp>().r8());
          else
            cc->mov(tmp.r8(), src.as<Gp>().r8());
          cc->pinsrw(dst.as<Vec>(), tmp, idx / 2u);
        }
        else if (op == UniOpVR::kInsertU32) {
          if (idx == 0) {
            Vec tmp = new_vec128("@tmp");
            cc->movd(tmp, src.as<Gp>());
            cc->movss(dst.as<Vec>(), tmp);
          }
          else {
            Gp tmp = new_gp32("@tmp");
            cc->pinsrw(dst.as<Vec>(), src.as<Gp>(), idx * 2u);
            cc->mov(tmp.as<Gp>(), src.as<Gp>());
            cc->shr(tmp.as<Gp>(), 16);
            cc->pinsrw(dst.as<Vec>(), tmp, idx * 2u + 1u);
          }
        }
        else {
          Vec tmp = new_vec128("@tmp");
          cc->movq(tmp, src.as<Gp>());

          if (idx == 0)
            cc->movsd(dst.as<Vec>(), tmp);
          else
            cc->punpcklqdq(dst.as<Vec>(), tmp);
        }

        return;
      }

      case UniOpVR::kExtractU8:
      case UniOpVR::kExtractU16:
      case UniOpVR::kExtractU32:
      case UniOpVR::kExtractU64: {
        ASMJIT_ASSERT(dst.is_gp());
        ASMJIT_ASSERT(src.is_vec());

        if (op != UniOpVR::kExtractU64)
          dst = dst.as<Gp>().r32();

        if (op == UniOpVR::kExtractU32 && idx == 0) {
          cc->movd(dst.as<Gp>(), src.as<Vec>());
        }
        else if (op == UniOpVR::kExtractU64 && idx == 0) {
          cc->movq(dst.as<Gp>(), src.as<Vec>());
        }
        else if (has_sse_ext(SSEExt(op_info.sse_ext))) {
          cc->emit(op_info.sse_inst_id, dst, src, idx);
        }
        else if (op == UniOpVR::kExtractU8) {
          cc->pextrw(dst.as<Gp>(), src.as<Vec>(), idx / 2u);
          if (idx & 1)
            cc->shr(dst.as<Gp>(), 8);
          else
            cc->and_(dst.as<Gp>(), 0xFF);
        }
        else if (op == UniOpVR::kExtractU32) {
          Vec tmp = new_similar_reg(dst.as<Vec>(), "@tmp");
          cc->pshufd(tmp, src.as<Vec>(), x86::shuffle_imm(idx, idx, idx, idx));
          cc->movd(dst.as<Gp>(), tmp);
        }
        else {
          Vec tmp = new_similar_reg(dst.as<Vec>(), "@tmp");
          cc->pshufd(tmp, src.as<Vec>(), x86::shuffle_imm(3, 2, 3, 2));
          cc->movq(dst.as<Gp>(), tmp);
        }

        return;
      }

      case UniOpVR::kCvtIntToF32:
      case UniOpVR::kCvtIntToF64: {
        dst = dst.as<Vec>().xmm();
        cc->pxor(dst.as<Vec>(), dst.as<Vec>());
        cc->emit(op_info.sse_inst_id, dst, src);
        return;
      }

      case UniOpVR::kCvtTruncF32ToInt:
      case UniOpVR::kCvtRoundF32ToInt:
      case UniOpVR::kCvtTruncF64ToInt:
      case UniOpVR::kCvtRoundF64ToInt: {
        cc->emit(op_info.sse_inst_id, dst, src);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

// ujit::UniCompiler - Vector Instructions - Emit 2VM
// ==================================================

void UniCompiler::emit_vm(UniOpVM op, const Vec& dst_, const Mem& src_, Alignment alignment, uint32_t idx) {
  ASMJIT_ASSERT(dst_.is_vec());
  ASMJIT_ASSERT(src_.is_mem());

  Vec dst(dst_);
  Mem src(src_);
  UniOpVMInfo op_info = opcode_info_2vm[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    switch (op) {
      case UniOpVM::kLoad8: {
        dst = dst.xmm();
        src.set_size(1);
        avx_zero(*this, dst);
        cc->vpinsrb(dst, dst, src, 0);
        return;
      }

      case UniOpVM::kLoad16_U16:
        if (!has_avx512_fp16()) {
          dst = dst.xmm();
          src.set_size(1);
          avx_zero(*this, dst);
          cc->vpinsrw(dst, dst, src, 0);
        }
        [[fallthrough]];

      case UniOpVM::kLoad32_U32:
      case UniOpVM::kLoad32_F32:
      case UniOpVM::kLoad64_U32:
      case UniOpVM::kLoad64_U64:
      case UniOpVM::kLoad64_F32:
      case UniOpVM::kLoad64_F64: {
        dst.set_signature(signature_of_xmm_ymm_zmm[0]);
        src.set_size(op_info.mem_size);
        cc->emit(op_info.avx_inst_id, dst, src);
        return;
      }

      case UniOpVM::kLoad128_U32:
      case UniOpVM::kLoad128_U64:
      case UniOpVM::kLoad128_F32:
      case UniOpVM::kLoad128_F64:
      case UniOpVM::kLoad256_U32:
      case UniOpVM::kLoad256_U64:
      case UniOpVM::kLoad256_F32:
      case UniOpVM::kLoad256_F64:
      case UniOpVM::kLoad512_U32:
      case UniOpVM::kLoad512_U64:
      case UniOpVM::kLoad512_F32:
      case UniOpVM::kLoad512_F64:
        ASMJIT_ASSERT(dst.size() >= op_info.mem_size);
        dst.set_signature(signature_of_xmm_ymm_zmm[op_info.mem_size >> 5]);
        [[fallthrough]];

      case UniOpVM::kLoadN_U32:
      case UniOpVM::kLoadN_U64:
      case UniOpVM::kLoadN_F32:
      case UniOpVM::kLoadN_F64: {
        src.set_size(dst.size());
        cc->emit((uint32_t(alignment) == 0u || uint32_t(alignment) >= dst.size()) ? Inst::kIdVmovaps : Inst::kIdVmovups, dst, src);
        return;
      }

      case UniOpVM::kLoadCvt16_U8ToU64:
      case UniOpVM::kLoadCvt32_U8ToU64:
      case UniOpVM::kLoadCvt64_U8ToU64:
        dst.set_signature(signature_of_xmm_ymm_zmm[op_info.mem_size >> 2]);
        [[fallthrough]];

      case UniOpVM::kLoadCvtN_U8ToU64: {
        ASMJIT_ASSERT(dst.size() >= op_info.mem_size * 8u);
        src.set_size(dst.size() / 8u);
        cc->emit(op_info.avx_inst_id, dst, src);
        return;
      }

      case UniOpVM::kLoadCvt32_I8ToI32:
      case UniOpVM::kLoadCvt32_U8ToU32:
      case UniOpVM::kLoadCvt64_I8ToI32:
      case UniOpVM::kLoadCvt64_U8ToU32:
      case UniOpVM::kLoadCvt128_I8ToI32:
      case UniOpVM::kLoadCvt128_U8ToU32:
        dst.set_signature(signature_of_xmm_ymm_zmm[op_info.mem_size >> 3]);
        [[fallthrough]];

      case UniOpVM::kLoadCvtN_I8ToI32:
      case UniOpVM::kLoadCvtN_U8ToU32: {
        ASMJIT_ASSERT(dst.size() >= op_info.mem_size * 4u);
        src.set_size(dst.size() / 4u);
        cc->emit(op_info.avx_inst_id, dst, src);
        return;
      }

      case UniOpVM::kLoadCvt32_I8ToI16:
      case UniOpVM::kLoadCvt32_U8ToU16:
      case UniOpVM::kLoadCvt32_I16ToI32:
      case UniOpVM::kLoadCvt32_U16ToU32:
      case UniOpVM::kLoadCvt32_I32ToI64:
      case UniOpVM::kLoadCvt32_U32ToU64: {
        dst.set_signature(signature_of_xmm_ymm_zmm[0]);
        src.set_size(4);
        cc->vmovd(dst, src);
        cc->emit(op_info.avx_inst_id, dst, dst);
        return;
      }

      case UniOpVM::kLoadCvt64_I8ToI16:
      case UniOpVM::kLoadCvt64_U8ToU16:
      case UniOpVM::kLoadCvt64_I16ToI32:
      case UniOpVM::kLoadCvt64_U16ToU32:
      case UniOpVM::kLoadCvt64_I32ToI64:
      case UniOpVM::kLoadCvt64_U32ToU64:
      case UniOpVM::kLoadCvt128_I8ToI16:
      case UniOpVM::kLoadCvt128_U8ToU16:
      case UniOpVM::kLoadCvt128_I16ToI32:
      case UniOpVM::kLoadCvt128_U16ToU32:
      case UniOpVM::kLoadCvt128_I32ToI64:
      case UniOpVM::kLoadCvt128_U32ToU64:
      case UniOpVM::kLoadCvt256_I8ToI16:
      case UniOpVM::kLoadCvt256_U8ToU16:
      case UniOpVM::kLoadCvt256_I16ToI32:
      case UniOpVM::kLoadCvt256_U16ToU32:
      case UniOpVM::kLoadCvt256_I32ToI64:
      case UniOpVM::kLoadCvt256_U32ToU64:
        ASMJIT_ASSERT(dst.size() >= op_info.mem_size * 2u);
        dst.set_signature(signature_of_xmm_ymm_zmm[op_info.mem_size >> 4]);
        [[fallthrough]];

      case UniOpVM::kLoadCvtN_I8ToI16:
      case UniOpVM::kLoadCvtN_U8ToU16:
      case UniOpVM::kLoadCvtN_I16ToI32:
      case UniOpVM::kLoadCvtN_U16ToU32:
      case UniOpVM::kLoadCvtN_I32ToI64:
      case UniOpVM::kLoadCvtN_U32ToU64: {
        src.set_size(dst.size() / 2u);
        cc->emit(op_info.avx_inst_id, dst, src);
        return;
      }

      case UniOpVM::kLoadInsertU8:
      case UniOpVM::kLoadInsertU16:
      case UniOpVM::kLoadInsertU32:
      case UniOpVM::kLoadInsertF32: {
        dst = dst.as<Vec>().xmm();
        cc->emit(op_info.avx_inst_id, dst, dst, src, idx);
        return;
      }

      case UniOpVM::kLoadInsertU64: {
        dst = dst.as<Vec>().xmm();
        if (is_64bit()) {
          cc->emit(op_info.avx_inst_id, dst, dst, src, idx);
        }
        else {
          if (idx == 0)
            cc->vmovlpd(dst, dst, src);
          else
            cc->vmovhpd(dst, dst, src);
        }
        return;
      }

      case UniOpVM::kLoadInsertF32x2: {
        if (idx == 0)
          cc->emit(Inst::kIdVmovlps, dst, dst, src);
        else
          cc->emit(Inst::kIdVmovhps, dst, dst, src);
        return;
      }

      case UniOpVM::kLoadInsertF64: {
        if (idx == 0)
          cc->emit(Inst::kIdVmovlpd, dst, dst, src);
        else
          cc->emit(Inst::kIdVmovhpd, dst, dst, src);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    ASMJIT_ASSERT(dst.is_vec128());

    switch (op) {
      case UniOpVM::kLoad8: {
        src.set_size(1);

        if (has_sse4_1()) {
          cc->xorps(dst, dst);
          cc->pinsrb(dst, src, 0);
        }
        else {
          Gp tmp = new_gp32("@tmp");
          cc->movzx(tmp, src);
          cc->movd(dst, tmp);
        }
        return;
      }

      case UniOpVM::kLoad16_U16: {
        src.set_size(2);
        cc->xorps(dst, dst);
        cc->pinsrw(dst, src, 0);
        return;
      }

      case UniOpVM::kLoad32_U32:
      case UniOpVM::kLoad32_F32:
      case UniOpVM::kLoad64_U32:
      case UniOpVM::kLoad64_U64:
      case UniOpVM::kLoad64_F32:
      case UniOpVM::kLoad64_F64: {
        src.set_size(op_info.mem_size);
        cc->emit(op_info.sse_inst_id, dst, src);
        return;
      }

      case UniOpVM::kLoad128_U32:
      case UniOpVM::kLoad128_U64:
      case UniOpVM::kLoad128_F32:
      case UniOpVM::kLoad128_F64:
      case UniOpVM::kLoadN_U32:
      case UniOpVM::kLoadN_U64:
      case UniOpVM::kLoadN_F32:
      case UniOpVM::kLoadN_F64: {
        src.set_size(16);
        cc->emit((uint32_t(alignment) == 0u || uint32_t(alignment) >= 16u) ? Inst::kIdMovaps : Inst::kIdMovups, dst, src);
        return;
      }

      case UniOpVM::kLoadCvt16_U8ToU64:
      case UniOpVM::kLoadCvtN_U8ToU64: {
        if (has_sse4_1()) {
          src.set_size(2);
          cc->emit(op_info.avx_inst_id, dst, src);
        }
        else {
          src.set_size(1);
          Gp tmp = new_gp32("@tmp");
          cc->movzx(tmp, src);
          cc->movd(dst, tmp);

          src.add_offset(1);
          cc->movzx(tmp, src);
          cc->pinsrw(dst, src, 4);
        }
        return;
      }

      case UniOpVM::kLoadCvt32_I8ToI32:
      case UniOpVM::kLoadCvt32_U8ToU32:
      case UniOpVM::kLoadCvtN_I8ToI32:
      case UniOpVM::kLoadCvtN_U8ToU32:
        if (has_sse4_1()) {
          src.set_size(4);
          cc->emit(op_info.sse_inst_id, dst, src);
          return;
        }
        [[fallthrough]];

      case UniOpVM::kLoadCvt32_I8ToI16:
      case UniOpVM::kLoadCvt32_U8ToU16:
      case UniOpVM::kLoadCvt32_I16ToI32:
      case UniOpVM::kLoadCvt32_U16ToU32:
      case UniOpVM::kLoadCvt32_I32ToI64:
      case UniOpVM::kLoadCvt32_U32ToU64: {
        src.set_size(4);
        cc->vmovd(dst, src);
        sse_int_widen(*this, dst, dst, WideningOp(op_info.cvt));
        return;
      }

      case UniOpVM::kLoadCvt64_I8ToI16:
      case UniOpVM::kLoadCvt64_U8ToU16:
      case UniOpVM::kLoadCvt64_I16ToI32:
      case UniOpVM::kLoadCvt64_U16ToU32:
      case UniOpVM::kLoadCvt64_I32ToI64:
      case UniOpVM::kLoadCvt64_U32ToU64:
      case UniOpVM::kLoadCvtN_I8ToI16:
      case UniOpVM::kLoadCvtN_U8ToU16:
      case UniOpVM::kLoadCvtN_I16ToI32:
      case UniOpVM::kLoadCvtN_U16ToU32:
      case UniOpVM::kLoadCvtN_I32ToI64:
      case UniOpVM::kLoadCvtN_U32ToU64: {
        src.set_size(8);
        if (has_sse4_1()) {
          InstId inst = op_info.sse_inst_id;
          cc->emit(inst, dst, src);
        }
        else {
          cc->movq(dst, src);
          sse_int_widen(*this, dst, dst, WideningOp(op_info.cvt));
        }
        return;
      }

      case UniOpVM::kLoadInsertU16: {
        cc->emit(op_info.sse_inst_id, dst, dst, idx);
        return;
      }

      case UniOpVM::kLoadInsertF32:
        op = UniOpVM::kLoadInsertU32;
        [[fallthrough]];

      case UniOpVM::kLoadInsertU8:
      case UniOpVM::kLoadInsertU32:
      case UniOpVM::kLoadInsertU64: {
        if (has_sse4_1() && (op != UniOpVM::kLoadInsertU64 || is_64bit())) {
          cc->emit(op_info.sse_inst_id, dst, src, idx);
          return;
        }

        if (op == UniOpVM::kLoadInsertU8) {
          Gp tmp = new_gp32("@tmp");
          src.set_size(1);
          cc->pextrw(tmp, dst, idx / 2u);
          if (idx & 1)
            cc->mov(tmp.r8_hi(), src);
          else
            cc->mov(tmp.r8(), src);
          cc->pinsrw(dst, tmp, idx / 2u);
          return;
        }

        if (op == UniOpVM::kLoadInsertU32) {
          if (idx == 0) {
            Vec tmp = new_vec128("@tmp");
            cc->movd(tmp, src);
            cc->movss(dst, tmp);
          }
          else {
            cc->pinsrw(dst, src, idx * 2u);
            src.add_offset(2);
            cc->pinsrw(dst, src, idx * 2u + 1);
          }
          return;
        }

        ASMJIT_ASSERT(op == UniOpVM::kLoadInsertU64);
        if (idx == 0)
          cc->movlpd(dst, src);
        else
          cc->movhpd(dst, src);

        return;
      }

      case UniOpVM::kLoadInsertF32x2: {
        if (idx == 0)
          cc->movlps(dst, src);
        else
          cc->movhps(dst, src);
        return;
      }

      case UniOpVM::kLoadInsertF64: {
        if (idx == 0)
          cc->movlpd(dst, src);
        else
          cc->movhpd(dst, src);
        return;
      }

      case UniOpVM::kLoad256_U32:
      case UniOpVM::kLoad256_U64:
      case UniOpVM::kLoad256_F32:
      case UniOpVM::kLoad256_F64:
      case UniOpVM::kLoad512_U32:
      case UniOpVM::kLoad512_U64:
      case UniOpVM::kLoad512_F32:
      case UniOpVM::kLoad512_F64:
      case UniOpVM::kLoadCvt32_U8ToU64:
      case UniOpVM::kLoadCvt64_U8ToU64:
      case UniOpVM::kLoadCvt64_I8ToI32:
      case UniOpVM::kLoadCvt64_U8ToU32:
      case UniOpVM::kLoadCvt128_I8ToI16:
      case UniOpVM::kLoadCvt128_U8ToU16:
      case UniOpVM::kLoadCvt128_I8ToI32:
      case UniOpVM::kLoadCvt128_U8ToU32:
      case UniOpVM::kLoadCvt128_I16ToI32:
      case UniOpVM::kLoadCvt128_U16ToU32:
      case UniOpVM::kLoadCvt128_I32ToI64:
      case UniOpVM::kLoadCvt128_U32ToU64:
      case UniOpVM::kLoadCvt256_I8ToI16:
      case UniOpVM::kLoadCvt256_U8ToU16:
      case UniOpVM::kLoadCvt256_I16ToI32:
      case UniOpVM::kLoadCvt256_U16ToU32:
      case UniOpVM::kLoadCvt256_I32ToI64:
      case UniOpVM::kLoadCvt256_U32ToU64:
        ASMJIT_NOT_REACHED();

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_vm(UniOpVM op, const OpArray& dst_, const Mem& src_, Alignment alignment, uint32_t idx) {
  Mem src(src_);

  UniOpVMInfo op_info = opcode_info_2vm[size_t(op)];
  uint32_t mem_size = op_info.mem_size;

  if (mem_size == 0) {
    uint32_t mem_size_shift = op_info.mem_size_shift;
    for (size_t i = 0, n = dst_.size(); i < n; i++) {
      ASMJIT_ASSERT(dst_[i].is_reg() && dst_[i].is_vec());

      const Vec& dst = dst_[i].as<Vec>();
      mem_size = dst.size() >> mem_size_shift;

      emit_vm(op, dst, src, uint32_t(alignment) > 0u ? alignment : Alignment(mem_size), idx);
      src.add_offset_lo32(int32_t(mem_size));
    }
  }
  else {
    if (uint32_t(alignment) == 0u) {
      alignment = Alignment(mem_size);
    }

    for (size_t i = 0, n = dst_.size(); i < n; i++) {
      ASMJIT_ASSERT(dst_[i].is_reg() && dst_[i].is_vec());

      const Vec& dst = dst_[i].as<Vec>();
      emit_vm(op, dst, src, alignment, idx);
      src.add_offset_lo32(int32_t(mem_size));
    }
  }
}

void UniCompiler::emit_mv(UniOpMV op, const Mem& dst_, const Vec& src_, Alignment alignment, uint32_t idx) {
  ASMJIT_ASSERT(dst_.is_mem());
  ASMJIT_ASSERT(src_.is_reg() && src_.is_vec());

  Mem dst(dst_);
  Vec src(src_);
  UniOpVMInfo op_info = opcode_info_2mv[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    switch (op) {
      case UniOpMV::kStore8: {
        dst.set_size(1);
        cc->vpextrb(dst, src.xmm(), 0);
        return;
      }

      case UniOpMV::kStore16_U16: {
        dst.set_size(2);
        cc->vpextrw(dst, src.xmm(), 0);
        return;
      }

      case UniOpMV::kStore32_U32:
      case UniOpMV::kStore32_F32:
      case UniOpMV::kStore64_U32:
      case UniOpMV::kStore64_U64:
      case UniOpMV::kStore64_F32:
      case UniOpMV::kStore64_F64: {
        dst.set_size(op_info.mem_size);
        cc->emit(op_info.avx_inst_id, dst, src.xmm());
        return;
      }

      case UniOpMV::kStore128_U32:
      case UniOpMV::kStore128_U64:
      case UniOpMV::kStore128_F32:
      case UniOpMV::kStore128_F64:
      case UniOpMV::kStore256_U32:
      case UniOpMV::kStore256_U64:
      case UniOpMV::kStore256_F32:
      case UniOpMV::kStore256_F64:
      case UniOpMV::kStore512_U32:
      case UniOpMV::kStore512_U64:
      case UniOpMV::kStore512_F32:
      case UniOpMV::kStore512_F64:
        ASMJIT_ASSERT(src.size() >= op_info.mem_size);
        src.set_signature(signature_of_xmm_ymm_zmm[op_info.mem_size >> 5]);
        [[fallthrough]];

      case UniOpMV::kStoreN_U32:
      case UniOpMV::kStoreN_U64:
      case UniOpMV::kStoreN_F32:
      case UniOpMV::kStoreN_F64: {
        InstId inst = (uint32_t(alignment) == 0 || uint32_t(alignment) >= src.size()) ? Inst::kIdVmovaps : Inst::kIdVmovups;
        dst.set_size(src.size());
        cc->emit(inst, dst, src);
        return;
      }

      /*
      case UniOpMV::kStoreCvtz64_U16ToU8:
      case UniOpMV::kStoreCvtz64_U32ToU16:
      case UniOpMV::kStoreCvtz64_U64ToU32:
      case UniOpMV::kStoreCvts64_I16ToI8:
      case UniOpMV::kStoreCvts64_I16ToU8:
      case UniOpMV::kStoreCvts64_U16ToU8:
      case UniOpMV::kStoreCvts64_I32ToI16:
      case UniOpMV::kStoreCvts64_U32ToU16:
      case UniOpMV::kStoreCvts64_I64ToI32:
      case UniOpMV::kStoreCvts64_U64ToU32:
      case UniOpMV::kStoreCvtz128_U16ToU8:
      case UniOpMV::kStoreCvtz128_U32ToU16:
      case UniOpMV::kStoreCvtz128_U64ToU32:
      case UniOpMV::kStoreCvts128_I16ToI8:
      case UniOpMV::kStoreCvts128_I16ToU8:
      case UniOpMV::kStoreCvts128_U16ToU8:
      case UniOpMV::kStoreCvts128_I32ToI16:
      case UniOpMV::kStoreCvts128_U32ToU16:
      case UniOpMV::kStoreCvts128_I64ToI32:
      case UniOpMV::kStoreCvts128_U64ToU32:
      case UniOpMV::kStoreCvtz256_U16ToU8:
      case UniOpMV::kStoreCvtz256_U32ToU16:
      case UniOpMV::kStoreCvtz256_U64ToU32:
      case UniOpMV::kStoreCvts256_I16ToI8:
      case UniOpMV::kStoreCvts256_I16ToU8:
      case UniOpMV::kStoreCvts256_U16ToU8:
      case UniOpMV::kStoreCvts256_I32ToI16:
      case UniOpMV::kStoreCvts256_U32ToU16:
      case UniOpMV::kStoreCvts256_I64ToI32:
      case UniOpMV::kStoreCvts256_U64ToU32:
      case UniOpMV::kStoreCvtzN_U16ToU8:
      case UniOpMV::kStoreCvtzN_U32ToU16:
      case UniOpMV::kStoreCvtzN_U64ToU32:
      case UniOpMV::kStoreCvtsN_I16ToI8:
      case UniOpMV::kStoreCvtsN_I16ToU8:
      case UniOpMV::kStoreCvtsN_U16ToU8:
      case UniOpMV::kStoreCvtsN_I32ToI16:
      case UniOpMV::kStoreCvtsN_U32ToU16:
      case UniOpMV::kStoreCvtsN_I64ToI32:
      case UniOpMV::kStoreCvtsN_U64ToU32:
      */

      case UniOpMV::kStoreExtractU16:
      case UniOpMV::kStoreExtractU32:
      case UniOpMV::kStoreExtractU64: {
        src = src.xmm();

        if (op == UniOpMV::kStoreExtractU32) {
          if (idx == 0) {
            cc->vmovd(dst, src);
            return;
          }
        }

        if (op == UniOpMV::kStoreExtractU64) {
          if (idx == 0) {
            cc->vmovq(dst, src);
            return;
          }
          else if (!is_64bit()) {
            cc->vmovhpd(dst, src);
            return;
          }
        }

        cc->emit(op_info.avx_inst_id, dst, src, idx);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    ASMJIT_ASSERT(src.is_vec128());

    switch (op) {
      case UniOpMV::kStore8: {
        dst.set_size(1);

        if (has_sse4_1()) {
          cc->pextrb(dst, src, 0);
        }
        else {
          Gp tmp = new_gp32("@tmp");
          cc->movd(tmp, src);
          cc->mov(dst, tmp.r8());
        }
        return;
      }

      case UniOpMV::kStore16_U16: {
        dst.set_size(2);
        if (has_sse4_1()) {
          cc->pextrw(dst, src, 0);
        }
        else {
          Gp tmp = new_gp32("@tmp");
          cc->movd(tmp, src);
          cc->mov(dst, tmp.r16());
        }
        return;
      }

      case UniOpMV::kStore32_U32:
      case UniOpMV::kStore32_F32:
      case UniOpMV::kStore64_U32:
      case UniOpMV::kStore64_U64:
      case UniOpMV::kStore64_F32:
      case UniOpMV::kStore64_F64: {
        dst.set_size(op_info.mem_size);
        cc->emit(op_info.sse_inst_id, dst, src);
        return;
      }

      case UniOpMV::kStore128_U32:
      case UniOpMV::kStore128_U64:
      case UniOpMV::kStore128_F32:
      case UniOpMV::kStore128_F64:
      case UniOpMV::kStoreN_U32:
      case UniOpMV::kStoreN_U64:
      case UniOpMV::kStoreN_F32:
      case UniOpMV::kStoreN_F64: {
        InstId inst = (uint32_t(alignment) == 0u || uint32_t(alignment) >= 16u) ? Inst::kIdMovaps : Inst::kIdMovups;
        dst.set_size(16);
        cc->emit(inst, dst, src);
        return;

      }

      /*
      case UniOpMV::kStoreCvtz64_U16ToU8:
      case UniOpMV::kStoreCvtz64_U32ToU16:
      case UniOpMV::kStoreCvtz64_U64ToU32:
      case UniOpMV::kStoreCvts64_I16ToI8:
      case UniOpMV::kStoreCvts64_I16ToU8:
      case UniOpMV::kStoreCvts64_U16ToU8:
      case UniOpMV::kStoreCvts64_I32ToI16:
      case UniOpMV::kStoreCvts64_U32ToU16:
      case UniOpMV::kStoreCvts64_I64ToI32:
      case UniOpMV::kStoreCvts64_U64ToU32:
      case UniOpMV::kStoreCvtzN_U16ToU8:
      case UniOpMV::kStoreCvtzN_U32ToU16:
      case UniOpMV::kStoreCvtzN_U64ToU32:
      case UniOpMV::kStoreCvtsN_I16ToI8:
      case UniOpMV::kStoreCvtsN_I16ToU8:
      case UniOpMV::kStoreCvtsN_U16ToU8:
      case UniOpMV::kStoreCvtsN_I32ToI16:
      case UniOpMV::kStoreCvtsN_U32ToU16:
      case UniOpMV::kStoreCvtsN_I64ToI32:
      case UniOpMV::kStoreCvtsN_U64ToU32: {
        UNIMPLEMENTED();
        return;
      }
      */

      case UniOpMV::kStore256_U32:
      case UniOpMV::kStore256_U64:
      case UniOpMV::kStore256_F32:
      case UniOpMV::kStore256_F64:
      case UniOpMV::kStore512_U32:
      case UniOpMV::kStore512_U64:
      case UniOpMV::kStore512_F32:
      case UniOpMV::kStore512_F64:
      /*
      case UniOpMV::kStoreCvtz128_U16ToU8:
      case UniOpMV::kStoreCvtz128_U32ToU16:
      case UniOpMV::kStoreCvtz128_U64ToU32:
      case UniOpMV::kStoreCvts128_I16ToI8:
      case UniOpMV::kStoreCvts128_I16ToU8:
      case UniOpMV::kStoreCvts128_U16ToU8:
      case UniOpMV::kStoreCvts128_I32ToI16:
      case UniOpMV::kStoreCvts128_U32ToU16:
      case UniOpMV::kStoreCvts128_I64ToI32:
      case UniOpMV::kStoreCvts128_U64ToU32:
      case UniOpMV::kStoreCvtz256_U16ToU8:
      case UniOpMV::kStoreCvtz256_U32ToU16:
      case UniOpMV::kStoreCvtz256_U64ToU32:
      case UniOpMV::kStoreCvts256_I16ToI8:
      case UniOpMV::kStoreCvts256_I16ToU8:
      case UniOpMV::kStoreCvts256_U16ToU8:
      case UniOpMV::kStoreCvts256_I32ToI16:
      case UniOpMV::kStoreCvts256_U32ToU16:
      case UniOpMV::kStoreCvts256_I64ToI32:
      case UniOpMV::kStoreCvts256_U64ToU32:
      */
        ASMJIT_NOT_REACHED();

      case UniOpMV::kStoreExtractU16:
      case UniOpMV::kStoreExtractU32:
      case UniOpMV::kStoreExtractU64: {
        if (op == UniOpMV::kStoreExtractU32) {
          if (idx == 0) {
            cc->movd(dst, src);
            return;
          }
        }

        if (op == UniOpMV::kStoreExtractU64) {
          if (idx == 0) {
            cc->movq(dst, src);
            return;
          }

          if (idx == 1) {
            cc->movhps(dst, src);
            return;
          }
        }

        if (has_sse4_1()) {
          cc->emit(op_info.sse_inst_id, dst, src, idx);
          return;
        }

        // SSE4.1 not available - only required when extracting 16-bit and 32-bit quantities as 64-bit quantities
        // were already handled. Additionally, there is no PEXTRW instruction in SSE2 that would extract to memory,
        // this instruction was added by SSE4.1 as well (there are actually two forms of PEXTRW).
        if (op == UniOpMV::kStoreExtractU16) {
          Gp tmp = new_gp32("@pextrw_tmp");
          cc->pextrw(tmp, src, idx);
          cc->mov(dst, tmp);
          return;
        }

        if (op == UniOpMV::kStoreExtractU32) {
          Vec tmp = new_vec128("@pextrd_tmp");
          cc->pshufd(tmp, src, x86::shuffle_imm(idx, idx, idx, idx));
          cc->movd(dst, tmp);
          return;
        }

        ASMJIT_NOT_REACHED();
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_mv(UniOpMV op, const Mem& dst_, const OpArray& src_, Alignment alignment, uint32_t idx) {
  Support::maybe_unused(idx);

  Mem dst(dst_);

  UniOpVMInfo op_info = opcode_info_2mv[size_t(op)];
  uint32_t mem_size = op_info.mem_size;

  if (mem_size == 0) {
    for (size_t i = 0, n = src_.size(); i < n; i++) {
      ASMJIT_ASSERT(src_[i].is_reg() && src_[i].is_vec());

      const Vec& src = src_[i].as<Vec>();
      mem_size = src.size();

      emit_mv(op, dst, src, uint32_t(alignment) > 0u ? alignment : Alignment(mem_size));
      dst.add_offset_lo32(int32_t(mem_size));
    }
  }
  else {
    if (uint32_t(alignment) == 0) {
      alignment = Alignment(mem_size);
    }

    for (size_t i = 0, n = src_.size(); i < n; i++) {
      ASMJIT_ASSERT(src_[i].is_reg() && src_[i].is_vec());

      const Vec& src = src_[i].as<Vec>();
      emit_mv(op, dst, src, alignment);
      dst.add_offset_lo32(int32_t(mem_size));
    }
  }
}

// ujit::UniCompiler - Vector Instructions - Emit 3V
// =================================================

void UniCompiler::emit_3v(UniOpVVV op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_) {
  ASMJIT_ASSERT(dst_.is_vec());
  ASMJIT_ASSERT(src1_.is_vec());

  Vec dst(dst_.as<Vec>());
  Vec src1v(src1_.as<Vec>().clone_as(dst));
  Operand src2(src2_);
  UniOpVInfo op_info = opcode_info_3v[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    InstId inst_id = op_info.avx_inst_id;

    static constexpr InstId avx_vpmovm2v_table[] = {
      Inst::kIdVpmovm2b,
      Inst::kIdVpmovm2w,
      Inst::kIdVpmovm2d,
      Inst::kIdVpmovm2q
    };

    if (is_same_vec(src1v, src2)) {
      switch (SameVecOp(op_info.same_vec_op)) {
        case SameVecOp::kZero: avx_zero(*this, dst); return;
        case SameVecOp::kOnes: avx_ones(*this, dst); return;
        case SameVecOp::kSrc: avx_mov(*this, dst, src1v); return;

        default:
          break;
      }
    }

    if (has_avx_ext(AVXExt(op_info.avx_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      FloatMode fm = FloatMode(op_info.float_mode);
      if (is_scalar_fp_op(fm)) {
        dst.set_signature(signature_of_xmm_ymm_zmm[0]);
        src1v.set_signature(signature_of_xmm_ymm_zmm[0]);

        if (src2.is_vec())
          src2.as<Vec>().set_signature(signature_of_xmm_ymm_zmm[0]);
      }

      if (op >= UniOpVVV::kAndU32 && op <= UniOpVVV::kAndnU64 && !has_avx512()) {
        static constexpr uint16_t avx512_to_avx_bitwise_map[] = {
          Inst::kIdVpand , Inst::kIdVpand ,
          Inst::kIdVpor  , Inst::kIdVpor  ,
          Inst::kIdVpxor , Inst::kIdVpxor ,
          Inst::kIdVpandn, Inst::kIdVpandn
        };
        inst_id = avx512_to_avx_bitwise_map[size_t(op) - size_t(UniOpVVV::kAndU32)];
      }

      if (op_info.comparison && ((dst.is_vec512()) ||
                                (src2.is_mem() && src2.as<Mem>().has_broadcast()) ||
                                (AVXExt(op_info.avx_ext) == AVXExt::kAVX512))) {
        // AVX-512 instructions change semantics when it comes to comparisons. Instead of having a VEC destination
        // we need a K destination. To not change semantics to our users we just convert the predicate to a VEC mask.
        x86::KReg kTmp = cc->new_kq("@kTmp");
        InstId kMovM = avx_vpmovm2v_table[op_info.element_size];

        if (op_info.use_imm)
          cc->emit(inst_id, kTmp, src1v, src2, Imm(op_info.imm));
        else
          cc->emit(inst_id, kTmp, src1v, src2);

        cc->emit(kMovM, dst, kTmp);
        return;
      }

      if (op_info.use_imm)
        cc->emit(inst_id, dst, src1v, src2, Imm(op_info.imm));
      else
        cc->emit(inst_id, dst, src1v, src2);
      return;
    }

    switch (op) {
      case UniOpVVV::kBicU32:
      case UniOpVVV::kBicU64:
      case UniOpVVV::kBicF32:
      case UniOpVVV::kBicF64: {
        if (has_avx512()) {
          uint32_t ternlog_inst = ElementSize(op_info.element_size) == ElementSize::k32 ? Inst::kIdVpternlogd : Inst::kIdVpternlogq;
          if (src2.is_mem())
            cc->emit(ternlog_inst, dst, src1v, src2.as<Mem>(), 0x44);
          else
            cc->emit(inst_id, dst, src2, src1v);
          return;
        }

        if (op <= UniOpVVV::kBicU64)
          inst_id = Inst::kIdVpandn;

        if (src2.is_mem()) {
          src2 = UniCompiler_load_new(*this, dst, src2.as<Mem>(), op_info.broadcast_size);
        }

        cc->emit(inst_id, dst, src2, src1v);
        return;
      }

      // dst = a - (floor(a / b) * b).
      case UniOpVVV::kModF32S:
      case UniOpVVV::kModF64S:
      case UniOpVVV::kModF32:
      case UniOpVVV::kModF64: {
        FloatMode fm = FloatMode(op_info.float_mode);
        UniOpVV trunc_op = translate_op(op, UniOpVVV::kModF32S, UniOpVV::kTruncF32);
        const FloatInst& fi = avx_float_inst[fm];

        x86::Vec tmp = new_similar_reg(dst, "@mod_tmp");
        cc->emit(fi.fdiv, tmp, src1v, src2);
        emit_2v(trunc_op, tmp, tmp);
        cc->emit(fi.fmul, tmp, tmp, src2);
        cc->emit(fi.fsub, dst, src1v, tmp);

        return;
      }

      case UniOpVVV::kMulU64: {
        // Native operation requires AVX512, which is not supported by the target.
        if (src2.is_mem()) {
          src2 = UniCompiler_load_new(*this, dst, src2.as<Mem>(), op_info.broadcast_size);
        }

        Vec src2v = src2.as<Vec>().clone_as(dst);
        Vec al_bh = new_similar_reg(dst, "@al_bh");
        Vec ah_bl = new_similar_reg(dst, "@ah_bl");
        Vec hi_part = new_similar_reg(dst, "@hi_part");

        cc->vpsrlq(al_bh, src2v, 32);
        cc->vpsrlq(ah_bl, src1v, 32);

        cc->vpmuludq(al_bh, al_bh, src1v);
        cc->vpmuludq(ah_bl, ah_bl, src2v);
        cc->vpmuludq(dst, src1v, src2v);

        cc->vpaddq(hi_part, al_bh, ah_bl);
        cc->vpsllq(hi_part, hi_part, 32);
        cc->vpaddq(dst, dst, hi_part);

        return;
      }

      case UniOpVVV::kMulU64_LoU32: {
        // Intrinsic.
        Vec tmp = new_similar_reg(dst.as<Vec>(), "@tmp");

        if (has_avx512()) {
          Vec msk = simd_vec_const(&ct().p_FFFFFFFF00000000, Bcst::k64, dst);
          cc->emit(Inst::kIdVpandnq, tmp, msk, src2);
          cc->emit(Inst::kIdVpmullq, dst, src1v, tmp);
        }
        else {
          cc->emit(Inst::kIdVpshufd, tmp, src1v, x86::shuffle_imm(2, 3, 0, 1));
          cc->emit(Inst::kIdVpmuludq, tmp, tmp, src2);
          cc->emit(Inst::kIdVpmuludq, dst, src1v, src2);
          cc->emit(Inst::kIdVpsllq, tmp, tmp, 32);
          cc->emit(Inst::kIdVpaddq, dst, dst, tmp);
        }
        return;
      }

      case UniOpVVV::kMinI64:
      case UniOpVVV::kMaxI64: {
        // Native operation requires AVX512, which is not supported by the target.
        if (src2.is_mem()) {
          src2 = UniCompiler_load_new(*this, dst, src2.as<Mem>(), op_info.broadcast_size);
        }

        ASMJIT_ASSERT(src2.is_vec());
        Vec src2v = src2.as<Vec>().clone_as(dst);

        Vec msk = dst;
        if (dst.id() == src1v.id() || dst.id() == src2v.id()) {
          msk = new_similar_reg(dst, "@msk");
        }

        cc->vpcmpgtq(msk, src1v, src2v);          // msk = src1 > src2
        if (op == UniOpVVV::kMinI64)
          cc->vblendvpd(dst, src1v, src2v, msk);  // dst = msk == 0 ? src1 : src2;
        else
          cc->vblendvpd(dst, src2v, src1v, msk);  // dst = msk == 0 ? src2 : src1;
        return;
      }

      case UniOpVVV::kMinU64:
      case UniOpVVV::kMaxU64: {
        if (src2.is_mem()) {
          src2 = UniCompiler_load_new(*this, dst, src2.as<Mem>(), op_info.broadcast_size);
        }

        ASMJIT_ASSERT(src2.is_vec());
        Vec src2v = src2.as<Vec>().clone_as(dst);

        Vec tmp1 = dst;
        Vec tmp2 = new_similar_reg(dst, "@tmp2");

        if (dst.id() == src1v.id() || dst.id() == src2v.id()) {
          tmp1 = new_similar_reg(dst, "@tmp1");
        }

        avx_isign_flip(*this, tmp1, src1v, ElementSize::k64);
        avx_isign_flip(*this, tmp2, src2v, ElementSize::k64);

        cc->vpcmpgtq(tmp1, tmp1, tmp2);           // tmp1 = src1 > src2
        if (op == UniOpVVV::kMinU64)
          cc->vblendvpd(dst, src1v, src2v, tmp1); // dst = tmp1 == 0 ? src1 : src2;
        else
          cc->vblendvpd(dst, src2v, src1v, tmp1); // dst = tmp1 == 0 ? src2 : src1;
        return;
      }

      case UniOpVVV::kCmpGtU8:
      case UniOpVVV::kCmpGtU16:
      case UniOpVVV::kCmpGtU32: {
        // Native operation requires AVX512, which is not supported by the target.
        CmpMinMaxInst inst = avx_cmp_min_max[(size_t(op) - size_t(UniOpVVV::kCmpGtI8)) & 0x7u];
        if (is_same_vec(dst, src1v)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          cc->emit(inst.pmin, tmp, src1v, src2);
          cc->emit(inst.peq, dst, dst, tmp);
        }
        else {
          cc->emit(inst.pmin, dst, src1v, src2);
          cc->emit(inst.peq, dst, dst, src1v);
        }
        avx_bit_not(*this, dst, dst);
        return;
      }

      case UniOpVVV::kCmpGtU64:
      case UniOpVVV::kCmpLeU64: {
        Vec tmp = new_similar_reg(dst, "@tmp");
        avx_isign_flip(*this, tmp, src2, ElementSize::k64);
        avx_isign_flip(*this, dst, src1v, ElementSize::k64);
        cc->emit(Inst::kIdVpcmpgtq, dst, dst, tmp);

        if (op == UniOpVVV::kCmpLeU64) {
          avx_bit_not(*this, dst, dst);
        }
        return;
      }

      case UniOpVVV::kCmpGeI8:
      case UniOpVVV::kCmpGeU8:
      case UniOpVVV::kCmpGeI16:
      case UniOpVVV::kCmpGeU16:
      case UniOpVVV::kCmpGeI32:
      case UniOpVVV::kCmpGeU32: {
        CmpMinMaxInst inst = avx_cmp_min_max[(size_t(op) - size_t(UniOpVVV::kCmpGeI8)) & 0x7u];

        if (dst.id() == src1v.id()) {
          if (!src2.is_reg()) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            cc->emit(inst.pmax, tmp, src1v, src2);
            cc->emit(inst.peq, dst, tmp, src1v);
          }
          else {
            cc->emit(inst.pmin, dst, src1v, src2);
            cc->emit(inst.peq, dst, dst, src2);
          }
        }
        else {
          cc->emit(inst.pmax, dst, src1v, src2);
          cc->emit(inst.peq, dst, dst, src1v);
        }

        return;
      }

      case UniOpVVV::kCmpLtI8:
      case UniOpVVV::kCmpLtI16:
      case UniOpVVV::kCmpLtI32:
      case UniOpVVV::kCmpLtI64:
      case UniOpVVV::kCmpGeI64: {
        if (!src2.is_reg()) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          avx_mov(*this, tmp, src2);
          src2 = tmp;
        }

        CmpMinMaxInst inst = avx_cmp_min_max[(size_t(op) - size_t(UniOpVVV::kCmpLtI8)) & 0x7u];
        cc->emit(inst.pgt, dst, src2, src1v);

        if (op == UniOpVVV::kCmpGeI64) {
          avx_bit_not(*this, dst, dst);
        }
        return;
      }

      case UniOpVVV::kCmpLtU8:
      case UniOpVVV::kCmpLtU16:
      case UniOpVVV::kCmpLtU32:
      case UniOpVVV::kCmpLtU64:
      case UniOpVVV::kCmpGeU64: {
        Vec tmp = new_similar_reg(dst, "@tmp");
        avx_isign_flip(*this, tmp, src2, ElementSize(op_info.element_size));
        avx_isign_flip(*this, dst, src1v, ElementSize(op_info.element_size));

        CmpMinMaxInst inst = avx_cmp_min_max[(size_t(op) - size_t(UniOpVVV::kCmpLtI8)) & 0x7u];
        cc->emit(inst.pgt, dst, tmp, dst);

        if (op == UniOpVVV::kCmpGeU64) {
          avx_bit_not(*this, dst, dst);
        }
        return;
      }

      case UniOpVVV::kCmpLeI8:
      case UniOpVVV::kCmpLeU8:
      case UniOpVVV::kCmpLeI16:
      case UniOpVVV::kCmpLeU16:
      case UniOpVVV::kCmpLeI32:
      case UniOpVVV::kCmpLeU32: {
        CmpMinMaxInst inst = avx_cmp_min_max[(size_t(op) - size_t(UniOpVVV::kCmpLeI8)) & 0x7u];

        if (dst.id() == src1v.id()) {
          if (!src2.is_reg()) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            cc->emit(inst.pmin, tmp, src1v, src2);
            cc->emit(inst.peq, dst, tmp, src1v);
          }
          else {
            cc->emit(inst.pmax, dst, src1v, src2);
            cc->emit(inst.peq, dst, dst, src2);
          }
        }
        else {
          cc->emit(inst.pmin, dst, src1v, src2);
          cc->emit(inst.peq, dst, dst, src1v);
        }

        return;
      }

      case UniOpVVV::kCmpLeI64: {
        cc->emit(Inst::kIdVpcmpgtq, dst, src1v, src2);

        avx_bit_not(*this, dst, dst);
        return;
      }

      case UniOpVVV::kHAddF64: {
        if (has_avx512() && dst.is_vec512()) {
          // [B A]    [C A]
          // [D C] -> [D B]
          Vec tmp = new_similar_reg(dst, "@tmp");

          cc->emit(Inst::kIdVunpckhpd, tmp, src1v, src2);
          cc->emit(Inst::kIdVunpcklpd, dst, src1v, src2);
          cc->vaddpd(dst, dst, tmp);
        }
        else {
          cc->emit(inst_id, dst, src1v, src2);
        }
        return;
      }

      case UniOpVVV::kCombineLoHiU64:
      case UniOpVVV::kCombineLoHiF64: {
        // Intrinsic - dst = {src1.u64[0], src2.64[1]} - combining low part of src1 and high part of src1.
        if (!src2.is_reg()) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          avx_mov(*this, tmp, src2);
          src2 = tmp;
        }

        uint32_t shuf_imm = shuf_imm2_from_swizzle_with_width(swizzle(0, 1), VecWidthUtils::vec_width_of(dst));
        cc->emit(Inst::kIdVshufpd, dst, src2, src1v, shuf_imm);
        return;
      }

      case UniOpVVV::kCombineHiLoU64:
      case UniOpVVV::kCombineHiLoF64: {
        // Intrinsic - dst = {src1.u64[1], src2.u64[0]} - combining high part of src1 and low part of src2.
        if (dst.is_vec128()) {
          if (src2.is_vec())
            cc->emit(Inst::kIdVmovsd, dst, src1v.xmm(), src2.as<Vec>().xmm());
          else
            cc->emit(Inst::kIdVmovlpd, dst, src1v.xmm(), src2);
          return;
        }

        if (!src2.is_reg()) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          avx_mov(*this, tmp, src2);
          src2 = tmp;
        }

        uint32_t shuf_imm = shuf_imm2_from_swizzle_with_width(swizzle(1, 0), VecWidthUtils::vec_width_of(dst));
        cc->emit(Inst::kIdVshufpd, dst, src2, src1v, shuf_imm);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    InstId inst_id = op_info.sse_inst_id;

    // SSE floating point comparison cannot use the extended predicates as introduced by AVX.
    static constexpr uint8_t sse_fcmp_imm_table[] = {
      0x00u, // kCmpEq    (eq ordered quiet).
      0x04u, // kCmpNe    (ne ordered quiet).
      0x01u, // kCmpGt    (lt ordered quiet <reversed>).
      0x02u, // kCmpGe    (le ordered quiet <reversed>).
      0x01u, // kCmpLt    (lt ordered quiet).
      0x02u, // kCmpLe    (le ordered quiet).
      0x07u, // kCmpOrd   (ordered quiet).
      0x03u  // kCmpUnord (unordered quiet).
    };

    if (is_same_vec(dst, src2) && op_info.commutative) {
      std::swap(src1v, src2.as<Vec>());
    }

    if (is_same_vec(src1v, src2)) {
      switch (SameVecOp(op_info.same_vec_op)) {
        case SameVecOp::kZero:
          cc->emit(Inst::kIdPxor, dst, dst);
          return;

        case SameVecOp::kOnes:
          cc->emit(Inst::kIdPcmpeqb, dst, dst);
          return;

        case SameVecOp::kSrc:
          sse_mov(*this, dst, src1v);
          return;

        default:
          break;
      }
    }

    if (has_sse_ext(SSEExt(op_info.sse_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      if (!is_same_vec(dst, src1v)) {
        if (is_same_vec(dst, src2)) {
          Vec tmp = new_similar_reg(dst, "tmp");
          sse_mov(*this, tmp, src2);
          src2 = tmp;
        }

        sse_mov(*this, dst, src1v);
      }

      if (op_info.use_imm)
        cc->emit(inst_id, dst, src2, Imm(op_info.imm));
      else
        cc->emit(inst_id, dst, src2);
      return;
    }

    switch (op) {
      case UniOpVVV::kBicU32:
      case UniOpVVV::kBicU64:
      case UniOpVVV::kBicF32:
      case UniOpVVV::kBicF64: {
        if (is_same_vec(dst, src2)) {
          cc->emit(inst_id, dst, src1v);
          return;
        }

        if (is_same_vec(dst, src1v)) {
          Vec tmp = new_similar_reg(dst);
          sse_mov(*this, tmp, src1v);
          src1v = tmp;
        }

        sse_mov(*this, dst, src2);
        cc->emit(inst_id, dst, src1v);
        return;
      }

      // dst = a - (floor(a / b) * b).
      case UniOpVVV::kModF32S:
      case UniOpVVV::kModF64S:
      case UniOpVVV::kModF32:
      case UniOpVVV::kModF64: {
        FloatMode fm = FloatMode(op_info.float_mode);
        UniOpVV trunc_op = translate_op(op, UniOpVVV::kModF32S, UniOpVV::kTruncF32);
        const FloatInst& fi = sse_float_inst[fm];

        x86::Vec tmp = new_similar_reg(dst, "@mod_tmp");

        cc->emit(fi.fmova, tmp, src1v);
        cc->emit(fi.fdiv, tmp, src2);

        emit_2v(trunc_op, tmp, tmp);
        cc->emit(fi.fmul, tmp, src2);

        sse_fmov(*this, dst, src1v, fm);
        cc->emit(fi.fsub, dst, tmp);
        return;
      }

      case UniOpVVV::kMulU32: {
        // Native operation requires SSE4.1, which is not supported by the target.
        Vec tmp1 = new_similar_reg(dst, "tmp1");
        Vec tmp2 = new_similar_reg(dst, "tmp2");

        cc->emit(Inst::kIdPshufd, tmp1, src1v, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdPshufd, tmp2, src2, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdPmuludq, tmp1, tmp2);

        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPmuludq, dst, src2);
        cc->emit(Inst::kIdShufps, dst, tmp1, x86::shuffle_imm(2, 0, 2, 0));
        cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(3, 1, 2, 0));
        return;
      }

      case UniOpVVV::kMulU64: {
        // Native operation requires AVX512, which is not supported by the target.
        Vec al_bh = new_similar_reg(dst, "@al_bh");
        Vec ah_bl = new_similar_reg(dst, "@ah_bl");

        cc->emit(Inst::kIdPshufd, al_bh, src2, x86::shuffle_imm(3, 3, 1, 1));
        cc->emit(Inst::kIdPshufd, ah_bl, src1v, x86::shuffle_imm(3, 3, 1, 1));

        cc->emit(Inst::kIdPmuludq, al_bh, src1v);
        cc->emit(Inst::kIdPmuludq, ah_bl, src2);
        cc->emit(Inst::kIdPaddq, al_bh, ah_bl);

        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPmuludq, dst, src2);
        cc->emit(Inst::kIdPsllq, al_bh, 32);
        cc->emit(Inst::kIdPaddq, dst, al_bh);
        return;
      }

      case UniOpVVV::kMulU64_LoU32: {
        Vec tmp = new_similar_reg(dst.as<Vec>(), "@tmp");

        cc->emit(Inst::kIdPshufd, tmp, src1v, x86::shuffle_imm(2, 3, 0, 1));
        cc->emit(Inst::kIdPmuludq, tmp, src2);

        if (dst.id() == src2.id()) {
          cc->emit(Inst::kIdPmuludq, dst, src1v);
        }
        else {
          sse_mov(*this, dst, src1v);
          cc->emit(Inst::kIdPmuludq, dst, src2);
        }
        cc->emit(Inst::kIdPsllq, tmp, 32);
        cc->emit(Inst::kIdPaddq, dst, tmp);

        return;
      }

      // Native operation requires AVX512, which is not supported by the target.
      case UniOpVVV::kMinI64:
        if (!has_sse4_2()) {
          Vec msk = new_vec128("@msk");
          sse_cmp_gt_i64(*this, msk, src2, src1v);
          sse_select(*this, dst, src1v, src2, msk);
          return;
        }
        [[fallthrough]];

      case UniOpVVV::kMinI8:
      case UniOpVVV::kMinI32: {
        // Native operation requires SSE4.1, which is not supported by the target.
        InstId cmp_inst_id = op == UniOpVVV::kMinI8  ? Inst::kIdPcmpgtb :
                             op == UniOpVVV::kMinI32 ? Inst::kIdPcmpgtd : Inst::kIdPcmpgtq;
        Vec msk = new_vec128("@msk");
        cc->emit(Inst::kIdMovaps, msk, src2);
        cc->emit(cmp_inst_id, msk, src1v);
        sse_select(*this, dst, src1v, src2, msk);
        return;
      }

      case UniOpVVV::kMaxI64:
        // Native operation requires AVX512, which is not supported by the target.
        if (!has_sse4_2()) {
          Vec msk = new_vec128("@msk");
          sse_cmp_gt_i64(*this, msk, src1v, src2);
          sse_select(*this, dst, src1v, src2, msk);
          return;
        }
        [[fallthrough]];

      case UniOpVVV::kMaxI8:
      case UniOpVVV::kMaxI32: {
        // Native operation requires SSE4.1, which is not supported by the target.
        InstId cmp_inst_id = op == UniOpVVV::kMaxI8  ? Inst::kIdPcmpgtb :
                           op == UniOpVVV::kMaxI32 ? Inst::kIdPcmpgtd : Inst::kIdPcmpgtq;
        Vec msk = new_vec128("@msk");
        cc->emit(Inst::kIdMovaps, msk, src1v);
        cc->emit(cmp_inst_id, msk, src2);
        sse_select(*this, dst, src1v, src2, msk);
        return;
      }

      case UniOpVVV::kMinU16: {
        // Native operation requires SSE4.1, which is not supported by the target.
        Vec tmp = new_vec128("@tmp");
        cc->emit(Inst::kIdMovaps, tmp, src1v);
        cc->emit(Inst::kIdPsubusw, tmp, src2);
        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPsubw, dst, tmp);
        return;
      }

      case UniOpVVV::kMaxU16: {
        // Native operation requires SSE4.1, which is not supported by the target.
        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPsubusw, dst, src2);
        cc->emit(Inst::kIdPaddw, dst, src2);
        return;
      }

      case UniOpVVV::kMinU32:
      case UniOpVVV::kMaxU32: {
        // Native operation requires SSE4.1, which is not supported by the target.
        Operand flip_mask = simd_const(&ct().p_8000000080000000, Bcst::kNA, dst);
        Vec tmp1 = new_similar_reg(dst, "@tmp1");
        Vec tmp2 = new_similar_reg(dst, "@tmp2");

        if (op == UniOpVVV::kMinU32) {
          sse_mov(*this, tmp1, src2);
          sse_mov(*this, tmp2, src1v);
        }
        else {
          sse_mov(*this, tmp1, src1v);
          sse_mov(*this, tmp2, src2);
        }

        cc->emit(Inst::kIdPxor, tmp1, flip_mask);
        cc->emit(Inst::kIdPxor, tmp2, flip_mask);
        cc->emit(Inst::kIdPcmpgtd, tmp1, tmp2);

        sse_select(*this, dst, src1v, src2, tmp1);
        return;
      }

      case UniOpVVV::kMinU64: {
        // Native operation requires AVX512, which is not supported by the target.
        Vec msk = new_similar_reg(dst, "@tmp1");
        sse_cmp_gt_u64(*this, msk, src2, src1v);
        sse_select(*this, dst, src1v, src2, msk);
        return;
      }

      case UniOpVVV::kMaxU64: {
        // Native operation requires AVX512, which is not supported by the target.
        Vec msk = new_similar_reg(dst, "@tmp1");
        sse_cmp_gt_u64(*this, msk, src1v, src2);
        sse_select(*this, dst, src1v, src2, msk);
        return;
      }

      case UniOpVVV::kCmpEqU64: {
        // Native operation requires SSE4.1, which is not supported by the target.
        Vec tmp = new_similar_reg(dst, "@tmp");
        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPcmpeqd, dst, src2);
        cc->emit(Inst::kIdPshufd, tmp, dst, x86::shuffle_imm(2, 3, 0, 1));
        cc->emit(Inst::kIdPand, dst, tmp);
        return;
      }

      case UniOpVVV::kCmpGtI64: {
        // Native operation requires SSE4.2, which is not supported by the target.
        sse_cmp_gt_i64(*this, dst, src1v, src2);
        return;
      }

      case UniOpVVV::kCmpGtU8:
      case UniOpVVV::kCmpGtU16:
      case UniOpVVV::kCmpGtU32: {
        CmpMinMaxInst inst = sse_cmp_min_max[size_t(op) - size_t(UniOpVVV::kCmpGtI8)];

        if (has_sse4_1() || op == UniOpVVV::kCmpGtU8) {
          if (dst.id() == src1v.id()) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            cc->emit(Inst::kIdMovaps, tmp, src1v);
            cc->emit(inst.pmin, tmp, src2);
            cc->emit(inst.peq, dst, tmp);
          }
          else if (is_same_vec(dst, src2)) {
            cc->emit(inst.pmin, dst, src1v);
            cc->emit(inst.peq, dst, src1v);
          }
          else {
            cc->emit(Inst::kIdMovaps, dst, src1v);
            cc->emit(inst.pmin, dst, src2);
            cc->emit(inst.peq, dst, src1v);
          }

          sse_bit_not(*this, dst, dst);
          return;
        }

        Vec tmp = new_similar_reg(dst, "@tmp");
        sse_msb_flip(*this, tmp, src2, ElementSize(op_info.element_size));
        sse_msb_flip(*this, dst, src1v, ElementSize(op_info.element_size));
        cc->emit(inst.pgt, dst, tmp);
        return;
      }

      case UniOpVVV::kCmpGtU64: {
        // Native operation requires AVX512, which is not supported by the target.
        sse_cmp_gt_u64(*this, dst, src1v, src2);
        return;
      }

      case UniOpVVV::kCmpGeI8:
      case UniOpVVV::kCmpGeU8:
      case UniOpVVV::kCmpGeI16:
      case UniOpVVV::kCmpGeU16:
      case UniOpVVV::kCmpGeI32:
      case UniOpVVV::kCmpGeU32:
        // Native operation requires AVX512, which is not supported by the target.
        if (has_sse4_1() || op == UniOpVVV::kCmpGeU8 || op == UniOpVVV::kCmpGeI16) {
          CmpMinMaxInst inst = sse_cmp_min_max[size_t(op) - size_t(UniOpVVV::kCmpGeI8)];

          if (dst.id() == src1v.id()) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            cc->emit(Inst::kIdMovaps, tmp, src1v);
            cc->emit(inst.pmax, tmp, src2);
            cc->emit(inst.peq, dst, tmp);
          }
          else if (is_same_vec(dst, src2)) {
            cc->emit(inst.pmax, dst, src1v);
            cc->emit(inst.peq, dst, src1v);
          }
          else {
            cc->emit(Inst::kIdMovaps, dst, src1v);
            cc->emit(inst.pmax, dst, src2);
            cc->emit(inst.peq, dst, src1v);
          }
          return;
        }

        if (op == UniOpVVV::kCmpGeU16) {
          Vec tmp = new_similar_reg(dst, "@tmp");

          sse_mov(*this, tmp, src1v);
          cc->emit(Inst::kIdPsubusw, tmp, src2);
          cc->emit(Inst::kIdPaddw, tmp, src2);

          sse_mov(*this, dst, src1v);
          cc->emit(Inst::kIdPcmpeqw, dst, tmp);
          return;
        }
        [[fallthrough]];

      case UniOpVVV::kCmpGeI64:
      case UniOpVVV::kCmpGeU64:
        // Native operation requires AVX512, which is not supported by the target.
        if (src2.is_mem()) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, src2);
          src2 = tmp;
        }

        switch (op) {
          case UniOpVVV::kCmpGeI8: v_cmp_gt_i8(dst, src2, src1v); break;
          case UniOpVVV::kCmpGeI32: v_cmp_gt_i32(dst, src2, src1v); break;
          case UniOpVVV::kCmpGeU32: v_cmp_gt_u32(dst, src2, src1v); break;
          case UniOpVVV::kCmpGeI64: v_cmp_gt_i64(dst, src2, src1v); break;
          case UniOpVVV::kCmpGeU64: v_cmp_gt_u64(dst, src2, src1v); break;

          default:
            ASMJIT_NOT_REACHED();
        }

        sse_bit_not(*this, dst, dst);
        return;

      case UniOpVVV::kCmpLtI8:
      case UniOpVVV::kCmpLtI16:
      case UniOpVVV::kCmpLtI32: {
        if (is_same_vec(dst, src1v)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, src1v);
          src1v = tmp;
        }

        sse_mov(*this, dst, src2);
        cc->emit(inst_id, dst, src1v);
        return;
      }

      case UniOpVVV::kCmpLtU8:
      case UniOpVVV::kCmpLtU16:
      case UniOpVVV::kCmpLtU32: {
        Vec tmp = new_similar_reg(dst, "@tmp");
        sse_mov(*this, tmp, src1v);
        sse_msb_flip(*this, tmp, src1v, ElementSize(op_info.element_size));
        sse_msb_flip(*this, dst, src2, ElementSize(op_info.element_size));
        cc->emit(inst_id, dst, tmp);
        return;
      }

      case UniOpVVV::kCmpLtI64: {
        // Native operation requires AVX512, which is not supported by the target.
        sse_cmp_gt_i64(*this, dst, src2, src1v);
        return;
      }

      case UniOpVVV::kCmpLtU64: {
        // Native operation requires AVX512, which is not supported by the target.
        sse_cmp_gt_u64(*this, dst, src2, src1v);
        return;
      }

      case UniOpVVV::kCmpLeU8: {
        if (is_same_vec(dst, src2)) {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, src2);
          src2 = tmp;
        }

        sse_mov(*this, dst, src1v);
        cc->emit(Inst::kIdPsubusb, dst, src2);

        Vec zeros = simd_vec_const(&ct().p_0000000000000000, Bcst::k32, dst);
        cc->emit(Inst::kIdPcmpeqb, dst, zeros);
        return;
      }

      case UniOpVVV::kCmpLeI8:
      case UniOpVVV::kCmpLeI16:
      case UniOpVVV::kCmpLeU16:
      case UniOpVVV::kCmpLeI32:
      case UniOpVVV::kCmpLeU32:
        if (has_sse4_1() || op == UniOpVVV::kCmpLeU8 || op == UniOpVVV::kCmpLeI16) {
          CmpMinMaxInst inst = sse_cmp_min_max[size_t(op) - size_t(UniOpVVV::kCmpLeI8)];

          if (dst.id() == src1v.id()) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            cc->emit(Inst::kIdMovaps, tmp, src1v);
            cc->emit(inst.pmin, tmp, src2);
            cc->emit(inst.peq, dst, tmp);
          }
          else if (is_same_vec(dst, src2)) {
            cc->emit(inst.pmin, dst, src1v);
            cc->emit(inst.peq, dst, src1v);
          }
          else {
            cc->emit(Inst::kIdMovaps, dst, src1v);
            cc->emit(inst.pmin, dst, src2);
            cc->emit(inst.peq, dst, src1v);
          }
          return;
        }
        [[fallthrough]];

      case UniOpVVV::kCmpLeI64:
      case UniOpVVV::kCmpLeU64:
        switch (op) {
          case UniOpVVV::kCmpLeI8: v_cmp_gt_i8(dst, src1v, src2); break;
          case UniOpVVV::kCmpLeU16: v_cmp_gt_u16(dst, src1v, src2); break;
          case UniOpVVV::kCmpLeI32: v_cmp_gt_i32(dst, src1v, src2); break;
          case UniOpVVV::kCmpLeU32: v_cmp_gt_u32(dst, src1v, src2); break;
          case UniOpVVV::kCmpLeI64: v_cmp_gt_i64(dst, src1v, src2); break;
          case UniOpVVV::kCmpLeU64: v_cmp_gt_u64(dst, src1v, src2); break;

          default:
            ASMJIT_NOT_REACHED();
        }

        sse_bit_not(*this, dst, dst);
        return;

      case UniOpVVV::kCmpLtF32S:
      case UniOpVVV::kCmpLtF64S:
      case UniOpVVV::kCmpLtF32:
      case UniOpVVV::kCmpLtF64:
      case UniOpVVV::kCmpLeF32S:
      case UniOpVVV::kCmpLeF64S:
      case UniOpVVV::kCmpLeF32:
      case UniOpVVV::kCmpLeF64:
        if (is_same_vec(dst, src2)) {
          uint8_t pred = sse_fcmp_imm_table[(size_t(op) - size_t(UniOpVVV::kCmpEqF32S)) / 4u];

          // Unfortunately we have to do two moves, because there are no predicates that
          // we could use in case of reversed operands (AVX is much better in this regard).
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, src2);
          sse_mov(*this, dst, src1v);
          cc->emit(inst_id, dst, tmp, pred);
          return;
        }
        [[fallthrough]];

      case UniOpVVV::kCmpEqF32S:
      case UniOpVVV::kCmpEqF64S:
      case UniOpVVV::kCmpEqF32:
      case UniOpVVV::kCmpEqF64:
      case UniOpVVV::kCmpNeF32S:
      case UniOpVVV::kCmpNeF64S:
      case UniOpVVV::kCmpNeF32:
      case UniOpVVV::kCmpNeF64:
      case UniOpVVV::kCmpOrdF32S:
      case UniOpVVV::kCmpOrdF64S:
      case UniOpVVV::kCmpOrdF32:
      case UniOpVVV::kCmpOrdF64:
      case UniOpVVV::kCmpUnordF32S:
      case UniOpVVV::kCmpUnordF64S:
      case UniOpVVV::kCmpUnordF32:
      case UniOpVVV::kCmpUnordF64: {
        uint8_t pred = sse_fcmp_imm_table[(size_t(op) - size_t(UniOpVVV::kCmpEqF32S)) / 4u];
        sse_mov(*this, dst, src1v);
        cc->emit(inst_id, dst, src2, pred);
        return;
      }

      case UniOpVVV::kCmpGtF32S:
      case UniOpVVV::kCmpGtF64S:
      case UniOpVVV::kCmpGtF32:
      case UniOpVVV::kCmpGtF64:
      case UniOpVVV::kCmpGeF32S:
      case UniOpVVV::kCmpGeF64S:
      case UniOpVVV::kCmpGeF32:
      case UniOpVVV::kCmpGeF64: {
        // Since SSE compare doesn't provide these modes natively, we have to reverse the operands.
        uint8_t pred = sse_fcmp_imm_table[(size_t(op) - size_t(UniOpVVV::kCmpEqF32S)) / 4u];

        if (dst.id() != src1v.id()) {
          sse_mov(*this, dst, src2);
          cc->emit(inst_id, dst, src1v, pred);
        }
        else {
          Vec tmp = new_similar_reg(dst, "@tmp");
          sse_mov(*this, tmp, src2);
          cc->emit(inst_id, tmp, src1v, pred);
          sse_mov(*this, dst, tmp);
        }
        return;
      }

      case UniOpVVV::kHAddF64: {
        // Native operation requires SSE3, which is not supported by the target.
        if (is_same_vec(src1v, src2)) {
          if (is_same_vec(dst, src1v)) {
            Vec tmp = cc->new_similar_reg(dst, "@tmp");
            v_swap_f64(tmp, dst);
            cc->addpd(dst, tmp);
          }
          else {
            v_swap_f64(dst, src1v);
            cc->addpd(dst, src1v);
          }
        }
        else {
          // [B A]    [C A]
          // [D C] -> [D B]
          Vec tmp = new_similar_reg(dst, "@tmp");
          if (src2.is_mem()) {
            Mem m(src2.as<Mem>());

            sse_mov(*this, dst, src1v);
            v_swap_f64(tmp, dst);
            cc->movhpd(dst, m);

            m.add_offset(8);
            cc->movhpd(tmp, m);
            cc->addpd(dst, tmp);
          }
          else if (is_same_vec(dst, src2)) {
            sse_mov(*this, tmp, src1v);
            cc->unpcklpd(tmp, src2.as<Vec>());
            cc->movhlps(dst, src1v);
            cc->addpd(dst, tmp);
          }
          else {
            sse_mov(*this, tmp, src1v);
            cc->unpckhpd(tmp, src2.as<Vec>());

            sse_mov(*this, dst, src1v);
            cc->unpcklpd(dst, src2.as<Vec>());

            cc->addpd(dst, tmp.as<Vec>());
          }
        }
        return;
      }

      case UniOpVVV::kCombineLoHiU64:
      case UniOpVVV::kCombineLoHiF64: {
        // Intrinsic - dst = {src1.u64[0], src2.64[1]} - combining low part of src1 and high part of src1.
        if (src2.is_mem()) {
          Mem m = src2.as<Mem>().clone_adjusted(8);
          cc->emit(Inst::kIdPshufd, dst, src1v, x86::shuffle_imm(1, 0, 1, 0));
          cc->emit(Inst::kIdMovlpd, dst, m);
          return;
        }

        if (is_same_vec(dst, src2)) {
          // dst = {src1.u64[0], dst.u64[1]}
          cc->emit(Inst::kIdShufpd, dst, src1v, x86::shuffle_imm(0, 1));
          return;
        }
        else if (is_same_vec(dst, src1v)) {
          // dst = {dst.u64[0], src2.u64[1]}
          if (has_ssse3()) {
            cc->emit(Inst::kIdPalignr, dst, src2, 8);
            return;
          }
        }

        if (has_sse3())
          cc->emit(Inst::kIdMovddup, dst, src1v);
        else
          cc->emit(Inst::kIdPshufd, dst, src1v, x86::shuffle_imm(1, 0, 1, 0));

        cc->emit(Inst::kIdMovhlps, dst, src2);
        return;
      }

      case UniOpVVV::kCombineHiLoU64:
      case UniOpVVV::kCombineHiLoF64: {
        // Intrinsic - dst = {src1.u64[1], src2.64[0]} - combining high part of src1 and low part of src2.
        if (src2.is_mem()) {
          sse_mov(*this, dst, src1v);
          cc->emit(Inst::kIdMovlpd, dst, src2);
        }
        else if (is_same_vec(dst, src2)) {
          // dst = {src1.u64[1], dst.u64[0]}
          cc->emit(Inst::kIdShufpd, dst, src1v, 0x2);
        }
        else {
          // dst = {src1.u64[1], src2.u64[0]}
          sse_mov(*this, dst, src1v);
          cc->emit(Inst::kIdMovsd, dst, src2);
        }
        return;
      }

      case UniOpVVV::kPacksI32_U16: {
        // Native operation requires SSE4.1, which is not supported by the target.

        // NOTE: This one is generally tricky and involves a lot of operations. There are hacks available to shorten the
        // sequence, but then it would not cover all the inputs, so this is essentially a code necessary to handle all of
        // them. The trick here is to perform unsigned saturation first (that's why we fill one reg with MSB bits of the
        // input and then use ANDN), and then to bias the input in a way to make the result use signed saturation. The
        // last step is to convert the biased value back.
        //
        // In general, if you hit this code-path (not having SSE4.1 and still needing exactly this instruction) I would
        // recommend using a different strategy in this case, completely avoiding this code path. Usually, inputs are not
        // arbitrary and knowing the range could help a lot to reduce the approach to use a native 'packssdw' instruction.
        Operand bias = simd_const(&ct().p_0000800000008000, Bcst::kNA, dst);
        Operand unbias = simd_const(&ct().p_8000800080008000, Bcst::kNA, dst);

        if (is_same_vec(src1v, src2)) {
          Vec tmp = dst;
          if (is_same_vec(dst, src1v))
            tmp = new_similar_reg(dst, "@tmp1");

          sse_mov(*this, tmp, src1v);

          cc->emit(Inst::kIdPsrad, tmp, 31);
          cc->emit(Inst::kIdPandn, tmp, src1v);
          cc->emit(Inst::kIdPsubd, tmp, bias);
          cc->emit(Inst::kIdPackssdw, tmp, tmp);
          cc->emit(Inst::kIdPaddw, tmp, unbias);

          sse_mov(*this, dst, tmp);
        }
        else {
          Vec tmp1 = new_similar_reg(dst, "@tmp1");
          Vec tmp2 = new_similar_reg(dst, "@tmp2");

          sse_mov(*this, tmp1, src1v);
          sse_mov(*this, tmp2, src2);

          cc->emit(Inst::kIdPsrad, tmp1, 31);
          cc->emit(Inst::kIdPsrad, tmp2, 31);
          cc->emit(Inst::kIdPandn, tmp1, src1v);
          cc->emit(Inst::kIdPandn, tmp2, src2);
          cc->emit(Inst::kIdPsubd, tmp1, bias);
          cc->emit(Inst::kIdPsubd, tmp2, bias);
          cc->emit(Inst::kIdPackssdw, tmp1, tmp2);
          cc->emit(Inst::kIdPaddw, tmp1, unbias);

          sse_mov(*this, dst, tmp1);
        }
        return;
      }

      case UniOpVVV::kSwizzlev_U8: {
        // Native operation requires SSSE3, which is not supported by the target.
        //
        // NOTE: This is basically a very slow emulation as there is no way how to implement this operation with SSE2 SIMD.
        Mem m_data = tmp_stack(StackId::kCustom, 64);
        Mem m_pred = m_data.clone_adjusted(32);

        m_data.set_size(1);
        m_pred.set_size(1);

        cc->movaps(m_data, src1v);

        // The trick is to AND all indexes by 0x0F and then to do unsigned minimum so all indexes are in [0, 17) range,
        // where index 16 maps to zero.
        Vec tmp = new_similar_reg(dst, "@tmp");
        cc->vmovaps(tmp, simd_mem_const(&ct().p_0F0F0F0F0F0F0F0F, Bcst::kNA, tmp));
        cc->pand(tmp, src2.as<Vec>());
        cc->pminub(tmp, simd_mem_const(&ct().p_1010101010101010, Bcst::kNA, tmp));
        cc->movaps(m_pred, tmp);
        cc->mov(m_data.clone_adjusted(16), 0);

        Gp acc = new_gpz("@acc");
        Gp idx = new_gpz("@idx");

        // Process 2 bytes at a time, then use PINSRW to merge them with the destination.
        for (uint32_t i = 0; i < 8; i++) {
          cc->movzx(acc.r32(), m_pred); m_pred.add_offset(1);
          cc->movzx(idx.r32(), m_pred); m_pred.add_offset(1);

          m_data.set_index(acc);
          cc->movzx(acc, m_data);

          m_data.set_index(idx);
          cc->mov(acc.r8_hi(), m_data);

          if (i == 0)
            cc->movd(dst, acc.r32());
          else
            cc->pinsrw(dst, acc.r32(), i);
        }

        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_3v(UniOpVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_) { emit_3v_t(*this, op, dst_, src1_, src2_); }
void UniCompiler::emit_3v(UniOpVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_) { emit_3v_t(*this, op, dst_, src1_, src2_); }
void UniCompiler::emit_3v(UniOpVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_) { emit_3v_t(*this, op, dst_, src1_, src2_); }

// ujit::UniCompiler - Vector Instructions - Emit 3VI
// ==================================================

void UniCompiler::emit_3vi(UniOpVVVI op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_, uint32_t imm) {
  ASMJIT_ASSERT(dst_.is_vec());
  ASMJIT_ASSERT(src1_.is_vec());

  Vec dst(dst_.as<Vec>());
  Vec src1v(src1_.as<Vec>().clone_as(dst));
  Operand src2(src2_);
  UniOpVInfo op_info = opcode_info_3vi[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    InstId inst_id = op_info.avx_inst_id;

    if (has_avx_ext(AVXExt(op_info.avx_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      cc->emit(inst_id, dst, src1v, src2, imm);
      return;
    }

    switch (op) {
      // Intrin - short-circuit if possible based on the predicate.
      case UniOpVVVI::kAlignr_U128: {
        if (imm == 0) {
          avx_mov(*this, dst, src2);
          return;
        }

        if (is_same_vec(src1v, src2)) {
          if (imm == 4 || imm == 8 || imm == 12) {
            uint32_t pred = imm ==  4 ? x86::shuffle_imm(0, 3, 2, 1) :
                            imm ==  8 ? x86::shuffle_imm(1, 0, 3, 2) :
                            imm == 12 ? x86::shuffle_imm(2, 1, 0, 3) : 0;
            cc->vpshufd(dst, src1v, pred);
            return;
          }
        }

        cc->emit(Inst::kIdVpalignr, dst, src1v, src2, imm);
        return;
      }

      // Intrin - maps directly to the corresponding instruction, but imm must be converted.
      case UniOpVVVI::kInterleaveShuffleU32x4:
      case UniOpVVVI::kInterleaveShuffleF32x4: {
        if (is_same_vec(src1v, src2)) {
          UniOpVVI simplified_op = (op == UniOpVVVI::kInterleaveShuffleU32x4) ? UniOpVVI::kSwizzleU32x4 : UniOpVVI::kSwizzleF32x4;
          emit_2vi(simplified_op, dst, src1v, imm);
        }
        else {
          uint32_t shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
          cc->emit(inst_id, dst, src1v, src2, shuf_imm);
        }
        return;
      }

      // Intrin - maps directly to the corresponding instruction, but imm must be converted.
      case UniOpVVVI::kInterleaveShuffleU64x2:
      case UniOpVVVI::kInterleaveShuffleF64x2: {
        if (is_same_vec(src1v, src2)) {
          UniOpVVI simplified_op = (op == UniOpVVVI::kInterleaveShuffleU64x2) ? UniOpVVI::kSwizzleU64x2 : UniOpVVI::kSwizzleF64x2;
          emit_2vi(simplified_op, dst, src1v, imm);
        }
        else {
          uint32_t shuf_imm = shuf_imm2_from_swizzle_with_width(Swizzle2{imm}, VecWidthUtils::vec_width_of(dst));
          cc->emit(inst_id, dst, src1v, src2, shuf_imm);
        }
        return;
      }

      case UniOpVVVI::kInsertV128_U32:
      case UniOpVVVI::kInsertV128_F32:
      case UniOpVVVI::kInsertV128_U64:
      case UniOpVVVI::kInsertV128_F64: {
        src1v.set_signature(dst.signature());

        if (src2.is_mem())
          src2.as<Mem>().set_size(16);
        else
          src2.set_signature(signature_of_xmm_ymm_zmm[0]);

        if (!has_avx512()) {
          if (has_avx2() && (op == UniOpVVVI::kInsertV128_U32 || op == UniOpVVVI::kInsertV128_U64))
            inst_id = Inst::kIdVinserti128;
          else
            inst_id = Inst::kIdVinsertf128;
        }

        cc->emit(inst_id, dst, src1v, src2, imm);
        return;
      }

      case UniOpVVVI::kInsertV256_U32:
      case UniOpVVVI::kInsertV256_F32:
      case UniOpVVVI::kInsertV256_U64:
      case UniOpVVVI::kInsertV256_F64: {
        ASMJIT_ASSERT(has_avx512());
        src1v.set_signature(dst.signature());

        if (src2.is_mem())
          src2.as<Mem>().set_size(32);
        else
          src2.set_signature(signature_of_xmm_ymm_zmm[1]);

        cc->emit(inst_id, dst, src1v, src2, imm);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    InstId inst_id = op_info.sse_inst_id;

    if (is_same_vec(dst, src2) && op_info.commutative) {
      std::swap(src1v, src2.as<Vec>());
    }

    // All operations are intrinsics in this case - no direct mapping to instructions without an additional logic.
    ASMJIT_ASSERT(!has_sse_ext(SSEExt(op_info.sse_ext)));

    switch (op) {
      // Intrin - short-circuit if possible based on the predicate.
      case UniOpVVVI::kAlignr_U128: {
        if (imm == 0) {
          sse_mov(*this, dst, src2);
          return;
        }

        if (is_same_vec(src1v, src2)) {
          if (imm == 4 || imm == 8 || imm == 12) {
            uint32_t pred = imm ==  4 ? x86::shuffle_imm(0, 3, 2, 1) :
                            imm ==  8 ? x86::shuffle_imm(1, 0, 3, 2) :
                            imm == 12 ? x86::shuffle_imm(2, 1, 0, 3) : 0;
            cc->emit(Inst::kIdPshufd, dst, src1v, pred);
            return;
          }
        }

        if (has_ssse3()) {
          if (is_same_vec(dst, src2) && !is_same_vec(dst, src1v)) {
            Vec tmp = new_similar_reg(dst, "@tmp");
            sse_mov(*this, tmp, src2);
            src2 = tmp;
          }

          sse_mov(*this, dst, src1v);
          cc->emit(Inst::kIdPalignr, dst, src2, imm);
          return;
        }

        Vec tmp = new_similar_reg(dst, "@tmp");
        uint32_t src1_shift = (16u - imm) & 15;
        uint32_t src2_shift = imm;

        if (is_same_vec(dst, src1v)) {
          sse_mov(*this, tmp, src2);
          cc->emit(Inst::kIdPsrldq, tmp, src2_shift);
          cc->emit(Inst::kIdPslldq, dst, src1_shift);
        }
        else {
          sse_mov(*this, tmp, src1v);
          sse_mov(*this, dst, src2);
          cc->emit(Inst::kIdPslldq, tmp, src1_shift);
          cc->emit(Inst::kIdPsrldq, dst, src2_shift);
        }

        cc->emit(Inst::kIdPor, dst, tmp);
        return;
      }

      // Intrin - maps directly to the corresponding instruction, but imm must be converted.
      case UniOpVVVI::kInterleaveShuffleU32x4:
      case UniOpVVVI::kInterleaveShuffleU64x2:
      case UniOpVVVI::kInterleaveShuffleF32x4:
      case UniOpVVVI::kInterleaveShuffleF64x2: {
        uint32_t shuf_imm;
        ElementSize element_size = ElementSize(op_info.element_size);

        if (element_size == ElementSize::k32)
          shuf_imm = shuf_imm4_from_swizzle(Swizzle4{imm});
        else
          shuf_imm = shuf_imm2_from_swizzle(Swizzle2{imm});

        if (is_same_vec(src1v, src2)) {
          UniOpVVI vvi_op =  translate_op(op, UniOpVVVI::kInterleaveShuffleU32x4, UniOpVVI::kSwizzleU32x4);
          emit_2vi(vvi_op, dst, src1v, imm);
          return;
        }

        else if (is_same_vec(dst, src1v)) {
          cc->emit(inst_id, dst, src2, shuf_imm);
        }
        else if (is_same_vec(dst, src2)) {
          // The predicate has to be reversed as we want to swap low/high 64-bit lanes afterwards.
          if (element_size == ElementSize::k32)
            shuf_imm = (shuf_imm >> 4) | ((shuf_imm & 0xF) << 4);
          else
            shuf_imm = (shuf_imm >> 1) | ((shuf_imm & 0x1) << 1);

          cc->emit(inst_id, dst, src1v, shuf_imm);
          cc->emit(Inst::kIdPshufd, dst, dst, x86::shuffle_imm(1, 0, 3, 2));
        }
        else {
          sse_mov(*this, dst, src1v);
          cc->emit(inst_id, dst, src2, shuf_imm);
        }
        return;
      }

      case UniOpVVVI::kInsertV128_U32:
      case UniOpVVVI::kInsertV128_F32:
      case UniOpVVVI::kInsertV128_U64:
      case UniOpVVVI::kInsertV128_F64:
      case UniOpVVVI::kInsertV256_U32:
      case UniOpVVVI::kInsertV256_F32:
      case UniOpVVVI::kInsertV256_U64:
      case UniOpVVVI::kInsertV256_F64:
        // These are not available in SSE mode (256-bit vectors require AVX)
        ASMJIT_NOT_REACHED();

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_3vi(UniOpVVVI op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, uint32_t imm) { emit_3vi_t(*this, op, dst_, src1_, src2_, imm); }
void UniCompiler::emit_3vi(UniOpVVVI op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, uint32_t imm) { emit_3vi_t(*this, op, dst_, src1_, src2_, imm); }
void UniCompiler::emit_3vi(UniOpVVVI op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, uint32_t imm) { emit_3vi_t(*this, op, dst_, src1_, src2_, imm); }

// ujit::UniCompiler - Vector Instructions - Emit 4V
// =================================================

void UniCompiler::emit_4v(UniOpVVVV op, const Operand_& dst_, const Operand_& src1_, const Operand_& src2_, const Operand_& src3_) {
  ASMJIT_ASSERT(dst_.is_vec());
  ASMJIT_ASSERT(src1_.is_vec());

  Vec dst(dst_.as<Vec>());
  Vec src1(src1_.as<Vec>().clone_as(dst));
  Operand src2(src2_);
  Operand src3(src3_);
  UniOpVInfo op_info = opcode_info_4v[size_t(op)];

  if (has_avx()) {
    // AVX Implementation
    // ------------------

    InstId inst_id = op_info.avx_inst_id;

    if (is_same_vec(dst, src2) && op_info.commutative) {
      std::swap(src1, src2.as<Vec>());
    }

    if (has_avx_ext(AVXExt(op_info.avx_ext))) {
      ASMJIT_ASSERT(inst_id != Inst::kIdNone);

      cc->emit(inst_id, dst, src1, src2, src3);
      return;
    }

    switch (op) {
      case UniOpVVVV::kBlendV_U8: {
        // Blend(a, b, cond) == (a & ~cond) | (b & cond)
        avx_make_vec(*this, src3, dst, "msk");
        cc->emit(op_info.avx_inst_id, dst, src1, src2, src3);
        return;
      }

      case UniOpVVVV::kMAddU16:
      case UniOpVVVV::kMAddU32: {
        static constexpr uint16_t add_inst_table[2] = {
          Inst::kIdVpaddw,
          Inst::kIdVpaddd
        };

        Vec tmp = dst;
        if (is_same_vec(dst, src3)) {
          tmp = new_similar_reg(dst, "@tmp");
        }

        InstId add_inst_id = add_inst_table[size_t(op) - size_t(UniOpVVVV::kMAddU16)];

        cc->emit(inst_id, tmp, src1, src2);
        cc->emit(add_inst_id, dst, tmp, src3);

        return;
      }

      case UniOpVVVV::kMAddF32S:
      case UniOpVVVV::kMAddF64S:
      case UniOpVVVV::kMAddF32:
      case UniOpVVVV::kMAddF64:
      case UniOpVVVV::kMSubF32S:
      case UniOpVVVV::kMSubF64S:
      case UniOpVVVV::kMSubF32:
      case UniOpVVVV::kMSubF64:
      case UniOpVVVV::kNMAddF32S:
      case UniOpVVVV::kNMAddF64S:
      case UniOpVVVV::kNMAddF32:
      case UniOpVVVV::kNMAddF64:
      case UniOpVVVV::kNMSubF32S:
      case UniOpVVVV::kNMSubF64S:
      case UniOpVVVV::kNMSubF32:
      case UniOpVVVV::kNMSubF64: {
        // 4 operand operation:
        //
        //   madd(dst, a, b, c) -> dst = a * b + c
        //   msub(dst, a, b, c) -> dst = a * b - c
        //   nmadd(dst, a, b, c) -> dst = -a * b + c
        //   nmsub(dst, a, b, c) -> dst = -a * b - c
        //
        // 3 operand operation (FMA):
        //
        //   vfmadd213  a, b, c -> a =  a * b + c
        //   vfmadd132  a, b, c -> a =  a * c + b
        //   vfmadd231  a, b, c -> a =  b * c + a
        //   vfnmadd213 a, b, c -> a = -a * b + c
        //   vfnmadd132 a, b, c -> a = -a * c + b
        //   vfnmadd231 a, b, c -> a = -b * c + a
        //   vfsubd213  a, b, c -> a =  a * b - c
        //   vfsubd132  a, b, c -> a =  a * c - b
        //   vfsubd231  a, b, c -> a =  b * c - a
        //   vfnsubd213 a, b, c -> a = -a * b - c
        //   vfnsubd132 a, b, c -> a = -a * c - b
        //   vfnsubd231 a, b, c -> a = -b * c - a
        size_t fma_id = size_t(op) - size_t(UniOpVVVV::kMAddF32S);
        FloatMode fm = FloatMode(op_info.float_mode);

        if (is_scalar_fp_op(fm)) {
          dst.set_signature(signature_of_xmm_ymm_zmm[0]);
          src1.set_signature(signature_of_xmm_ymm_zmm[0]);

          if (src2.is_vec())
            src2.set_signature(signature_of_xmm_ymm_zmm[0]);

          if (src3.is_vec())
            src3.set_signature(signature_of_xmm_ymm_zmm[0]);
        }

        if (has_fma()) {
          // There is a variation of instructions, which can be used, but each has only 3 operands. Since we
          // allow 4 operands (having a separate desgination) we have to map our 4 operand representation to
          // 3 operand representation as used by FMA.

          static constexpr uint16_t fma_ab_add_c[16] = {
            Inst::kIdVfmadd213ss , Inst::kIdVfmadd213sd , Inst::kIdVfmadd213ps , Inst::kIdVfmadd213pd ,
            Inst::kIdVfmsub213ss , Inst::kIdVfmsub213sd , Inst::kIdVfmsub213ps , Inst::kIdVfmsub213pd ,
            Inst::kIdVfnmadd213ss, Inst::kIdVfnmadd213sd, Inst::kIdVfnmadd213ps, Inst::kIdVfnmadd213pd,
            Inst::kIdVfnmsub213ss, Inst::kIdVfnmsub213sd, Inst::kIdVfnmsub213ps, Inst::kIdVfnmsub213pd
           };

          static constexpr uint16_t fma_ac_add_b[16] = {
            Inst::kIdVfmadd132ss , Inst::kIdVfmadd132sd , Inst::kIdVfmadd132ps , Inst::kIdVfmadd132pd ,
            Inst::kIdVfmsub132ss , Inst::kIdVfmsub132sd , Inst::kIdVfmsub132ps , Inst::kIdVfmsub132pd ,
            Inst::kIdVfnmadd132ss, Inst::kIdVfnmadd132sd, Inst::kIdVfnmadd132ps, Inst::kIdVfnmadd132pd,
            Inst::kIdVfnmsub132ss, Inst::kIdVfnmsub132sd, Inst::kIdVfnmsub132ps, Inst::kIdVfnmsub132pd
          };

          static constexpr uint16_t fma_bc_add_a[16] = {
            Inst::kIdVfmadd231ss , Inst::kIdVfmadd231sd , Inst::kIdVfmadd231ps , Inst::kIdVfmadd231pd ,
            Inst::kIdVfmsub231ss , Inst::kIdVfmsub231sd , Inst::kIdVfmsub231ps , Inst::kIdVfmsub231pd ,
            Inst::kIdVfnmadd231ss, Inst::kIdVfnmadd231sd, Inst::kIdVfnmadd231ps, Inst::kIdVfnmadd231pd,
            Inst::kIdVfnmsub231ss, Inst::kIdVfnmsub231sd, Inst::kIdVfnmsub231ps, Inst::kIdVfnmsub231pd
          };

          if (is_same_vec(dst, src1)) {
            if (src2.is_reg())
              cc->emit(fma_ab_add_c[fma_id], dst, src2, src3);
            else
              cc->emit(fma_ac_add_b[fma_id], dst, src3, src2);
          }
          else if (is_same_vec(dst, src2)) {
            cc->emit(fma_ab_add_c[fma_id], dst, src1, src3);
          }
          else if (is_same_vec(dst, src3)) {
            cc->emit(fma_bc_add_a[fma_id], dst, src1, src2);
          }
          else {
            avx_mov(*this, dst, src1);
            if (!src2.is_reg())
              cc->emit(fma_ac_add_b[fma_id], dst, src3, src2);
            else if (!src3.is_reg())
              cc->emit(fma_ab_add_c[fma_id], dst, src1, src3);
            else
              cc->emit(fma_ab_add_c[fma_id], dst, src2, src3);
          }
          return;
        }
        else {
          // MAdd/MSub - native FMA not available so we have to do MUL followed by either ADD or SUB.
          const FloatInst& fi = avx_float_inst[size_t(fm)];

          bool mul_add = (op_info.imm & 0x01u) == 0u;
          bool neg_mul = (op_info.imm & 0x02u) != 0u;
          InstId fi_facc = mul_add ? fi.fadd : fi.fsub;

          if (!neg_mul) {
            // MAdd or MSub Operation.
            if (is_same_vec(dst, src3)) {
              Vec tmp = new_similar_reg(dst, "@tmp");
              cc->emit(fi.fmul, tmp, src1, src2);
              cc->emit(fi_facc, dst, tmp, src3);
            }
            else {
              cc->emit(fi.fmul, dst, src1, src2);
              cc->emit(fi_facc, dst, dst, src3);
            }
          }
          else {
            // NMAdd or NMSub Operation.
            Vec tmp = new_similar_reg(dst, "@tmp");
            avx_fsign_flip(*this, tmp, src1, fm);

            cc->emit(fi.fmul, tmp, tmp, src2);
            cc->emit(fi_facc, dst, tmp, src3);
          }
          return;
        }
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
  else {
    // SSE Implementation
    // ------------------

    switch (op) {
      case UniOpVVVV::kBlendV_U8: {
        // Blend(a, b, cond) == (a & ~cond) | (b & cond)
        if (has_sse4_1()) {
          if (is_same_vec(dst, src1) || (!is_same_vec(dst, src2) && !is_same_vec(dst, src3))) {
            sse_make_vec(*this, src3, "tmp");
            sse_mov(*this, dst, src1);
            cc->emit(op_info.sse_inst_id, dst, src2, src3);
            return;
          }
        }

        // Blend(a, b, cond) == a ^ ((a ^ b) &  cond)
        //                   == b ^ ((a ^ b) & ~cond)
        if (is_same_vec(dst, src1)) {
          Vec tmp = new_vec128("@tmp");
          v_xor_i32(tmp, dst, src2);
          v_and_i32(tmp, tmp, src3);
          v_xor_i32(dst, dst, tmp);
        }
        else if (is_same_vec(dst, src3)) {
          Vec tmp = new_vec128("@tmp");
          v_xor_i32(tmp, src1, src2);
          v_andn_i32(dst, dst, tmp);
          v_xor_i32(dst, dst, src2);
        }
        else {
          v_xor_i32(dst, src2, src1);
          v_and_i32(dst, dst, src3);
          v_xor_i32(dst, dst, src1);
        }
        return;
      }

      case UniOpVVVV::kMAddU16:
      case UniOpVVVV::kMAddU32: {
        Vec tmp = dst;
        if (is_same_vec(dst, src3)) {
          tmp = new_similar_reg(dst, "@tmp");
        }

        if (op == UniOpVVVV::kMAddU16) {
          v_mul_u16(tmp, src1, src2);
          v_add_u16(dst, tmp, src3);
        }
        else {
          v_mul_u32(tmp, src1, src2);
          v_add_u32(dst, tmp, src3);
        }

        return;
      }

      case UniOpVVVV::kMAddF32S:
      case UniOpVVVV::kMAddF64S:
      case UniOpVVVV::kMSubF32S:
      case UniOpVVVV::kMSubF64S:
      case UniOpVVVV::kMAddF32:
      case UniOpVVVV::kMAddF64:
      case UniOpVVVV::kMSubF32:
      case UniOpVVVV::kMSubF64:
      case UniOpVVVV::kNMAddF32S:
      case UniOpVVVV::kNMAddF64S:
      case UniOpVVVV::kNMSubF32S:
      case UniOpVVVV::kNMSubF64S:
      case UniOpVVVV::kNMAddF32:
      case UniOpVVVV::kNMAddF64:
      case UniOpVVVV::kNMSubF32:
      case UniOpVVVV::kNMSubF64: {
        FloatMode fm = FloatMode(op_info.float_mode);

        bool mul_add = (op_info.imm & 0x01u) == 0u;
        bool neg_mul = (op_info.imm & 0x02u) != 0u;

        if (is_same_vec(dst, src2)) {
          // Unfortunately, to follow the FMA behavior in scalar case, we have to copy.
          if (fm <= FloatMode::kF64S)
            src2 = sse_copy(*this, src2.as<Vec>(), "@copy_src2");
          else
            std::swap(src1, src2.as<Vec>());
        }

        const FloatInst& fi = sse_float_inst[size_t(fm)];
        InstId fi_facc = mul_add ? fi.fadd : fi.fsub;

        if (is_same_vec(dst, src3)) {
          if (fm <= FloatMode::kF64S || !mul_add) {
            // Copy if we couldn't avoid the extra move.
            src3 = sse_copy(*this, src3.as<Vec>(), "@copy_src3");
          }
          else {
            Vec tmp = cc->new_similar_reg(dst, "@tmp");
            sse_mov(*this, tmp, src1);
            cc->emit(fi.fmul, tmp, src2);
            cc->emit(neg_mul ? fi.fsub : fi.fadd, dst, tmp);
            return;
          }
        }

        if (neg_mul)
          sse_fsign_flip(*this, dst, src1, fm);
        else
          sse_mov(*this, dst, src1);

        cc->emit(fi.fmul, dst, src2);
        cc->emit(fi_facc, dst, src3);
        return;
      }

      default:
        ASMJIT_NOT_REACHED();
    }
  }
}

void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const Operand_& src2_, const OpArray& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, const Operand& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const Operand_& src1_, const OpArray& src2_, const OpArray& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, const Operand& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const Operand_& src2_, const OpArray& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, const Operand& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }
void UniCompiler::emit_4v(UniOpVVVV op, const OpArray& dst_, const OpArray& src1_, const OpArray& src2_, const OpArray& src3_) { emit_4v_t(*this, op, dst_, src1_, src2_, src3_); }

ASMJIT_END_SUB_NAMESPACE

#endif
