// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/string.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

// String - Globals
// ================

static const char String_base_n[] = "0123456789ABCDEF";

constexpr size_t kMinAllocSize = 128;
constexpr size_t kMaxAllocSize = SIZE_MAX - Globals::kGrowThreshold;

// Based on ArenaVector_growCapacity().
//
// NOTE: The sizes here include null terminators - that way we can have aligned allocations that are power of 2s
// initially.
static ASMJIT_INLINE size_t String_grow_capacity(size_t byte_size, size_t min_byte_size) noexcept {
  static constexpr size_t kGrowThreshold = Globals::kGrowThreshold;

  ASMJIT_ASSERT(min_byte_size < kMaxAllocSize);

  // This is more than exponential growth at the beginning.
  if (byte_size < kMinAllocSize) {
    byte_size = kMinAllocSize;
  }
  else if (byte_size < 512) {
    byte_size = 512;
  }

  if (byte_size < min_byte_size) {
    // Exponential growth before we reach `kGrowThreshold`.
    byte_size = Support::align_up_power_of_2(min_byte_size);

    // Bail to `min_byte_size` in case of overflow - most likely whatever that is happening afterwards would just fail.
    if (byte_size < min_byte_size) {
      return min_byte_size;
    }

    // Pretty much chunked growth advancing by `kGrowThreshold` after we exceed it.
    if (byte_size > kGrowThreshold) {
      // Align to kGrowThreshold.
      size_t remainder = min_byte_size % kGrowThreshold;

      byte_size = min_byte_size + remainder;

      // Bail to `min_byte_size` in case of overflow.
      if (byte_size < min_byte_size) {
        return min_byte_size;
      }
    }
  }

  return Support::min<size_t>(byte_size, kMaxAllocSize);
}

// String - Clear & Reset
// ======================

Error String::reset() noexcept {
  if (_type == kTypeLarge) {
    ::free(_large.data);
  }

  _reset_internal();
  return Error::kOk;
}

Error String::clear() noexcept {
  if (is_large_or_external()) {
    _large.size = 0;
    _large.data[0] = '\0';
  }
  else {
    _raw.uptr[0] = 0;
  }

  return Error::kOk;
}

// String - Prepare
// ================

char* String::prepare(ModifyOp op, size_t size) noexcept {
  uint8_t type = _type;

  char* cur_data;
  size_t cur_size;
  size_t cur_capacity;

  if (is_large_or_external(type)) {
    cur_data = _large.data;
    cur_size = _large.size;
    cur_capacity = _large.capacity;
  }
  else {
    // For some reason clang's static analysis flags this function having "use-after-free". The step
    // to get to that is to execute this branch (`is_large_or_external()` returning false) and then
    // assuming `type == kTypeLarge` in another condition, which contradicts the first condition.
    ASMJIT_ASSERT(type < kTypeLarge);

    cur_data = _small.data;
    cur_size = _small.type;
    cur_capacity = kSSOCapacity;
  }

  if (op == ModifyOp::kAssign) {
    if (size > cur_capacity) {
      // Prevent arithmetic overflow.
      if (ASMJIT_UNLIKELY(size >= kMaxAllocSize)) {
        return nullptr;
      }

      size_t new_capacity = Support::align_up<size_t>(size + 1, kMinAllocSize);
      char* new_data = static_cast<char*>(::malloc(new_capacity));

      if (ASMJIT_UNLIKELY(!new_data)) {
        return nullptr;
      }

      if (type == kTypeLarge) {
        ::free(cur_data);
      }

      _large.type = kTypeLarge;
      _large.size = size;
      _large.capacity = new_capacity - 1;
      _large.data = new_data;

      new_data[size] = '\0';
      return new_data;
    }
    else {
      _set_size(size);
      cur_data[size] = '\0';
      return cur_data;
    }
  }
  else {
    // Prevent arithmetic overflow.
    if (ASMJIT_UNLIKELY(size >= kMaxAllocSize - cur_size - 1)) {
      return nullptr;
    }

    size_t new_size = size + cur_size;
    size_t new_size_plus_one = new_size + 1;

    if (new_size > cur_capacity) {
      size_t new_capacity_plus_one = String_grow_capacity(size + 1u, new_size_plus_one);
      ASMJIT_ASSERT(new_capacity_plus_one >= new_size_plus_one);

      if (ASMJIT_UNLIKELY(new_capacity_plus_one < new_size_plus_one)) {
        return nullptr;
      }

      char* new_data = static_cast<char*>(::malloc(new_capacity_plus_one));
      if (ASMJIT_UNLIKELY(!new_data)) {
        return nullptr;
      }

      memcpy(new_data, cur_data, cur_size);

      if (type == kTypeLarge) {
        ::free(cur_data);
      }

      _large.type = kTypeLarge;
      _large.size = new_size;
      _large.capacity = new_capacity_plus_one - 1;
      _large.data = new_data;

      new_data[new_size] = '\0';
      return new_data + cur_size;
    }
    else {
      _set_size(new_size);
      cur_data[new_size] = '\0';
      return cur_data + cur_size;
    }
  }
}

