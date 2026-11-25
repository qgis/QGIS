// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef BLEND2D_UNICODE_UNICODE_P_H_INCLUDED
#define BLEND2D_UNICODE_UNICODE_P_H_INCLUDED

#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

//! \cond INTERNAL
//! \addtogroup blend2d_internal
//! \{

namespace bl::Unicode {

// bl::Unicode - Constants
// =======================

//! Special unicode characters.
enum CharCode : uint32_t {
  kCharBOM                    = 0x00FEFFu,   //!< Native Byte-Order-Mark.
  kCharMax                    = 0x10FFFFu,   //!< Last code-point.

  kCharReplacement            = 0x00FFFDu,   //!< Replacement character.

  kCharFVS1                   = 0x00180Bu,   //!< First char in Mongolian 'free variation selectors' FVS1..FVS3.
  kCharFVS3                   = 0x00180Du,   //!< Last char in Mongolian 'free variation selectors' FVS1..FVS3.

  kCharVS1                    = 0x00FE00u,   //!< First char in 'variation selectors' VS1..VS16.
  kCharVS16                   = 0x00FE0Fu,   //!< Last char in 'variation selectors' VS1..VS16.

  kCharVS17                   = 0x0E0100u,   //!< First char in 'variation selectors supplement' VS17..VS256.
  kCharVS256                  = 0x0E01EFu,   //!< Last char in 'variation selectors supplement' VS17..VS256.

  kCharSurrogateFirst         = 0x00D800u,   //!< First surrogate code-point.
  kCharSurrogateLast          = 0x00DFFFu,   //!< Last surrogate code-point.

  kCharHiSurrogateFirst       = 0x00D800u,   //!< First high-surrogate code-point
  kCharHiSurrogateLast        = 0x00DBFFu,   //!< Last high-surrogate code-point

  kCharLoSurrogateFirst       = 0x00DC00u,   //!< First low-surrogate code-point
  kCharLoSurrogateLast        = 0x00DFFFu    //!< Last low-surrogate code-point
};

//! Flags that can be used to parametrize unicode I/O iterators.
enum class IOFlags : uint32_t {
  kNoFlags     = 0u,
  kUnaligned   = 0x00000001u,
  kByteSwap    = 0x00000002u,
  kStrict      = 0x00000004u,
  kCalcIndex   = 0x00000008u,

