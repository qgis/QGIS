// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED
#define ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/environment.h>
#include <asmjit/core/func.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/raconstraints_p.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

static inline OperandSignature get_suitable_reg_for_mem_to_mem_move(Arch arch, TypeId dst_type_id, TypeId src_type_id) noexcept {
  const ArchTraits& arch_traits = ArchTraits::by_arch(arch);

  uint32_t signature = 0u;
  uint32_t dst_size = TypeUtils::size_of(dst_type_id);
  uint32_t src_size = TypeUtils::size_of(src_type_id);
  uint32_t max_size = Support::max<uint32_t>(dst_size, src_size);
  uint32_t reg_size = Environment::reg_size_of_arch(arch);

  if (max_size <= reg_size || (TypeUtils::is_int(dst_type_id) && TypeUtils::is_int(src_type_id))) {
    signature = max_size <= 4 ? RegTraits<RegType::kGp32>::kSignature
                              : RegTraits<RegType::kGp64>::kSignature;
  }
  else if (max_size <= 8 && arch_traits.has_reg_type(RegType::kVec64)) {
    signature = RegTraits<RegType::kVec64>::kSignature;
  }
  else if (max_size <= 16 && arch_traits.has_reg_type(RegType::kVec128)) {
    signature = RegTraits<RegType::kVec128>::kSignature;
  }
  else if (max_size <= 32 && arch_traits.has_reg_type(RegType::kVec256)) {
    signature = RegTraits<RegType::kVec256>::kSignature;
  }
  else if (max_size <= 64 && arch_traits.has_reg_type(RegType::kVec512)) {
    signature = RegTraits<RegType::kVec512>::kSignature;
  }

  return OperandSignature{signature};
}

class FuncArgsContext {
public:
  static inline constexpr uint32_t kVarIdNone = 0xFF;

  //! Contains information about a single argument or SA register that may need shuffling.
  struct Var {
    FuncValue cur;
    FuncValue out;

    inline void init(const FuncValue& cur_, const FuncValue& out_) noexcept {
      cur = cur_;
      out = out_;
    }

    //! Reset the value to its unassigned state.
    inline void reset() noexcept {
      cur.reset();
      out.reset();
    }

    ASMJIT_INLINE_NODEBUG bool is_done() const noexcept { return cur.is_done(); }
    ASMJIT_INLINE_NODEBUG void mark_done() noexcept { cur.add_flags(FuncValue::kFlagIsDone); }
  };

  struct WorkData {
    //! All allocable registers provided by the architecture.
    RegMask _arch_regs;
    //! All registers that can be used by the shuffler.
    RegMask _work_regs;
    //! Registers used by the shuffler (all).
    RegMask _used_regs;
    //! Assigned registers.
    RegMask _assigned_regs;
    //! Destination registers assigned to arguments or SA.
    RegMask _dst_regs;
    //! Destination registers that require shuffling.
    RegMask _dst_shuf;
    //! Number of register swaps.
    uint8_t _num_swaps;
    //! Number of stack loads.
    uint8_t _num_stack_args;
    //! Whether this work data would need reassignment.
    uint8_t _needs_scratch;
    //! Reserved (only used as padding).
    uint8_t _reserved[5];
    //! Physical ID to variable ID mapping.
    uint8_t _phys_to_var_id[32];

    inline void reset() noexcept {
      _arch_regs = 0;
      _work_regs = 0;
      _used_regs = 0;
      _assigned_regs = 0;
      _dst_regs = 0;
      _dst_shuf = 0;
      _num_swaps = 0;
      _num_stack_args = 0;
      _needs_scratch = 0;
      memset(_reserved, 0, sizeof(_reserved));
      memset(_phys_to_var_id, kVarIdNone, 32);
    }

    [[nodiscard]]
    inline bool is_assigned(uint32_t reg_id) const noexcept {
      ASMJIT_ASSERT(reg_id < 32);
      return Support::bit_test(_assigned_regs, reg_id);
    }

