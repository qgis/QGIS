// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86OPERAND_H_INCLUDED
#define ASMJIT_X86_X86OPERAND_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>
#include <asmjit/x86/x86globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

class Mem;
class Gp;
class Vec;
class Mm;
class KReg;
class SReg;
class CReg;
class DReg;
class St;
class Bnd;
class Tmm;
class Rip;

//! General purpose register (X86|X86_64).
//!
//! To get a specific register you can use:
//!   - specific registers directly, like `x86::eax` or `x86::rbx`, etc...
//!   - construct a register operand dynamically, like `x86::gp8(id)`, `x86::gp64(id)`, etc...
//!   - use `Gp::make_r[8|8lo|8hi|16|32|64](id)` API for convenience
//!
//! To cast a register to a specific type, use \ref Gp::r8(), \ref Gp::r8_lo(), \ref Gp::r8_hi(), \ref Gp::r16(),
//! \ref Gp::r32(), and \ref Gp::r64() member functions. Each cast first clones the register and then changes
//! its signature to match the register it has been casted to.
class Gp : public UniGp {
public:
  //! \name Constants
  //! \{

  //! Physical id (X86|X86_64).
  //!
  //! \note Register indexes have been reduced to only support general purpose registers. There is no need to
  //! have enumerations with number suffix that expands to the exactly same value as the suffix value itself.
  enum Id : uint32_t {
    kIdAx  = 0,  //!< Physical id of AL|AH|AX|EAX|RAX registers.
    kIdCx  = 1,  //!< Physical id of CL|CH|CX|ECX|RCX registers.
    kIdDx  = 2,  //!< Physical id of DL|DH|DX|EDX|RDX registers.
    kIdBx  = 3,  //!< Physical id of BL|BH|BX|EBX|RBX registers.
    kIdSp  = 4,  //!< Physical id of SPL|SP|ESP|RSP registers.
    kIdBp  = 5,  //!< Physical id of BPL|BP|EBP|RBP registers.
    kIdSi  = 6,  //!< Physical id of SIL|SI|ESI|RSI registers.
    kIdDi  = 7,  //!< Physical id of DIL|DI|EDI|RDI registers.
    kIdR8  = 8,  //!< Physical id of R8B|R8W|R8D|R8 registers (X86_64).
    kIdR9  = 9,  //!< Physical id of R9B|R9W|R9D|R9 registers (X86_64).
    kIdR10 = 10, //!< Physical id of R10B|R10W|R10D|R10 registers (X86_64).
    kIdR11 = 11, //!< Physical id of R11B|R11W|R11D|R11 registers (X86_64).
    kIdR12 = 12, //!< Physical id of R12B|R12W|R12D|R12 registers (X86_64).
    kIdR13 = 13, //!< Physical id of R13B|R13W|R13D|R13 registers (X86_64).
    kIdR14 = 14, //!< Physical id of R14B|R14W|R14D|R14 registers (X86_64).
    kIdR15 = 15  //!< Physical id of R15B|R15W|R15D|R15 registers (X86_64).
  };

  //! \}

  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)

  //! \name Static Constructors
  //! \{

  //! Creates a new 8-bit low general purpose register (GPB) having the given register id `reg_id`.
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Lo>(), reg_id); }
  [[nodiscard]]

  //! Creates a new 8-bit low general purpose register (GPB) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8_lo(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Lo>(), reg_id); }

  //! Creates a new 8-bit high general purpose register (GPB) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r8_hi(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp8Hi>(), reg_id); }

  //! Creates a new 16-bit low general purpose register (GPB) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r16(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp16>(), reg_id); }

  //! Creates a new 32-bit low general purpose register (GPB) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r32(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp32>(), reg_id); }

  //! Creates a new 64-bit low general purpose register (GPB) having the given register id `reg_id` (X86_64).
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r64(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp64>(), reg_id); }

  //! \}

  //! \name Gp Register Accessors
  //! \{

  //! Clones and casts this register to 8-bit (LO) part.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8() const noexcept { return make_r8(id()); }

  //! Clones and casts this register to 8-bit (LO) part.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8_lo() const noexcept { return make_r8_lo(id()); }

  //! Clones and casts this register to 8-bit (HI) part.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r8_hi() const noexcept { return make_r8_hi(id()); }

  //! Clones and casts this register to 16-bit.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r16() const noexcept { return make_r16(id()); }

  //! Clones and casts this register to 32-bit.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r32() const noexcept { return make_r32(id()); }

  //! Clones and casts this register to 64-bit.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r64() const noexcept { return make_r64(id()); }

  //! \}
};

//! Vector register (XMM|YMM|ZMM) (X86|X86_64).
//!
//! To get a specific register you can use:
//!   - specific registers directly, like `x86::xmm0` or `x86::ymm1`, `x86::zmm2`, etc...
//!   - construct a register operand dynamically, like `x86::xmm(id)`, `x86::ymm(id)`, and `x86::zmm(id)`
//!   - use `Vec::make_v[128|256|512](id)` or `Vec::make_[xmm|ymm|zmm](id)` API for convenience
//!
//! To cast a register to a specific type, use \ref Vec::v128(), \ref Vec::v256(), \ref Vec::v512(), \ref Vec::xmm(),
//! \ref Vec::ymm(), and \ref Vec::zmm() member functions. Each cast first clones the register and then changes its
//! signature to match the register it has been casted to.
class Vec : public UniVec {
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  //! \name Static Constructors
  //! \{

  //! Creates a new 128-bit vector register (XMM) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }

