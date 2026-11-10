// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/operand.h>

ASMJIT_BEGIN_NAMESPACE

// Operand - Tests
// ===============

#if defined(ASMJIT_TEST)
enum class StrongEnumForImmTests : uint32_t {
  kValue0,
  kValue0xFFFFFFFF = 0xFFFFFFFFu
};

UNIT(operand) {
  INFO("Checking operand sizes");
  EXPECT_EQ(sizeof(Operand), 16u);
  EXPECT_EQ(sizeof(Reg), 16u);
  EXPECT_EQ(sizeof(BaseMem), 16u);
  EXPECT_EQ(sizeof(Imm), 16u);
  EXPECT_EQ(sizeof(Label), 16u);

  INFO("Checking basic functionality of Operand");
  Operand a, b;
  Operand dummy;

  EXPECT_TRUE(a.is_none());
  EXPECT_FALSE(a.is_reg());
  EXPECT_FALSE(a.is_mem());
  EXPECT_FALSE(a.is_imm());
  EXPECT_FALSE(a.is_label());
  EXPECT_EQ(a, b);
  EXPECT_EQ(a._data[0], 0u);
  EXPECT_EQ(a._data[1], 0u);

  INFO("Checking basic functionality of Label");
  Label label;
  EXPECT_FALSE(label.is_valid());
  EXPECT_EQ(label.id(), Globals::kInvalidId);

  INFO("Checking basic functionality of Reg");
  EXPECT_TRUE(Reg().is_reg());
  EXPECT_FALSE(Reg().is_valid());
  EXPECT_EQ(Reg()._data[0], 0u);
  EXPECT_EQ(Reg()._data[1], 0u);
  EXPECT_FALSE(dummy.as<Reg>().is_valid());

  // Create some register (not specific to any architecture).
  OperandSignature reg_sig = OperandSignature::from_op_type(OperandType::kReg) |
                             OperandSignature::from_reg_type(RegType::kVec128) |
                             OperandSignature::from_reg_group(RegGroup::kVec) |
                             OperandSignature::from_size(8);
  Reg r1(reg_sig, 5);

  EXPECT_TRUE(r1.is_valid());
  EXPECT_TRUE(r1.is_reg());
  EXPECT_TRUE(r1.is_reg(RegType::kVec128));
  EXPECT_TRUE(r1.is_phys_reg());
  EXPECT_FALSE(r1.is_virt_reg());
  EXPECT_EQ(r1.signature(), reg_sig);
  EXPECT_EQ(r1.reg_type(), RegType::kVec128);
  EXPECT_EQ(r1.reg_group(), RegGroup::kVec);
  EXPECT_EQ(r1.size(), 8u);
  EXPECT_EQ(r1.id(), 5u);
  EXPECT_TRUE(r1.is_reg(RegType::kVec128, 5)); // RegType and Id.
  EXPECT_EQ(r1._data[0], 0u);
  EXPECT_EQ(r1._data[1], 0u);

  // The same type of register having different id.
  Reg r2(r1, 6);
  EXPECT_TRUE(r2.is_valid());
  EXPECT_TRUE(r2.is_reg());
  EXPECT_TRUE(r2.is_reg(RegType::kVec128));
  EXPECT_TRUE(r2.is_phys_reg());
  EXPECT_FALSE(r2.is_virt_reg());
  EXPECT_EQ(r2.signature(), reg_sig);
  EXPECT_EQ(r2.reg_type(), r1.reg_type());
  EXPECT_EQ(r2.reg_group(), r1.reg_group());
  EXPECT_EQ(r2.size(), r1.size());
  EXPECT_EQ(r2.id(), 6u);
  EXPECT_TRUE(r2.is_reg(RegType::kVec128, 6));

  r1.reset();
  EXPECT_FALSE(r1.is_reg());
  EXPECT_FALSE(r1.is_valid());

  INFO("Checking basic functionality of BaseMem");
  BaseMem m;
  EXPECT_TRUE(m.is_mem());
  EXPECT_EQ(m, BaseMem());
  EXPECT_FALSE(m.has_base());
  EXPECT_FALSE(m.has_index());
  EXPECT_FALSE(m.has_offset());
  EXPECT_TRUE(m.is_offset_64bit());
  EXPECT_EQ(m.offset(), 0);

  m.set_offset(-1);
  EXPECT_EQ(m.offset_lo32(), -1);
  EXPECT_EQ(m.offset(), -1);

  int64_t x = int64_t(0xFF00FF0000000001u);
  int32_t x_hi = int32_t(0xFF00FF00u);

  m.set_offset(x);
  EXPECT_EQ(m.offset(), x);
  EXPECT_EQ(m.offset_lo32(), 1);
  EXPECT_EQ(m.offset_hi32(), x_hi);

  INFO("Checking basic functionality of Imm");
  Imm imm_value(-42);
  EXPECT_EQ(imm_value.type(), ImmType::kInt);
  EXPECT_EQ(Imm(-1).value(), -1);
  EXPECT_EQ(imm(-1).value(), -1);
  EXPECT_EQ(imm_value.value(), -42);
  EXPECT_EQ(imm(0xFFFFFFFF).value(), int64_t(0xFFFFFFFF));

  Imm imm_double(0.4);
  EXPECT_EQ(imm_double.type(), ImmType::kDouble);
  EXPECT_EQ(imm_double.value_as<double>(), 0.4);
  EXPECT_EQ(imm_double, imm(0.4));

  EXPECT_EQ(Imm(StrongEnumForImmTests::kValue0).value(), 0);
  EXPECT_EQ(Imm(StrongEnumForImmTests::kValue0xFFFFFFFF).value(), 0xFFFFFFFFu);
}
#endif

ASMJIT_END_NAMESPACE
