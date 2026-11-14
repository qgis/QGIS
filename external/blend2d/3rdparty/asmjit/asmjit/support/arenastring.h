// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_SUPPORT_ARENASTRING_H_INCLUDED
#define ASMJIT_SUPPORT_ARENASTRING_H_INCLUDED

#include <asmjit/core/globals.h>
#include <asmjit/support/arena.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_support
//! \{

//! A helper class used by \ref ArenaString implementation.
struct ArenaStringBase {
  union {
    struct {
      uint32_t _size;
      char _embedded[sizeof(void*) * 2 - 4];
    };
    struct {
      void* _dummy;
      char* _external;
    };
  };

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _dummy = nullptr;
    _external = nullptr;
  }

  Error set_data(Arena& arena, uint32_t max_embedded_size, const char* str, size_t size) noexcept {
    if (size == SIZE_MAX)
      size = strlen(str);

    if (size <= max_embedded_size) {
      memcpy(_embedded, str, size);
      _embedded[size] = '\0';
    }
    else {
      char* external = static_cast<char*>(arena.dup(str, size, true));
      if (ASMJIT_UNLIKELY(!external))
        return make_error(Error::kOutOfMemory);
      _external = external;
    }

    _size = uint32_t(size);
    return Error::kOk;
  }
};

//! A string template that can be arena-allocated.
//!
//! Helps with creating strings that can be either statically allocated if they are small, or externally allocated
//! in case their size exceeds the limit. The `N` represents the size of the whole `ArenaString` structure, based on
//! that size the maximum size of the internal buffer is determined.
template<size_t N>
class ArenaString {
public:
  //! \name Constants
  //! \{

  static inline constexpr uint32_t kWholeSize = (N > sizeof(ArenaStringBase)) ? uint32_t(N) : uint32_t(sizeof(ArenaStringBase));
  static inline constexpr uint32_t kMaxEmbeddedSize = kWholeSize - 5;

  //! \}

  //! \name Members
  //! \{

  union {
    ArenaStringBase _base;
    char _whole_data[kWholeSize];
  };

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ArenaString() noexcept { reset(); }
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _base.reset(); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the string is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_empty() const noexcept { return _base._size == 0; }

  //! Returns the string data.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* data() const noexcept { return _base._size <= kMaxEmbeddedSize ? _base._embedded : _base._external; }

  //! Returns the string size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t size() const noexcept { return _base._size; }

  //! Tests whether the string is embedded (e.g. no dynamically allocated).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_embedded() const noexcept { return _base._size <= kMaxEmbeddedSize; }

  //! Copies a new `data` of the given `size` to the string.
  //!
  //! If the `size` exceeds the internal buffer the given `arena` will be used to duplicate the data, otherwise
  //! the internal buffer will be used as a storage.
  ASMJIT_INLINE_NODEBUG Error set_data(Arena& arena, const char* data, size_t size) noexcept {
    return _base.set_data(arena, kMaxEmbeddedSize, data, size);
  }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_SUPPORT_ARENASTRING_H_INCLUDED