  //! Creates a new 256-bit vector register (YMM) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v256(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec256>(), reg_id); }

  //! Creates a new 512-bit vector register (ZMM) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v512(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec512>(), reg_id); }

  //! Creates a new 128-bit vector register (XMM) having the given register id `reg_id`.
  //!
  //! \note This is an architecture-specific naming that does the same as \ref make_v128().
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_xmm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }

  //! Creates a new 256-bit vector register (YMM) having the given register id `reg_id`.
  //!
  //! \note This is an architecture-specific naming that does the same as \ref make_v256().
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_ymm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec256>(), reg_id); }

  //! Creates a new 512-bit vector register (ZMM) having the given register id `reg_id`.
  //!
  //! \note This is an architecture-specific naming that does the same as \ref make_v512().
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_zmm(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec512>(), reg_id); }

  //! \}

  //! \name Vec Register Accessors
  //! \{

  //! Tests whether the register is a 128-bit vector (XMM).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref Reg::is_vec128().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_xmm() const noexcept { return is_vec128(); }

  //! Tests whether the register is a 256-bit vector (YMM).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref Reg::is_vec256().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_ymm() const noexcept { return is_vec256(); }

  //! Tests whether the register is a 512-bit vector (ZMM).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref Reg::is_vec512().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_zmm() const noexcept { return is_vec512(); }

  //! Clones and casts this register to XMM (Vec128).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v128() const noexcept { return make_v128(id()); }

  //! Clones and casts this register to YMM (Vec256).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v256() const noexcept { return make_v256(id()); }

  //! Clones and casts this register to ZMM (Vec512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v512() const noexcept { return make_v512(id()); }

  //! Clones and casts this register to XMM (Vec128).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref v128().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec xmm() const noexcept { return make_v128(id()); }

  //! Clones and casts this register to YMM (Vec256).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref v256().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec ymm() const noexcept { return make_v256(id()); }

  //! Clones and casts this register to ZMM (Vec512).
  //!
  //! \note This is an architecture-specific naming that does the same as \ref v512().
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec zmm() const noexcept { return make_v512(id()); }

  //! Clones and casts this register to a register that has half the size (or XMM if it's already XMM).
  //!
  //! \note AsmJit defines vector registers of various sizes to target multiple architectures, however, this
  //! function never returns a vector register that would have less than 128 bits as X86|X86_64 architecture
  //! doesn't provide such registers. So, effectively this function returns either YMM register if the input
  //! was ZMM, or XMM for whatever else input.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec half() const noexcept {
    return Vec(is_vec512() ? signature_of_t<RegType::kVec256>() : signature_of_t<RegType::kVec512>(), id());
  }

  //! \}
};

//! Segment register (X86|X86_64).
class SReg : public Reg {
  ASMJIT_DEFINE_FINAL_REG(SReg, Reg, RegTraits<RegType::kSegment>)

  //! X86 segment id.
  enum Id : uint32_t {
    //! No segment (default).
    kIdNone = 0,
    //! ES segment.
    kIdEs = 1,
    //! CS segment.
    kIdCs = 2,
    //! SS segment.
    kIdSs = 3,
    //! DS segment.
    kIdDs = 4,
    //! FS segment.
    kIdFs = 5,
    //! GS segment.
    kIdGs = 6,

    //! Count of X86 segment registers supported by AsmJit.
    //!
    //! \note X86 architecture has 6 segment registers - ES, CS, SS, DS, FS, GS. X64 architecture lowers them down to
    //! just FS and GS. AsmJit supports 7 segment registers - all addressable in both X86 and X64 modes and one extra
    //! called `SReg::kIdNone`, which is AsmJit specific and means that there is no segment register specified.
    kIdCount = 7
  };
};

//! 64-bit K register (AVX512+).
class KReg : public Reg { ASMJIT_DEFINE_FINAL_REG(KReg, Reg, RegTraits<RegType::kMask>) };
//! 64-bit MMX register (MMX+).
class Mm : public Reg { ASMJIT_DEFINE_FINAL_REG(Mm, Reg, RegTraits<RegType::kX86_Mm>) };
//! 32-bit or 64-bit control register (X86).
class CReg : public Reg { ASMJIT_DEFINE_FINAL_REG(CReg, Reg, RegTraits<RegType::kControl>) };
//! 32-bit or 64-bit debug register (X86).
class DReg : public Reg { ASMJIT_DEFINE_FINAL_REG(DReg, Reg, RegTraits<RegType::kDebug>) };
//! 80-bit FPU register (X86).
class St : public Reg { ASMJIT_DEFINE_FINAL_REG(St, Reg, RegTraits<RegType::kX86_St>) };
//! 128-bit BND register (BND+).
class Bnd : public Reg { ASMJIT_DEFINE_FINAL_REG(Bnd, Reg, RegTraits<RegType::kX86_Bnd>) };
//! 8192-bit TMM register (AMX).
class Tmm : public Reg { ASMJIT_DEFINE_FINAL_REG(Tmm, Reg, RegTraits<RegType::kTile>) };
//! RIP register (X86).
class Rip : public Reg { ASMJIT_DEFINE_FINAL_REG(Rip, Reg, RegTraits<RegType::kPC>) };

//! \namespace asmjit::x86::regs
//!
//! Registers provided by X86 and X64 ISAs are in both `asmjit::x86` and `asmjit::x86::regs` namespaces so they can
//! be included with using directive. For example `using namespace asmjit::x86::regs` would include all registers,
//! but not other X86-specific API, whereas `using namespace asmjit::x86` would include everything X86-specific.
#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates an 8-bit low GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb(uint32_t reg_id) noexcept { return Gp::make_r8(reg_id); }