// String - Assign
// ===============

Error String::assign(const char* data, size_t size) noexcept {
  uint8_t type = _type;
  char* dst = nullptr;

  // Null terminated string without `size` specified.
  if (size == SIZE_MAX) {
    size = data ? strlen(data) : size_t(0);
  }

  if (is_large_or_external(type)) {
    if (size <= _large.capacity) {
      dst = _large.data;
      _large.size = size;
    }
    else {
      size_t capacity_plus_one = Support::align_up(size + 1, 32);
      if (ASMJIT_UNLIKELY(capacity_plus_one < size)) {
        return make_error(Error::kOutOfMemory);
      }

      dst = static_cast<char*>(::malloc(capacity_plus_one));
      if (ASMJIT_UNLIKELY(!dst)) {
        return make_error(Error::kOutOfMemory);
      }

      if (type == kTypeLarge) {
        ::free(_large.data);
      }

      _large.type = kTypeLarge;
      _large.data = dst;
      _large.size = size;
      _large.capacity = capacity_plus_one - 1;
    }
  }
  else {
    if (size <= kSSOCapacity) {
      ASMJIT_ASSERT(size < 0xFFu);

      dst = _small.data;
      _small.type = uint8_t(size);
    }
    else {
      dst = static_cast<char*>(::malloc(size + 1));
      if (ASMJIT_UNLIKELY(!dst)) {
        return make_error(Error::kOutOfMemory);
      }

      _large.type = kTypeLarge;
      _large.data = dst;
      _large.size = size;
      _large.capacity = size;
    }
  }

  // Optionally copy data from `data` and null-terminate.
  if (data && size) {
    // NOTE: It's better to use `memmove()`. If, for any reason, somebody uses
    // this function to substring the same string it would work as expected.
    ::memmove(dst, data, size);
  }

  dst[size] = '\0';
  return Error::kOk;
}

// String - Operations
// ===================

Error String::_op_string(ModifyOp op, const char* str, size_t size) noexcept {
  if (size == SIZE_MAX) {
    size = str ? strlen(str) : size_t(0);
  }

  if (!size) {
    return Error::kOk;
  }

  char* p = prepare(op, size);
  if (!p) {
    return make_error(Error::kOutOfMemory);
  }

  memcpy(p, str, size);
  return Error::kOk;
}

Error String::_op_char(ModifyOp op, char c) noexcept {
  char* p = prepare(op, 1);
  if (!p) {
    return make_error(Error::kOutOfMemory);
  }

  *p = c;
  return Error::kOk;
}

Error String::_op_chars(ModifyOp op, char c, size_t n) noexcept {
  if (!n) {
    return Error::kOk;
  }

  char* p = prepare(op, n);
  if (!p) {
    return make_error(Error::kOutOfMemory);
  }

  memset(p, c, n);
  return Error::kOk;
}

Error String::pad_end(size_t n, char c) noexcept {
  size_t size = this->size();
  return n > size ? append_chars(c, n - size) : Error::kOk;
}