  kByteOrderLE = BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_LE ? 0 : kByteSwap,
  kByteOrderBE = BL_BYTE_ORDER_NATIVE == BL_BYTE_ORDER_BE ? 0 : kByteSwap
};

BL_DEFINE_ENUM_FLAGS(IOFlags)

// bl::Unicode - Data
// ==================

BL_HIDDEN extern const uint8_t utf8_size_data[256];

// bl::Unicode - Utilities
// =======================

namespace {

template<typename T>
[[nodiscard]]
BL_INLINE uint32_t utf8_char_size(const T& c) noexcept {
  return utf8_size_data[std::make_unsigned_t<T>(c)];
}

template<typename T>
[[nodiscard]]
BL_INLINE bool isValidUtf8(const T& c) noexcept {
  using U = std::make_unsigned_t<T>;
  return U(c) < 128 || (U(c) - U(194) < U(245 - 194));
}

template<typename T>
[[nodiscard]]
constexpr bool is_ascii_alpha(const T& x) noexcept { return T(x | 0x20) >= T('a') && T(x | 0x20) <= T('z'); }

template<typename T>
[[nodiscard]]
constexpr bool is_ascii_digit(const T& x) noexcept { return x >= T('0') && x <= T('9'); }

template<typename T>
[[nodiscard]]
constexpr bool is_ascii_alnum(const T& x) noexcept { return is_ascii_alpha(x) || (x >= T('0') && x <= T('9')); }

template<typename T>
[[nodiscard]]
constexpr T ascii_to_lower(const T& x) noexcept { return x >= T('A') && x <= T('Z') ? T(x |  T(0x20)) : x; }

template<typename T>
[[nodiscard]]
constexpr T ascii_to_upper(const T& x) noexcept { return x >= T('a') && x <= T('z') ? T(x & ~T(0x20)) : x; }

//! Tests whether the unicode character `uc` is high or low surrogate.
template<typename T>
[[nodiscard]]
constexpr bool is_surrogate(const T& uc) noexcept { return uc >= kCharSurrogateFirst && uc <= kCharSurrogateLast; }

//! Tests whether the unicode character `uc` is a high (leading) surrogate.
template<typename T>
[[nodiscard]]
constexpr bool is_hi_surrogate(const T& uc) noexcept { return uc >= kCharHiSurrogateFirst && uc <= kCharHiSurrogateLast; }

//! Tests whether the unicode character `uc` is a low (trailing) surrogate.
template<typename T>
[[nodiscard]]
constexpr bool is_lo_surrogate(const T& uc) noexcept { return uc >= kCharLoSurrogateFirst && uc <= kCharLoSurrogateLast; }

//! Composes `hi` and `lo` surrogates into a unicode code-point.
template<typename T>
[[nodiscard]]
constexpr uint32_t char_from_surrogate(const T& hi, const T& lo) noexcept {
  return (uint32_t(hi) << 10) + uint32_t(lo) - uint32_t((kCharSurrogateFirst << 10) + kCharLoSurrogateFirst - 0x10000u);
}

//! Decomposes a unicode code-point into `hi` and `lo` surrogates.
template<typename T>
BL_INLINE void bl_char_to_surrogate(uint32_t uc, T& hi, T& lo) noexcept {
  uc -= 0x10000u;
  hi = T(kCharHiSurrogateFirst | (uc >> 10));
  lo = T(kCharLoSurrogateFirst | (uc & 0x3FFu));
}

} // {anonymous}

// bl::Unicode - Validation
// ========================

struct ValidationState {
  size_t utf8_index;
  size_t utf16_index;
  size_t utf32_index;

  BL_INLINE void reset() noexcept { *this = ValidationState{}; }

  [[nodiscard]]
  BL_INLINE bool has_smp() const noexcept { return utf16_index != utf32_index; }
};

BL_HIDDEN BLResult bl_validate_unicode(const void* data, size_t size_in_bytes, BLTextEncoding encoding, ValidationState& state) noexcept;

static BL_INLINE BLResult bl_validate_utf8(const char* data, size_t size, ValidationState& state) noexcept {
  return bl_validate_unicode(data, size, BL_TEXT_ENCODING_UTF8, state);
}

static BL_INLINE BLResult bl_validate_utf16(const uint16_t* data, size_t size, ValidationState& state) noexcept {
  return bl_validate_unicode(data, size * 2u, BL_TEXT_ENCODING_UTF16, state);
}

static BL_INLINE BLResult bl_validate_utf32(const uint32_t* data, size_t size, ValidationState& state) noexcept {
  return bl_validate_unicode(data, size * 4u, BL_TEXT_ENCODING_UTF32, state);
}

// bl::Unicode - Conversion
// ========================

struct ConversionState {
  size_t dst_index;
  size_t src_index;

  BL_INLINE void reset() noexcept { *this = ConversionState{}; }
};

//! Converts a string from one encoding to another.
//!
//! Convert function works at a byte level. All sizes here are including those stored
//! in a `bl::Unicode::ConversionState` are byte entities. So for example to convert
//! a single UTF-16 BMP character the source size must be 2, etc...
BL_HIDDEN BLResult convert_unicode(
  void* dst, size_t dst_size_in_bytes, uint32_t dst_encoding,
  const void* src, size_t src_size_in_bytes, uint32_t src_encoding, ConversionState& state) noexcept;

// bl::Unicode - UTF8 Reader
// =========================

//! UTF-8 reader.
class Utf8Reader {
public:
  enum : uint32_t { kCharSize = 1 };

