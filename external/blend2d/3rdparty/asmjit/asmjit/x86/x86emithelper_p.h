// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
#define ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED

#include <asmjit/core/api-config.h>

#include <asmjit/core/emithelper_p.h>
#include <asmjit/core/func.h>
#include <asmjit/x86/x86emitter.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG RegType vec_type_id_to_reg_type(TypeId type_id) noexcept {
  return uint32_t(type_id) <= uint32_t(TypeId::_kVec128End) ? RegType::kVec128 :
         uint32_t(type_id) <= uint32_t(TypeId::_kVec256End) ? RegType::kVec256 : RegType::kVec512;
}

//! Instruction identifiers for targeting SSE and AVX backends.
struct EmitHelperInstructionIds {
  //! 16-bit identifier is enough for us here as X86|X86_64 doesn't have more than 65536 instructions.
  using IId = uint16_t;

  IId _movd_movss[2];
  IId _movq_movsd[2];
  IId _movups_movaps[2];
  IId _movupd_movapd[2];
  IId _movdqu_movdqa[2];
  IId _movlps[1];
  IId _cvtss2sd_cvtsd2ss[2];
  IId _cvtps2pd_cvtpd2ps[2];

  ASMJIT_INLINE_NODEBUG InstId movd() const noexcept { return _movd_movss[0]; }
  ASMJIT_INLINE_NODEBUG InstId movss() const noexcept { return _movd_movss[1]; }
  ASMJIT_INLINE_NODEBUG InstId movd_or_movss(size_t idx) const noexcept { return _movd_movss[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movq() const noexcept { return _movq_movsd[0]; }
  ASMJIT_INLINE_NODEBUG InstId movsd() const noexcept { return _movq_movsd[1]; }
  ASMJIT_INLINE_NODEBUG InstId movq_or_movsd(size_t idx) const noexcept { return _movq_movsd[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movups() const noexcept { return _movups_movaps[0]; }
  ASMJIT_INLINE_NODEBUG InstId movaps() const noexcept { return _movups_movaps[1]; }
  ASMJIT_INLINE_NODEBUG InstId movups_or_movaps(size_t idx) const noexcept { return _movups_movaps[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movupd() const noexcept { return _movupd_movapd[0]; }
  ASMJIT_INLINE_NODEBUG InstId movapd() const noexcept { return _movupd_movapd[1]; }
  ASMJIT_INLINE_NODEBUG InstId movupd_or_movapd(size_t idx) const noexcept { return _movupd_movapd[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movdqu() const noexcept { return _movdqu_movdqa[0]; }
  ASMJIT_INLINE_NODEBUG InstId movdqa() const noexcept { return _movdqu_movdqa[1]; }
  ASMJIT_INLINE_NODEBUG InstId movdqu_or_movdqa(size_t idx) const noexcept { return _movdqu_movdqa[idx]; }

  ASMJIT_INLINE_NODEBUG InstId movlps() const noexcept { return _movlps[0]; }

  ASMJIT_INLINE_NODEBUG InstId cvtss2sd() const noexcept { return _cvtss2sd_cvtsd2ss[0]; }
  ASMJIT_INLINE_NODEBUG InstId cvtsd2ss() const noexcept { return _cvtss2sd_cvtsd2ss[1]; }

  ASMJIT_INLINE_NODEBUG InstId cvtps2pd() const noexcept { return _cvtps2pd_cvtpd2ps[0]; }
  ASMJIT_INLINE_NODEBUG InstId cvtpd2ps() const noexcept { return _cvtps2pd_cvtpd2ps[1]; }
};

//! Emit helper data for SSE at [0] and AVX/AVX-512 at [1].
extern const EmitHelperInstructionIds _emit_helper_instruction_ids[2];

class EmitHelper : public BaseEmitHelper {
protected:
  bool _avx_enabled;
  bool _avx512_enabled;
  const EmitHelperInstructionIds* _ids;

public:
  ASMJIT_INLINE_NODEBUG explicit EmitHelper(BaseEmitter* emitter = nullptr, bool avx_enabled = false, bool avx512_enabled = false) noexcept
    : BaseEmitHelper(emitter) { reset(emitter, avx_enabled, avx512_enabled); }

  ASMJIT_INLINE_NODEBUG ~EmitHelper() noexcept override = default;

  ASMJIT_INLINE_NODEBUG bool is_avx_enabled() const noexcept { return _avx_enabled; }
  ASMJIT_INLINE_NODEBUG bool is_avx512_enabled() const noexcept { return _avx512_enabled; }
  ASMJIT_INLINE_NODEBUG const EmitHelperInstructionIds& ids() const noexcept { return *_ids; }

  ASMJIT_INLINE void reset(BaseEmitter* emitter, bool avx_enabled, bool avx512_enabled) noexcept {
    _emitter = emitter;
    _avx_enabled = avx_enabled || avx512_enabled;
    _avx512_enabled = avx512_enabled;
    _ids = &_emit_helper_instruction_ids[size_t(_avx_enabled)];
  }

  Error emit_reg_move(const Operand_& dst_, const Operand_& src_, TypeId type_id, const char* comment = nullptr) override;
  Error emit_arg_move(const Reg& dst_, TypeId dst_type_id, const Operand_& src_, TypeId src_type_id, const char* comment = nullptr) override;
  Error emit_reg_swap(const Reg& a, const Reg& b, const char* comment = nullptr) override;

  Error emit_prolog(const FuncFrame& frame);
  Error emit_epilog(const FuncFrame& frame);
};

void init_emitter_funcs(BaseEmitter* emitter) noexcept;

static ASMJIT_INLINE void update_emitter_funcs(BaseEmitter* emitter) noexcept {
#ifndef ASMJIT_NO_INTROSPECTION
  emitter->_funcs.validate = emitter->is_32bit() ? InstInternal::validate_x86 : InstInternal::validate_x64;
#else
  Support::maybe_unused(emitter);
#endif
}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