Error String::_op_number(ModifyOp op, uint64_t i, uint32_t base, size_t width, StringFormatFlags flags) noexcept {
  if (base == 0) {
    base = 10;
  }

  char buf[128];
  char* p = buf + ASMJIT_ARRAY_SIZE(buf);

  uint64_t orig = i;
  char sign = '\0';

  // Format Sign
  // -----------

  if (Support::test(flags, StringFormatFlags::kSigned) && int64_t(i) < 0) {
    i = Support::neg(i);
    sign = '-';
  }
  else if (Support::test(flags, StringFormatFlags::kShowSign)) {
    sign = '+';
  }
  else if (Support::test(flags, StringFormatFlags::kShowSpace)) {
    sign = ' ';
  }

  // Format Number
  // -------------

  switch (base) {
    case 2:
    case 8:
    case 16: {
      uint32_t shift = Support::ctz(base);
      uint32_t mask = base - 1;

      do {
        uint64_t d = i >> shift;
        size_t r = size_t(i & mask);

        *--p = String_base_n[r];
        i = d;
      } while (i);

      break;
    }

    case 10: {
      do {
        uint64_t d = i / 10;
        uint64_t r = i % 10;

        *--p = char(uint32_t('0') + uint32_t(r));
        i = d;
      } while (i);

      break;
    }

    default:
      return make_error(Error::kInvalidArgument);
  }

  size_t number_size = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p);

  // Alternate Form
  // --------------

  if (Support::test(flags, StringFormatFlags::kAlternate)) {
    if (base == 8) {
      if (orig != 0) {
        *--p = '0';
      }
    }
    if (base == 16) {
      *--p = 'x';
      *--p = '0';
    }
  }

  // String Width
  // ------------

  if (sign != 0) {
    *--p = sign;
  }

  if (width > 256) {
    width = 256;
  }

  if (width <= number_size) {
    width = 0;
  }
  else {
    width -= number_size;
  }

  // Finalize
  // --------

  size_t prefix_size = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p) - number_size;
  char* data = prepare(op, prefix_size + width + number_size);

  if (!data) {
    return make_error(Error::kOutOfMemory);
  }

  memcpy(data, p, prefix_size);
  data += prefix_size;

  memset(data, '0', width);
  data += width;

  memcpy(data, p + prefix_size, number_size);
  return Error::kOk;
}

Error String::_op_hex(ModifyOp op, const void* data, size_t size, char separator) noexcept {
  char* dst;
  const uint8_t* src = static_cast<const uint8_t*>(data);

  if (!size) {
    return Error::kOk;
  }

  if (separator) {
    if (ASMJIT_UNLIKELY(size >= SIZE_MAX / 3)) {
      return make_error(Error::kOutOfMemory);
    }

    dst = prepare(op, size * 3 - 1);
    if (ASMJIT_UNLIKELY(!dst)) {
      return make_error(Error::kOutOfMemory);
    }

    size_t i = 0;
    for (;;) {
      dst[0] = String_base_n[(src[0] >> 4) & 0xF];
      dst[1] = String_base_n[(src[0]     ) & 0xF];

      if (++i == size) {
        break;
      }

      // This makes sure that the separator is only put between two hexadecimal bytes.
      dst[2] = separator;
      dst += 3;
      src++;
    }
  }
  else {
    if (ASMJIT_UNLIKELY(size >= SIZE_MAX / 2)) {
      return make_error(Error::kOutOfMemory);
    }

    dst = prepare(op, size * 2);
    if (ASMJIT_UNLIKELY(!dst)) {
      return make_error(Error::kOutOfMemory);
    }

    for (size_t i = 0; i < size; i++, dst += 2, src++) {
      dst[0] = String_base_n[(src[0] >> 4) & 0xF];
      dst[1] = String_base_n[(src[0]     ) & 0xF];
    }
  }

  return Error::kOk;
}

Error String::_op_format(ModifyOp op, const char* fmt, ...) noexcept {
  Error err;
  va_list ap;

  va_start(ap, fmt);
  err = _op_vformat(op, fmt, ap);
  va_end(ap);

  return err;
}

Error String::_op_vformat(ModifyOp op, const char* fmt, va_list ap) noexcept {
  size_t start_at = (op == ModifyOp::kAssign) ? size_t(0) : size();
  size_t remaining_capacity = capacity() - start_at;

  char buf[1024];
  int fmt_result;
  size_t output_size;

  va_list ap_copy;
  va_copy(ap_copy, ap);

  if (remaining_capacity >= 128) {
    fmt_result = vsnprintf(data() + start_at, remaining_capacity, fmt, ap);
    output_size = size_t(fmt_result);

    if (ASMJIT_LIKELY(output_size <= remaining_capacity)) {
      _set_size(start_at + output_size);
      return Error::kOk;
    }
  }
  else {
    fmt_result = vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    output_size = size_t(fmt_result);

    if (ASMJIT_LIKELY(output_size < ASMJIT_ARRAY_SIZE(buf))) {
      return _op_string(op, buf, output_size);
    }
  }

  if (ASMJIT_UNLIKELY(fmt_result < 0)) {
    return make_error(Error::kInvalidState);
  }

  char* p = prepare(op, output_size);
  if (ASMJIT_UNLIKELY(!p)) {
    return make_error(Error::kOutOfMemory);
  }

  fmt_result = vsnprintf(p, output_size + 1, fmt, ap_copy);
  ASMJIT_ASSERT(size_t(fmt_result) == output_size);

  return Error::kOk;
}

