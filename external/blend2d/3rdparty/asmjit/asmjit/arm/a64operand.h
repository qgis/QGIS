// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64OPERAND_H_INCLUDED
#define ASMJIT_ARM_A64OPERAND_H_INCLUDED

#include <asmjit/core/operand.h>
#include <asmjit/arm/armglobals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! General purpose register (AArch64).
class Gp : public UniGp {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, UniGp)

  //! \name Constants
  //! \{

  //! Special register id.
  enum Id : uint32_t {
    //! Register that depends on OS, could be used as TLS offset.
    kIdOs = 18,
    //! Frame pointer register id.
    kIdFp = 29,
    //! Link register id.
    kIdLr = 30,
    //! Stack register id.
    kIdSp = 31,
    //! Zero register id.
    //!
    //! Although zero register has the same id as stack register it has a special treatment, because we need to be
    //! able to distinguish between these two at API level. Some instructions were designed to be used with SP and
    //! some other with ZR - so we need a way to distinguish these two to make sure we emit the right thing.
    //!
    //! The number 63 is not random, when you perform `id & 31` you would always get 31 for both SP and ZR inputs,
    //! which is the identifier used by AArch64 ISA to encode either SP or ZR depending on the instruction.
    kIdZr = 63
  };

  //! \}

  //! \name Static Constructors
  //! \{

  //! Creates a new 32-bit low general purpose register (W) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r32(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp32>(), reg_id); }

  //! Creates a new 64-bit low general purpose register (X) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_r64(uint32_t reg_id) noexcept { return Gp(signature_of_t<RegType::kGp64>(), reg_id); }

  //! Creates a new 32-bit low general purpose register (W) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_w(uint32_t reg_id) noexcept { return make_r32(reg_id); }

  //! Creates a new 64-bit low general purpose register (X) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Gp make_x(uint32_t reg_id) noexcept { return make_r64(reg_id); }

  //! \}

  //! \name Gp Register Accessors
  //! \{

  //! Test whether this register is ZR register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_zr() const noexcept { return id() == kIdZr; }

  //! Test whether this register is SP register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_sp() const noexcept { return id() == kIdSp; }

  //! Clones and casts this register to a 32-bit (W) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r32() const noexcept { return make_r32(id()); }

  //! Clones and casts this register to a 64-bit (X) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp r64() const noexcept { return make_r64(id()); }

  //! Clones and casts this register to a 32-bit (W) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp w() const noexcept { return r32(); }

  //! Clones and casts this register to a 64-bit (X) register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Gp x() const noexcept { return r64(); }

  //! \}
};

//! Vector element type (AArch64).
enum class VecElementType : uint32_t {
  //! No element type specified.
  kNone = 0,
  //! Byte elements (B8 or B16).
  kB,
  //! Halfword elements (H4 or H8).
  kH,
  //! Singleword elements (S2 or S4).
  kS,
  //! Doubleword elements (D2).
  kD,
  //! Byte elements grouped by 4 bytes (B4).
  //!
  //! \note This element-type is only used by few instructions.
  kB4,
  //! Halfword elements grouped by 2 halfwords (H2).
  //!
  //! \note This element-type is only used by few instructions.
  kH2,

  //! Maximum value of \ref VecElementType
  kMaxValue = kH2
};

