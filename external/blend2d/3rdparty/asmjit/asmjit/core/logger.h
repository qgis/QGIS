// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_LOGGING_H_INCLUDED
#define ASMJIT_CORE_LOGGING_H_INCLUDED

#include <asmjit/core/inst.h>
#include <asmjit/core/string.h>
#include <asmjit/core/formatter.h>

#ifndef ASMJIT_NO_LOGGING

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_logging
//! \{

//! Logging interface.
//!
//! This class can be inherited and reimplemented to fit into your own logging needs. When reimplementing a logger
//! use \ref Logger::_log() method to log customize the output.
//!
//! There are two `Logger` implementations offered by AsmJit:
//!   - \ref FileLogger - logs into a `FILE*`.
//!   - \ref StringLogger - concatenates all logs into a \ref String.
class ASMJIT_VIRTAPI Logger {
public:
  ASMJIT_BASE_CLASS(Logger)
  ASMJIT_NONCOPYABLE(Logger)

  //! Format options.
  FormatOptions _options;

  //! \name Construction & Destruction
  //! \{

  //! Creates a `Logger` instance.
  ASMJIT_API Logger() noexcept;
  //! Destroys the `Logger` instance.
  ASMJIT_API virtual ~Logger() noexcept;

  //! \}

  //! \name Format Options
  //! \{

  //! Returns \ref FormatOptions of this logger.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatOptions& options() noexcept { return _options; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FormatOptions& options() const noexcept { return _options; }

  //! Sets formatting options of this Logger to `options`.
  ASMJIT_INLINE_NODEBUG void set_options(const FormatOptions& options) noexcept { _options = options; }

  //! Resets formatting options of this Logger to defaults.
  ASMJIT_INLINE_NODEBUG void reset_options() noexcept { _options.reset(); }

  //! Returns formatting flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatFlags flags() const noexcept { return _options.flags(); }

  //! Tests whether the logger has the given `flag` enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(FormatFlags flag) const noexcept { return _options.has_flag(flag); }

  //! Sets formatting flags to `flags`.
  ASMJIT_INLINE_NODEBUG void set_flags(FormatFlags flags) noexcept { _options.set_flags(flags); }

  //! Enables the given formatting `flags`.
  ASMJIT_INLINE_NODEBUG void add_flags(FormatFlags flags) noexcept { _options.add_flags(flags); }

  //! Disables the given formatting `flags`.
  ASMJIT_INLINE_NODEBUG void clear_flags(FormatFlags flags) noexcept { _options.clear_flags(flags); }

  //! Returns indentation of a given indentation `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t indentation(FormatIndentationGroup type) const noexcept { return _options.indentation(type); }

  //! Sets indentation of the given indentation `group` to `n` spaces.
  ASMJIT_INLINE_NODEBUG void set_indentation(FormatIndentationGroup type, uint32_t n) noexcept { _options.set_indentation(type, n); }

  //! Resets indentation of the given indentation `group` to 0 spaces.
  ASMJIT_INLINE_NODEBUG void reset_indentation(FormatIndentationGroup type) noexcept { _options.reset_indentation(type); }

  //! Returns padding of a given padding `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t padding(FormatPaddingGroup type) const noexcept { return _options.padding(type); }

  //! Sets padding of a given padding `group` to `n`.
  ASMJIT_INLINE_NODEBUG void set_padding(FormatPaddingGroup type, uint32_t n) noexcept { _options.set_padding(type, n); }

  //! Resets padding of a given padding `group` to 0, which means that a default will be used.
  ASMJIT_INLINE_NODEBUG void reset_padding(FormatPaddingGroup type) noexcept { _options.reset_padding(type); }

  //! \}

  //! \name Logging Interface
  //! \{

  //! Logs `str` - must be reimplemented.
  //!
  //! The function can accept either a null terminated string if `size` is `SIZE_MAX` or a non-null terminated
  //! string of the given `size`. The function cannot assume that the data is null terminated and must handle
  //! non-null terminated inputs.
  ASMJIT_API virtual Error _log(const char* data, size_t size) noexcept;

  //! Logs string `str`, which is either null terminated or having size `size`.
  ASMJIT_INLINE_NODEBUG Error log(const char* data, size_t size = SIZE_MAX) noexcept { return _log(data, size); }
  //! Logs content of a string `str`.
  ASMJIT_INLINE_NODEBUG Error log(const String& str) noexcept { return _log(str.data(), str.size()); }

  //! Formats the message by using `snprintf()` and then passes the formatted string to \ref _log().
  ASMJIT_API Error logf(const char* fmt, ...) noexcept;

  //! Formats the message by using `vsnprintf()` and then passes the formatted string to \ref _log().
  ASMJIT_API Error logv(const char* fmt, va_list ap) noexcept;

  //! \}
};

//! Logger that can log to a `FILE*`.
class ASMJIT_VIRTAPI FileLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(FileLogger)

  FILE* _file;

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FileLogger` that logs to `FILE*`.
  ASMJIT_API FileLogger(FILE* file = nullptr) noexcept;
  //! Destroys the `FileLogger`.
  ASMJIT_API ~FileLogger() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the logging output stream or null if the logger has no output stream.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FILE* file() const noexcept { return _file; }

  //! Sets the logging output stream to `stream` or null.
  //!
  //! \note If the `file` is null the logging will be disabled. When a logger is attached to `CodeHolder` or any
  //! emitter the logging API will always be called regardless of the output file. This means that if you really
  //! want to disable logging at emitter level you must not attach a logger to it.
  ASMJIT_INLINE_NODEBUG void set_file(FILE* file) noexcept { _file = file; }

  //! \}

  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};

//! Logger that stores everything in an internal string buffer.
class ASMJIT_VIRTAPI StringLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(StringLogger)

  //! Logger data as string.
  String _content;

  //! \name Construction & Destruction
  //! \{

  //! Create new `StringLogger`.
  ASMJIT_API StringLogger() noexcept;
  //! Destroys the `StringLogger`.
  ASMJIT_API ~StringLogger() noexcept override;

  //! \}

  //! \name Logger Data Accessors
  //! \{

  //! Returns the content of the logger as \ref String.
  //!
  //! It can be moved, if desired.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG String& content() noexcept { return _content; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const String& content() const noexcept { return _content; }

  //! Returns aggregated logger data as `char*` pointer.
  //!
  //! The pointer is owned by `StringLogger`, it can't be modified or freed.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* data() const noexcept { return _content.data(); }

  //! Returns size of the data returned by `data()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t data_size() const noexcept { return _content.size(); }

  //! \}

  //! \name Logger Data Manipulation
  //! \{

  //! Clears the accumulated logger data.
  ASMJIT_INLINE_NODEBUG void clear() noexcept { _content.clear(); }

  //! \}

  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};

//! \}

ASMJIT_END_NAMESPACE

#endif

#endif // ASMJIT_CORE_LOGGER_H_INCLUDED