Error String::truncate(size_t new_size) noexcept {
  if (is_large_or_external()) {
    if (new_size < _large.size) {
      _large.data[new_size] = '\0';
      _large.size = new_size;
    }
  }
  else {
    if (new_size < _type) {
      _small.data[new_size] = '\0';
      _small.type = uint8_t(new_size);
    }
  }

  return Error::kOk;
}

bool String::equals(const char* other, size_t size) const noexcept {
  const char* a_data = data();
  const char* b_data = other;

  size_t a_size = this->size();
  size_t b_size = size;

  if (b_size == SIZE_MAX) {
    size_t i;
    for (i = 0; i < a_size; i++) {
      if (a_data[i] != b_data[i] || b_data[i] == 0) {
        return false;
      }
    }
    return b_data[i] == 0;
  }
  else {
    if (a_size != b_size) {
      return false;
    }
    return ::memcmp(a_data, b_data, a_size) == 0;
  }
}

// String - Tests
// ==============

#if defined(ASMJIT_TEST)
static void test_string_grow() noexcept {
  String s;
  size_t c = s.capacity();

  INFO("Testing string grow strategy (SSO capacity: %zu)", c);
  for (size_t i = 0; i < 1000000; i++) {
    s.append('x');
    if (s.capacity() != c) {
      c = s.capacity();
      INFO("  String reallocated to new capacity: %zu", c);
    }
  }

  // We don't expect a 1 million character string to occupy 4MiB, for example. So verify that!
  EXPECT_LT(c, size_t(4 * 1024 * 1024));
}

UNIT(core_string) {
  String s;

  INFO("Testing string functionality");

  EXPECT_FALSE(s.is_large_or_external());
  EXPECT_FALSE(s.is_external());

  EXPECT_EQ(s.assign('a'), Error::kOk);
  EXPECT_EQ(s.size(), 1u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'a');
  EXPECT_EQ(s.data()[1], '\0');
  EXPECT_TRUE(s.equals("a"));
  EXPECT_TRUE(s.equals("a", 1));

  EXPECT_EQ(s.assign_chars('b', 4), Error::kOk);
  EXPECT_EQ(s.size(), 4u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'b');
  EXPECT_EQ(s.data()[1], 'b');
  EXPECT_EQ(s.data()[2], 'b');
  EXPECT_EQ(s.data()[3], 'b');
  EXPECT_EQ(s.data()[4], '\0');
  EXPECT_TRUE(s.equals("bbbb"));
  EXPECT_TRUE(s.equals("bbbb", 4));

  EXPECT_EQ(s.assign("abc"), Error::kOk);
  EXPECT_EQ(s.size(), 3u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'a');
  EXPECT_EQ(s.data()[1], 'b');
  EXPECT_EQ(s.data()[2], 'c');
  EXPECT_EQ(s.data()[3], '\0');
  EXPECT_TRUE(s.equals("abc"));
  EXPECT_TRUE(s.equals("abc", 3));

  const char* large = "Large string that will not fit into SSO buffer";
  EXPECT_EQ(s.assign(large), Error::kOk);
  EXPECT_TRUE(s.is_large_or_external());
  EXPECT_EQ(s.size(), strlen(large));
  EXPECT_GT(s.capacity(), String::kSSOCapacity);
  EXPECT_TRUE(s.equals(large));
  EXPECT_TRUE(s.equals(large, strlen(large)));

  const char* additional = " (additional content)";
  EXPECT_TRUE(s.is_large_or_external());
  EXPECT_EQ(s.append(additional), Error::kOk);
  EXPECT_EQ(s.size(), strlen(large) + strlen(additional));

  EXPECT_EQ(s.clear(), Error::kOk);
  EXPECT_EQ(s.size(), 0u);
  EXPECT_TRUE(s.is_empty());
  EXPECT_EQ(s.data()[0], '\0');
  EXPECT_TRUE(s.is_large_or_external()); // Clear should never release the memory.

  EXPECT_EQ(s.append_uint(1234), Error::kOk);
  EXPECT_TRUE(s.equals("1234"));

  EXPECT_EQ(s.assign_uint(0xFFFF, 16, 0, StringFormatFlags::kAlternate), Error::kOk);
  EXPECT_TRUE(s.equals("0xFFFF"));

  StringTmp<64> s_tmp;
  EXPECT_TRUE(s_tmp.is_large_or_external());
  EXPECT_TRUE(s_tmp.is_external());
  EXPECT_EQ(s_tmp.append_chars(' ', 1000), Error::kOk);
  EXPECT_FALSE(s_tmp.is_external());

  test_string_grow();
}
#endif

ASMJIT_END_NAMESPACE