//! Vector register (AArch64).
class Vec : public UniVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, UniVec)

  //! \cond

  // Register element type (3 bits).
  // |........|........|.XXX....|........|
  static inline constexpr uint32_t kSignatureRegElementTypeShift = 12;
  static inline constexpr uint32_t kSignatureRegElementTypeMask = 0x07 << kSignatureRegElementTypeShift;

  // Register has element index (1 bit).
  // |........|........|X.......|........|
  static inline constexpr uint32_t kSignatureRegElementFlagShift = 15;
  static inline constexpr uint32_t kSignatureRegElementFlagMask = 0x01 << kSignatureRegElementFlagShift;

  // Register element index (4 bits).
  // |........|....XXXX|........|........|
  static inline constexpr uint32_t kSignatureRegElementIndexShift = 16;
  static inline constexpr uint32_t kSignatureRegElementIndexMask = 0x0F << kSignatureRegElementIndexShift;

  static inline constexpr uint32_t kSignatureElementB = uint32_t(VecElementType::kB) << kSignatureRegElementTypeShift;
  static inline constexpr uint32_t kSignatureElementH = uint32_t(VecElementType::kH) << kSignatureRegElementTypeShift;
  static inline constexpr uint32_t kSignatureElementS = uint32_t(VecElementType::kS) << kSignatureRegElementTypeShift;
  static inline constexpr uint32_t kSignatureElementD = uint32_t(VecElementType::kD) << kSignatureRegElementTypeShift;
  static inline constexpr uint32_t kSignatureElementB4 = uint32_t(VecElementType::kB4) << kSignatureRegElementTypeShift;
  static inline constexpr uint32_t kSignatureElementH2 = uint32_t(VecElementType::kH2) << kSignatureRegElementTypeShift;

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR OperandSignature _make_element_access_signature(VecElementType element_type, uint32_t element_index) noexcept {
    return OperandSignature{
      uint32_t(RegTraits<RegType::kVec128>::kSignature)         |
      uint32_t(kSignatureRegElementFlagMask)                    |
      (uint32_t(element_type) << kSignatureRegElementTypeShift)  |
      (uint32_t(element_index << kSignatureRegElementIndexShift))
    };
  }

  //! \endcond

  //! \name Static Constructors
  //! \{

  //! Creates a new 8-bit vector register (B) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v8(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec8>(), reg_id); }

  //! Creates a new 16-bit vector register (H) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v16(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec16>(), reg_id); }

  //! Creates a new 32-bit vector register (S) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v32(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec32>(), reg_id); }

  //! Creates a new 64-bit vector register (D) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v64(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec64>(), reg_id); }

  //! Creates a new 128-bit vector register (Q) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128(uint32_t reg_id) noexcept { return Vec(signature_of_t<RegType::kVec128>(), reg_id); }

  //! Creates a new 8-bit vector register (B) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_b(uint32_t reg_id) noexcept { return make_v8(reg_id); }

  //! Creates a new 16-bit vector register (H) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_h(uint32_t reg_id) noexcept { return make_v16(reg_id); }

  //! Creates a new 32-bit vector register (S) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_s(uint32_t reg_id) noexcept { return make_v32(reg_id); }

  //! Creates a new 64-bit vector register (D) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_d(uint32_t reg_id) noexcept { return make_v64(reg_id); }

  //! Creates a new 128-bit vector register (Q) having the given register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_q(uint32_t reg_id) noexcept { return make_v128(reg_id); }

  //! Creates a new 32-bit vector register (S) having the given vector `element_type` and register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v32_with_element_type(VecElementType element_type, uint32_t reg_id) noexcept {
    uint32_t signature = RegTraits<RegType::kVec32>::kSignature | uint32_t(element_type) << kSignatureRegElementTypeShift;
    return Vec(OperandSignature{signature}, reg_id);
  }

  //! Creates a new 64-bit vector register (D) having the given vector `element_type` and register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v64_with_element_type(VecElementType element_type, uint32_t reg_id) noexcept {
    uint32_t signature = RegTraits<RegType::kVec64>::kSignature | uint32_t(element_type) << kSignatureRegElementTypeShift;
    return Vec(OperandSignature{signature}, reg_id);
  }

  //! Creates a new 128-bit vector register (Q) having the given vector `element_type` and register id `reg_id`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128_with_element_type(VecElementType element_type, uint32_t reg_id) noexcept {
    uint32_t signature = RegTraits<RegType::kVec128>::kSignature | uint32_t(element_type) << kSignatureRegElementTypeShift;
    return Vec(OperandSignature{signature}, reg_id);
  }

  //! Creates a new 128-bit vector of type specified by `element_type` and `element_index`.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR Vec make_v128_with_element_index(VecElementType element_type, uint32_t element_index, uint32_t reg_id) noexcept {
    return Vec(_make_element_access_signature(element_type, element_index), reg_id);
  }

  //! \}

  //! \name Vector Register Accessors
  //! \{

  //! Returns whether the register has element type or element index (or both).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_element_type_or_index() const noexcept {
    return _signature.has_field<kSignatureRegElementTypeMask | kSignatureRegElementFlagMask>();
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_b8() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec64>::kSignature | kSignatureElementB);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_h4() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec64>::kSignature | kSignatureElementH);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_s2() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec64>::kSignature | kSignatureElementS);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_d1() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec64>::kSignature);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_b16() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementB);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_h8() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementH);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_s4() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementS);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_d2() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementD);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_b4x4() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementB4);
  }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool is_vec_h2x4() const noexcept {
    return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kVec128>::kSignature | kSignatureElementH2);
  }

  //! Clones and casts the register to an 8-bit B register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v8() const noexcept { return make_v8(id()); }

  //! Clones and casts the register to a 16-bit H register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v16() const noexcept { return make_v16(id()); }

  //! Clones and casts the register to a 32-bit S register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v32() const noexcept { return make_v32(id()); }

  //! Clones and casts the register to a 64-bit D register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v64() const noexcept { return make_v64(id()); }

  //! Clones and casts the register to a 128-bit Q register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec v128() const noexcept { return make_v128(id()); }

  //! Clones and casts the register to an 8-bit B register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec b() const noexcept { return make_v8(id()); }

  //! Clones and casts the register to a 16-bit H register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h() const noexcept { return make_v16(id()); }

  //! Clones and casts the register to a 32-bit S register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s() const noexcept { return make_v32(id()); }

  //! Clones and casts the register to a 64-bit D register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec d() const noexcept { return make_v64(id()); }

  //! Clones and casts the register to a 128-bit Q register (element type & index is not cloned).
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec q() const noexcept { return make_v128(id()); }

  //! Clones and casts the register to a 128-bit V.B[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec b(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kB, element_index, id()); }

  //! Clones and casts the register to a 128-bit V.H[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kH, element_index, id()); }

  //! Clones and casts the register to a 128-bit V.S[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kS, element_index, id()); }

  //! Clones and casts the register to a 128-bit V.D[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec d(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kD, element_index, id()); }

  //! Clones and casts the register to a 128-bit V.H2[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h2(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kH2, element_index, id()); }

  //! Clones and casts the register to a 128-bit V.B4[element_index] register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec b4(uint32_t element_index) const noexcept { return make_v128_with_element_index(VecElementType::kB4, element_index, id()); }

  //! Clones and casts the register to V.8B.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec b8() const noexcept { return make_v64_with_element_type(VecElementType::kB, id()); }

  //! Clones and casts the register to V.16B.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec b16() const noexcept { return make_v128_with_element_type(VecElementType::kB, id()); }

  //! Clones and casts the register to V.2H.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h2() const noexcept { return make_v32_with_element_type(VecElementType::kH, id()); }

  //! Clones and casts the register to V.4H.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h4() const noexcept { return make_v64_with_element_type(VecElementType::kH, id()); }

  //! Clones and casts the register to V.8H.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec h8() const noexcept { return make_v128_with_element_type(VecElementType::kH, id()); }

  //! Clones and casts the register to V.2S.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s2() const noexcept { return make_v64_with_element_type(VecElementType::kS, id()); }

  //! Clones and casts the register to V.4S.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec s4() const noexcept { return make_v128_with_element_type(VecElementType::kS, id()); }

  //! Clones and casts the register to V.2D.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec d2() const noexcept { return make_v128_with_element_type(VecElementType::kD, id()); }

  //! \}

  //! \name Element Type Accessors
  //! \{

  //! Returns whether the vector register has associated a vector element type.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool has_element_type() const noexcept {
    return _signature.has_field<kSignatureRegElementTypeMask>();
  }

  //! Returns vector element type of the register.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR VecElementType element_type() const noexcept {
    return VecElementType(_signature.get_field<kSignatureRegElementTypeMask>());
  }

  //! Sets vector element type of the register to `element_type`.
  ASMJIT_INLINE_CONSTEXPR void set_element_type(VecElementType element_type) noexcept {
    _signature.set_field<kSignatureRegElementTypeMask>(uint32_t(element_type));
  }

  //! Resets vector element type to none.
  ASMJIT_INLINE_CONSTEXPR void reset_element_type() noexcept {
    _signature.set_field<kSignatureRegElementTypeMask>(0);
  }

  //! \}

  //! \name Element Index Accessors
  //! \{

  //! Returns whether the register has element index (it's an element index access).
  ASMJIT_INLINE_CONSTEXPR bool has_element_index() const noexcept {
    return _signature.has_field<kSignatureRegElementFlagMask>();
  }

  //! Returns element index of the register.
  ASMJIT_INLINE_CONSTEXPR uint32_t element_index() const noexcept {
    return _signature.get_field<kSignatureRegElementIndexMask>();
  }

  //! Sets element index of the register to `element_type`.
  ASMJIT_INLINE_CONSTEXPR void set_element_index(uint32_t element_index) noexcept {
    _signature |= kSignatureRegElementFlagMask;
    _signature.set_field<kSignatureRegElementIndexMask>(element_index);
  }

  //! Resets element index of the register.
  ASMJIT_INLINE_CONSTEXPR void reset_element_index() noexcept {
    _signature &= ~(kSignatureRegElementFlagMask | kSignatureRegElementIndexMask);
  }

  //! Clones a vector register with element access enabled at the given `element_index`.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR Vec at(uint32_t element_index) const noexcept {
    return Vec((signature() & ~kSignatureRegElementIndexMask) | (element_index << kSignatureRegElementIndexShift) | kSignatureRegElementFlagMask, id());
  }

  //! \}
};