  //! Current pointer.
  const char* _ptr;
  //! End of input.
  const char* _end;
  //! `index() - _utf32_index_subtract` yields the current `utf32_index`.
  size_t _utf32_index_subtract;
  //! Number of surrogates is required to calculate `utf16_index`.
  size_t _utf16_surrogate_count;

  BL_INLINE Utf8Reader(const void* data, size_t byte_size) noexcept {
    reset(data, byte_size);
  }

  BL_INLINE void reset(const void* data, size_t byte_size) noexcept {
    _ptr = static_cast<const char*>(data);
    _end = static_cast<const char*>(data) + byte_size;
    _utf32_index_subtract = 0;
    _utf16_surrogate_count = 0;
  }

  [[nodiscard]]
  BL_INLINE bool has_next() const noexcept { return _ptr != _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_byte_size() const noexcept { return (size_t)(_end - _ptr); }

  [[nodiscard]]
  BL_INLINE size_t byte_index(const void* start) const noexcept { return (size_t)(_ptr - static_cast<const char*>(start)); }

  [[nodiscard]]
  BL_INLINE size_t utf8_index(const void* start) const noexcept { return byte_index(start); }

  [[nodiscard]]
  BL_INLINE size_t utf16_index(const void* start) const noexcept { return utf32_index(start) + _utf16_surrogate_count; }

  [[nodiscard]]
  BL_INLINE size_t utf32_index(const void* start) const noexcept { return byte_index(start) - _utf32_index_subtract; }

  [[nodiscard]]
  BL_INLINE size_t native_index(const void* start) const noexcept { return utf8_index(start); }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc) noexcept {
    size_t uc_size_in_bytes;
    return next<kFlags>(uc, uc_size_in_bytes);
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc, size_t& uc_size_in_bytes) noexcept {
    BL_ASSERT(has_next());

    uc = MemOps::readU8(_ptr);
    uc_size_in_bytes = 1;

    _ptr++;
    if (uc < 0x80u) {
      // 1-Byte UTF-8 Sequence -> [0x00..0x7F].
      // ...nothing to do...
    }
    else {
      // Start of MultiByte.
      const uint32_t kMultiByte = 0xC2u;

      uc -= kMultiByte;
      if (uc < 0xE0u - kMultiByte) {
        // 2-Byte UTF-8 Sequence -> [0x80-0x7FF].
        _ptr++;
        uc_size_in_bytes = 2;

        // Truncated input.
        if (BL_UNLIKELY(_ptr > _end))
          goto TruncatedString;

        // All consecutive bytes must be '10xxxxxx'.
        uint32_t b1 = MemOps::readU8(_ptr - 1) ^ 0x80u;
        uc = ((uc + kMultiByte - 0xC0u) << 6) + b1;

        if (BL_UNLIKELY(b1 > 0x3Fu))
          goto InvalidString;

        // 2-Byte UTF-8 maps to one UTF-16 or UTF-32 code-point, so subtract 1.
        if (bl_test_flag(kFlags, IOFlags::kCalcIndex))
          _utf32_index_subtract += 1;
      }
      else if (uc < 0xF0u - kMultiByte) {
        // 3-Byte UTF-8 Sequence -> [0x800-0xFFFF].
        _ptr += 2;
        uc_size_in_bytes = 3;

        // Truncated input.
        if (BL_UNLIKELY(_ptr > _end))
          goto TruncatedString;

        uint32_t b1 = MemOps::readU8(_ptr - 2) ^ 0x80u;
        uint32_t b2 = MemOps::readU8(_ptr - 1) ^ 0x80u;
        uc = ((uc + kMultiByte - 0xE0u) << 12) + (b1 << 6) + b2;

        // 1. All consecutive bytes must be '10xxxxxx'.
        // 2. Refuse overlong UTF-8.
        if (BL_UNLIKELY((b1 | b2) > 0x3Fu || uc < 0x800u))
          goto InvalidString;

        // 3-Byte UTF-8 maps to one UTF-16 or UTF-32 code-point, so subtract 2.
        if (bl_test_flag(kFlags, IOFlags::kCalcIndex))
          _utf32_index_subtract += 2;
      }
      else {
        // 4-Byte UTF-8 Sequence -> [0x010000-0x10FFFF].
        _ptr += 3;
        uc_size_in_bytes = 4;

        // Truncated input.
        if (BL_UNLIKELY(_ptr > _end)) {
          // If this happens we want to report a correct error, bytes 0xF5
          // and above are always invalid and normally caught later.
          if (uc >= 0xF5u - kMultiByte)
            goto InvalidString;
          else
            goto TruncatedString;
        }

        uint32_t b1 = MemOps::readU8(_ptr - 3) ^ 0x80u;
        uint32_t b2 = MemOps::readU8(_ptr - 2) ^ 0x80u;
        uint32_t b3 = MemOps::readU8(_ptr - 1) ^ 0x80u;
        uc = ((uc + kMultiByte - 0xF0u) << 18) + (b1 << 12) + (b2 << 6) + b3;

        // 1. All consecutive bytes must be '10xxxxxx'.
        // 2. Refuse overlong UTF-8.
        // 3. Make sure the final character is <= U+10FFFF.
        if (BL_UNLIKELY((b1 | b2 | b3) > 0x3Fu || uc < 0x010000u || uc > kCharMax))
          goto InvalidString;

        // 4-Byte UTF-8 maps to one UTF-16 or UTF-32 code-point, so subtract 3.
        if (bl_test_flag(kFlags, IOFlags::kCalcIndex)) {
          _utf32_index_subtract += 3;
          _utf16_surrogate_count += 1;
        }
      }
    }
    return BL_SUCCESS;

InvalidString:
    _ptr -= uc_size_in_bytes;
    return bl_make_error(BL_ERROR_INVALID_STRING);

TruncatedString:
    _ptr -= uc_size_in_bytes;
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  BL_INLINE void skip_one_unit() noexcept {
    BL_ASSERT(has_next());
    _ptr++;
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  [[nodiscard]]
  BL_INLINE BLResult validate() noexcept {
    BLResult result = BL_SUCCESS;
    while (has_next()) {
      uint32_t uc;
      result = next<kFlags>(uc);
      if (result)
        break;
    }
    return result;
  }
};

// bl::Unicode - UTF16 Reader
// ==========================

//! UTF-16 reader.
class Utf16Reader {
public:
  enum : uint32_t { kCharSize = 2 };

  const char* _ptr;
  const char* _end;

  size_t _utf8_index_add;
  size_t _utf16_surrogate_count;

  BL_INLINE Utf16Reader(const void* data, size_t byte_size) noexcept {
    reset(data, byte_size);
  }

  //! \name Reset
  //! \{

  BL_INLINE void reset(const void* data, size_t byte_size) noexcept {
    _ptr = static_cast<const char*>(data);
    _end = static_cast<const char*>(data) + IntOps::align_down(byte_size, 2);
    _utf8_index_add = 0;
    _utf16_surrogate_count = 0;
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  BL_INLINE bool has_next() const noexcept { return _ptr != _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_byte_size() const noexcept { return (size_t)(_end - _ptr); }

  [[nodiscard]]
  BL_INLINE size_t byte_index(const void* start) const noexcept { return (size_t)(_ptr - static_cast<const char*>(start)); }

  [[nodiscard]]
  BL_INLINE size_t utf8_index(const void* start) const noexcept { return utf16_index(start) + _utf8_index_add; }

  [[nodiscard]]
  BL_INLINE size_t utf16_index(const void* start) const noexcept { return byte_index(start) / 2u; }

  [[nodiscard]]
  BL_INLINE size_t utf32_index(const void* start) const noexcept { return utf16_index(start) - _utf16_surrogate_count; }

  [[nodiscard]]
  BL_INLINE size_t native_index(const void* start) const noexcept { return utf16_index(start); }

  //! \}

  //! \name Iterator
  //! \{

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc) noexcept {
    size_t uc_size_in_bytes;
    return next<kFlags>(uc, uc_size_in_bytes);
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc, size_t& uc_size_in_bytes) noexcept {
    BL_ASSERT(has_next());

    uc = readU16<kFlags>(_ptr);
    _ptr += 2;

    if (is_surrogate(uc)) {
      if (BL_LIKELY(is_hi_surrogate(uc))) {
        if (BL_LIKELY(_ptr != _end)) {
          uint32_t lo = readU16<kFlags>(_ptr);
          if (BL_LIKELY(is_lo_surrogate(lo))) {
            uc = char_from_surrogate(uc, lo);
            _ptr += 2;

            // Add two to `_utf8_index_add` as two surrogates count as 2, so we
            // have to add 2 more to have UTF-8 length of a valid surrogate.
            if (bl_test_flag(kFlags, IOFlags::kCalcIndex)) {
              _utf8_index_add += 2;
              _utf16_surrogate_count += 1;
            }

            uc_size_in_bytes = 4;
            return BL_SUCCESS;
          }
          else {
            if (bl_test_flag(kFlags, IOFlags::kStrict))
              goto InvalidString;
          }
        }
        else {
          if (bl_test_flag(kFlags, IOFlags::kStrict))
            goto TruncatedString;
        }
      }
      else {
        if (bl_test_flag(kFlags, IOFlags::kStrict))
          goto InvalidString;
      }
    }

    // Either not surrogate or fallback in non-strict mode.
    if (bl_test_flag(kFlags, IOFlags::kCalcIndex))
      _utf8_index_add += size_t(uc >= 0x0080u) + size_t(uc >= 0x0800u);

    uc_size_in_bytes = 2;
    return BL_SUCCESS;

InvalidString:
    _ptr -= 2;
    return bl_make_error(BL_ERROR_INVALID_STRING);

TruncatedString:
    _ptr -= 2;
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  }

  BL_INLINE void skip_one_unit() noexcept {
    BL_ASSERT(has_next());
    _ptr += 2;
  }

  //! \}

  //! \name Validator
  //! \{

  template<IOFlags kFlags = IOFlags::kNoFlags>
  [[nodiscard]]
  BL_INLINE BLResult validate() noexcept {
    BLResult result = BL_SUCCESS;
    while (has_next()) {
      uint32_t uc;
      result = next<kFlags>(uc);
      if (result)
        break;
    }
    return result;
  }

  //! \}

  //! \name Utilities
  //! \{

  template<IOFlags kFlags = IOFlags::kNoFlags>
  [[nodiscard]]
  static BL_INLINE uint32_t readU16(const char* ptr) noexcept {
    constexpr uint32_t kByteOrder = bl_test_flag(kFlags, IOFlags::kByteSwap) ? BL_BYTE_ORDER_SWAPPED : BL_BYTE_ORDER_NATIVE;
    constexpr uint32_t kAlignment = bl_test_flag(kFlags, IOFlags::kUnaligned) ? 1 : 2;
    return MemOps::readU16<kByteOrder, kAlignment>(ptr);
  }

  //! \}
};

// bl::Unicode - UTF32 Reader
// ==========================

//! UTF-32 reader.
class Utf32Reader {
public:
  enum : uint32_t { kCharSize = 4 };

  const char* _ptr;
  const char* _end;

  size_t _utf8_index_add;
  size_t _utf16_surrogate_count;

  BL_INLINE Utf32Reader(const void* data, size_t byte_size) noexcept {
    reset(data, byte_size);
  }

  BL_INLINE void reset(const void* data, size_t byte_size) noexcept {
    _ptr = static_cast<const char*>(data);
    _end = static_cast<const char*>(data) + IntOps::align_down(byte_size, 4);
    _utf8_index_add = 0;
    _utf16_surrogate_count = 0;
  }

  [[nodiscard]]
  BL_INLINE bool has_next() const noexcept { return _ptr != _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_byte_size() const noexcept { return (size_t)(_end - _ptr); }

  [[nodiscard]]
  BL_INLINE size_t byte_index(const void* start) const noexcept { return (size_t)(_ptr - static_cast<const char*>(start)); }

  [[nodiscard]]
  BL_INLINE size_t utf8_index(const void* start) const noexcept { return utf32_index(start) + _utf16_surrogate_count + _utf8_index_add; }

  [[nodiscard]]
  BL_INLINE size_t utf16_index(const void* start) const noexcept { return utf32_index(start) + _utf16_surrogate_count; }

  [[nodiscard]]
  BL_INLINE size_t utf32_index(const void* start) const noexcept { return byte_index(start) / 4u; }

  [[nodiscard]]
  BL_INLINE size_t native_index(const void* start) const noexcept { return utf32_index(start); }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc) noexcept {
    size_t uc_size_in_bytes;
    return next<kFlags>(uc, uc_size_in_bytes);
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  BL_INLINE BLResult next(uint32_t& uc, size_t& uc_size_in_bytes) noexcept {
    BL_ASSERT(has_next());

    uc = readU32<kFlags>(_ptr);
    if (BL_UNLIKELY(uc > kCharMax))
      return bl_make_error(BL_ERROR_INVALID_STRING);

    if (bl_test_flag(kFlags, IOFlags::kStrict)) {
      if (BL_UNLIKELY(is_surrogate(uc)))
        return bl_make_error(BL_ERROR_INVALID_STRING);
    }

    if (bl_test_flag(kFlags, IOFlags::kCalcIndex)) {
      _utf8_index_add += size_t(uc >= 0x800u) + size_t(uc >= 0x80u);
      _utf16_surrogate_count += size_t(uc >= 0x10000u);
    }

    _ptr += 4;
    uc_size_in_bytes = 4;
    return BL_SUCCESS;
  }

  BL_INLINE void skip_one_unit() noexcept {
    BL_ASSERT(has_next());
    _ptr += 4;
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  [[nodiscard]]
  BL_INLINE BLResult validate() noexcept {
    BLResult result = BL_SUCCESS;
    while (has_next()) {
      uint32_t uc;
      result = next<kFlags>(uc);
      if (result)
        break;
    }
    return result;
  }

  template<IOFlags kFlags = IOFlags::kNoFlags>
  [[nodiscard]]
  static BL_INLINE uint32_t readU32(const char* ptr) noexcept {
    constexpr uint32_t kByteOrder = bl_test_flag(kFlags, IOFlags::kByteSwap) ? BL_BYTE_ORDER_SWAPPED : BL_BYTE_ORDER_NATIVE;
    constexpr uint32_t kAlignment = bl_test_flag(kFlags, IOFlags::kUnaligned) ? 1 : 4;
    return MemOps::readU32<kByteOrder, kAlignment>(ptr);
  }
};

// bl::Unicode - UTF8 Writer
// ===========================

//! UTF8 writer.
class Utf8Writer {
public:
  typedef char CharType;

  char* _ptr;
  char* _end;

  BL_INLINE Utf8Writer(char* dst, size_t size) noexcept {
    reset(dst, size);
  }

  BL_INLINE void reset(char* dst, size_t size) noexcept {
    _ptr = dst;
    _end = dst + size;
  }

  [[nodiscard]]
  BL_INLINE size_t index(const char* start) const noexcept { return (size_t)(_ptr - start); }

  [[nodiscard]]
  BL_INLINE bool at_end() const noexcept { return _ptr == _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }

  BL_INLINE BLResult write(uint32_t uc) noexcept {
    if (uc <= 0x7F)
      return write_byte(uc);
    else if (uc <= 0x7FFu)
      return write2_bytes(uc);
    else if (uc <= 0xFFFFu)
      return write3_bytes(uc);
    else
      return write4_bytes(uc);
  }

  BL_INLINE BLResult write_unsafe(uint32_t uc) noexcept {
    if (uc <= 0x7F)
      return write_byte_unsafe(uc);
    else if (uc <= 0x7FFu)
      return write2_bytes_unsafe(uc);
    else if (uc <= 0xFFFFu)
      return write3_bytes_unsafe(uc);
    else
      return write4_bytes_unsafe(uc);
  }

  BL_INLINE BLResult write_byte(uint32_t uc) noexcept {
    BL_ASSERT(uc <= 0x7Fu);
    if (BL_UNLIKELY(at_end()))
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);

    _ptr[0] = char(uint8_t(uc));
    _ptr++;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write_byte_unsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 1);
    _ptr[0] = char(uint8_t(uc));
    _ptr++;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write2_bytes(uint32_t uc) noexcept {
    BL_ASSERT(uc >= 0x80u && uc <= 0x7FFu);

    _ptr += 2;
    if (BL_UNLIKELY(_ptr > _end)) {
      _ptr -= 2;
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);
    }

    _ptr[-2] = char(uint8_t(0xC0u | (uc >> 6)));
    _ptr[-1] = char(uint8_t(0x80u | (uc & 63)));
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write2_bytes_unsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 2);
    BL_ASSERT(uc >= 0x80u && uc <= 0x7FFu);

    _ptr[0] = char(uint8_t(0xC0u | (uc >> 6)));
    _ptr[1] = char(uint8_t(0x80u | (uc & 63)));

    _ptr += 2;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write3_bytes(uint32_t uc) noexcept {
    BL_ASSERT(uc >= 0x800u && uc <= 0xFFFFu);

    _ptr += 3;
    if (BL_UNLIKELY(_ptr > _end)) {
      _ptr -= 3;
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);
    }

    _ptr[-3] = char(uint8_t(0xE0u | ((uc >> 12)     )));
    _ptr[-2] = char(uint8_t(0x80u | ((uc >>  6) & 63)));
    _ptr[-1] = char(uint8_t(0x80u | ((uc      ) & 63)));
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write3_bytes_unsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 3);
    BL_ASSERT(uc >= 0x800u && uc <= 0xFFFFu);

    _ptr[0] = char(uint8_t(0xE0u | ((uc >> 12)     )));
    _ptr[1] = char(uint8_t(0x80u | ((uc >>  6) & 63)));
    _ptr[2] = char(uint8_t(0x80u | ((uc      ) & 63)));

    _ptr += 3;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write4_bytes(uint32_t uc) noexcept {
    BL_ASSERT(uc >= 0x10000u && uc <= 0x10FFFFu);

    _ptr += 4;
    if (BL_UNLIKELY(_ptr > _end)) {
      _ptr -= 4;
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);
    }

    _ptr[-4] = char(uint8_t(0xF0u | ((uc >> 18)     )));
    _ptr[-3] = char(uint8_t(0x80u | ((uc >> 12) & 63)));
    _ptr[-2] = char(uint8_t(0x80u | ((uc >>  6) & 63)));
    _ptr[-1] = char(uint8_t(0x80u | ((uc      ) & 63)));
    return BL_SUCCESS;
  }

  BL_INLINE BLResult write4_bytes_unsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 4);
    BL_ASSERT(uc >= 0x10000u && uc <= 0x10FFFFu);