//! Creates an 8-bit low GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8(uint32_t reg_id) noexcept { return Gp::make_r8(reg_id); }

//! Creates an 8-bit low GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb_lo(uint32_t reg_id) noexcept { return Gp::make_r8_lo(reg_id); }

//! Creates an 8-bit low GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8_lo(uint32_t reg_id) noexcept { return Gp::make_r8_lo(reg_id); }

//! Creates an 8-bit high GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpb_hi(uint32_t reg_id) noexcept { return Gp::make_r8_hi(reg_id); }

//! Creates an 8-bit high GPB register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp8_hi(uint32_t reg_id) noexcept { return Gp::make_r8_hi(reg_id); }

//! Creates a 16-bit GPW register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpw(uint32_t reg_id) noexcept { return Gp::make_r16(reg_id); }

//! Creates a 16-bit GPW register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp16(uint32_t reg_id) noexcept { return Gp::make_r16(reg_id); }

//! Creates a 32-bit GPD register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpd(uint32_t reg_id) noexcept { return Gp::make_r32(reg_id); }

//! Creates a 32-bit GPD register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp32(uint32_t reg_id) noexcept { return Gp::make_r32(reg_id); }

//! Creates a 64-bit GPQ register operand (64-bit).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gpq(uint32_t reg_id) noexcept { return Gp::make_r64(reg_id); }

//! Creates a 64-bit GPQ register operand (64-bit).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp64(uint32_t reg_id) noexcept { return Gp::make_r64(reg_id); }

//! Creates a 128-bit XMM register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec xmm(uint32_t reg_id) noexcept { return Vec::make_v128(reg_id); }

//! Creates a 256-bit YMM register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec ymm(uint32_t reg_id) noexcept { return Vec::make_v256(reg_id); }

//! Creates a 512-bit ZMM register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec zmm(uint32_t reg_id) noexcept { return Vec::make_v512(reg_id); }

//! Creates a 64-bit K register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR KReg k(uint32_t reg_id) noexcept { return KReg(reg_id); }

//! Creates a 64-bit MM register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mm mm(uint32_t reg_id) noexcept { return Mm(reg_id); }

//! Creates a 32-bit or 64-bit control register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR CReg cr(uint32_t reg_id) noexcept { return CReg(reg_id); }

//! Creates a 32-bit or 64-bit debug register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR DReg dr(uint32_t reg_id) noexcept { return DReg(reg_id); }

//! Creates an 80-bit st register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR St st(uint32_t reg_id) noexcept { return St(reg_id); }

//! Creates a 128-bit bound register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Bnd bnd(uint32_t reg_id) noexcept { return Bnd(reg_id); }

//! Creates a TMM register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Tmm tmm(uint32_t reg_id) noexcept { return Tmm(reg_id); }

static constexpr Gp al = Gp::make_r8(Gp::kIdAx);
static constexpr Gp bl = Gp::make_r8(Gp::kIdBx);
static constexpr Gp cl = Gp::make_r8(Gp::kIdCx);
static constexpr Gp dl = Gp::make_r8(Gp::kIdDx);
static constexpr Gp spl = Gp::make_r8(Gp::kIdSp);
static constexpr Gp bpl = Gp::make_r8(Gp::kIdBp);
static constexpr Gp sil = Gp::make_r8(Gp::kIdSi);
static constexpr Gp dil = Gp::make_r8(Gp::kIdDi);
static constexpr Gp r8b = Gp::make_r8(Gp::kIdR8);
static constexpr Gp r9b = Gp::make_r8(Gp::kIdR9);
static constexpr Gp r10b = Gp::make_r8(Gp::kIdR10);
static constexpr Gp r11b = Gp::make_r8(Gp::kIdR11);
static constexpr Gp r12b = Gp::make_r8(Gp::kIdR12);
static constexpr Gp r13b = Gp::make_r8(Gp::kIdR13);
static constexpr Gp r14b = Gp::make_r8(Gp::kIdR14);
static constexpr Gp r15b = Gp::make_r8(Gp::kIdR15);

static constexpr Gp ah = Gp::make_r8_hi(Gp::kIdAx);
static constexpr Gp bh = Gp::make_r8_hi(Gp::kIdBx);
static constexpr Gp ch = Gp::make_r8_hi(Gp::kIdCx);
static constexpr Gp dh = Gp::make_r8_hi(Gp::kIdDx);

static constexpr Gp ax = Gp::make_r16(Gp::kIdAx);
static constexpr Gp bx = Gp::make_r16(Gp::kIdBx);
static constexpr Gp cx = Gp::make_r16(Gp::kIdCx);
static constexpr Gp dx = Gp::make_r16(Gp::kIdDx);
static constexpr Gp sp = Gp::make_r16(Gp::kIdSp);
static constexpr Gp bp = Gp::make_r16(Gp::kIdBp);
static constexpr Gp si = Gp::make_r16(Gp::kIdSi);
static constexpr Gp di = Gp::make_r16(Gp::kIdDi);
static constexpr Gp r8w = Gp::make_r16(Gp::kIdR8);
static constexpr Gp r9w = Gp::make_r16(Gp::kIdR9);
static constexpr Gp r10w = Gp::make_r16(Gp::kIdR10);
static constexpr Gp r11w = Gp::make_r16(Gp::kIdR11);
static constexpr Gp r12w = Gp::make_r16(Gp::kIdR12);
static constexpr Gp r13w = Gp::make_r16(Gp::kIdR13);
static constexpr Gp r14w = Gp::make_r16(Gp::kIdR14);
static constexpr Gp r15w = Gp::make_r16(Gp::kIdR15);