//! Memory operand (AArch64).
class Mem : public BaseMem {
public:
  //! \cond INTERNAL

  // Index shift value (5 bits).
  // |........|.....XXX|XX......|........|
  static inline constexpr uint32_t kSignatureMemShiftValueShift = 14;
  static inline constexpr uint32_t kSignatureMemShiftValueMask = 0x1Fu << kSignatureMemShiftValueShift;

  // Index shift operation (4 bits).
  // |........|XXXX....|........|........|
  static inline constexpr uint32_t kSignatureMemShiftOpShift = 20;
  static inline constexpr uint32_t kSignatureMemShiftOpMask = 0x0Fu << kSignatureMemShiftOpShift;

  // Offset mode type (2 bits).
  // |......XX|........|........|........|
  static inline constexpr uint32_t kSignatureMemOffsetModeShift = 24;
  static inline constexpr uint32_t kSignatureMemOffsetModeMask = 0x03u << kSignatureMemOffsetModeShift;

  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Construct a default `Mem` operand, that points to [0].
  ASMJIT_INLINE_CONSTEXPR Mem() noexcept
    : BaseMem() {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  ASMJIT_INLINE_NODEBUG explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Signature& signature, uint32_t base_id, uint32_t index_id, int32_t offset) noexcept
    : BaseMem(signature, base_id, index_id, offset) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Label& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(RegType::kLabelTag) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(const Reg& base, int32_t off = 0, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              signature, base.id(), 0, off) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_CONSTEXPR Mem(const Reg& base, const Reg& index, const Shift& shift, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              Signature::from_mem_base_type(base.reg_type()) |
              Signature::from_mem_index_type(index.reg_type()) |
              Signature::from_value<kSignatureMemShiftOpMask>(uint32_t(shift.op())) |
              Signature::from_value<kSignatureMemShiftValueMask>(shift.value()) |
              signature, base.id(), index.id(), 0) {}

