// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_PIPELINE_JIT_PIPEDEBUG_P_H_INCLUDED
#define BLEND2D_PIPELINE_JIT_PIPEDEBUG_P_H_INCLUDED

#include <blend2d/pipeline/jit/pipecompiler_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_pipeline_jit
//! \{

namespace bl::Pipeline::JIT {

//! Pipeline debugging.
struct PipeDebug {
  static void print_gp(PipeCompiler* pc, const char* key, const Gp& reg) noexcept {
    asmjit::InvokeNode* invoke_node;
    Gp func_ptr = pc->new_gpz("func_ptr");

    if (reg.size() <= 4) {
      pc->mov(func_ptr, Imm((uintptr_t)_printGp32Cb));
      pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int32_t>());
    }
    else {
      pc->mov(func_ptr, Imm((uintptr_t)_printGp64Cb));
      pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int64_t>());
    }

    invoke_node->set_arg(0, imm(key));
    invoke_node->set_arg(1, reg);
  }

  static void load_address(PipeCompiler* pc, const Gp& dst, const Mem& mem) noexcept {
#if defined(BL_JIT_ARCH_X86)
    pc->cc->lea(dst, mem);
#else
    pc->cc->load_address_of(dst, mem);
#endif
  }

  static void printVecU32(PipeCompiler* pc, const char* key, const Vec& reg) noexcept {
    Mem m = pc->cc->new_stack(16, 16, "dump_mem");
    Gp a = pc->cc->new_int_ptr("dump_tmp");

    pc->v_storeu128(m, reg);
    load_address(pc, a, m);

    Gp func_ptr = pc->new_gpz("func_ptr");
    pc->mov(func_ptr, Imm((uintptr_t)_print_xmm_pi_cb));

    asmjit::InvokeNode* invoke_node;
    pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int>());

    invoke_node->set_arg(0, imm(key));
    invoke_node->set_arg(1, a);
  }

  static void printScalarF32(PipeCompiler* pc, const char* key, const Vec& reg) noexcept {
    Mem m = pc->cc->new_stack(16, 16, "dump_mem");
    Gp a = pc->cc->new_int_ptr("dump_tmp");

    pc->v_storeu32(m, reg);
    load_address(pc, a, m);

    Gp func_ptr = pc->new_gpz("func_ptr");
    pc->mov(func_ptr, Imm((uintptr_t)_printScalarF32));

    asmjit::InvokeNode* invoke_node;
    pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int>());

    invoke_node->set_arg(0, imm(key));
    invoke_node->set_arg(1, a);
  }

  static void printVecF32(PipeCompiler* pc, const char* key, const Vec& reg) noexcept {
    Mem m = pc->cc->new_stack(16, 16, "dump_mem");
    Gp a = pc->cc->new_int_ptr("dump_tmp");

    pc->v_storeu128(m, reg);
    load_address(pc, a, m);

    Gp func_ptr = pc->new_gpz("func_ptr");
    pc->mov(func_ptr, Imm((uintptr_t)_print_xmm_ps_cb));

    asmjit::InvokeNode* invoke_node;
    pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int>());

    invoke_node->set_arg(0, imm(key));
    invoke_node->set_arg(1, a);
  }

  static void printVecF64(PipeCompiler* pc, const char* key, const Vec& reg) noexcept {
    Mem m = pc->cc->new_stack(16, 16, "dump_mem");
    Gp a = pc->cc->new_int_ptr("dump_tmp");

    pc->v_storeu128(m, reg);
    load_address(pc, a, m);

    Gp func_ptr = pc->new_gpz("func_ptr");
    pc->mov(func_ptr, Imm((uintptr_t)_print_xmm_pd_cb));

    asmjit::InvokeNode* invoke_node;
    pc->cc->invoke(&invoke_node, func_ptr, asmjit::FuncSignature::build<void, void*, int>());

    invoke_node->set_arg(0, imm(key));
    invoke_node->set_arg(1, a);
  }

  static void _printGp32Cb(const char* key, int32_t value) noexcept {
    bl_runtime_message_fmt("%s=%08X (%u) (%d)\n", key, (unsigned int)value, (unsigned int)value, (int)value);
  }

  static void _printGp64Cb(const char* key, int64_t value) noexcept {
    bl_runtime_message_fmt("%s=%016X (%llu) (%lld)\n", key, (unsigned long long)value, (unsigned long long)value, (long long)value);
  }

  static void _print_xmm_pi_cb(const char* key, const void* data) noexcept {
    const uint32_t* u = static_cast<const uint32_t*>(data);

    bl_runtime_message_fmt("%s=[0x%08X | 0x%08X | 0x%08X | 0x%08X] (%d %d %d %d)\n",
      key,
      (unsigned)u[0], (unsigned)u[1], (unsigned)u[2], (unsigned)u[3],
      (unsigned)u[0], (unsigned)u[1], (unsigned)u[2], (unsigned)u[3]);
  }

  static void _printScalarF32(const char* key, const void* data) noexcept {
    const float* f = static_cast<const float*>(data);
    const uint32_t* u = static_cast<const uint32_t*>(data);

    bl_runtime_message_fmt("%s=[0x%08X (%3.9g)]\n",
      key,
      (unsigned)u[0], f[0]);
  }

  static void _print_xmm_ps_cb(const char* key, const void* data) noexcept {
    const float* f = static_cast<const float*>(data);
    const uint32_t* u = static_cast<const uint32_t*>(data);

    bl_runtime_message_fmt("%s=[0x%08X (%3.9g)  |  0x%08X (%3.9g)  |  0x%08X (%3.9g)  |  0x%08X (%3.9g)]\n",
      key,
      (unsigned)u[0], f[0],
      (unsigned)u[1], f[1],
      (unsigned)u[2], f[2],
      (unsigned)u[3], f[3]);
  }

  static void _print_xmm_pd_cb(const char* key, const void* data) noexcept {
    const double* d = static_cast<const double*>(data);
    const uint64_t* u = static_cast<const uint64_t*>(data);

    bl_runtime_message_fmt("%s=[0x%016llX (%3.9g)  |  0x%016llX (%3.9g)]\n",
      key,
      (unsigned long long)u[0], d[0],
      (unsigned long long)u[1], d[1]);
  }
};

} // {bl::Pipeline::JIT}

//! \}
//! \endcond

#endif // BLEND2D_PIPELINE_JIT_PIPEDEBUG_P_H_INCLUDED
