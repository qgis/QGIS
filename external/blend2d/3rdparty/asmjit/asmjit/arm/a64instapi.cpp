// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH64)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/support/support_p.h>
#include <asmjit/arm/a64instapi_p.h>
#include <asmjit/arm/a64instdb_p.h>
#include <asmjit/arm/a64operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

namespace InstInternal {

// a64::InstInternal - Text
// ========================

#ifndef ASMJIT_NO_TEXT
Error inst_id_to_string(InstId inst_id, InstStringifyOptions options, String& output) noexcept {
  uint32_t real_id = inst_id & uint32_t(InstIdParts::kRealId);
  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(real_id))) {
    return make_error(Error::kInvalidInstruction);
  }

  return InstNameUtils::decode(InstDB::_inst_name_index_table[real_id], options, InstDB::_inst_name_string_table, output);
}

InstId string_to_inst_id(const char* s, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!s)) {
    return BaseInst::kIdNone;
  }

  if (len == SIZE_MAX) {
    len = strlen(s);
  }

  if (len == 0u || len > InstDB::_inst_name_index.max_name_length) {
    return BaseInst::kIdNone;
  }

  return InstNameUtils::find_instruction(s, len, InstDB::_inst_name_index_table, InstDB::_inst_name_string_table, InstDB::_inst_name_index);
}
#endif // !ASMJIT_NO_TEXT

// a64::InstInternal - Validate
// ============================