  ASMJIT_INLINE_CONSTEXPR explicit Mem(uint64_t base, Signature signature = Signature{0}) noexcept
    : BaseMem(Signature::from_op_type(OperandType::kMem) |
              signature, uint32_t(base >> 32), 0, int32_t(uint32_t(base & 0xFFFFFFFFu))) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_CONSTEXPR Mem& operator=(const Mem& other) noexcept {
    copy_from(other);
    return *this;
  }

  //! \}

  //! \name Clone
  //! \{

  //! Clones the memory operand.
  ASMJIT_INLINE_CONSTEXPR Mem clone() const noexcept { return Mem(*this); }

  //! Gets new memory operand adjusted by `off`.
  ASMJIT_INLINE_CONSTEXPR Mem clone_adjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.add_offset(off);
    return result;
  }

  //! Clones the memory operand and makes it pre-index.
  ASMJIT_INLINE_CONSTEXPR Mem pre() const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPreIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it pre-index.
  ASMJIT_INLINE_CONSTEXPR Mem pre(int64_t off) const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPreIndex);
    result.add_offset(off);
    return result;
  }

  //! Clones the memory operand and makes it post-index.
  ASMJIT_INLINE_CONSTEXPR Mem post() const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPostIndex);
    return result;
  }

  //! Clones the memory operand, applies a given offset `off` and makes it post-index.
  ASMJIT_INLINE_CONSTEXPR Mem post(int64_t off) const noexcept {
    Mem result(*this);
    result.set_offset_mode(OffsetMode::kPostIndex);
    result.add_offset(off);
    return result;
  }

  //! \}

  //! \name Base & Index
  //! \{

  //! Converts memory `base_type` and `base_id` to `arm::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg base_reg() const noexcept { return Reg::from_type_and_id(base_type(), base_id()); }

  //! Converts memory `index_type` and `index_id` to `arm::Reg` instance.
  //!
  //! The memory must have a valid index register otherwise the result will be wrong.
  ASMJIT_INLINE_NODEBUG Reg index_reg() const noexcept { return Reg::from_type_and_id(index_type(), index_id()); }

  using BaseMem::set_index;

  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, uint32_t shift) noexcept {
    set_index(index);
    set_shift(shift);
  }

  ASMJIT_INLINE_CONSTEXPR void set_index(const Reg& index, Shift shift) noexcept {
    set_index(index);
    set_shift(shift);
  }

  //! \}

  //! \name ARM Specific Features
  //! \{

  //! Gets offset mode.
  ASMJIT_INLINE_CONSTEXPR OffsetMode offset_mode() const noexcept { return OffsetMode(_signature.get_field<kSignatureMemOffsetModeMask>()); }
  //! Sets offset mode to `mode`.
  ASMJIT_INLINE_CONSTEXPR void set_offset_mode(OffsetMode mode) noexcept { _signature.set_field<kSignatureMemOffsetModeMask>(uint32_t(mode)); }
  //! Resets offset mode to default (fixed offset, without write-back).
  ASMJIT_INLINE_CONSTEXPR void reset_offset_mode() noexcept { _signature.set_field<kSignatureMemOffsetModeMask>(uint32_t(OffsetMode::kFixed)); }

  //! Tests whether the current memory offset mode is fixed (see \ref arm::OffsetMode::kFixed).
  ASMJIT_INLINE_CONSTEXPR bool is_fixed_offset() const noexcept { return offset_mode() == OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is either pre-index or post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_pre_or_post() const noexcept { return offset_mode() != OffsetMode::kFixed; }
  //! Tests whether the current memory offset mode is pre-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_pre_index() const noexcept { return offset_mode() == OffsetMode::kPreIndex; }
  //! Tests whether the current memory offset mode is post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR bool is_post_index() const noexcept { return offset_mode() == OffsetMode::kPostIndex; }

  //! Sets offset mode of this memory operand to pre-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR void make_pre_index() noexcept { set_offset_mode(OffsetMode::kPreIndex); }
  //! Sets offset mode of this memory operand to post-index (write-back is used).
  ASMJIT_INLINE_CONSTEXPR void make_post_index() noexcept { set_offset_mode(OffsetMode::kPostIndex); }

  //! Gets shift operation that is used by index register.
  ASMJIT_INLINE_CONSTEXPR ShiftOp shift_op() const noexcept { return ShiftOp(_signature.get_field<kSignatureMemShiftOpMask>()); }
  //! Sets shift operation that is used by index register.
  ASMJIT_INLINE_CONSTEXPR void set_shift_op(ShiftOp sop) noexcept { _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(sop)); }
  //! Resets shift operation that is used by index register to LSL (default value).
  ASMJIT_INLINE_CONSTEXPR void reset_shift_op() noexcept { _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(ShiftOp::kLSL)); }

  //! Gets whether the memory operand has shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR bool has_shift() const noexcept { return _signature.has_field<kSignatureMemShiftValueMask>(); }
  //! Gets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR uint32_t shift() const noexcept { return _signature.get_field<kSignatureMemShiftValueMask>(); }
  //! Sets the memory operand's shift (aka scale) constant.
  ASMJIT_INLINE_CONSTEXPR void set_shift(uint32_t shift) noexcept { _signature.set_field<kSignatureMemShiftValueMask>(shift); }

  //! Sets the memory operand's shift and shift operation.
  ASMJIT_INLINE_CONSTEXPR void set_shift(Shift shift) noexcept {
    _signature.set_field<kSignatureMemShiftOpMask>(uint32_t(shift.op()));
    _signature.set_field<kSignatureMemShiftValueMask>(shift.value());
  }

  //! Resets the memory operand's shift (aka scale) constant to zero.
  ASMJIT_INLINE_CONSTEXPR void reset_shift() noexcept { _signature.set_field<kSignatureMemShiftValueMask>(0); }

  //! \}
};