    _ptr[0] = char(uint8_t(0xF0u | ((uc >> 18)     )));
    _ptr[1] = char(uint8_t(0x80u | ((uc >> 12) & 63)));
    _ptr[2] = char(uint8_t(0x80u | ((uc >>  6) & 63)));
    _ptr[3] = char(uint8_t(0x80u | ((uc      ) & 63)));

    _ptr += 4;
    return BL_SUCCESS;
  }
};

// bl::Unicode - UTF16 Writer
// ==========================

//! UTF16 writer that can be parametrized by `ByteOrder` and `Alignment`.
template<uint32_t ByteOrder = BL_BYTE_ORDER_NATIVE, uint32_t Alignment = 2>
class Utf16Writer {
public:
  typedef uint16_t CharType;

  uint16_t* _ptr;
  uint16_t* _end;

  BL_INLINE Utf16Writer(uint16_t* dst, size_t size) noexcept {
    reset(dst, size);
  }

  BL_INLINE void reset(uint16_t* dst, size_t size) noexcept {
    _ptr = dst;
    _end = dst + size;
  }

  [[nodiscard]]
  BL_INLINE size_t index(const uint16_t* start) const noexcept { return (size_t)(_ptr - start); }

  [[nodiscard]]
  BL_INLINE bool at_end() const noexcept { return _ptr == _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }

  BL_INLINE BLResult write(uint32_t uc) noexcept {
    if (uc <= 0xFFFFu)
      return writeBMP(uc);
    else
      return writeSMP(uc);
  }

  BL_INLINE BLResult writeBMP(uint32_t uc) noexcept {
    BL_ASSERT(uc <= 0xFFFFu);

    if (BL_UNLIKELY(at_end()))
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);

    _writeMemU16(_ptr, uc);
    _ptr++;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult writeBMPUnsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 1);

    _writeMemU16(_ptr, uc);
    _ptr++;
    return BL_SUCCESS;
  }

  BL_INLINE BLResult writeSMP(uint32_t uc) noexcept {
    BL_ASSERT(uc >= 0x10000u && uc <= 0x10FFFFu);

    _ptr += 2;
    if (BL_UNLIKELY(_ptr > _end)) {
      _ptr -= 2;
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);
    }

    uint32_t hi, lo;
    bl_char_to_surrogate(uc, hi, lo);

    _writeMemU16(_ptr - 2, hi);
    _writeMemU16(_ptr - 1, lo);
    return BL_SUCCESS;
  }

  BL_INLINE BLResult writeSMPUnsafe(uint32_t uc) noexcept {
    BL_ASSERT(remaining_size() >= 2);
    BL_ASSERT(uc >= 0x10000u && uc <= 0x10FFFFu);

    uint32_t hi, lo;
    bl_char_to_surrogate(uc, hi, lo);

    _writeMemU16(_ptr + 0, hi);
    _writeMemU16(_ptr + 1, lo);

    _ptr += 2;
    return BL_SUCCESS;
  }

  //! \name Utilities
  //! \{

  static BL_INLINE void _writeMemU16(void* dst, uint32_t value) noexcept {
    MemOps::writeU16<ByteOrder, Alignment>(dst, value);
  }

  //! \}
};

