// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86)

#include <asmjit/core/misc_p.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::Operand - Tests
// ====================

#if defined(ASMJIT_TEST)
UNIT(x86_operand) {
  Label L(1000); // Label with some ID.

  INFO("Checking basic properties of built-in X86 registers");
  EXPECT_EQ(gpb(Gp::kIdAx), al);
  EXPECT_EQ(gpb(Gp::kIdBx), bl);
  EXPECT_EQ(gpb(Gp::kIdCx), cl);
  EXPECT_EQ(gpb(Gp::kIdDx), dl);

  EXPECT_EQ(gpb_lo(Gp::kIdAx), al);
  EXPECT_EQ(gpb_lo(Gp::kIdBx), bl);
  EXPECT_EQ(gpb_lo(Gp::kIdCx), cl);
  EXPECT_EQ(gpb_lo(Gp::kIdDx), dl);

  EXPECT_EQ(gpb_hi(Gp::kIdAx), ah);
  EXPECT_EQ(gpb_hi(Gp::kIdBx), bh);
  EXPECT_EQ(gpb_hi(Gp::kIdCx), ch);
  EXPECT_EQ(gpb_hi(Gp::kIdDx), dh);

  EXPECT_EQ(gpw(Gp::kIdAx), ax);
  EXPECT_EQ(gpw(Gp::kIdBx), bx);
  EXPECT_EQ(gpw(Gp::kIdCx), cx);
  EXPECT_EQ(gpw(Gp::kIdDx), dx);

  EXPECT_EQ(gpd(Gp::kIdAx), eax);
  EXPECT_EQ(gpd(Gp::kIdBx), ebx);
  EXPECT_EQ(gpd(Gp::kIdCx), ecx);
  EXPECT_EQ(gpd(Gp::kIdDx), edx);

  EXPECT_EQ(gpq(Gp::kIdAx), rax);
  EXPECT_EQ(gpq(Gp::kIdBx), rbx);
  EXPECT_EQ(gpq(Gp::kIdCx), rcx);
  EXPECT_EQ(gpq(Gp::kIdDx), rdx);

  EXPECT_NE(gpb(Gp::kIdAx), dl);
  EXPECT_NE(gpw(Gp::kIdBx), cx);
  EXPECT_NE(gpd(Gp::kIdCx), ebx);
  EXPECT_NE(gpq(Gp::kIdDx), rax);

  INFO("Checking if x86::reg(...) matches built-in IDs");
  EXPECT_EQ(gpb(5), bpl);
  EXPECT_EQ(gpw(5), bp);
  EXPECT_EQ(gpd(5), ebp);
  EXPECT_EQ(gpq(5), rbp);
  EXPECT_EQ(st(5) , st5);
  EXPECT_EQ(mm(5) , mm5);
  EXPECT_EQ(k(5)  , k5);
  EXPECT_EQ(cr(5) , cr5);
  EXPECT_EQ(dr(5) , dr5);
  EXPECT_EQ(xmm(5), xmm5);
  EXPECT_EQ(ymm(5), ymm5);
  EXPECT_EQ(zmm(5), zmm5);

  INFO("Checking x86::Gp register properties");
  EXPECT_TRUE(Gp::make_r32(0).is_reg());
  EXPECT_TRUE(Gp::make_r32(0).is_gp());
  EXPECT_TRUE(Gp::make_r32(0).is_gp32());
  EXPECT_TRUE(eax.is_reg());
  EXPECT_TRUE(eax.is_gp());
  EXPECT_TRUE(eax.is_gp32());
  EXPECT_EQ(eax.id(), 0u);
  EXPECT_EQ(eax.size(), 4u);
  EXPECT_EQ(eax.reg_type(), RegType::kGp32);
  EXPECT_EQ(eax.reg_group(), RegGroup::kGp);

  EXPECT_TRUE(Gp::make_r64(0).is_reg());
  EXPECT_TRUE(Gp::make_r64(0).is_gp());
  EXPECT_TRUE(Gp::make_r64(0).is_gp64());
  EXPECT_TRUE(rax.is_reg());
  EXPECT_TRUE(rax.is_gp());
  EXPECT_TRUE(rax.is_gp64());
  EXPECT_EQ(rax.id(), 0u);
  EXPECT_EQ(rax.size(), 8u);
  EXPECT_EQ(rax.reg_type(), RegType::kGp64);
  EXPECT_EQ(rax.reg_group(), RegGroup::kGp);

  INFO("Checking x86::Vec register properties");
  EXPECT_TRUE(Vec().is_reg());

  EXPECT_TRUE(Vec::make_xmm(0).is_reg());
  EXPECT_TRUE(Vec::make_xmm(0).is_vec128());
  EXPECT_TRUE(xmm4.is_reg());
  EXPECT_TRUE(xmm4.is_vec());
  EXPECT_EQ(xmm4.id(), 4u);
  EXPECT_EQ(xmm4.size(), 16u);
  EXPECT_EQ(xmm4.reg_type(), RegType::kVec128);
  EXPECT_EQ(xmm4.reg_group(), RegGroup::kVec);

  EXPECT_TRUE(Vec::make_ymm(0).is_reg());
  EXPECT_TRUE(Vec::make_ymm(0).is_vec256());
  EXPECT_TRUE(ymm5.is_reg());
  EXPECT_TRUE(ymm5.is_vec());
  EXPECT_EQ(ymm5.id(), 5u);
  EXPECT_EQ(ymm5.size(), 32u);
  EXPECT_EQ(ymm5.reg_type(), RegType::kVec256);
  EXPECT_EQ(ymm5.reg_group(), RegGroup::kVec);

  EXPECT_TRUE(Vec::make_zmm(0).is_reg());
  EXPECT_TRUE(Vec::make_zmm(0).is_vec512());
  EXPECT_TRUE(zmm6.is_reg());
  EXPECT_TRUE(zmm6.is_vec());
  EXPECT_EQ(zmm6.id(), 6u);
  EXPECT_EQ(zmm6.size(), 64u);
  EXPECT_EQ(zmm6.reg_type(), RegType::kVec512);
  EXPECT_EQ(zmm6.reg_group(), RegGroup::kVec);

  // Converts a VEC register to a type of the passed register, but keeps the ID.
  EXPECT_EQ(xmm4.clone_as(ymm10), ymm4);
  EXPECT_EQ(xmm4.clone_as(zmm11), zmm4);
  EXPECT_EQ(ymm5.clone_as(xmm12), xmm5);
  EXPECT_EQ(ymm5.clone_as(zmm13), zmm5);
  EXPECT_EQ(zmm6.clone_as(xmm14), xmm6);
  EXPECT_EQ(zmm6.clone_as(ymm15), ymm6);

  EXPECT_EQ(xmm7.xmm(), xmm7);
  EXPECT_EQ(xmm7.ymm(), ymm7);
  EXPECT_EQ(xmm7.zmm(), zmm7);

  EXPECT_EQ(ymm7.xmm(), xmm7);
  EXPECT_EQ(ymm7.ymm(), ymm7);
  EXPECT_EQ(ymm7.zmm(), zmm7);

  EXPECT_EQ(zmm7.xmm(), xmm7);
  EXPECT_EQ(zmm7.ymm(), ymm7);
  EXPECT_EQ(zmm7.zmm(), zmm7);

  INFO("Checking x86::Mm register properties");
  EXPECT_TRUE(Mm().is_reg());
  EXPECT_TRUE(mm2.is_reg());
  EXPECT_EQ(mm2.id(), 2u);
  EXPECT_EQ(mm2.size(), 8u);
  EXPECT_EQ(mm2.reg_type(), RegType::kX86_Mm);
  EXPECT_EQ(mm2.reg_group(), RegGroup::kX86_MM);

  INFO("Checking x86::KReg register properties");
  EXPECT_TRUE(KReg().is_reg());
  EXPECT_TRUE(k3.is_reg());
  EXPECT_EQ(k3.id(), 3u);
  EXPECT_EQ(k3.size(), 0u);
  EXPECT_EQ(k3.reg_type(), RegType::kMask);
  EXPECT_EQ(k3.reg_group(), RegGroup::kMask);

  INFO("Checking x86::St register properties");
  EXPECT_TRUE(St().is_reg());
  EXPECT_TRUE(st1.is_reg());
  EXPECT_EQ(st1.id(), 1u);
  EXPECT_EQ(st1.size(), 10u);
  EXPECT_EQ(st1.reg_type(), RegType::kX86_St);
  EXPECT_EQ(st1.reg_group(), RegGroup::kX86_St);

  INFO("Checking if default constructed regs behave as expected");
  EXPECT_FALSE(Reg().is_valid());
  EXPECT_FALSE(Gp().is_valid());
  EXPECT_FALSE(Vec().is_valid());
  EXPECT_FALSE(Mm().is_valid());
  EXPECT_FALSE(KReg().is_valid());
  EXPECT_FALSE(SReg().is_valid());
  EXPECT_FALSE(CReg().is_valid());
  EXPECT_FALSE(DReg().is_valid());
  EXPECT_FALSE(St().is_valid());
  EXPECT_FALSE(Bnd().is_valid());

  INFO("Checking x86::Mem operand");
  Mem m;
  EXPECT_EQ(m, Mem());

  m = ptr(L);
  EXPECT_TRUE(m.has_base());
  EXPECT_FALSE(m.has_base_reg());
  EXPECT_TRUE(m.has_base_label());
  EXPECT_FALSE(m.has_offset());
  EXPECT_FALSE(m.is_offset_64bit());
  EXPECT_EQ(m.offset(), 0);
  EXPECT_EQ(m.offset_lo32(), 0);

  m = ptr(0x0123456789ABCDEFu);
  EXPECT_FALSE(m.has_base());
  EXPECT_FALSE(m.has_base_reg());
  EXPECT_FALSE(m.has_index());
  EXPECT_FALSE(m.has_index_reg());
  EXPECT_TRUE(m.has_offset());
  EXPECT_TRUE(m.is_offset_64bit());
  EXPECT_EQ(m.offset(), int64_t(0x0123456789ABCDEFu));
  EXPECT_EQ(m.offset_lo32(), int32_t(0x89ABCDEFu));
  m.add_offset(1);
  EXPECT_EQ(m.offset(), int64_t(0x0123456789ABCDF0u));

  m = ptr(0x0123456789ABCDEFu, rdi, 3);
  EXPECT_FALSE(m.has_segment());
  EXPECT_FALSE(m.has_base());
  EXPECT_FALSE(m.has_base_reg());
  EXPECT_TRUE(m.has_index());
  EXPECT_TRUE(m.has_index_reg());
  EXPECT_EQ(m.index_type(), rdi.reg_type());
  EXPECT_EQ(m.index_id(), rdi.id());
  EXPECT_EQ(m.shift(), 3u);
  EXPECT_TRUE(m.has_offset());
  EXPECT_TRUE(m.is_offset_64bit());
  EXPECT_EQ(m.offset(), int64_t(0x0123456789ABCDEFu));
  EXPECT_EQ(m.offset_lo32(), int32_t(0x89ABCDEFu));
  m.reset_index();
  EXPECT_FALSE(m.has_index());
  EXPECT_FALSE(m.has_index_reg());

  m = ptr(rax);
  EXPECT_TRUE(m.has_base());
  EXPECT_TRUE(m.has_base_reg());
  EXPECT_EQ(m.base_type(), rax.reg_type());
  EXPECT_EQ(m.base_id(), rax.id());
  EXPECT_FALSE(m.has_index());
  EXPECT_FALSE(m.has_index_reg());
  EXPECT_EQ(m.index_type(), RegType::kNone);
  EXPECT_EQ(m.index_id(), 0u);
  EXPECT_FALSE(m.has_offset());
  EXPECT_FALSE(m.is_offset_64bit());
  EXPECT_EQ(m.offset(), 0);
  EXPECT_EQ(m.offset_lo32(), 0);
  m.set_index(rsi);
  EXPECT_TRUE(m.has_index());
  EXPECT_TRUE(m.has_index_reg());
  EXPECT_EQ(m.index_type(), rsi.reg_type());
  EXPECT_EQ(m.index_id(), rsi.id());
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86