#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates a 32-bit W register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp w(uint32_t id) noexcept { return Gp::make_r32(id); }

//! Creates a 32-bit W register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp32(uint32_t id) noexcept { return Gp::make_r32(id); }

//! Creates a 64-bit X register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp x(uint32_t id) noexcept { return Gp::make_r64(id); }

//! Creates a 64-bit X register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Gp gp64(uint32_t id) noexcept { return Gp::make_r64(id); }

//! Creates an 8-bit B register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec b(uint32_t id) noexcept { return Vec::make_v8(id); }

//! Creates a 16-bit H register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec h(uint32_t id) noexcept { return Vec::make_v16(id); }

//! Creates a 32-bit S register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec s(uint32_t id) noexcept { return Vec::make_v32(id); }

//! Creates a 64-bit D register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec d(uint32_t id) noexcept { return Vec::make_v64(id); }

//! Creates a 128-bit Q register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec q(uint32_t id) noexcept { return Vec::make_v128(id); }

//! Creates a 128-bit V register operand.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Vec v(uint32_t id) noexcept { return Vec::make_v128(id); }

static constexpr Gp w0 = Gp::make_r32(0);
static constexpr Gp w1 = Gp::make_r32(1);
static constexpr Gp w2 = Gp::make_r32(2);
static constexpr Gp w3 = Gp::make_r32(3);
static constexpr Gp w4 = Gp::make_r32(4);
static constexpr Gp w5 = Gp::make_r32(5);
static constexpr Gp w6 = Gp::make_r32(6);
static constexpr Gp w7 = Gp::make_r32(7);
static constexpr Gp w8 = Gp::make_r32(8);
static constexpr Gp w9 = Gp::make_r32(9);
static constexpr Gp w10 = Gp::make_r32(10);
static constexpr Gp w11 = Gp::make_r32(11);
static constexpr Gp w12 = Gp::make_r32(12);
static constexpr Gp w13 = Gp::make_r32(13);
static constexpr Gp w14 = Gp::make_r32(14);
static constexpr Gp w15 = Gp::make_r32(15);
static constexpr Gp w16 = Gp::make_r32(16);
static constexpr Gp w17 = Gp::make_r32(17);
static constexpr Gp w18 = Gp::make_r32(18);
static constexpr Gp w19 = Gp::make_r32(19);
static constexpr Gp w20 = Gp::make_r32(20);
static constexpr Gp w21 = Gp::make_r32(21);
static constexpr Gp w22 = Gp::make_r32(22);
static constexpr Gp w23 = Gp::make_r32(23);
static constexpr Gp w24 = Gp::make_r32(24);
static constexpr Gp w25 = Gp::make_r32(25);
static constexpr Gp w26 = Gp::make_r32(26);
static constexpr Gp w27 = Gp::make_r32(27);
static constexpr Gp w28 = Gp::make_r32(28);
static constexpr Gp w29 = Gp::make_r32(29);
static constexpr Gp w30 = Gp::make_r32(30);
static constexpr Gp wzr = Gp::make_r32(Gp::kIdZr);
static constexpr Gp wsp = Gp::make_r32(Gp::kIdSp);

static constexpr Gp x0 = Gp::make_r64(0);
static constexpr Gp x1 = Gp::make_r64(1);
static constexpr Gp x2 = Gp::make_r64(2);
static constexpr Gp x3 = Gp::make_r64(3);
static constexpr Gp x4 = Gp::make_r64(4);
static constexpr Gp x5 = Gp::make_r64(5);
static constexpr Gp x6 = Gp::make_r64(6);
static constexpr Gp x7 = Gp::make_r64(7);
static constexpr Gp x8 = Gp::make_r64(8);
static constexpr Gp x9 = Gp::make_r64(9);
static constexpr Gp x10 = Gp::make_r64(10);
static constexpr Gp x11 = Gp::make_r64(11);
static constexpr Gp x12 = Gp::make_r64(12);
static constexpr Gp x13 = Gp::make_r64(13);
static constexpr Gp x14 = Gp::make_r64(14);
static constexpr Gp x15 = Gp::make_r64(15);
static constexpr Gp x16 = Gp::make_r64(16);
static constexpr Gp x17 = Gp::make_r64(17);
static constexpr Gp x18 = Gp::make_r64(18);
static constexpr Gp x19 = Gp::make_r64(19);
static constexpr Gp x20 = Gp::make_r64(20);
static constexpr Gp x21 = Gp::make_r64(21);
static constexpr Gp x22 = Gp::make_r64(22);
static constexpr Gp x23 = Gp::make_r64(23);
static constexpr Gp x24 = Gp::make_r64(24);
static constexpr Gp x25 = Gp::make_r64(25);
static constexpr Gp x26 = Gp::make_r64(26);
static constexpr Gp x27 = Gp::make_r64(27);
static constexpr Gp x28 = Gp::make_r64(28);
static constexpr Gp x29 = Gp::make_r64(29);
static constexpr Gp x30 = Gp::make_r64(30);
static constexpr Gp xzr = Gp::make_r64(Gp::kIdZr);
static constexpr Gp sp = Gp::make_r64(Gp::kIdSp);