static constexpr Gp eax = Gp::make_r32(Gp::kIdAx);
static constexpr Gp ebx = Gp::make_r32(Gp::kIdBx);
static constexpr Gp ecx = Gp::make_r32(Gp::kIdCx);
static constexpr Gp edx = Gp::make_r32(Gp::kIdDx);
static constexpr Gp esp = Gp::make_r32(Gp::kIdSp);
static constexpr Gp ebp = Gp::make_r32(Gp::kIdBp);
static constexpr Gp esi = Gp::make_r32(Gp::kIdSi);
static constexpr Gp edi = Gp::make_r32(Gp::kIdDi);
static constexpr Gp r8d = Gp::make_r32(Gp::kIdR8);
static constexpr Gp r9d = Gp::make_r32(Gp::kIdR9);
static constexpr Gp r10d = Gp::make_r32(Gp::kIdR10);
static constexpr Gp r11d = Gp::make_r32(Gp::kIdR11);
static constexpr Gp r12d = Gp::make_r32(Gp::kIdR12);
static constexpr Gp r13d = Gp::make_r32(Gp::kIdR13);
static constexpr Gp r14d = Gp::make_r32(Gp::kIdR14);
static constexpr Gp r15d = Gp::make_r32(Gp::kIdR15);

static constexpr Gp rax = Gp::make_r64(Gp::kIdAx);
static constexpr Gp rbx = Gp::make_r64(Gp::kIdBx);
static constexpr Gp rcx = Gp::make_r64(Gp::kIdCx);
static constexpr Gp rdx = Gp::make_r64(Gp::kIdDx);
static constexpr Gp rsp = Gp::make_r64(Gp::kIdSp);
static constexpr Gp rbp = Gp::make_r64(Gp::kIdBp);
static constexpr Gp rsi = Gp::make_r64(Gp::kIdSi);
static constexpr Gp rdi = Gp::make_r64(Gp::kIdDi);
static constexpr Gp r8 = Gp::make_r64(Gp::kIdR8);
static constexpr Gp r9 = Gp::make_r64(Gp::kIdR9);
static constexpr Gp r10 = Gp::make_r64(Gp::kIdR10);
static constexpr Gp r11 = Gp::make_r64(Gp::kIdR11);
static constexpr Gp r12 = Gp::make_r64(Gp::kIdR12);
static constexpr Gp r13 = Gp::make_r64(Gp::kIdR13);
static constexpr Gp r14 = Gp::make_r64(Gp::kIdR14);
static constexpr Gp r15 = Gp::make_r64(Gp::kIdR15);

static constexpr Vec xmm0 = Vec::make_v128(0);
static constexpr Vec xmm1 = Vec::make_v128(1);
static constexpr Vec xmm2 = Vec::make_v128(2);
static constexpr Vec xmm3 = Vec::make_v128(3);
static constexpr Vec xmm4 = Vec::make_v128(4);
static constexpr Vec xmm5 = Vec::make_v128(5);
static constexpr Vec xmm6 = Vec::make_v128(6);
static constexpr Vec xmm7 = Vec::make_v128(7);
static constexpr Vec xmm8 = Vec::make_v128(8);
static constexpr Vec xmm9 = Vec::make_v128(9);
static constexpr Vec xmm10 = Vec::make_v128(10);
static constexpr Vec xmm11 = Vec::make_v128(11);
static constexpr Vec xmm12 = Vec::make_v128(12);
static constexpr Vec xmm13 = Vec::make_v128(13);
static constexpr Vec xmm14 = Vec::make_v128(14);
static constexpr Vec xmm15 = Vec::make_v128(15);
static constexpr Vec xmm16 = Vec::make_v128(16);
static constexpr Vec xmm17 = Vec::make_v128(17);
static constexpr Vec xmm18 = Vec::make_v128(18);
static constexpr Vec xmm19 = Vec::make_v128(19);
static constexpr Vec xmm20 = Vec::make_v128(20);
static constexpr Vec xmm21 = Vec::make_v128(21);
static constexpr Vec xmm22 = Vec::make_v128(22);
static constexpr Vec xmm23 = Vec::make_v128(23);
static constexpr Vec xmm24 = Vec::make_v128(24);
static constexpr Vec xmm25 = Vec::make_v128(25);
static constexpr Vec xmm26 = Vec::make_v128(26);
static constexpr Vec xmm27 = Vec::make_v128(27);
static constexpr Vec xmm28 = Vec::make_v128(28);
static constexpr Vec xmm29 = Vec::make_v128(29);
static constexpr Vec xmm30 = Vec::make_v128(30);
static constexpr Vec xmm31 = Vec::make_v128(31);

