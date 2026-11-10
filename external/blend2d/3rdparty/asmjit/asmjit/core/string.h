// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_STRING_H_INCLUDED
#define ASMJIT_CORE_STRING_H_INCLUDED

#include <asmjit/support/span.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_utilities
//! \{

//! Format flags used by \ref String API.
enum class StringFormatFlags : uint32_t {
  //! No flags.
  kNone = 0x00000000u,
  //! Show sign.
  kShowSign = 0x00000001u,
  //! Show space.
  kShowSpace = 0x00000002u,
  //! Alternate form (use 0x when formatting HEX number).
  kAlternate = 0x00000004u,
  //! The input is signed.
  kSigned = 0x80000000u
};
ASMJIT_DEFINE_ENUM_FLAGS(StringFormatFlags)

//! Fixed string - only useful for strings that would never exceed `N - 1` characters; always null-terminated.
template<size_t N>
union FixedString {
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kNumUInt32Words = uint32_t((N + sizeof(uint32_t) - 1) / sizeof(uint32_t));

  //! \}

  //! \name Members
  //! \{

  char str[kNumUInt32Words * sizeof(uint32_t)];
  uint32_t u32[kNumUInt32Words];

  //! \}

  //! \name Utilities
  //! \{

  [[nodiscard]]
  inline bool equals(const char* other) const noexcept { return strcmp(str, other) == 0; }

  //! \}
};

//! A simple non-reference counted string that uses small string optimization (SSO).
//!
//! This string has 3 allocation possibilities:
//!
//!   1. Small    - embedded buffer is used for up to `kSSOCapacity` characters. This should handle most small
//!                 strings and thus avoid dynamic memory allocation for most use-cases.
//!
//!   2. Large    - string that doesn't fit into an embedded buffer (or string that was truncated from a larger
//!                 buffer) and is owned by AsmJit. When you destroy the string AsmJit would automatically
//!                 release the large buffer.
//!
//!   3. External - like Large (2), however, the large buffer is not owned by AsmJit and won't be released when
//!                 the string is destroyed or reallocated. This is mostly useful for working with larger temporary
//!                 strings allocated on stack or with immutable strings.
class String {
public:
  ASMJIT_NONCOPYABLE(String)

  //! String operation.
  enum class ModifyOp : uint32_t {
    //! Assignment - a new content replaces the current one.
    kAssign = 0,
    //! Append - a new content is appended to the string.
    kAppend = 1
  };

  //! \cond INTERNAL
  static inline constexpr uint32_t kLayoutSize = 32;
  static inline constexpr uint32_t kSSOCapacity = kLayoutSize - 2;

  //! Large string (owned by String).
  static inline constexpr uint8_t kTypeLarge = 0x1Fu;
  //! External string (arena allocated or not owned by String).
  static inline constexpr uint8_t kTypeExternal = 0x20u;

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_external(uint8_t type) noexcept { return type == kTypeExternal; }

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG bool is_large_or_external(uint8_t type) noexcept { return type >= kTypeLarge; }

  union Raw {
    uint8_t u8[kLayoutSize];
    uint64_t u64[kLayoutSize / sizeof(uint64_t)];
    uintptr_t uptr[kLayoutSize / sizeof(uintptr_t)];
  };

  struct Small {
    uint8_t type;
    char data[kSSOCapacity + 1u];
  };

  struct Large {
    uint8_t type;
    uint8_t reserved[sizeof(uintptr_t) - 1];
    size_t size;
    size_t capacity;
    char* data;
  };

  union {
    uint8_t _type;
    Raw _raw;
    Small _small;
    Large _large;
  };
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  //! Creates a default-initialized string if zero length.
  ASMJIT_INLINE_NODEBUG String() noexcept
    : _small {} {}

  //! Creates a string that takes ownership of the content of the `other` string.
  ASMJIT_INLINE_NODEBUG String(String&& other) noexcept {
    _raw = other._raw;
    other._reset_internal();
  }

  ASMJIT_INLINE_NODEBUG ~String() noexcept {
    reset();
  }

  //! Reset the string into a construction state.
  ASMJIT_API Error reset() noexcept;

  //! \}

  //! \name Overloaded Operators
  //! \{