// bl::Unicode - UTF32 Writer
// ==========================

//! UTF32 writer that can be parametrized by `ByteOrder` and `Alignment`.
template<uint32_t ByteOrder = BL_BYTE_ORDER_NATIVE, uint32_t Alignment = 4>
class Utf32Writer {
public:
  typedef uint32_t CharType;

  uint32_t* _ptr;
  uint32_t* _end;

  BL_INLINE Utf32Writer(uint32_t* dst, size_t size) noexcept {
    reset(dst, size);
  }

  BL_INLINE void reset(uint32_t* dst, size_t size) noexcept {
    _ptr = dst;
    _end = dst + size;
  }

  [[nodiscard]]
  BL_INLINE size_t index(const uint32_t* start) const noexcept { return (size_t)(_ptr - start); }

  [[nodiscard]]
  BL_INLINE bool at_end() const noexcept { return _ptr == _end; }

  [[nodiscard]]
  BL_INLINE size_t remaining_size() const noexcept { return (size_t)(_end - _ptr); }

  BL_INLINE BLResult write(uint32_t uc) noexcept {
    if (BL_UNLIKELY(at_end()))
      return bl_make_error(BL_ERROR_NO_SPACE_LEFT);

    _writeMemU32(_ptr, uc);
    _ptr++;
    return BL_SUCCESS;
  }

  static BL_INLINE void _writeMemU32(void* dst, uint32_t value) noexcept {
    MemOps::writeU32<ByteOrder, Alignment>(dst, value);
  }
};

} // {bl::Unicode}

//! \}
//! \endcond

#endif // BLEND2D_UNICODE_UNICODE_P_H_INCLUDED
