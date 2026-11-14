// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_EMITTERUTILS_P_H_INCLUDED
#define ASMJIT_CORE_EMITTERUTILS_P_H_INCLUDED

#include <asmjit/core/emitter.h>
#include <asmjit/core/operand.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

class BaseAssembler;
class FormatOptions;

//! \cond INTERNAL
//! \addtogroup asmjit_core
//! \{

//! Utilities used by various emitters, mostly Assembler implementations.
namespace EmitterUtils {

//! Default paddings used by Emitter utils and Formatter.

static constexpr Operand no_ext[3] = { {}, {}, {} };

enum kOpIndex : uint32_t {
  kOp3 = 0,
  kOp4 = 1,
  kOp5 = 2
};

[[nodiscard]]
static ASMJIT_INLINE uint32_t op_count_from_emit_args(const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) noexcept {
  uint32_t op_count = 0;

  if (op_ext[kOp3].is_none()) {
    if (!o0.is_none()) op_count = 1;
    if (!o1.is_none()) op_count = 2;
    if (!o2.is_none()) op_count = 3;
  }
  else {
    op_count = 4;
    if (!op_ext[kOp4].is_none()) {
      op_count = 5 + uint32_t(!op_ext[kOp5].is_none());
    }
  }

  return op_count;
}

static ASMJIT_INLINE void op_array_from_emit_args(Operand_ dst[Globals::kMaxOpCount], const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext) noexcept {
  dst[0].copy_from(o0);
  dst[1].copy_from(o1);
  dst[2].copy_from(o2);
  dst[3].copy_from(op_ext[kOp3]);
  dst[4].copy_from(op_ext[kOp4]);
  dst[5].copy_from(op_ext[kOp5]);
}

[[nodiscard]]
static bool ASMJIT_INLINE_NODEBUG is_encodable_offset_32(int32_t offset, uint32_t num_bits) noexcept {
  uint32_t n_rev = 32 - num_bits;
  return Support::sar(Support::shl(offset, n_rev), n_rev) == offset;
}

[[nodiscard]]
static bool ASMJIT_INLINE_NODEBUG is_encodable_offset_64(int64_t offset, uint32_t num_bits) noexcept {
  uint32_t n_rev = 64 - num_bits;
  return Support::sar(Support::shl(offset, n_rev), n_rev) == offset;
}

#ifndef ASMJIT_NO_LOGGING
Error finish_formatted_line(String& sb, const FormatOptions& format_options, const uint8_t* bin_data, size_t bin_size, size_t offset_size, size_t imm_size, const char* comment) noexcept;

void log_label_bound(BaseAssembler* self, const Label& label) noexcept;

void log_instruction_emitted(
  BaseAssembler* self,
  InstId inst_id,
  InstOptions options,
  const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext,
  uint32_t rel_size, uint32_t imm_size, uint8_t* after_cursor);

Error log_instruction_failed(
  BaseEmitter* self,
  Error err,
  InstId inst_id,
  InstOptions options,
  const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext);
#endif

}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_EMITTERUTILS_P_H_INCLUDED