  inline String& operator=(String&& other) noexcept {
    swap(other);
    other.reset();
    return *this;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const char* other) const noexcept { return  equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const char* other) const noexcept { return !equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const String& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const String& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_external() const noexcept { return is_external(_type); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_large_or_external() const noexcept { return is_large_or_external(_type); }

  //! Tests whether the string is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return size() == 0; }

  //! Returns the size of the string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return is_large_or_external() ? size_t(_large.size) : size_t(_type); }

  //! Returns the capacity of the string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t capacity() const noexcept { return is_large_or_external() ? _large.capacity : size_t(kSSOCapacity); }

  //! Returns the data of the string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG char* data() noexcept { return is_large_or_external() ? _large.data : _small.data; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* data() const noexcept { return is_large_or_external() ? _large.data : _small.data; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG char* begin() noexcept { return data(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* begin() const noexcept { return data(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG char* end() noexcept { return data() + size(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* end() const noexcept { return data() + size(); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<char> as_span() noexcept { return Span<char>(data(), size()); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<const char> as_span() const noexcept { return Span<const char>(data(), size()); }

  //! \}

  //! \name String Operations
  //! \{

  //! Swaps the content of this string with `other`.
  ASMJIT_INLINE_NODEBUG void swap(String& other) noexcept {
    std::swap(_raw, other._raw);
  }

  //! Clears the content of the string.
  ASMJIT_API Error clear() noexcept;

  [[nodiscard]]
  ASMJIT_API char* prepare(ModifyOp op, size_t size) noexcept;

  //! \cond INTERNAL
  ASMJIT_API Error _op_string(ModifyOp op, const char* str, size_t size = SIZE_MAX) noexcept;
  ASMJIT_API Error _op_char(ModifyOp op, char c) noexcept;
  ASMJIT_API Error _op_chars(ModifyOp op, char c, size_t n) noexcept;
  ASMJIT_API Error _op_number(ModifyOp op, uint64_t i, uint32_t base = 0, size_t width = 0, StringFormatFlags flags = StringFormatFlags::kNone) noexcept;
  ASMJIT_API Error _op_hex(ModifyOp op, const void* data, size_t size, char separator = '\0') noexcept;
  ASMJIT_API Error _op_format(ModifyOp op, const char* fmt, ...) noexcept;
  ASMJIT_API Error _op_vformat(ModifyOp op, const char* fmt, va_list ap) noexcept;
  //! \endcond

  //! Replaces the current of the string with `data` of the given `size`.
  //!
  //! Null terminated strings can set `size` to `SIZE_MAX`.
  ASMJIT_API Error assign(const char* data, size_t size = SIZE_MAX) noexcept;

  //! Appends the given `span` to the string.
  ASMJIT_INLINE_NODEBUG Error assign(Span<const char> span) noexcept {
    return _op_string(ModifyOp::kAssign, span.data(), span.size());
  }

  //! Replaces the current of the string with `other` string.
  ASMJIT_INLINE_NODEBUG Error assign(const String& other) noexcept {
    return assign(other.data(), other.size());
  }

  //! Replaces the current of the string by a single `c` character.
  ASMJIT_INLINE_NODEBUG Error assign(char c) noexcept {
    return _op_char(ModifyOp::kAssign, c);
  }

  //! Replaces the current of the string by a `c` character, repeated `n` times.
  ASMJIT_INLINE_NODEBUG Error assign_chars(char c, size_t n) noexcept {
    return _op_chars(ModifyOp::kAssign, c, n);
  }

  //! Replaces the current of the string by a formatted integer `i` (signed).
  ASMJIT_INLINE_NODEBUG Error assign_int(int64_t i, uint32_t base = 0, size_t width = 0, StringFormatFlags flags = StringFormatFlags::kNone) noexcept {
    return _op_number(ModifyOp::kAssign, uint64_t(i), base, width, flags | StringFormatFlags::kSigned);
  }

  //! Replaces the current of the string by a formatted integer `i` (unsigned).
  ASMJIT_INLINE_NODEBUG Error assign_uint(uint64_t i, uint32_t base = 0, size_t width = 0, StringFormatFlags flags = StringFormatFlags::kNone) noexcept {
    return _op_number(ModifyOp::kAssign, i, base, width, flags);
  }

  //! Replaces the current of the string by the given `data` converted to a HEX string.
  ASMJIT_INLINE_NODEBUG Error assign_hex(const void* data, size_t size, char separator = '\0') noexcept {
    return _op_hex(ModifyOp::kAssign, data, size, separator);
  }

  //! Replaces the current of the string by a formatted string `fmt`.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Error assign_format(const char* fmt, Args&&... args) noexcept {
    return _op_format(ModifyOp::kAssign, fmt, std::forward<Args>(args)...);
  }

  //! Replaces the current of the string by a formatted string `fmt` (va_list version).
  ASMJIT_INLINE_NODEBUG Error assign_vformat(const char* fmt, va_list ap) noexcept {
    return _op_vformat(ModifyOp::kAssign, fmt, ap);
  }

  //! Appends `str` having the given size `size` to the string.
  //!
  //! Null terminated strings can set `size` to `SIZE_MAX`.
  ASMJIT_INLINE_NODEBUG Error append(const char* str, size_t size = SIZE_MAX) noexcept {
    return _op_string(ModifyOp::kAppend, str, size);
  }

  //! Appends the given `span` to the string.
  ASMJIT_INLINE_NODEBUG Error append(Span<const char> span) noexcept {
    return _op_string(ModifyOp::kAppend, span.data(), span.size());
  }

  //! Appends `other` string to this string.
  ASMJIT_INLINE_NODEBUG Error append(const String& other) noexcept {
    return append(other.data(), other.size());
  }

  //! Appends a single `c` character.
  ASMJIT_INLINE_NODEBUG Error append(char c) noexcept {
    return _op_char(ModifyOp::kAppend, c);
  }

  //! Appends `c` character repeated `n` times.
  ASMJIT_INLINE_NODEBUG Error append_chars(char c, size_t n) noexcept {
    return _op_chars(ModifyOp::kAppend, c, n);
  }

  //! Appends a formatted integer `i` (signed).
  ASMJIT_INLINE_NODEBUG Error append_int(int64_t i, uint32_t base = 0, size_t width = 0, StringFormatFlags flags = StringFormatFlags::kNone) noexcept {
    return _op_number(ModifyOp::kAppend, uint64_t(i), base, width, flags | StringFormatFlags::kSigned);
  }

  //! Appends a formatted integer `i` (unsigned).
  ASMJIT_INLINE_NODEBUG Error append_uint(uint64_t i, uint32_t base = 0, size_t width = 0, StringFormatFlags flags = StringFormatFlags::kNone) noexcept {
    return _op_number(ModifyOp::kAppend, i, base, width, flags);
  }

  //! Appends the given `data` converted to a HEX string.
  ASMJIT_INLINE_NODEBUG Error append_hex(const void* data, size_t size, char separator = '\0') noexcept {
    return _op_hex(ModifyOp::kAppend, data, size, separator);
  }

  //! Appends a formatted string `fmt` with `args`.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Error append_format(const char* fmt, Args&&... args) noexcept {
    return _op_format(ModifyOp::kAppend, fmt, std::forward<Args>(args)...);
  }