static constexpr Vec b0 = Vec::make_v8(0);
static constexpr Vec b1 = Vec::make_v8(1);
static constexpr Vec b2 = Vec::make_v8(2);
static constexpr Vec b3 = Vec::make_v8(3);
static constexpr Vec b4 = Vec::make_v8(4);
static constexpr Vec b5 = Vec::make_v8(5);
static constexpr Vec b6 = Vec::make_v8(6);
static constexpr Vec b7 = Vec::make_v8(7);
static constexpr Vec b8 = Vec::make_v8(8);
static constexpr Vec b9 = Vec::make_v8(9);
static constexpr Vec b10 = Vec::make_v8(10);
static constexpr Vec b11 = Vec::make_v8(11);
static constexpr Vec b12 = Vec::make_v8(12);
static constexpr Vec b13 = Vec::make_v8(13);
static constexpr Vec b14 = Vec::make_v8(14);
static constexpr Vec b15 = Vec::make_v8(15);
static constexpr Vec b16 = Vec::make_v8(16);
static constexpr Vec b17 = Vec::make_v8(17);
static constexpr Vec b18 = Vec::make_v8(18);
static constexpr Vec b19 = Vec::make_v8(19);
static constexpr Vec b20 = Vec::make_v8(20);
static constexpr Vec b21 = Vec::make_v8(21);
static constexpr Vec b22 = Vec::make_v8(22);
static constexpr Vec b23 = Vec::make_v8(23);
static constexpr Vec b24 = Vec::make_v8(24);
static constexpr Vec b25 = Vec::make_v8(25);
static constexpr Vec b26 = Vec::make_v8(26);
static constexpr Vec b27 = Vec::make_v8(27);
static constexpr Vec b28 = Vec::make_v8(28);
static constexpr Vec b29 = Vec::make_v8(29);
static constexpr Vec b30 = Vec::make_v8(30);
static constexpr Vec b31 = Vec::make_v8(31);

static constexpr Vec h0 = Vec::make_v16(0);
static constexpr Vec h1 = Vec::make_v16(1);
static constexpr Vec h2 = Vec::make_v16(2);
static constexpr Vec h3 = Vec::make_v16(3);
static constexpr Vec h4 = Vec::make_v16(4);
static constexpr Vec h5 = Vec::make_v16(5);
static constexpr Vec h6 = Vec::make_v16(6);
static constexpr Vec h7 = Vec::make_v16(7);
static constexpr Vec h8 = Vec::make_v16(8);
static constexpr Vec h9 = Vec::make_v16(9);
static constexpr Vec h10 = Vec::make_v16(10);
static constexpr Vec h11 = Vec::make_v16(11);
static constexpr Vec h12 = Vec::make_v16(12);
static constexpr Vec h13 = Vec::make_v16(13);
static constexpr Vec h14 = Vec::make_v16(14);
static constexpr Vec h15 = Vec::make_v16(15);
static constexpr Vec h16 = Vec::make_v16(16);
static constexpr Vec h17 = Vec::make_v16(17);
static constexpr Vec h18 = Vec::make_v16(18);
static constexpr Vec h19 = Vec::make_v16(19);
static constexpr Vec h20 = Vec::make_v16(20);
static constexpr Vec h21 = Vec::make_v16(21);
static constexpr Vec h22 = Vec::make_v16(22);
static constexpr Vec h23 = Vec::make_v16(23);
static constexpr Vec h24 = Vec::make_v16(24);
static constexpr Vec h25 = Vec::make_v16(25);
static constexpr Vec h26 = Vec::make_v16(26);
static constexpr Vec h27 = Vec::make_v16(27);
static constexpr Vec h28 = Vec::make_v16(28);
static constexpr Vec h29 = Vec::make_v16(29);
static constexpr Vec h30 = Vec::make_v16(30);
static constexpr Vec h31 = Vec::make_v16(31);

static constexpr Vec s0 = Vec::make_v32(0);
static constexpr Vec s1 = Vec::make_v32(1);
static constexpr Vec s2 = Vec::make_v32(2);
static constexpr Vec s3 = Vec::make_v32(3);
static constexpr Vec s4 = Vec::make_v32(4);
static constexpr Vec s5 = Vec::make_v32(5);
static constexpr Vec s6 = Vec::make_v32(6);
static constexpr Vec s7 = Vec::make_v32(7);
static constexpr Vec s8 = Vec::make_v32(8);
static constexpr Vec s9 = Vec::make_v32(9);
static constexpr Vec s10 = Vec::make_v32(10);
static constexpr Vec s11 = Vec::make_v32(11);
static constexpr Vec s12 = Vec::make_v32(12);
static constexpr Vec s13 = Vec::make_v32(13);
static constexpr Vec s14 = Vec::make_v32(14);
static constexpr Vec s15 = Vec::make_v32(15);
static constexpr Vec s16 = Vec::make_v32(16);
static constexpr Vec s17 = Vec::make_v32(17);
static constexpr Vec s18 = Vec::make_v32(18);
static constexpr Vec s19 = Vec::make_v32(19);
static constexpr Vec s20 = Vec::make_v32(20);
static constexpr Vec s21 = Vec::make_v32(21);
static constexpr Vec s22 = Vec::make_v32(22);
static constexpr Vec s23 = Vec::make_v32(23);
static constexpr Vec s24 = Vec::make_v32(24);
static constexpr Vec s25 = Vec::make_v32(25);
static constexpr Vec s26 = Vec::make_v32(26);
static constexpr Vec s27 = Vec::make_v32(27);
static constexpr Vec s28 = Vec::make_v32(28);
static constexpr Vec s29 = Vec::make_v32(29);
static constexpr Vec s30 = Vec::make_v32(30);
static constexpr Vec s31 = Vec::make_v32(31);