static constexpr Vec ymm0 = Vec::make_v256(0);
static constexpr Vec ymm1 = Vec::make_v256(1);
static constexpr Vec ymm2 = Vec::make_v256(2);
static constexpr Vec ymm3 = Vec::make_v256(3);
static constexpr Vec ymm4 = Vec::make_v256(4);
static constexpr Vec ymm5 = Vec::make_v256(5);
static constexpr Vec ymm6 = Vec::make_v256(6);
static constexpr Vec ymm7 = Vec::make_v256(7);
static constexpr Vec ymm8 = Vec::make_v256(8);
static constexpr Vec ymm9 = Vec::make_v256(9);
static constexpr Vec ymm10 = Vec::make_v256(10);
static constexpr Vec ymm11 = Vec::make_v256(11);
static constexpr Vec ymm12 = Vec::make_v256(12);
static constexpr Vec ymm13 = Vec::make_v256(13);
static constexpr Vec ymm14 = Vec::make_v256(14);
static constexpr Vec ymm15 = Vec::make_v256(15);
static constexpr Vec ymm16 = Vec::make_v256(16);
static constexpr Vec ymm17 = Vec::make_v256(17);
static constexpr Vec ymm18 = Vec::make_v256(18);
static constexpr Vec ymm19 = Vec::make_v256(19);
static constexpr Vec ymm20 = Vec::make_v256(20);
static constexpr Vec ymm21 = Vec::make_v256(21);
static constexpr Vec ymm22 = Vec::make_v256(22);
static constexpr Vec ymm23 = Vec::make_v256(23);
static constexpr Vec ymm24 = Vec::make_v256(24);
static constexpr Vec ymm25 = Vec::make_v256(25);
static constexpr Vec ymm26 = Vec::make_v256(26);
static constexpr Vec ymm27 = Vec::make_v256(27);
static constexpr Vec ymm28 = Vec::make_v256(28);
static constexpr Vec ymm29 = Vec::make_v256(29);
static constexpr Vec ymm30 = Vec::make_v256(30);
static constexpr Vec ymm31 = Vec::make_v256(31);

static constexpr Vec zmm0 = Vec::make_v512(0);
static constexpr Vec zmm1 = Vec::make_v512(1);
static constexpr Vec zmm2 = Vec::make_v512(2);
static constexpr Vec zmm3 = Vec::make_v512(3);
static constexpr Vec zmm4 = Vec::make_v512(4);
static constexpr Vec zmm5 = Vec::make_v512(5);
static constexpr Vec zmm6 = Vec::make_v512(6);
static constexpr Vec zmm7 = Vec::make_v512(7);
static constexpr Vec zmm8 = Vec::make_v512(8);
static constexpr Vec zmm9 = Vec::make_v512(9);
static constexpr Vec zmm10 = Vec::make_v512(10);
static constexpr Vec zmm11 = Vec::make_v512(11);
static constexpr Vec zmm12 = Vec::make_v512(12);
static constexpr Vec zmm13 = Vec::make_v512(13);
static constexpr Vec zmm14 = Vec::make_v512(14);
static constexpr Vec zmm15 = Vec::make_v512(15);
static constexpr Vec zmm16 = Vec::make_v512(16);
static constexpr Vec zmm17 = Vec::make_v512(17);
static constexpr Vec zmm18 = Vec::make_v512(18);
static constexpr Vec zmm19 = Vec::make_v512(19);
static constexpr Vec zmm20 = Vec::make_v512(20);
static constexpr Vec zmm21 = Vec::make_v512(21);
static constexpr Vec zmm22 = Vec::make_v512(22);
static constexpr Vec zmm23 = Vec::make_v512(23);
static constexpr Vec zmm24 = Vec::make_v512(24);
static constexpr Vec zmm25 = Vec::make_v512(25);
static constexpr Vec zmm26 = Vec::make_v512(26);
static constexpr Vec zmm27 = Vec::make_v512(27);
static constexpr Vec zmm28 = Vec::make_v512(28);
static constexpr Vec zmm29 = Vec::make_v512(29);
static constexpr Vec zmm30 = Vec::make_v512(30);
static constexpr Vec zmm31 = Vec::make_v512(31);

static constexpr Mm mm0 = Mm(0);
static constexpr Mm mm1 = Mm(1);
static constexpr Mm mm2 = Mm(2);
static constexpr Mm mm3 = Mm(3);
static constexpr Mm mm4 = Mm(4);
static constexpr Mm mm5 = Mm(5);
static constexpr Mm mm6 = Mm(6);
static constexpr Mm mm7 = Mm(7);

static constexpr KReg k0 = KReg(0);
static constexpr KReg k1 = KReg(1);
static constexpr KReg k2 = KReg(2);
static constexpr KReg k3 = KReg(3);
static constexpr KReg k4 = KReg(4);
static constexpr KReg k5 = KReg(5);
static constexpr KReg k6 = KReg(6);
static constexpr KReg k7 = KReg(7);

static constexpr SReg no_seg = SReg(SReg::kIdNone);
static constexpr SReg es = SReg(SReg::kIdEs);
static constexpr SReg cs = SReg(SReg::kIdCs);
static constexpr SReg ss = SReg(SReg::kIdSs);
static constexpr SReg ds = SReg(SReg::kIdDs);
static constexpr SReg fs = SReg(SReg::kIdFs);
static constexpr SReg gs = SReg(SReg::kIdGs);

static constexpr CReg cr0 = CReg(0);
static constexpr CReg cr1 = CReg(1);
static constexpr CReg cr2 = CReg(2);
static constexpr CReg cr3 = CReg(3);
static constexpr CReg cr4 = CReg(4);
static constexpr CReg cr5 = CReg(5);
static constexpr CReg cr6 = CReg(6);
static constexpr CReg cr7 = CReg(7);
static constexpr CReg cr8 = CReg(8);
static constexpr CReg cr9 = CReg(9);
static constexpr CReg cr10 = CReg(10);
static constexpr CReg cr11 = CReg(11);
static constexpr CReg cr12 = CReg(12);
static constexpr CReg cr13 = CReg(13);
static constexpr CReg cr14 = CReg(14);
static constexpr CReg cr15 = CReg(15);

