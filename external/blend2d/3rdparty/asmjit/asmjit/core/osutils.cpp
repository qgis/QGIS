// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/osutils_p.h>
#include <asmjit/support/support.h>

#if !defined(_WIN32)
  #include <fcntl.h>
  #include <unistd.h>
#endif

ASMJIT_BEGIN_NAMESPACE

#if !defined(_WIN32)
Error OSUtils::read_file(const char* name, String& dst, size_t max_size) noexcept {
  char* buffer = dst.prepare(String::ModifyOp::kAssign, max_size);
  if (ASMJIT_UNLIKELY(!buffer)) {
    return make_error(Error::kOutOfMemory);
  }

  int fd = ASMJIT_FILE64_API(::open)(name, O_RDONLY);
  if (fd < 0) {
    dst.clear();
    return make_error(Error::kFailedToOpenFile);
  }

  intptr_t len = ::read(fd, buffer, max_size);
  if (len >= 0) {
    buffer[len] = '\0';
    dst._set_size(size_t(len));
  }

  ::close(fd);
  return Error::kOk;
}
#endif

ASMJIT_END_NAMESPACE
