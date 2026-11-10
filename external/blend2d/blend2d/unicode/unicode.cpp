// This file is part of Blend2D project <https://blend2d.com>
//
// See blend2d.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <blend2d/core/api-build_p.h>
#include <blend2d/unicode/unicode_p.h>
#include <blend2d/support/intops_p.h>
#include <blend2d/support/memops_p.h>

namespace bl::Unicode {

// bl::Unicode - Data
// ==================

// NOTE: Theoretically UTF-8 sequence can be extended to support sequences up to 6 bytes, however, since UCS-4
// code-point's maximum value is 0x10FFFF it also limits the maximum length of a UTF-8 encoded character to 4 bytes.
const uint8_t utf8_size_data[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0   - 15
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 16  - 31
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 32  - 47
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 48  - 63
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 64  - 79
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 80  - 95
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 96  - 111
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 112 - 127
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 128 - 143
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 144 - 159
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 160 - 175
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 176 - 191
  0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 192 - 207
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 208 - 223
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 224 - 239
  4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 240 - 255
};

// bl::Unicode - Validation
// ========================

// Not really anything to validate, we just want to calculate a corresponding UTF-8 size.
static BL_INLINE BLResult validate_latin1_string(const char* data, size_t size, ValidationState& state) noexcept {
  size_t extra = 0;
  state.utf16_index = size;
  state.utf32_index = size;

  for (size_t i = 0; i < size; i++)
    extra += size_t(uint8_t(data[i])) >> 7;

  bl::OverflowFlag of{};
  size_t utf8_size = bl::IntOps::add_overflow(size, extra, &of);

  if (BL_UNLIKELY(of))
    return bl_make_error(BL_ERROR_DATA_TOO_LARGE);

  state.utf8_index = utf8_size;
  return BL_SUCCESS;
}

template<typename Iterator, IOFlags kFlags>
static BL_INLINE BLResult validate_unicode_string(const void* data, size_t size, ValidationState& state) noexcept {
  Iterator it(data, size);
  BLResult result = it.template validate<kFlags | IOFlags::kCalcIndex>();
  state.utf8_index = it.utf8_index(data);
  state.utf16_index = it.utf16_index(data);
  state.utf32_index = it.utf32_index(data);
  return result;
}

BLResult bl_validate_unicode(const void* data, size_t size_in_bytes, BLTextEncoding encoding, ValidationState& state) noexcept {
  BLResult result;
  state.reset();

  switch (encoding) {
    case BL_TEXT_ENCODING_LATIN1:
      return validate_latin1_string(static_cast<const char*>(data), size_in_bytes, state);

    case BL_TEXT_ENCODING_UTF8:
      return validate_unicode_string<Utf8Reader, IOFlags::kStrict>(data, size_in_bytes, state);

    case BL_TEXT_ENCODING_UTF16:
      // This will make sure we won't compile specialized code for architectures that don't penalize unaligned reads.
      if (MemOps::kUnalignedMem16 || !IntOps::is_aligned(data, 2))
        result = validate_unicode_string<Utf16Reader, IOFlags::kStrict | IOFlags::kUnaligned>(data, size_in_bytes, state);
      else
        result = validate_unicode_string<Utf16Reader, IOFlags::kStrict>(data, size_in_bytes, state);

      if (result == BL_SUCCESS && BL_UNLIKELY(size_in_bytes & 0x1))
        result = bl_make_error(BL_ERROR_DATA_TRUNCATED);
      return result;

    case BL_TEXT_ENCODING_UTF32:
      // This will make sure we won't compile specialized code for architectures that don't penalize unaligned reads.
      if (MemOps::kUnalignedMem32 || !IntOps::is_aligned(data, 4))
        result = validate_unicode_string<Utf32Reader, IOFlags::kStrict | IOFlags::kUnaligned>(data, size_in_bytes, state);
      else
        result = validate_unicode_string<Utf32Reader, IOFlags::kStrict>(data, size_in_bytes, state);

      if (result == BL_SUCCESS && BL_UNLIKELY(size_in_bytes & 0x3))
        result = bl_make_error(BL_ERROR_DATA_TRUNCATED);
      return result;

    default:
      return bl_make_error(BL_ERROR_INVALID_VALUE);
  }
}

// bl::Unicode - Conversion
// ========================

static BL_INLINE size_t offset_of_ptr(const void* base, const void* advanced) noexcept {
  return (size_t)(static_cast<const char*>(advanced) - static_cast<const char*>(base));
}

// A simple implementation. It iterates `src` char-by-char and writes it to the destination. The advantage of this
// implementation is that switching `Writer` and `Iterator` can customize strictness, endianness, etc, so we don't
// have to repeat the code for different variations of UTF16 and UTF32.
template<typename Writer, typename Iterator, IOFlags kFlags>
static BL_INLINE BLResult convert_unicode_impl(void* dst, size_t dst_size_in_bytes, const void* src, size_t src_size_in_bytes, ConversionState& state) noexcept {
  typedef typename Writer::CharType DstChar;

  Writer writer(static_cast<DstChar*>(dst), dst_size_in_bytes / sizeof(DstChar));
  Iterator iter(src, IntOps::align_down(src_size_in_bytes, Iterator::kCharSize));

  BLResult result = BL_SUCCESS;
  while (iter.has_next()) {
    uint32_t uc;
    size_t uc_size_in_bytes;

    result = iter.template next<kFlags>(uc, uc_size_in_bytes);
    if (BL_UNLIKELY(result != BL_SUCCESS))
      break;

    result = writer.write(uc);
    if (BL_UNLIKELY(result != BL_SUCCESS)) {
      state.dst_index = offset_of_ptr(dst, writer._ptr);
      state.src_index = offset_of_ptr(src, iter._ptr) - uc_size_in_bytes;
      return result;
    }
  }

  state.dst_index = offset_of_ptr(dst, writer._ptr);
  state.src_index = offset_of_ptr(src, iter._ptr);

  if (Iterator::kCharSize > 1 && result == BL_SUCCESS && !IntOps::is_aligned(state.src_index, Iterator::kCharSize))
    return bl_make_error(BL_ERROR_DATA_TRUNCATED);
  else
    return result;
}

BLResult convert_unicode(
  void* dst, size_t dst_size_in_bytes, uint32_t dst_encoding,
  const void* src, size_t src_size_in_bytes, uint32_t src_encoding,
  ConversionState& state) noexcept {

  constexpr bool kUnalignedAny = MemOps::kUnalignedMem16 && MemOps::kUnalignedMem32;

  BLResult result = BL_SUCCESS;
  state.reset();

  uint32_t encoding_combined = (dst_encoding << 2) | src_encoding;
  switch (encoding_combined) {
    // MemCpy
    // ------

    case (BL_TEXT_ENCODING_LATIN1 << 2) | BL_TEXT_ENCODING_LATIN1: {
      size_t copy_size = bl_min(dst_size_in_bytes, src_size_in_bytes);
      memcpy(dst, src, copy_size);

      state.dst_index = copy_size;
      state.src_index = copy_size;

      if (dst_size_in_bytes < src_size_in_bytes)
        result = bl_make_error(BL_ERROR_NO_SPACE_LEFT);
      break;
    }

    // Utf8 <- Latin1
    // --------------

    case (BL_TEXT_ENCODING_UTF8 << 2) | BL_TEXT_ENCODING_LATIN1: {
      Utf8Writer writer(static_cast<char*>(dst), dst_size_in_bytes);
      const uint8_t* src8 = static_cast<const uint8_t*>(src);

      if (dst_size_in_bytes / 2 >= src_size_in_bytes) {
        // Fast case, there is enough space in `dst` even for the worst-case scenario.
        for (size_t i = 0; i < src_size_in_bytes; i++) {
          uint32_t uc = src8[i];
          if (uc <= 0x7F)
            writer.write_byte_unsafe(uc);
          else
            writer.write2_bytes_unsafe(uc);
        }

        state.src_index = src_size_in_bytes;
        state.dst_index = writer.index(static_cast<char*>(dst));
      }
      else {
        for (size_t i = 0; i < src_size_in_bytes; i++) {
          uint32_t uc = src8[i];
          if (uc <= 0x7F)
            result = writer.write_byte(uc);
          else
            result = writer.write2_bytes(uc);

          if (BL_UNLIKELY(result != BL_SUCCESS)) {
            state.dst_index = writer.index(static_cast<char*>(dst));
            state.src_index = i;
            break;
          }
        }
      }

      break;
    }

    // Utf8 <- Utf8
    // ------------

    case (BL_TEXT_ENCODING_UTF8 << 2) | BL_TEXT_ENCODING_UTF8: {
      size_t copy_size = bl_min(dst_size_in_bytes, src_size_in_bytes);
      ValidationState validation_state;

      result = bl_validate_unicode(src, copy_size, BL_TEXT_ENCODING_UTF8, validation_state);
      size_t validated_size = validation_state.utf8_index;

      if (validated_size)
        memcpy(dst, src, validated_size);

      // Prevent `BL_ERROR_DATA_TRUNCATED` in case there is not enough space in destination.
      if (copy_size < src_size_in_bytes && (result == BL_SUCCESS || result == BL_ERROR_DATA_TRUNCATED))
        result = bl_make_error(BL_ERROR_NO_SPACE_LEFT);

      state.dst_index = validated_size;
      state.src_index = validated_size;
      break;
    }

    // Utf8 <- Utf16
    // -------------

    case (BL_TEXT_ENCODING_UTF8 << 2) | BL_TEXT_ENCODING_UTF16:
      if (MemOps::kUnalignedMem16 || !IntOps::is_aligned(src, 2))
        result = convert_unicode_impl<Utf8Writer, Utf16Reader, IOFlags::kStrict | IOFlags::kUnaligned>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf8Writer, Utf16Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;

    // Utf8 <- Utf32
    // -------------

    case (BL_TEXT_ENCODING_UTF8 << 2) | BL_TEXT_ENCODING_UTF32: {
      if (MemOps::kUnalignedMem32 || !IntOps::is_aligned(src, 4))
        result = convert_unicode_impl<Utf8Writer, Utf32Reader, IOFlags::kStrict | IOFlags::kUnaligned>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf8Writer, Utf32Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Utf16 <- Latin1
    // ---------------

    case (BL_TEXT_ENCODING_UTF16 << 2) | BL_TEXT_ENCODING_LATIN1: {
      size_t count = bl_min(dst_size_in_bytes / 2, src_size_in_bytes);

      if (MemOps::kUnalignedMem16 || IntOps::is_aligned(dst, 2)) {
        for (size_t i = 0; i < count; i++)
          MemOps::writeU16aLE(static_cast<uint8_t*>(dst) + i * 2u, static_cast<const uint8_t*>(src)[i]);
      }
      else {
        for (size_t i = 0; i < count; i++)
          MemOps::writeU16uLE(static_cast<uint8_t*>(dst) + i * 2u, static_cast<const uint8_t*>(src)[i]);
      }

      if (count < src_size_in_bytes)
        result = bl_make_error(BL_ERROR_NO_SPACE_LEFT);

      state.dst_index = count * 2u;
      state.src_index = count;
      break;
    }

    // Utf16 <- Utf8
    // -------------

    case (BL_TEXT_ENCODING_UTF16 << 2) | BL_TEXT_ENCODING_UTF8: {
      if (MemOps::kUnalignedMem16 || !IntOps::is_aligned(dst, 2))
        result = convert_unicode_impl<Utf16Writer<BL_BYTE_ORDER_NATIVE, 1>, Utf8Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf16Writer<BL_BYTE_ORDER_NATIVE, 2>, Utf8Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Utf16 <- Utf16
    // --------------

    case (BL_TEXT_ENCODING_UTF16 << 2) | BL_TEXT_ENCODING_UTF16: {
      size_t copy_size = IntOps::align_down(bl_min(dst_size_in_bytes, src_size_in_bytes), 2);
      ValidationState validation_state;

      result = bl_validate_unicode(src, copy_size, BL_TEXT_ENCODING_UTF16, validation_state);
      size_t validated_size = validation_state.utf16_index * 2;

      memmove(dst, src, validated_size);

      // Prevent `BL_ERROR_DATA_TRUNCATED` in case there is not enough space in destination.
      if (copy_size < src_size_in_bytes && (result == BL_SUCCESS || result == BL_ERROR_DATA_TRUNCATED))
        result = bl_make_error(BL_ERROR_NO_SPACE_LEFT);

      // Report `BL_ERROR_DATA_TRUNCATED` is everything went right, but the
      // source size was not aligned to 2 bytes.
      if (result == BL_SUCCESS && !IntOps::is_aligned(src_size_in_bytes, 2))
        result = bl_make_error(BL_ERROR_DATA_TRUNCATED);

      state.dst_index = validated_size;
      state.src_index = validated_size;
      break;
    }

    // Utf16 <- Utf32
    // --------------

    case (BL_TEXT_ENCODING_UTF16 << 2) | BL_TEXT_ENCODING_UTF32: {
      if (kUnalignedAny || !IntOps::is_aligned(dst, 2) || !IntOps::is_aligned(src, 4))
        result = convert_unicode_impl<Utf16Writer<BL_BYTE_ORDER_NATIVE, 1>, Utf32Reader, IOFlags::kStrict | IOFlags::kUnaligned>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf16Writer<BL_BYTE_ORDER_NATIVE, 2>, Utf32Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Utf32 <- Latin1
    // ---------------

    case (BL_TEXT_ENCODING_UTF32 << 2) | BL_TEXT_ENCODING_LATIN1: {
      size_t count = bl_min(dst_size_in_bytes / 4, src_size_in_bytes);

      if (MemOps::kUnalignedMem32 || IntOps::is_aligned(dst, 4)) {
        for (size_t i = 0; i < count; i++)
          MemOps::writeU32a(static_cast<uint8_t*>(dst) + i * 4u, static_cast<const uint8_t*>(src)[i]);
      }
      else {
        for (size_t i = 0; i < count; i++)
          MemOps::writeU32u(static_cast<uint8_t*>(dst) + i * 4u, static_cast<const uint8_t*>(src)[i]);
      }

      if (count < src_size_in_bytes)
        result = bl_make_error(BL_ERROR_NO_SPACE_LEFT);

      state.dst_index = count * 4u;
      state.src_index = count;
      break;
    }

    // Utf32 <- Utf8
    // -------------

    case (BL_TEXT_ENCODING_UTF32 << 2) | BL_TEXT_ENCODING_UTF8: {
      if (MemOps::kUnalignedMem32 || !IntOps::is_aligned(dst, 4))
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 1>, Utf8Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 4>, Utf8Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Utf32 <- Utf16
    // --------------

    case (BL_TEXT_ENCODING_UTF32 << 2) | BL_TEXT_ENCODING_UTF16: {
      if (kUnalignedAny || !IntOps::is_aligned(dst, 4) || !IntOps::is_aligned(src, 2))
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 1>, Utf16Reader, IOFlags::kStrict | IOFlags::kUnaligned>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 4>, Utf16Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Utf32 <- Utf32
    // --------------

    case (BL_TEXT_ENCODING_UTF32 << 2) | BL_TEXT_ENCODING_UTF32: {
      if (MemOps::kUnalignedMem32 || !IntOps::is_aligned(dst, 4) || !IntOps::is_aligned(src, 4))
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 1>, Utf32Reader, IOFlags::kStrict>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      else
        result = convert_unicode_impl<Utf32Writer<BL_BYTE_ORDER_NATIVE, 4>, Utf32Reader, IOFlags::kStrict | IOFlags::kUnaligned>(dst, dst_size_in_bytes, src, src_size_in_bytes, state);
      break;
    }

    // Invalid
    // -------

    default: {
      return bl_make_error(BL_ERROR_INVALID_VALUE);
    }
  }

  return result;
}

} // {bl::Unicode}