static constexpr DReg dr0 = DReg(0);
static constexpr DReg dr1 = DReg(1);
static constexpr DReg dr2 = DReg(2);
static constexpr DReg dr3 = DReg(3);
static constexpr DReg dr4 = DReg(4);
static constexpr DReg dr5 = DReg(5);
static constexpr DReg dr6 = DReg(6);
static constexpr DReg dr7 = DReg(7);
static constexpr DReg dr8 = DReg(8);
static constexpr DReg dr9 = DReg(9);
static constexpr DReg dr10 = DReg(10);
static constexpr DReg dr11 = DReg(11);
static constexpr DReg dr12 = DReg(12);
static constexpr DReg dr13 = DReg(13);
static constexpr DReg dr14 = DReg(14);
static constexpr DReg dr15 = DReg(15);

static constexpr St st0 = St(0);
static constexpr St st1 = St(1);
static constexpr St st2 = St(2);
static constexpr St st3 = St(3);
static constexpr St st4 = St(4);
static constexpr St st5 = St(5);
static constexpr St st6 = St(6);
static constexpr St st7 = St(7);

static constexpr Bnd bnd0 = Bnd(0);
static constexpr Bnd bnd1 = Bnd(1);
static constexpr Bnd bnd2 = Bnd(2);
static constexpr Bnd bnd3 = Bnd(3);

static constexpr Tmm tmm0 = Tmm(0);
static constexpr Tmm tmm1 = Tmm(1);
static constexpr Tmm tmm2 = Tmm(2);
static constexpr Tmm tmm3 = Tmm(3);
static constexpr Tmm tmm4 = Tmm(4);
static constexpr Tmm tmm5 = Tmm(5);
static constexpr Tmm tmm6 = Tmm(6);
static constexpr Tmm tmm7 = Tmm(7);

static constexpr Rip rip = Rip(0);

#ifndef _DOXYGEN
} // {regs}

// Make `x86::regs` accessible through `x86` namespace as well.
using namespace regs;
#endif

//! Memory operand specific to X86 and X86_64 architecture.
class Mem : public BaseMem {
public:
  //! \name Constants
  //! \{

  // Memory address type (2 bits).
  // |........|........|XX......|........|
  static inline constexpr uint32_t kSignatureMemAddrTypeShift = 14;
  static inline constexpr uint32_t kSignatureMemAddrTypeMask = 0x03u << kSignatureMemAddrTypeShift;

  // Memory shift amount (2 bits).
  // |........|......XX|........|........|
  static inline constexpr uint32_t kSignatureMemShiftValueShift = 16;
  static inline constexpr uint32_t kSignatureMemShiftValueMask = 0x03u << kSignatureMemShiftValueShift;

  // Memory segment reg (3 bits).
  // |........|...XXX..|........|........|
  static inline constexpr uint32_t kSignatureMemSegmentShift = 18;
  static inline constexpr uint32_t kSignatureMemSegmentMask = 0x07u << kSignatureMemSegmentShift;

  // Memory broadcast type (3 bits).
  // |........|XXX.....|........|........|
  static inline constexpr uint32_t kSignatureMemBroadcastShift = 21;
  static inline constexpr uint32_t kSignatureMemBroadcastMask = 0x7u << kSignatureMemBroadcastShift;

  //! Address type.
  enum class AddrType : uint32_t {
    //! Default address type, Assembler will select the best type when necessary.
    kDefault = 0,
    //! Absolute address type.
    kAbs = 1,
    //! Relative address type.
    kRel = 2,

    //! Maximum value of `AddrType`.
    kMaxValue = kRel
  };

  //! Memory broadcast type.
  enum class Broadcast : uint32_t {
    //! No broadcast (regular memory operand).
    kNone = 0,
    //! Broadcast {1to2}.
    k1To2 = 1,
    //! Broadcast {1to4}.
    k1To4 = 2,
    //! Broadcast {1to8}.
    k1To8 = 3,
    //! Broadcast {1to16}.
    k1To16 = 4,
    //! Broadcast {1to32}.
    k1To32 = 5,
    //! Broadcast {1to64}.
    k1To64 = 6,

