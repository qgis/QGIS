// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_COMPILERDEFS_H_INCLUDED
#define ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

#include <asmjit/core/api-config.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/support/arenastring.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

class RAWorkReg;

//! \addtogroup asmjit_compiler
//! \{

//! Flags associated with a virtual register \ref VirtReg.
enum class VirtRegFlags : uint8_t {
  kNone = 0x00u,

  //! True if this is a fixed register, never reallocated.
  kIsFixed = 0x01u,

  //! True if the virtual register is only used as a stack area (never accessed as register). Stack area is allocated
  //! via \ref BaseCompiler::_new_stack() and then architecture dependent compilers like \ref x86::Compiler::new_stack().
  kIsStackArea = 0x02u,

  //! True if the virtual register has a stack slot.
  //!
  //! Stack slots are assigned by the register allocator - so initially when a \ref VirtReg is created this flag would
  //! not be set. When a virtual register is spilled, stack slot is automatically created for the register and the
  //! \ref VirtReg::_stack_offset member is updated. Stack areas will always have associated stack slot during register
  //! allocation.
  kHasStackSlot = 0x04u,

  //! Virtual register `log2(alignment)` mask (for spilling) (3 bits in flags).
  //!
  //! \note For space purposes the alignment is stored as log2(alignment). So the alignment is `1 << log2(alignment)`.
  kAlignmentLog2Mask = 0xE0u
};
ASMJIT_DEFINE_ENUM_FLAGS(VirtRegFlags)