static constexpr Vec d0 = Vec::make_v64(0);
static constexpr Vec d1 = Vec::make_v64(1);
static constexpr Vec d2 = Vec::make_v64(2);
static constexpr Vec d3 = Vec::make_v64(3);
static constexpr Vec d4 = Vec::make_v64(4);
static constexpr Vec d5 = Vec::make_v64(5);
static constexpr Vec d6 = Vec::make_v64(6);
static constexpr Vec d7 = Vec::make_v64(7);
static constexpr Vec d8 = Vec::make_v64(8);
static constexpr Vec d9 = Vec::make_v64(9);
static constexpr Vec d10 = Vec::make_v64(10);
static constexpr Vec d11 = Vec::make_v64(11);
static constexpr Vec d12 = Vec::make_v64(12);
static constexpr Vec d13 = Vec::make_v64(13);
static constexpr Vec d14 = Vec::make_v64(14);
static constexpr Vec d15 = Vec::make_v64(15);
static constexpr Vec d16 = Vec::make_v64(16);
static constexpr Vec d17 = Vec::make_v64(17);
static constexpr Vec d18 = Vec::make_v64(18);
static constexpr Vec d19 = Vec::make_v64(19);
static constexpr Vec d20 = Vec::make_v64(20);
static constexpr Vec d21 = Vec::make_v64(21);
static constexpr Vec d22 = Vec::make_v64(22);
static constexpr Vec d23 = Vec::make_v64(23);
static constexpr Vec d24 = Vec::make_v64(24);
static constexpr Vec d25 = Vec::make_v64(25);
static constexpr Vec d26 = Vec::make_v64(26);
static constexpr Vec d27 = Vec::make_v64(27);
static constexpr Vec d28 = Vec::make_v64(28);
static constexpr Vec d29 = Vec::make_v64(29);
static constexpr Vec d30 = Vec::make_v64(30);
static constexpr Vec d31 = Vec::make_v64(31);

static constexpr Vec q0 = Vec::make_v128(0);
static constexpr Vec q1 = Vec::make_v128(1);
static constexpr Vec q2 = Vec::make_v128(2);
static constexpr Vec q3 = Vec::make_v128(3);
static constexpr Vec q4 = Vec::make_v128(4);
static constexpr Vec q5 = Vec::make_v128(5);
static constexpr Vec q6 = Vec::make_v128(6);
static constexpr Vec q7 = Vec::make_v128(7);
static constexpr Vec q8 = Vec::make_v128(8);
static constexpr Vec q9 = Vec::make_v128(9);
static constexpr Vec q10 = Vec::make_v128(10);
static constexpr Vec q11 = Vec::make_v128(11);
static constexpr Vec q12 = Vec::make_v128(12);
static constexpr Vec q13 = Vec::make_v128(13);
static constexpr Vec q14 = Vec::make_v128(14);
static constexpr Vec q15 = Vec::make_v128(15);
static constexpr Vec q16 = Vec::make_v128(16);
static constexpr Vec q17 = Vec::make_v128(17);
static constexpr Vec q18 = Vec::make_v128(18);
static constexpr Vec q19 = Vec::make_v128(19);
static constexpr Vec q20 = Vec::make_v128(20);
static constexpr Vec q21 = Vec::make_v128(21);
static constexpr Vec q22 = Vec::make_v128(22);
static constexpr Vec q23 = Vec::make_v128(23);
static constexpr Vec q24 = Vec::make_v128(24);
static constexpr Vec q25 = Vec::make_v128(25);
static constexpr Vec q26 = Vec::make_v128(26);
static constexpr Vec q27 = Vec::make_v128(27);
static constexpr Vec q28 = Vec::make_v128(28);
static constexpr Vec q29 = Vec::make_v128(29);
static constexpr Vec q30 = Vec::make_v128(30);
static constexpr Vec q31 = Vec::make_v128(31);