    //! Maximum value of `Broadcast`.
    kMaxValue = k1To64
  };

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default `Mem` operand that points to [0].
  ASMJIT_INLINE_CONSTEXPR Mem() noexcept
    : BaseMem() {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t base_id, uint32_t index_id, int32_t offset) noexcept
    : BaseMem(signature, base_id, index_id, offset) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Label& base, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              Signature::from_size(size) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Label& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, base.id(), index.id(), off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_size(size) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, uint32_t shift, int32_t off, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, base.id(), index.id(), off) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t base, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_size(size) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  ASMJIT_INLINE_CONSTEXPR Mem(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0, Signature signature = OperandSignature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift) |
              Signature::from_size(size) |
              signature, uint32_t(base >> 32), index.id(), int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG Mem& operator=(const Mem& other) noexcept = default;

  //! \}

  //! \name Clone
  //! \{

  //! Clones the memory operand.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }

  //! Creates a copy of this memory operand adjusted by `off`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_adjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.add_offset(off);
    return result;
  }

  //! Creates a copy of this memory operand resized to `size`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_resized(uint32_t size) const noexcept {
    Mem result(*this);
    result.set_size(size);
    return result;
  }

  //! Creates a copy of this memory operand with a broadcast `bcst`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem clone_broadcasted(Broadcast bcst) const noexcept {
    return Mem((_signature & ~Signature{kSignatureMemBroadcastMask}) | Signature::from_value<kSignatureMemBroadcastMask>(bcst), _base_id, _data[0], int32_t(_data[1]));
  }

  //! \}

  //! \name Base & Index
  //! \{

  //! Converts memory `base_type` and `base_id` to `Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg base_reg() const noexcept { return Reg::from_type_and_id(base_type(), base_id()); }

  //! Converts memory `index_type` and `index_id` to `Reg` instance.
  //!
  //! The memory must have a valid index register otherwise the result will be wrong.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Reg index_reg() const noexcept { return Reg::from_type_and_id(index_type(), index_id()); }

  using BaseMem::set_index;

  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, uint32_t shift) noexcept {
    set_index(index);
    set_shift(shift);
  }

  //! \}

  //! \name Memory Size
  //! \{

  //! Tests whether the memory operand specifies a size (i.e. the size is not zero).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size() const noexcept { return _signature.has_field<Signature::kSizeMask>(); }

  //! Tests whether the memory operand size matches size `s`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_size(uint32_t s) const noexcept { return size() == s; }

  //! Returns the size of the memory operand in bytes.
  //!
  //! \note Most instructions would deduce the size of the memory operand, so in most cases it's expected that the
  //! returned value would be zero. However, some instruction require the size to select between multiple variations,
  //! so in some cases size is required would be non-zero (for example `inc [mem], immediate` requires size to
  //! distinguish between 8-bit, 16-bit, 32-bit, and 64-bit increments.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t size() const noexcept { return _signature.get_field<Signature::kSizeMask>(); }

  //! Sets the memory operand size (in bytes).
  ASMJIT_INLINE_CONSTEXPR void set_size(uint32_t size) noexcept { _signature.set_field<Signature::kSizeMask>(size); }

  //! \}

  //! \name Address Type
  //! \{

  //! Returns the address type of the memory operand.
  //!
  //! By default, address type of newly created memory operands is always \ref AddrType::kDefault.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR AddrType addr_type() const noexcept { return (AddrType)_signature.get_field<kSignatureMemAddrTypeMask>(); }

  //! Sets the address type to `addr_type`.
  ASMJIT_INLINE_CONSTEXPR void set_addr_type(AddrType addr_type) noexcept { _signature.set_field<kSignatureMemAddrTypeMask>(uint32_t(addr_type)); }

  //! Resets the address type to \ref AddrType::kDefault.
  ASMJIT_INLINE_CONSTEXPR void reset_addr_type() noexcept { _signature.set_field<kSignatureMemAddrTypeMask>(uint32_t(AddrType::kDefault)); }

  //! Tests whether the address type is \ref AddrType::kAbs.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_addr_abs() const noexcept { return addr_type() == AddrType::kAbs; }

  //! Sets the address type to \ref AddrType::kAbs.
  ASMJIT_INLINE_CONSTEXPR void set_addr_abs() noexcept { set_addr_type(AddrType::kAbs); }

  //! Tests whether the address type is \ref AddrType::kRel.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_addr_rel() const noexcept { return addr_type() == AddrType::kRel; }

  //! Sets the address type to \ref AddrType::kRel.
  ASMJIT_INLINE_CONSTEXPR void set_addr_rel() noexcept { set_addr_type(AddrType::kRel); }

  //! \}

  //! \name Segment
  //! \{

  //! Tests whether the memory operand has a segment override.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_segment() const noexcept { return _signature.has_field<kSignatureMemSegmentMask>(); }

  //! Returns the associated segment override as `SReg` operand.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR SReg segment() const noexcept { return SReg(segment_id()); }

  //! Returns segment override register id, see `SReg::Id`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t segment_id() const noexcept { return _signature.get_field<kSignatureMemSegmentMask>(); }

  //! Sets the segment override to `seg`.
  ASMJIT_INLINE_CONSTEXPR void set_segment(const SReg& seg) noexcept { set_segment(seg.id()); }

  //! Sets the segment override to `id`.
  ASMJIT_INLINE_CONSTEXPR void set_segment(uint32_t reg_id) noexcept { _signature.set_field<kSignatureMemSegmentMask>(reg_id); }

  //! Resets the segment override.
  ASMJIT_INLINE_CONSTEXPR void reset_segment() noexcept { _signature.set_field<kSignatureMemSegmentMask>(0); }

  //! \}

  //! \name Shift
  //! \{

  //! Tests whether the memory operand has shift (aka scale) value.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_shift() const noexcept { return _signature.has_field<kSignatureMemShiftValueMask>(); }

  //! Returns the memory operand's shift (aka scale) value.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR uint32_t shift() const noexcept { return _signature.get_field<kSignatureMemShiftValueMask>(); }

  //! Sets the memory operand's shift (aka scale) value.
  ASMJIT_INLINE_CONSTEXPR void set_shift(uint32_t shift) noexcept { _signature.set_field<kSignatureMemShiftValueMask>(shift); }

  //! Resets the memory operand's shift (aka scale) value to zero.
  ASMJIT_INLINE_CONSTEXPR void reset_shift() noexcept { _signature.set_field<kSignatureMemShiftValueMask>(0); }

  //! \}

  //! \name Broadcast
  //! \{

  //! Tests whether the memory operand has broadcast {1tox}.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_broadcast() const noexcept { return _signature.has_field<kSignatureMemBroadcastMask>(); }

  //! Returns the memory operand's broadcast.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Broadcast get_broadcast() const noexcept { return (Broadcast)_signature.get_field<kSignatureMemBroadcastMask>(); }

  //! Sets the memory operand's broadcast.
  ASMJIT_INLINE_CONSTEXPR void set_broadcast(Broadcast b) noexcept { _signature.set_field<kSignatureMemBroadcastMask>(uint32_t(b)); }

  //! Resets the memory operand's broadcast to none.
  ASMJIT_INLINE_CONSTEXPR void reset_broadcast() noexcept { _signature.set_field<kSignatureMemBroadcastMask>(0); }

  //! Returns a new `Mem` without a broadcast (the possible broadcast is cleared).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to1() const noexcept { return clone_broadcasted(Broadcast::kNone); }

  //! Returns a new `Mem` with {1to2} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to2() const noexcept { return clone_broadcasted(Broadcast::k1To2); }

  //! Returns a new `Mem` with {1to4} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to4() const noexcept { return clone_broadcasted(Broadcast::k1To4); }

  //! Returns a new `Mem` with {1to8} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to8() const noexcept { return clone_broadcasted(Broadcast::k1To8); }

  //! Returns a new `Mem` with {1to16} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to16() const noexcept { return clone_broadcasted(Broadcast::k1To16); }

  //! Returns a new `Mem` with {1to32} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to32() const noexcept { return clone_broadcasted(Broadcast::k1To32); }

  //! Returns a new `Mem` with {1to64} broadcast (AVX-512).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Mem _1to64() const noexcept { return clone_broadcasted(Broadcast::k1To64); }

  //! \}
};