#ifndef ASMJIT_NO_INTROSPECTION
ASMJIT_FAVOR_SIZE Error validate(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
  // TODO:
  Support::maybe_unused(inst, operands, op_count, validation_flags);
  return Error::kOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

// a64::InstInternal - QueryRWInfo
// ===============================

#ifndef ASMJIT_NO_INTROSPECTION
struct InstRWInfoData {
  uint8_t rwx[Globals::kMaxOpCount];
};

static const InstRWInfoData inst_rw_info_table[] = {
  #define R uint8_t(OpRWFlags::kRead)
  #define W uint8_t(OpRWFlags::kWrite)
  #define X uint8_t(OpRWFlags::kRW)

  {{ R, R, R, R, R, R }}, // kRWI_R
  {{ R, W, R, R, R, R }}, // kRWI_RW
  {{ R, X, R, R, R, R }}, // kRWI_RX
  {{ R, R, W, R, R, R }}, // kRWI_RRW
  {{ R, W, X, R, R, R }}, // kRWI_RWX
  {{ W, R, R, R, R, R }}, // kRWI_W
  {{ W, R, W, R, R, R }}, // kRWI_WRW
  {{ W, R, X, R, R, R }}, // kRWI_WRX
  {{ W, R, R, W, R, R }}, // kRWI_WRRW
  {{ W, R, R, X, R, R }}, // kRWI_WRRX
  {{ W, W, R, R, R, R }}, // kRWI_WW
  {{ X, R, R, R, R, R }}, // kRWI_X
  {{ X, R, X, R, R, R }}, // kRWI_XRX
  {{ X, X, R, R, X, R }}, // kRWI_XXRRX

  {{ W, R, R, R, R, R }}, // kRWI_LDn
  {{ R, W, R, R, R, R }}, // kRWI_STn
  {{ R, R, R, R, R, R }}  // kRWI_TODO

  #undef R
  #undef W
  #undef X
};

static const uint8_t element_type_size_table[8] = { 0, 1, 2, 4, 8, 4, 4, 0 };

Error query_rw_info(const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept {
  // Get the instruction data.
  uint32_t real_id = inst.inst_id() & uint32_t(InstIdParts::kRealId);

  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(real_id))) {
    return make_error(Error::kInvalidInstruction);
  }

  out->_inst_flags = InstRWFlags::kNone;
  out->_op_count = uint8_t(op_count);
  out->_rm_feature = 0;
  out->_extra_reg.reset();
  out->_read_flags = CpuRWFlags::kNone; // TODO: [ARM] Read PSTATUS.
  out->_write_flags = CpuRWFlags::kNone; // TODO: [ARM] Write PSTATUS

  const InstDB::InstInfo& inst_info = InstDB::_inst_info_table[real_id];
  const InstRWInfoData& rw_info = inst_rw_info_table[inst_info.rw_info_index()];

  if (inst_info.has_flag(InstDB::kInstFlagConsecutive) && op_count > 2) {
    for (uint32_t i = 0; i < op_count; i++) {
      OpRWInfo& op = out->_operands[i];
      const Operand_& src_op = operands[i];

      if (!src_op.is_reg_or_mem()) {
        op.reset();
        continue;
      }

      OpRWFlags rw_flags = i < op_count - 1 ? (OpRWFlags)rw_info.rwx[0] : (OpRWFlags)rw_info.rwx[1];

      op._op_flags = rw_flags & ~(OpRWFlags::kZExt);
      op._phys_id = Reg::kIdBad;
      op._rm_size = 0;
      op._reset_reserved();

      uint64_t r_byte_mask = op.is_read() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
      uint64_t w_byte_mask = op.is_write() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

      op._read_byte_mask = r_byte_mask;
      op._write_byte_mask = w_byte_mask;
      op._extend_byte_mask = 0;
      op._consecutive_lead_count = 0;

      if (src_op.is_reg()) {
        if (i == 0) {
          op._consecutive_lead_count = uint8_t(op_count - 1);
        }
        else {
          op.add_op_flags(OpRWFlags::kConsecutive);
        }
      }
      else {
        const Mem& mem_op = src_op.as<Mem>();

        if (mem_op.has_base()) {
          op.add_op_flags(OpRWFlags::kMemBaseRead);
          if ((mem_op.has_index() || mem_op.has_offset()) && mem_op.is_pre_or_post()) {
            op.add_op_flags(OpRWFlags::kMemBaseWrite);
          }
        }

        if (mem_op.has_index()) {
          op.add_op_flags(OpRWFlags::kMemIndexRead);
        }
      }
    }
  }
  else {
    for (uint32_t i = 0; i < op_count; i++) {
      OpRWInfo& op = out->_operands[i];
      const Operand_& src_op = operands[i];

      if (!src_op.is_reg_or_mem()) {
        op.reset();
        continue;
      }

      OpRWFlags rw_flags = (OpRWFlags)rw_info.rwx[i];

      op._op_flags = rw_flags & ~(OpRWFlags::kZExt);
      op._phys_id = Reg::kIdBad;
      op._rm_size = 0;
      op._reset_reserved();

      uint64_t r_byte_mask = op.is_read() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
      uint64_t w_byte_mask = op.is_write() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

      op._read_byte_mask = r_byte_mask;
      op._write_byte_mask = w_byte_mask;
      op._extend_byte_mask = 0;
      op._consecutive_lead_count = 0;

      if (src_op.is_reg()) {
        if (src_op.as<Vec>().has_element_index()) {
          // Only part of the vector is accessed if element index [] is used.
          VecElementType element_type = src_op.as<Vec>().element_type();
          uint32_t element_index = src_op.as<Vec>().element_index();

          uint32_t element_size = element_type_size_table[size_t(element_type)];
          uint64_t access_mask = uint64_t(Support::lsb_mask<uint32_t>(element_size)) << (element_index * element_size);

          op._read_byte_mask &= access_mask;
          op._write_byte_mask &= access_mask;
        }

        // TODO: [ARM] RW info is not finished.
      }
      else {
        const Mem& mem_op = src_op.as<Mem>();

        if (mem_op.has_base()) {
          op.add_op_flags(OpRWFlags::kMemBaseRead);
          if ((mem_op.has_index() || mem_op.has_offset()) && mem_op.is_pre_or_post()) {
            op.add_op_flags(OpRWFlags::kMemBaseWrite);
          }
        }

        if (mem_op.has_index()) {
          op.add_op_flags(OpRWFlags::kMemIndexRead);
        }
      }
    }
  }

  return Error::kOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

// a64::InstInternal - QueryFeatures
// =================================

#ifndef ASMJIT_NO_INTROSPECTION
Error query_features(const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept {
  // TODO: [ARM] QueryFeatures not implemented yet.
  Support::maybe_unused(inst, operands, op_count, out);
  return Error::kOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

// a64::InstInternal - Unit
// ========================

#if defined(ASMJIT_TEST)
UNIT(arm_inst_api_text) {
  // TODO:
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