    inline void assign(uint32_t var_id, uint32_t reg_id) noexcept {
      ASMJIT_ASSERT(!is_assigned(reg_id));
      ASMJIT_ASSERT(_phys_to_var_id[reg_id] == kVarIdNone);

      _phys_to_var_id[reg_id] = uint8_t(var_id);
      _assigned_regs ^= Support::bit_mask<RegMask>(reg_id);
    }

    inline void reassign(uint32_t var_id, uint32_t new_id, uint32_t old_id) noexcept {
      ASMJIT_ASSERT( is_assigned(old_id));
      ASMJIT_ASSERT(!is_assigned(new_id));
      ASMJIT_ASSERT(_phys_to_var_id[old_id] == var_id);
      ASMJIT_ASSERT(_phys_to_var_id[new_id] == kVarIdNone);

      _phys_to_var_id[old_id] = uint8_t(kVarIdNone);
      _phys_to_var_id[new_id] = uint8_t(var_id);
      _assigned_regs ^= Support::bit_mask<RegMask>(new_id) ^ Support::bit_mask<RegMask>(old_id);
    }

    inline void swap(uint32_t a_var_id, uint32_t a_reg_id, uint32_t b_var_id, uint32_t b_reg_id) noexcept {
      ASMJIT_ASSERT(is_assigned(a_reg_id));
      ASMJIT_ASSERT(is_assigned(b_reg_id));
      ASMJIT_ASSERT(_phys_to_var_id[a_reg_id] == a_var_id);
      ASMJIT_ASSERT(_phys_to_var_id[b_reg_id] == b_var_id);

      _phys_to_var_id[a_reg_id] = uint8_t(b_var_id);
      _phys_to_var_id[b_reg_id] = uint8_t(a_var_id);
    }

    inline void unassign(uint32_t var_id, uint32_t reg_id) noexcept {
      ASMJIT_ASSERT(is_assigned(reg_id));
      ASMJIT_ASSERT(_phys_to_var_id[reg_id] == var_id);

      Support::maybe_unused(var_id);
      _phys_to_var_id[reg_id] = uint8_t(kVarIdNone);
      _assigned_regs ^= Support::bit_mask<RegMask>(reg_id);
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask arch_regs() const noexcept { return _arch_regs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask work_regs() const noexcept { return _work_regs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask used_regs() const noexcept { return _used_regs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask assigned_regs() const noexcept { return _assigned_regs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask dst_regs() const noexcept { return _dst_regs; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG RegMask available_regs() const noexcept { return _work_regs & ~_assigned_regs; }
  };

  //! Architecture traits.
  const ArchTraits* _arch_traits = nullptr;
  //! Architecture constraints.
  const RAConstraints* _constraints = nullptr;
  //! Target architecture.
  Arch _arch = Arch::kUnknown;
  //! Has arguments passed via stack (SRC).
  bool _has_stack_src = false;
  //! Has preserved frame-pointer (FP).
  bool _has_preserved_fp = false;
  //! Has arguments assigned to stack (DST).
  uint8_t _stack_dst_mask = 0;
  //! Register swap groups (bit-mask).
  uint8_t _reg_swaps_mask = 0;
  uint8_t _sa_var_id = kVarIdNone;
  uint32_t _var_count = 0;
  Support::Array<WorkData, Globals::kNumVirtGroups> _work_data;
  Var _vars[Globals::kMaxFuncArgs * Globals::kMaxValuePack + 1];

  FuncArgsContext() noexcept;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const ArchTraits& arch_traits() const noexcept { return *_arch_traits; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return _arch; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t var_count() const noexcept { return _var_count; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t index_of(const Var* var) const noexcept { return (size_t)(var - _vars); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Var& var(size_t var_id) noexcept { return _vars[var_id]; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Var& var(size_t var_id) const noexcept { return _vars[var_id]; }

  Error init_work_data(const FuncFrame& frame, const FuncArgsAssignment& args, const RAConstraints* constraints) noexcept;
  Error mark_scratch_regs(FuncFrame& frame) noexcept;
  Error mark_dst_regs_dirty(FuncFrame& frame) noexcept;
  Error mark_stack_args_reg(FuncFrame& frame) noexcept;
};

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FUNCARGSCONTEXT_P_H_INCLUDED