//! Creates `[base.reg + offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}

//! Creates `[base.reg + (index << shift) + offset]` memory operand (scalar index).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[base.reg + (index << shift) + offset]` memory operand (vector index).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[base + offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}

//! Creates `[base + (index << shift) + offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[base + (index << shift) + offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[rip + offset]` memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Rip& rip_, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(rip_, offset, size);
}

//! Creates `[base]` absolute memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size);
}

//! Creates `[base + (index.reg << shift)]` absolute memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}

//! Creates `[base + (index.reg << shift)]` absolute memory operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}

//! Creates `[base]` absolute memory operand (absolute).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}

//! Creates `[base + (index.reg << shift)]` absolute memory operand (absolute).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}

//! Creates `[base + (index.reg << shift)]` absolute memory operand (absolute).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_abs(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs));
}

//! Creates `[base]` relative memory operand (relative).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}

//! Creates `[base + (index.reg << shift)]` relative memory operand (relative).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}

//! Creates `[base + (index.reg << shift)]` relative memory operand (relative).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_rel(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel));
}

#define ASMJIT_MEM_PTR(func, size)                                                              \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, int32_t offset = 0) noexcept                                                \
      { return Mem(base, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept           \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0) noexcept          \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Label& base, int32_t offset = 0) noexcept                                             \
      { return Mem(base, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept        \
      { return Mem(base, index, shift, offset, size); }                                         \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    const Rip& rip_, int32_t offset = 0) noexcept                                               \
      { return Mem(rip_, offset, size); }                                                       \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size); }                                                               \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size); }                                                 \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func(                                                      \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size); }                                                 \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size,                                                                  \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_abs(                                                \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kAbs)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base) noexcept                                                                     \
      { return Mem(base, size,                                                                  \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base, const Gp& index, uint32_t shift = 0) noexcept                                \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); } \
                                                                                                \
  [[nodiscard]]                                                                                 \
  static ASMJIT_INLINE_CONSTEXPR Mem func##_rel(                                                \
    uint64_t base, const Vec& index, uint32_t shift = 0) noexcept                               \
      { return Mem(base, index, shift, size,                                                    \
          OperandSignature::from_value<Mem::kSignatureMemAddrTypeMask>(Mem::AddrType::kRel)); }

// Definition of memory operand constructors that use platform independent naming.
ASMJIT_MEM_PTR(ptr_8, 1)
ASMJIT_MEM_PTR(ptr_16, 2)
ASMJIT_MEM_PTR(ptr_32, 4)
ASMJIT_MEM_PTR(ptr_48, 6)
ASMJIT_MEM_PTR(ptr_64, 8)
ASMJIT_MEM_PTR(ptr_80, 10)
ASMJIT_MEM_PTR(ptr_128, 16)
ASMJIT_MEM_PTR(ptr_256, 32)
ASMJIT_MEM_PTR(ptr_512, 64)

// Definition of memory operand constructors that use X86-specific convention.
ASMJIT_MEM_PTR(byte_ptr, 1)
ASMJIT_MEM_PTR(word_ptr, 2)
ASMJIT_MEM_PTR(dword_ptr, 4)
ASMJIT_MEM_PTR(fword_ptr, 6)
ASMJIT_MEM_PTR(qword_ptr, 8)
ASMJIT_MEM_PTR(tbyte_ptr, 10)
ASMJIT_MEM_PTR(tword_ptr, 10)
ASMJIT_MEM_PTR(oword_ptr, 16)
ASMJIT_MEM_PTR(dqword_ptr, 16)
ASMJIT_MEM_PTR(qqword_ptr, 32)
ASMJIT_MEM_PTR(xmmword_ptr, 16)
ASMJIT_MEM_PTR(ymmword_ptr, 32)
ASMJIT_MEM_PTR(zmmword_ptr, 64)

#undef ASMJIT_MEM_PTR

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86OPERAND_H_INCLUDED
