// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CODEBUFFERWRITER_P_H_INCLUDED
#define ASMJIT_CORE_CODEBUFFERWRITER_P_H_INCLUDED

#include <asmjit/core/assembler.h>
#include <asmjit/core/codebuffer.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_assembler
//! \{

struct OffsetFormat;

//! Helper that is used to write into a \ref CodeBuffer held by \ref BaseAssembler.
class CodeWriter {
public:
  uint8_t* _cursor;

  ASMJIT_INLINE_NODEBUG explicit CodeWriter(BaseAssembler* a) noexcept
    : _cursor(a->_buffer_ptr) {}

  [[nodiscard]]
  ASMJIT_INLINE Error ensure_space(BaseAssembler* a, size_t n) noexcept {
    size_t remaining_space = (size_t)(a->_buffer_end - _cursor);
    if (ASMJIT_UNLIKELY(remaining_space < n)) {
      CodeBuffer& buffer = a->_section->_buffer;
      Error err = a->_code->grow_buffer(&buffer, n);
      if (ASMJIT_UNLIKELY(err != Error::kOk)) {
        return a->report_error(err);
      }
      _cursor = a->_buffer_ptr;
    }
    return Error::kOk;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* cursor() const noexcept { return _cursor; }

  ASMJIT_INLINE_NODEBUG void set_cursor(uint8_t* cursor) noexcept { _cursor = cursor; }
  ASMJIT_INLINE_NODEBUG void advance(size_t n) noexcept { _cursor += n; }

  [[nodiscard]]
  ASMJIT_INLINE size_t offset_from(uint8_t* from) const noexcept {
    ASMJIT_ASSERT(_cursor >= from);
    return (size_t)(_cursor - from);
  }

  template<typename T>
  ASMJIT_INLINE void emit8(T val) noexcept {
    using U = std::make_unsigned_t<T>;
    _cursor[0] = uint8_t(U(val) & U(0xFF));
    _cursor++;
  }

  template<typename T, typename Y>
  ASMJIT_INLINE void emit8_if(T val, Y cond) noexcept {
    using U = std::make_unsigned_t<T>;
    ASMJIT_ASSERT(size_t(cond) <= 1u);

    _cursor[0] = uint8_t(U(val) & U(0xFF));
    _cursor += size_t(cond);
  }

  template<typename T>
  ASMJIT_INLINE void emit16u_le(T val) noexcept {
    using U = std::make_unsigned_t<T>;
    Support::storeu_u16_le(_cursor, uint16_t(U(val) & 0xFFFFu));
    _cursor += 2;
  }

  template<typename T>
  ASMJIT_INLINE void emit16u_be(T val) noexcept {
    using U = std::make_unsigned_t<T>;
    Support::storeu_u16_be(_cursor, uint16_t(U(val) & 0xFFFFu));
    _cursor += 2;
  }

  template<typename T>
  ASMJIT_INLINE void emit32u_le(T val) noexcept {
    using U = std::make_unsigned_t<T>;
    Support::storeu_u32_le(_cursor, uint32_t(U(val) & 0xFFFFFFFFu));
    _cursor += 4;
  }

  template<typename T>
  ASMJIT_INLINE void emit32u_be(T val) noexcept {
    using U = std::make_unsigned_t<T>;
    Support::storeu_u32_be(_cursor, uint32_t(U(val) & 0xFFFFFFFFu));
    _cursor += 4;
  }

  ASMJIT_INLINE void emit_data(const void* data, size_t size) noexcept {
    ASMJIT_ASSERT(size != 0);
    memcpy(_cursor, data, size);
    _cursor += size;
  }

  template<typename T>
  ASMJIT_INLINE void emit_value_le(const T& value, size_t size) noexcept {
    using U = std::make_unsigned_t<T>;
    ASMJIT_ASSERT(size <= sizeof(T));

    U v = U(value);
    for (uint32_t i = 0; i < size; i++) {
      _cursor[i] = uint8_t(v & 0xFFu);
      v >>= 8;
    }
    _cursor += size;
  }

  template<typename T>
  ASMJIT_INLINE void emit_value_be(const T& value, size_t size) noexcept {
    using U = std::make_unsigned_t<T>;
    ASMJIT_ASSERT(size <= sizeof(T));

    U v = U(value);
    for (uint32_t i = 0; i < size; i++) {
      _cursor[i] = uint8_t(v >> (sizeof(T) - 8));
      v <<= 8;
    }
    _cursor += size;
  }

  ASMJIT_INLINE void emit_zeros(size_t size) noexcept {
    ASMJIT_ASSERT(size != 0);
    memset(_cursor, 0, size);
    _cursor += size;
  }

  ASMJIT_INLINE void remove8(uint8_t* where) noexcept {
    ASMJIT_ASSERT(where < _cursor);

    uint8_t* p = where;
    while (++p != _cursor)
      p[-1] = p[0];
    _cursor--;
  }

  template<typename T>
  ASMJIT_INLINE void insert8(uint8_t* where, T val) noexcept {
    uint8_t* p = _cursor;

    while (p != where) {
      p[0] = p[-1];
      p--;
    }

    *p = uint8_t(val & 0xFF);
    _cursor++;
  }

  ASMJIT_INLINE void done(BaseAssembler* a) noexcept {
    CodeBuffer& buffer = a->_section->_buffer;
    size_t new_size = (size_t)(_cursor - a->_buffer_data);
    ASMJIT_ASSERT(new_size <= buffer.capacity());

    a->_buffer_ptr = _cursor;
    buffer._size = Support::max(buffer._size, new_size);
  }
};

//! Code writer utilities.
namespace CodeWriterUtils {

[[nodiscard]]
bool encode_offset32(uint32_t* dst, int64_t offset64, const OffsetFormat& format) noexcept;

[[nodiscard]]
bool encode_offset64(uint64_t* dst, int64_t offset64, const OffsetFormat& format) noexcept;

[[nodiscard]]
bool write_offset(void* dst, int64_t offset64, const OffsetFormat& format) noexcept;

} // {CodeWriterUtils}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CODEBUFFERWRITER_P_H_INCLUDED