  //! Appends a formatted string `fmt` (va_list version).
  ASMJIT_INLINE_NODEBUG Error append_vformat(const char* fmt, va_list ap) noexcept {
    return _op_vformat(ModifyOp::kAppend, fmt, ap);
  }

  ASMJIT_API Error pad_end(size_t n, char c = ' ') noexcept;

  //! Truncate the string length into `new_size`.
  ASMJIT_API Error truncate(size_t new_size) noexcept;

  [[nodiscard]]
  ASMJIT_API bool equals(const char* other, size_t size = SIZE_MAX) const noexcept;

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool equals(const String& other) const noexcept { return equals(other.data(), other.size()); }

  //! \}

  //! \name Internal Functions
  //! \{

  //! Resets string to embedded and makes it empty (zero length, zero first char)
  //!
  //! \note This is always called internally after an external buffer was released as it zeroes all bytes
  //! used by String's embedded storage.
  inline void _reset_internal() noexcept {
    for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_raw.uptr); i++) {
      _raw.uptr[i] = 0;
    }
  }

  inline void _set_size(size_t new_size) noexcept {
    if (is_large_or_external()) {
      _large.size = new_size;
    }
    else {
      _small.type = uint8_t(new_size);
    }
  }

  //! \}
};

//! Temporary string builder, has statically allocated `N` bytes.
template<size_t N>
class StringTmp : public String {
public:
  ASMJIT_NONCOPYABLE(StringTmp)

  //! Embedded data.
  char _embedded_data[Support::align_up(N + 1, sizeof(size_t))];

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG StringTmp() noexcept {
    _reset_to_temporary();
  }

  inline void _reset_to_temporary() noexcept {
    _large.type = kTypeExternal;
    _large.capacity = ASMJIT_ARRAY_SIZE(_embedded_data) - 1;
    _large.data = _embedded_data;
    _embedded_data[0] = '\0';
  }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_STRING_H_INCLUDED
