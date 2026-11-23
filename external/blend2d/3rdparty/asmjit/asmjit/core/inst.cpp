// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/archtraits.h>
#include <asmjit/core/inst.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86instapi_p.h>
#endif

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64instapi_p.h>
#endif

ASMJIT_BEGIN_NAMESPACE

// InstAPI - InstId <-> String
// ===========================

#ifndef ASMJIT_NO_TEXT
Error InstAPI::inst_id_to_string(Arch arch, InstId inst_id, InstStringifyOptions options, String& output) noexcept {
#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::InstInternal::inst_id_to_string(inst_id, options, output);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::InstInternal::inst_id_to_string(inst_id, options, output);
  }
#endif

  return make_error(Error::kInvalidArch);
}

InstId InstAPI::string_to_inst_id(Arch arch, const char* s, size_t len) noexcept {
#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::InstInternal::string_to_inst_id(s, len);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::InstInternal::string_to_inst_id(s, len);
  }
#endif

  return 0;
}
#endif // !ASMJIT_NO_TEXT

// InstAPI - Validate
// ==================

#ifndef ASMJIT_NO_INTROSPECTION
Error InstAPI::validate(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    if (arch == Arch::kX86) {
      return x86::InstInternal::validate_x86(inst, operands, op_count, validation_flags);
    }
    else {
      return x86::InstInternal::validate_x64(inst, operands, op_count, validation_flags);
    }
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::InstInternal::validate(inst, operands, op_count, validation_flags);
  }
#endif

  return make_error(Error::kInvalidArch);
}
#endif // !ASMJIT_NO_INTROSPECTION

// InstAPI - QueryRWInfo
// =====================

#ifndef ASMJIT_NO_INTROSPECTION
Error InstAPI::query_rw_info(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept {
  if (ASMJIT_UNLIKELY(op_count > Globals::kMaxOpCount)) {
    return make_error(Error::kInvalidArgument);
  }

#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::InstInternal::query_rw_info(arch, inst, operands, op_count, out);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::InstInternal::query_rw_info(inst, operands, op_count, out);
  }
#endif

  return make_error(Error::kInvalidArch);
}
#endif // !ASMJIT_NO_INTROSPECTION

// InstAPI - QueryFeatures
// =======================

#ifndef ASMJIT_NO_INTROSPECTION
Error InstAPI::query_features(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept {
#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::InstInternal::query_features(arch, inst, operands, op_count, out);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::InstInternal::query_features(inst, operands, op_count, out);
  }
#endif

  return make_error(Error::kInvalidArch);
}
#endif // !ASMJIT_NO_INTROSPECTION

ASMJIT_END_NAMESPACE
