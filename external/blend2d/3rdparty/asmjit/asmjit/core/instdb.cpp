// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#include <asmjit/core/instdb_p.h>

ASMJIT_BEGIN_NAMESPACE

namespace InstNameUtils {

static constexpr uint32_t kBufferSize = 32;

static ASMJIT_INLINE_CONSTEXPR char decode_5bit_char(uint32_t c) noexcept {
  uint32_t base = c <= 26 ? uint32_t('a') - 1u : uint32_t('0') - 27u;
  return char(base + c);
}

static ASMJIT_INLINE size_t decode_to_buffer(char name_out[kBufferSize], uint32_t name_value, InstStringifyOptions options, const char* string_table) noexcept {
  size_t i;

  if (name_value & 0x80000000u) {
    // Small string of 5-bit characters.
    //
    // NOTE: Small string optimization never provides additional
    // aliases formatting, so we don't have to consider `options`.
    for (i = 0; i < 6; i++, name_value >>= 5) {
      uint32_t c = name_value & 0x1F;
      if (c == 0)
        break;
      name_out[i] = decode_5bit_char(c);
    }
    return i;
  }
  else {
    size_t prefix_base = name_value & 0xFFFu;
    size_t prefix_size = (name_value >> 12) & 0xFu;

    size_t suffix_base = (name_value >> 16) & 0xFFFu;
    size_t suffix_size = (name_value >> 28) & 0x7u;

    if (Support::test(options, InstStringifyOptions::kAliases) && suffix_base == 0xFFFu) {
      // Alias formatting immediately follows the instruction name in string table.
      // The first character specifies the length and then string data follows.
      prefix_base += prefix_size;
      prefix_size = uint8_t(string_table[prefix_base]);
      ASMJIT_ASSERT(prefix_size <= kBufferSize);

      prefix_base += 1; // Skip the byte that specifies the length of a formatted alias.
    }

    for (i = 0; i < prefix_size; i++) {
      name_out[i] = string_table[prefix_base + i];
    }

    char* suffix_out = name_out + prefix_size;
    for (i = 0; i < suffix_size; i++) {
      suffix_out[i] = string_table[suffix_base + i];
    }

    return prefix_size + suffix_size;
  }
}

Error decode(uint32_t name_value, InstStringifyOptions options, const char* string_table, String& output) noexcept {
  char name_data[kBufferSize];
  size_t name_size = decode_to_buffer(name_data, name_value, options, string_table);

  return output.append(name_data, name_size);
}

InstId find_instruction(const char* s, size_t len, const uint32_t* name_table, const char* string_table, const InstNameIndex& name_index) noexcept {
  ASMJIT_ASSERT(s != nullptr);
  ASMJIT_ASSERT(len > 0u);

  uint32_t prefix = uint32_t(s[0]) - uint32_t('a');
  if (ASMJIT_UNLIKELY(prefix > uint32_t('z') - uint32_t('a'))) {
    return BaseInst::kIdNone;
  }

  size_t base = name_index.data[prefix].start;
  size_t end = name_index.data[prefix].end;

  if (ASMJIT_UNLIKELY(!base)) {
    return BaseInst::kIdNone;
  }

  char name_data[kBufferSize];
  for (size_t lim = end - base; lim != 0; lim >>= 1) {
    size_t inst_id = base + (lim >> 1);
    size_t name_size = decode_to_buffer(name_data, name_table[inst_id], InstStringifyOptions::kNone, string_table);

    int result = Support::compare_string_views(s, len, name_data, name_size);
    if (result < 0) {
      continue;
    }

    if (result > 0) {
      base = inst_id + 1;
      lim--;
      continue;
    }

    return InstId(inst_id);
  }

  return BaseInst::kIdNone;
}


uint32_t find_alias(const char* s, size_t len, const uint32_t* name_table, const char* string_table, uint32_t alias_name_count) noexcept {
  ASMJIT_ASSERT(s != nullptr);
  ASMJIT_ASSERT(len > 0u);

  size_t base = 0;
  char name_data[kBufferSize];

  for (size_t lim = size_t(alias_name_count) - base; lim != 0; lim >>= 1) {
    size_t index = base + (lim >> 1);
    size_t name_size = decode_to_buffer(name_data, name_table[index], InstStringifyOptions::kNone, string_table);

    int result = Support::compare_string_views(s, len, name_data, name_size);
    if (result < 0) {
      continue;
    }

    if (result > 0) {
      base = index + 1;
      lim--;
      continue;
    }

    return uint32_t(index);
  }

  return Globals::kInvalidId;
}

} // {InstNameUtils}

ASMJIT_END_NAMESPACE
