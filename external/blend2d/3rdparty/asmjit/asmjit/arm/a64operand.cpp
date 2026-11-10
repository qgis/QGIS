// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH64)

#include <asmjit/core/misc_p.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

// a64::Operand - Tests
// ====================

#if defined(ASMJIT_TEST)
UNIT(a64_operand) {
  INFO("Checking if a64::reg(...) matches built-in IDs");
  EXPECT_EQ(w(5), w5);
  EXPECT_EQ(x(5), x5);

  INFO("Checking Gp register properties");
  EXPECT_TRUE(Gp().is_reg());
  EXPECT_TRUE(w0.is_reg());
  EXPECT_TRUE(x0.is_reg());
  EXPECT_EQ(w0.id(), 0u);
  EXPECT_EQ(x0.id(), 0u);
  EXPECT_EQ(wzr.id(), Gp::kIdZr);
  EXPECT_EQ(xzr.id(), Gp::kIdZr);
  EXPECT_EQ(wsp.id(), Gp::kIdSp);
  EXPECT_EQ(sp.id(), Gp::kIdSp);
  EXPECT_EQ(w0.size(), 4u);
  EXPECT_EQ(x0.size(), 8u);
  EXPECT_EQ(w0.reg_type(), RegType::kGp32);
  EXPECT_EQ(x0.reg_type(), RegType::kGp64);
  EXPECT_EQ(w0.reg_group(), RegGroup::kGp);
  EXPECT_EQ(x0.reg_group(), RegGroup::kGp);

  INFO("Checking Vec register properties");
  EXPECT_EQ(v0.reg_type(), RegType::kVec128);
  EXPECT_EQ(d0.reg_type(), RegType::kVec64);
  EXPECT_EQ(s0.reg_type(), RegType::kVec32);
  EXPECT_EQ(h0.reg_type(), RegType::kVec16);
  EXPECT_EQ(b0.reg_type(), RegType::kVec8);

  EXPECT_EQ(v0.reg_group(), RegGroup::kVec);
  EXPECT_EQ(d0.reg_group(), RegGroup::kVec);
  EXPECT_EQ(s0.reg_group(), RegGroup::kVec);
  EXPECT_EQ(h0.reg_group(), RegGroup::kVec);
  EXPECT_EQ(b0.reg_group(), RegGroup::kVec);

  INFO("Checking Vec register element[] access");
  Vec vd_1 = v15.d(1);
  EXPECT_EQ(vd_1.reg_type(), RegType::kVec128);
  EXPECT_EQ(vd_1.reg_group(), RegGroup::kVec);
  EXPECT_EQ(vd_1.id(), 15u);
  EXPECT_TRUE(vd_1.is_vec_d2());
  EXPECT_EQ(vd_1.element_type(), VecElementType::kD);
  EXPECT_TRUE(vd_1.has_element_index());
  EXPECT_EQ(vd_1.element_index(), 1u);

  Vec vs_3 = v15.s(3);
  EXPECT_EQ(vs_3.reg_type(), RegType::kVec128);
  EXPECT_EQ(vs_3.reg_group(), RegGroup::kVec);
  EXPECT_EQ(vs_3.id(), 15u);
  EXPECT_TRUE(vs_3.is_vec_s4());
  EXPECT_EQ(vs_3.element_type(), VecElementType::kS);
  EXPECT_TRUE(vs_3.has_element_index());
  EXPECT_EQ(vs_3.element_index(), 3u);

  Vec vb_4 = v15.b4(3);
  EXPECT_EQ(vb_4.reg_type(), RegType::kVec128);
  EXPECT_EQ(vb_4.reg_group(), RegGroup::kVec);
  EXPECT_EQ(vb_4.id(), 15u);
  EXPECT_TRUE(vb_4.is_vec_b4x4());
  EXPECT_EQ(vb_4.element_type(), VecElementType::kB4);
  EXPECT_TRUE(vb_4.has_element_index());
  EXPECT_EQ(vb_4.element_index(), 3u);
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
