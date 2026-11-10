// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_LOGGING

#include <asmjit/core/logger.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// Logger - Implementation
// =======================

Logger::Logger() noexcept
  : _options() {}
Logger::~Logger() noexcept {}

// [[pure virtual]]
Error Logger::_log(const char* data, size_t size) noexcept {
  Support::maybe_unused(data, size);

  // Do not error in this case - the logger would just sink to /dev/null.
  return Error::kOk;
}

Error Logger::logf(const char* fmt, ...) noexcept {
  Error err;
  va_list ap;

  va_start(ap, fmt);
  err = logv(fmt, ap);
  va_end(ap);

  return err;
}

Error Logger::logv(const char* fmt, va_list ap) noexcept {
  StringTmp<2048> sb;
  ASMJIT_PROPAGATE(sb.append_vformat(fmt, ap));
  return log(sb);
}

// FileLogger - Implementation
// ===========================

FileLogger::FileLogger(FILE* file) noexcept
  : _file(file) {}
FileLogger::~FileLogger() noexcept {}

Error FileLogger::_log(const char* data, size_t size) noexcept {
  if (!_file) {
    return Error::kOk;
  }

  if (size == SIZE_MAX) {
    size = strlen(data);
  }

  fwrite(data, 1, size, _file);
  return Error::kOk;
}

// StringLogger - Implementation
// =============================

StringLogger::StringLogger() noexcept {}
StringLogger::~StringLogger() noexcept {}

Error StringLogger::_log(const char* data, size_t size) noexcept {
  return _content.append(data, size);
}

ASMJIT_END_NAMESPACE

#endif
