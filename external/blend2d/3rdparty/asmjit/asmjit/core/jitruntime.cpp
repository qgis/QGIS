// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_JIT

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/jitruntime.h>

ASMJIT_BEGIN_NAMESPACE

JitRuntime::JitRuntime(const JitAllocator::CreateParams* params) noexcept
  : _allocator(params) {
  _environment = Environment::host();
  _environment.set_object_format(ObjectFormat::kJIT);

  const CpuInfo& host_cpu = CpuInfo::host();
  _cpu_features = host_cpu.features();
  _cpu_hints = host_cpu.hints();
}

JitRuntime::~JitRuntime() noexcept {}

Error JitRuntime::_add(void** dst, CodeHolder* code) noexcept {
  *dst = nullptr;

  ASMJIT_PROPAGATE(code->flatten());
  ASMJIT_PROPAGATE(code->resolve_cross_section_fixups());

  size_t estimated_code_size = code->code_size();
  if (ASMJIT_UNLIKELY(estimated_code_size == 0)) {
    return make_error(Error::kNoCodeGenerated);
  }

  JitAllocator::Span span;
  ASMJIT_PROPAGATE(_allocator.alloc(Out(span), estimated_code_size));

  // Relocate the code.
  CodeHolder::RelocationSummary relocation_summary;
  Error err = code->relocate_to_base(uintptr_t(span.rx()), &relocation_summary);
  if (ASMJIT_UNLIKELY(err != Error::kOk)) {
    _allocator.release(span.rx());
    return err;
  }

  // Recalculate the final code size and shrink the memory we allocated for it
  // in case that some relocations didn't require records in an address table.
  size_t code_size = estimated_code_size - relocation_summary.code_size_reduction;

  // If not true it means that `relocate_to_base()` filled wrong information in `relocation_summary`.
  ASMJIT_ASSERT(code_size == code->code_size());

  _allocator.write(span, [&](JitAllocator::Span& span) noexcept -> Error {
    uint8_t* rw = static_cast<uint8_t*>(span.rw());

    for (Section* section : code->_sections) {
      size_t offset = size_t(section->offset());
      size_t buffer_size = size_t(section->buffer_size());
      size_t virtual_size = size_t(section->virtual_size());

      ASMJIT_ASSERT(offset + buffer_size <= span.size());
      memcpy(rw + offset, section->data(), buffer_size);

      if (virtual_size > buffer_size) {
        ASMJIT_ASSERT(offset + virtual_size <= span.size());
        memset(rw + offset + buffer_size, 0, virtual_size - buffer_size);
      }
    }

    span.shrink(code_size);
    return Error::kOk;
  });

  *dst = span.rx();
  return Error::kOk;
}

Error JitRuntime::_release(void* p) noexcept {
  return _allocator.release(p);
}

ASMJIT_END_NAMESPACE

#endif