static constexpr Vec v0 = Vec::make_v128(0);
static constexpr Vec v1 = Vec::make_v128(1);
static constexpr Vec v2 = Vec::make_v128(2);
static constexpr Vec v3 = Vec::make_v128(3);
static constexpr Vec v4 = Vec::make_v128(4);
static constexpr Vec v5 = Vec::make_v128(5);
static constexpr Vec v6 = Vec::make_v128(6);
static constexpr Vec v7 = Vec::make_v128(7);
static constexpr Vec v8 = Vec::make_v128(8);
static constexpr Vec v9 = Vec::make_v128(9);
static constexpr Vec v10 = Vec::make_v128(10);
static constexpr Vec v11 = Vec::make_v128(11);
static constexpr Vec v12 = Vec::make_v128(12);
static constexpr Vec v13 = Vec::make_v128(13);
static constexpr Vec v14 = Vec::make_v128(14);
static constexpr Vec v15 = Vec::make_v128(15);
static constexpr Vec v16 = Vec::make_v128(16);
static constexpr Vec v17 = Vec::make_v128(17);
static constexpr Vec v18 = Vec::make_v128(18);
static constexpr Vec v19 = Vec::make_v128(19);
static constexpr Vec v20 = Vec::make_v128(20);
static constexpr Vec v21 = Vec::make_v128(21);
static constexpr Vec v22 = Vec::make_v128(22);
static constexpr Vec v23 = Vec::make_v128(23);
static constexpr Vec v24 = Vec::make_v128(24);
static constexpr Vec v25 = Vec::make_v128(25);
static constexpr Vec v26 = Vec::make_v128(26);
static constexpr Vec v27 = Vec::make_v128(27);
static constexpr Vec v28 = Vec::make_v128(28);
static constexpr Vec v29 = Vec::make_v128(29);
static constexpr Vec v30 = Vec::make_v128(30);
static constexpr Vec v31 = Vec::make_v128(31);

#ifndef _DOXYGEN
} // {regs}

// Make `a64::regs` accessible through `a64` namespace as well.
using namespace regs;
#endif

//! \name Shift Operation Construction
//! \{

//! Constructs a `LSL #value` shift (logical shift left).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsl(uint32_t value) noexcept { return Shift(ShiftOp::kLSL, value); }

//! Constructs a `LSR #value` shift (logical shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift lsr(uint32_t value) noexcept { return Shift(ShiftOp::kLSR, value); }

//! Constructs a `ASR #value` shift (arithmetic shift right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift asr(uint32_t value) noexcept { return Shift(ShiftOp::kASR, value); }

//! Constructs a `ROR #value` shift (rotate right).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift ror(uint32_t value) noexcept { return Shift(ShiftOp::kROR, value); }

//! Constructs a `RRX` shift (rotate with carry by 1).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift rrx() noexcept { return Shift(ShiftOp::kRRX, 0); }

//! Constructs a `MSL #value` shift (logical shift left filling ones).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift msl(uint32_t value) noexcept { return Shift(ShiftOp::kMSL, value); }

//! Constructs a `UXTB #value` extend and shift (unsigned byte extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift uxtb(uint32_t value) noexcept { return Shift(ShiftOp::kUXTB, value); }

//! Constructs a `UXTH #value` extend and shift (unsigned hword extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift uxth(uint32_t value) noexcept { return Shift(ShiftOp::kUXTH, value); }

//! Constructs a `UXTW #value` extend and shift (unsigned word extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift uxtw(uint32_t value) noexcept { return Shift(ShiftOp::kUXTW, value); }

//! Constructs a `UXTX #value` extend and shift (unsigned dword extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift uxtx(uint32_t value) noexcept { return Shift(ShiftOp::kUXTX, value); }

//! Constructs a `SXTB #value` extend and shift (signed byte extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift sxtb(uint32_t value) noexcept { return Shift(ShiftOp::kSXTB, value); }

//! Constructs a `SXTH #value` extend and shift (signed hword extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift sxth(uint32_t value) noexcept { return Shift(ShiftOp::kSXTH, value); }

//! Constructs a `SXTW #value` extend and shift (signed word extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift sxtw(uint32_t value) noexcept { return Shift(ShiftOp::kSXTW, value); }

//! Constructs a `SXTX #value` extend and shift (signed dword extend) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Shift sxtx(uint32_t value) noexcept { return Shift(ShiftOp::kSXTX, value); }

//! \}

//! \name Memory Operand Construction
//! \{

//! Creates `[base, offset]` memory operand (offset mode) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base, offset]!` memory operand (pre-index mode) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], offset` memory operand (post-index mode) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index]` memory operand (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index);
}

//! Creates `[base, index]!` memory operand (pre-index mode) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_pre(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPreIndex));
}

//! Creates `[base], index` memory operand (post-index mode) (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr_post(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index, OperandSignature::from_value<Mem::kSignatureMemOffsetModeMask>(OffsetMode::kPostIndex));
}

//! Creates `[base, index, SHIFT_OP #shift]` memory operand (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift);
}

//! Creates `[base, offset]` memory operand (AArch64).
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const Label& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base]` absolute memory operand (AArch32 or AArch64).
//!
//! \note The concept of absolute memory operands doesn't exist on ARM, the ISA only provides PC relative addressing.
//! Absolute memory operands can only be used if it's known that the PC relative offset is encodable and that it
//! would be within the limits. Absolute address is also often output from disassemblers, so AsmJit supports it to
//! make it possible to assemble such output back.
static ASMJIT_INLINE_CONSTEXPR Mem ptr(uint64_t base) noexcept { return Mem(base); }

// TODO: [ARM] PC + offset address.
#if 0
//! Creates `[PC + offset]` (relative) memory operand.
static ASMJIT_INLINE_CONSTEXPR Mem ptr(const PC& pc, int32_t offset = 0) noexcept {
  return Mem(pc, offset);
}
#endif

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A64OPERAND_H_INCLUDED