//! Public virtual register interface, managed by \ref BaseCompiler.
//!
//! When a virtual register is created by \ref BaseCompiler a `VirtReg` is linked with the register operand id it
//! returns. This `VirtReg` can be accessed via \ref BaseCompiler::virt_reg_by_reg() function, which returns a pointer
//! to `VirtReg`.
//!
//! In general, `VirtReg` should be only introspected as it contains important variables that are needed and managed
//! by AsmJit, however, the `VirtReg` API can also be used to influence register allocation. For example there is
//! a \ref VirtReg::set_weight() function, which could be used to increase a weight of a virtual register (thus make
//! it hard to spill, for example). In addition, there is a \ref VirtReg::set_home_id_hint() function, which can be used
//! to do an initial assignment of a physical register of a virtual register. However, AsmJit could still override
//! the physical register assigned in some special cases.
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)

  //! \name Constants
  //! \{

  static constexpr inline uint32_t kAlignmentLog2Mask  = uint32_t(VirtRegFlags::kAlignmentLog2Mask);
  static constexpr inline uint32_t kAlignmentLog2Shift = Support::ctz_const<kAlignmentLog2Mask>;

  static ASMJIT_INLINE_CONSTEXPR VirtRegFlags _flags_from_alignment_log2(uint32_t alignment_log2) noexcept {
    return VirtRegFlags(alignment_log2 << kAlignmentLog2Shift);
  }

  static ASMJIT_INLINE_CONSTEXPR uint32_t _alignment_log2_from_flags(VirtRegFlags flags) noexcept {
    return uint32_t(flags) >> kAlignmentLog2Shift;
  }

  //! \}

  //! \name Members
  //! \{

  //! Virtual register id.
  uint32_t _id = 0;
  //! Virtual register size (can be smaller than a real register size if only a part of the register is used).
  uint32_t _virt_size = 0;

  //! Virtual register type.
  RegType _reg_type = RegType::kNone;
  //! Virtual register flags.
  VirtRegFlags _reg_flags = VirtRegFlags::kNone;

  //! Virtual register weight.
  //!
  //! Weight is used for alloc/spill decisions. Higher weight means a higher priority to keep the virtual
  //! register always allocated as a physical register. The default weight is zero, which means standard
  //! weight (no weight is added to the initial priority, which is calculated based on the number of uses
  //! divided by the sum of widths of all live spans).
  uint8_t _weight = 0;

  //! Type id.
  TypeId _type_id = TypeId::kVoid;

  //! Home register hint for the register allocator (initially unassigned).
  uint8_t _home_id_hint = Reg::kIdBad;

  //! Stack offset assigned by the register allocator relative to stack pointer (can be negative as well).
  int32_t _stack_offset = 0;

  //! Virtual register name (either empty or user provided).
  ArenaString<16> _name {};

  // The following members are used exclusively by RAPass. They are initialized when the VirtReg is created to
  // null pointers and then changed during RAPass execution. RAPass sets them back to NULL before it returns.

  //! Reference to `RAWorkReg`, used during register allocation.
  RAWorkReg* _work_reg = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG VirtReg(RegType reg_type, VirtRegFlags reg_flags, uint32_t id, uint32_t virt_size, TypeId type_id) noexcept
    : _id(id),
      _virt_size(virt_size),
      _reg_type(reg_type),
      _reg_flags(reg_flags),
      _type_id(type_id) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the virtual register id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }

  //! Returns a virtual register type (maps to the physical register type as well).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RegType reg_type() const noexcept { return _reg_type; }

  //! Returns a virtual register flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtRegFlags reg_flags() const noexcept { return _reg_flags; }

  //! Returns the virtual register size.
  //!
  //! The virtual register size describes how many bytes the virtual register needs to store its content. It can be
  //! smaller than the physical register size, see `register_size()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t virt_size() const noexcept { return _virt_size; }

  //! Returns the virtual register alignment required for memory operations (load/spill).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return 1u << _alignment_log2_from_flags(_reg_flags); }

  //! Returns the virtual register type id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG TypeId type_id() const noexcept { return _type_id; }

  //! Returns the virtual register weight - the register allocator can use it as explicit hint for alloc/spill
  //! decisions.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t weight() const noexcept { return _weight; }

  //! Sets the virtual register weight (0 to 255) - the register allocator can use it as explicit hint for alloc/spill
  //! decisions and initial bin-packing.
  ASMJIT_INLINE_NODEBUG void set_weight(uint32_t weight) noexcept { _weight = uint8_t(weight); }

  //! Returns whether the virtual register is always allocated to a fixed physical register (and never reallocated).
  //!
  //! \note This is only used for special purposes and it's mostly internal.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_fixed() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kIsFixed); }

  //! Tests whether the virtual register is in fact a stack that only uses the virtual register id.
  //!
  //! \note It's an error if a stack is accessed as a register.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_stack_area() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kIsStackArea); }

  //! Tests whether this virtual register (or stack) has assigned a stack offset.
  //!
  //! If this is a virtual register that was never allocated on stack, it would return false, otherwise if
  //! it's a virtual register that was spilled or explicitly allocated stack, the return value would be true.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_stack_slot() const noexcept { return Support::test(_reg_flags, VirtRegFlags::kHasStackSlot); }

  //! Assigns a stack offset of this virtual register to `stack_offset` and adds `VirtRegFlags::kHasStackSlot` flag.
  ASMJIT_INLINE_NODEBUG void assign_stack_slot(int32_t stack_offset) noexcept {
    _reg_flags |= VirtRegFlags::kHasStackSlot;
    _stack_offset = stack_offset;
  }

  //! Tests whether this virtual register has assigned a physical register as a hint to the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_home_id_hint() const noexcept { return _home_id_hint != Reg::kIdBad; }

  //! Returns a physical register hint, which will be used by the register allocator.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t home_id_hint() const noexcept { return _home_id_hint; }

  //! Assigns a physical register hint, which will be used by the register allocator.
  ASMJIT_INLINE_NODEBUG void set_home_id_hint(uint32_t home_id) noexcept { _home_id_hint = uint8_t(home_id); }
  //! Resets a physical register hint.
  ASMJIT_INLINE_NODEBUG void reset_home_id_hint() noexcept { _home_id_hint = Reg::kIdBad; }

  //! Returns a stack offset associated with a virtual register or explicit stack allocation.
  //!
  //! \note Always verify that the stack offset has been assigned by calling \ref has_stack_slot(). The return
  //! value will be zero when the stack offset was not assigned.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t stack_offset() const noexcept { return _stack_offset; }

  //! Returns the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.data(); }

  //! Returns the size of the virtual register name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t name_size() const noexcept { return _name.size(); }

  //! Tests whether the virtual register has an associated `RAWorkReg` at the moment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_work_reg() const noexcept { return _work_reg != nullptr; }

  //! Returns an associated RAWorkReg with this virtual register (only valid during register allocation).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RAWorkReg* work_reg() const noexcept { return _work_reg; }

  //! Associates a RAWorkReg with this virtual register (used by register allocator).
  ASMJIT_INLINE_NODEBUG void set_work_reg(RAWorkReg* work_reg) noexcept { _work_reg = work_reg; }

  //! Reset the RAWorkReg association (used by register allocator).
  ASMJIT_INLINE_NODEBUG void reset_work_reg() noexcept { _work_reg = nullptr; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_COMPILERDEFS_H_INCLUDED

