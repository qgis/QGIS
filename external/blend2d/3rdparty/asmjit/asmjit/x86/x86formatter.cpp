// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_LOGGING)

#include <asmjit/core/cpuinfo.h>
#include <asmjit/core/formatter_p.h>
#include <asmjit/core/misc_p.h>
#include <asmjit/support/support.h>
#include <asmjit/x86/x86formatter_p.h>
#include <asmjit/x86/x86instapi_p.h>
#include <asmjit/x86/x86instdb_p.h>
#include <asmjit/x86/x86operand.h>

#ifndef ASMJIT_NO_COMPILER
  #include <asmjit/core/compiler.h>
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// x86::FormatterInternal - Constants
// ==================================

struct RegFormatInfo {
  struct TypeEntry {
    uint8_t index;
  };

  struct NameEntry {
    uint8_t count;
    uint8_t format_index;
    uint8_t special_index;
    uint8_t special_count;
  };

  TypeEntry type_entries[uint32_t(RegType::kMaxValue) + 1];
  char type_strings[128 - 32];

  NameEntry name_entries[uint32_t(RegType::kMaxValue) + 1];
  char name_strings[280];
};

template<uint32_t X>
struct RegFormatInfo_T {
  static inline constexpr uint32_t kTypeIndex =
    X == uint32_t(RegType::kPC       ) ? 39  :
    X == uint32_t(RegType::kGp8Lo    ) ? 1   :
    X == uint32_t(RegType::kGp8Hi    ) ? 8   :
    X == uint32_t(RegType::kGp16     ) ? 15  :
    X == uint32_t(RegType::kGp32     ) ? 19  :
    X == uint32_t(RegType::kGp64     ) ? 23  :
    X == uint32_t(RegType::kVec128   ) ? 27  :
    X == uint32_t(RegType::kVec256   ) ? 31  :
    X == uint32_t(RegType::kVec512   ) ? 35  :
    X == uint32_t(RegType::kMask     ) ? 53  :
    X == uint32_t(RegType::kX86_Mm   ) ? 50  :
    X == uint32_t(RegType::kSegment  ) ? 43  :
    X == uint32_t(RegType::kControl  ) ? 59  :
    X == uint32_t(RegType::kDebug    ) ? 62  :
    X == uint32_t(RegType::kX86_St   ) ? 47  :
    X == uint32_t(RegType::kX86_Bnd  ) ? 55  :
    X == uint32_t(RegType::kTile     ) ? 65  : 0;

  static inline constexpr uint32_t kFormatIndex =
    X == uint32_t(RegType::kPC       ) ? 43  :
    X == uint32_t(RegType::kGp8Lo    ) ? 1   :
    X == uint32_t(RegType::kGp8Hi    ) ? 6   :
    X == uint32_t(RegType::kGp16     ) ? 11  :
    X == uint32_t(RegType::kGp32     ) ? 16  :
    X == uint32_t(RegType::kGp64     ) ? 21  :
    X == uint32_t(RegType::kVec128   ) ? 25  :
    X == uint32_t(RegType::kVec256   ) ? 31  :
    X == uint32_t(RegType::kVec512   ) ? 37  :
    X == uint32_t(RegType::kMask     ) ? 65  :
    X == uint32_t(RegType::kX86_Mm   ) ? 60  :
    X == uint32_t(RegType::kSegment  ) ? 49  :
    X == uint32_t(RegType::kControl  ) ? 75  :
    X == uint32_t(RegType::kDebug    ) ? 80  :
    X == uint32_t(RegType::kX86_St   ) ? 55  :
    X == uint32_t(RegType::kX86_Bnd  ) ? 69  :
    X == uint32_t(RegType::kTile     ) ? 89  : 0;

  static inline constexpr uint32_t kSpecialIndex =
    X == uint32_t(RegType::kPC       ) ? 85  :
    X == uint32_t(RegType::kGp8Lo    ) ? 96  :
    X == uint32_t(RegType::kGp8Hi    ) ? 128 :
    X == uint32_t(RegType::kGp16     ) ? 161 :
    X == uint32_t(RegType::kGp32     ) ? 160 :
    X == uint32_t(RegType::kGp64     ) ? 192 :
    X == uint32_t(RegType::kSegment  ) ? 224 : 0;

  static inline constexpr uint32_t kSpecialCount =
    X == uint32_t(RegType::kPC       ) ? 1   :
    X == uint32_t(RegType::kGp8Lo    ) ? 8   :
    X == uint32_t(RegType::kGp8Hi    ) ? 4   :
    X == uint32_t(RegType::kGp16     ) ? 8   :
    X == uint32_t(RegType::kGp32     ) ? 8   :
    X == uint32_t(RegType::kGp64     ) ? 8   :
    X == uint32_t(RegType::kSegment  ) ? 7   : 0;

  static inline constexpr uint32_t kRegCount =
    X == uint32_t(RegType::kPC       ) ? 1   :
    X == uint32_t(RegType::kGp8Lo    ) ? 32  :
    X == uint32_t(RegType::kGp8Hi    ) ? 4   :
    X == uint32_t(RegType::kGp16     ) ? 32  :
    X == uint32_t(RegType::kGp32     ) ? 32  :
    X == uint32_t(RegType::kGp64     ) ? 32  :
    X == uint32_t(RegType::kVec128   ) ? 32  :
    X == uint32_t(RegType::kVec256   ) ? 32  :
    X == uint32_t(RegType::kVec512   ) ? 32  :
    X == uint32_t(RegType::kMask     ) ? 8   :
    X == uint32_t(RegType::kX86_Mm   ) ? 8   :
    X == uint32_t(RegType::kSegment  ) ? 7   :
    X == uint32_t(RegType::kControl  ) ? 16  :
    X == uint32_t(RegType::kDebug    ) ? 16  :
    X == uint32_t(RegType::kX86_St   ) ? 8   :
    X == uint32_t(RegType::kX86_Bnd  ) ? 4   :
    X == uint32_t(RegType::kTile     ) ? 8   : 0;
};

#define ASMJIT_REG_TYPE_ENTRY(TYPE) {   \
  RegFormatInfo_T<TYPE>::kTypeIndex     \
}

#define ASMJIT_REG_NAME_ENTRY(TYPE) {   \
  RegFormatInfo_T<TYPE>::kRegCount,     \
  RegFormatInfo_T<TYPE>::kFormatIndex,  \
  RegFormatInfo_T<TYPE>::kSpecialIndex, \
  RegFormatInfo_T<TYPE>::kSpecialCount  \
}

static const RegFormatInfo reg_format_info = {
  // Register type entries and strings.
  { ASMJIT_LOOKUP_TABLE_32(ASMJIT_REG_TYPE_ENTRY, 0) },

  "\0"             // #0
  "gpb\0\0\0\0"    // #1
  "gpb.hi\0"       // #8
  "gpw\0"          // #15
  "gpd\0"          // #19
  "gpq\0"          // #23
  "xmm\0"          // #27
  "ymm\0"          // #31
  "zmm\0"          // #35
  "rip\0"          // #39
  "seg\0"          // #43
  "st\0"           // #47
  "mm\0"           // #50
  "k\0"            // #53
  "bnd\0"          // #55
  "cr\0"           // #59
  "dr\0"           // #62
  "tmm\0"          // #65
  ,

  // Register name entries and strings.
  { ASMJIT_LOOKUP_TABLE_32(ASMJIT_REG_NAME_ENTRY, 0) },

  "\0"
  "r%ub\0"         // #1
  "r%uh\0"         // #6
  "r%uw\0"         // #11
  "r%ud\0"         // #16
  "r%u\0"          // #21
  "xmm%u\0"        // #25
  "ymm%u\0"        // #31
  "zmm%u\0"        // #37
  "rip%u\0"        // #43
  "seg%u\0"        // #49
  "st%u\0"         // #55
  "mm%u\0"         // #60
  "k%u\0"          // #65
  "bnd%u\0"        // #69
  "cr%u\0"         // #75
  "dr%u\0"         // #80

  "rip\0"          // #85
  "tmm%u\0"        // #89
  "\0"             // #95

  "al\0\0" "cl\0\0" "dl\0\0" "bl\0\0" "spl\0"  "bpl\0"  "sil\0"  "dil\0" // #96
  "ah\0\0" "ch\0\0" "dh\0\0" "bh\0\0" "n/a\0"  "n/a\0"  "n/a\0"  "n/a\0" // #128
  "eax\0"  "ecx\0"  "edx\0"  "ebx\0"  "esp\0"  "ebp\0"  "esi\0"  "edi\0" // #160
  "rax\0"  "rcx\0"  "rdx\0"  "rbx\0"  "rsp\0"  "rbp\0"  "rsi\0"  "rdi\0" // #192
  "n/a\0"  "es\0\0" "cs\0\0" "ss\0\0" "ds\0\0" "fs\0\0" "gs\0\0" "n/a\0" // #224
};
#undef ASMJIT_REG_NAME_ENTRY
#undef ASMJIT_REG_TYPE_ENTRY

static const char* get_address_size_string(uint32_t size) noexcept {
  switch (size) {
    case 1 : return "byte ptr ";
    case 2 : return "word ptr ";
    case 4 : return "dword ptr ";
    case 6 : return "fword ptr ";
    case 8 : return "qword ptr ";
    case 10: return "tbyte ptr ";
    case 16: return "xmmword ptr ";
    case 32: return "ymmword ptr ";
    case 64: return "zmmword ptr ";
    default: return "";
  }
}

// x86::FormatterInternal - Format FeatureId
// =========================================

Error FormatterInternal::format_feature(String& sb, uint32_t feature_id) noexcept {
  // @EnumStringBegin{"enum": "CpuFeatures::X86", "output": "feature_string", "strip": "k"}@
  static const char feature_string_data[] =
    "None\0"
    "MT\0"
    "NX\0"
    "ADX\0"
    "ALTMOVCR8\0"
    "APX_F\0"
    "BMI\0"
    "BMI2\0"
    "CET_IBT\0"
    "CET_SS\0"
    "CET_SSS\0"
    "CLDEMOTE\0"
    "CLFLUSH\0"
    "CLFLUSHOPT\0"
    "CLWB\0"
    "CLZERO\0"
    "CMOV\0"
    "CMPCCXADD\0"
    "CMPXCHG16B\0"
    "CMPXCHG8B\0"
    "ENCLV\0"
    "ENQCMD\0"
    "ERMS\0"
    "FSGSBASE\0"
    "FSRM\0"
    "FSRC\0"
    "FSRS\0"
    "FXSR\0"
    "FXSROPT\0"
    "FZRM\0"
    "HRESET\0"
    "I486\0"
    "INVLPGB\0"
    "LAHFSAHF\0"
    "LAM\0"
    "LWP\0"
    "LZCNT\0"
    "MCOMMIT\0"
    "MONITOR\0"
    "MONITORX\0"
    "MOVBE\0"
    "MOVDIR64B\0"
    "MOVDIRI\0"
    "MOVRS\0"
    "MPX\0"
    "MSR\0"
    "MSRLIST\0"
    "MSR_IMM\0"
    "MSSE\0"
    "OSXSAVE\0"
    "OSPKE\0"
    "PCONFIG\0"
    "POPCNT\0"
    "PREFETCHI\0"
    "PREFETCHW\0"
    "PREFETCHWT1\0"
    "PTWRITE\0"
    "RAO_INT\0"
    "RMPQUERY\0"
    "RDPID\0"
    "RDPRU\0"
    "RDRAND\0"
    "RDSEED\0"
    "RDTSC\0"
    "RDTSCP\0"
    "RTM\0"
    "SEAM\0"
    "SERIALIZE\0"
    "SEV\0"
    "SEV_ES\0"
    "SEV_SNP\0"
    "SKINIT\0"
    "SMAP\0"
    "SME\0"
    "SMEP\0"
    "SMX\0"
    "SVM\0"
    "TBM\0"
    "TSE\0"
    "TSXLDTRK\0"
    "UINTR\0"
    "VMX\0"
    "WAITPKG\0"
    "WBNOINVD\0"
    "WRMSRNS\0"
    "XSAVE\0"
    "XSAVEC\0"
    "XSAVEOPT\0"
    "XSAVES\0"
    "FPU\0"
    "MMX\0"
    "MMX2\0"
    "3DNOW\0"
    "3DNOW2\0"
    "GEODE\0"
    "SSE\0"
    "SSE2\0"
    "SSE3\0"
    "SSSE3\0"
    "SSE4_1\0"
    "SSE4_2\0"
    "SSE4A\0"
    "PCLMULQDQ\0"
    "AVX\0"
    "AVX2\0"
    "AVX_IFMA\0"
    "AVX_NE_CONVERT\0"
    "AVX_VNNI\0"
    "AVX_VNNI_INT16\0"
    "AVX_VNNI_INT8\0"
    "F16C\0"
    "FMA\0"
    "FMA4\0"
    "XOP\0"
    "AVX512_BF16\0"
    "AVX512_BITALG\0"
    "AVX512_BW\0"
    "AVX512_CD\0"
    "AVX512_DQ\0"
    "AVX512_F\0"
    "AVX512_FP16\0"
    "AVX512_IFMA\0"
    "AVX512_VBMI\0"
    "AVX512_VBMI2\0"
    "AVX512_VL\0"
    "AVX512_VNNI\0"
    "AVX512_VP2INTERSECT\0"
    "AVX512_VPOPCNTDQ\0"
    "AESNI\0"
    "GFNI\0"
    "SHA\0"
    "SHA512\0"
    "SM3\0"
    "SM4\0"
    "VAES\0"
    "VPCLMULQDQ\0"
    "KL\0"
    "AESKLE\0"
    "AESKLEWIDE_KL\0"
    "AVX10_1\0"
    "AVX10_2\0"
    "AMX_AVX512\0"
    "AMX_BF16\0"
    "AMX_COMPLEX\0"
    "AMX_FP16\0"
    "AMX_FP8\0"
    "AMX_INT8\0"
    "AMX_MOVRS\0"
    "AMX_TF32\0"
    "AMX_TILE\0"
    "AMX_TRANSPOSE\0"
    "<Unknown>\0";

  static const uint16_t feature_string_index[] = {
    0, 5, 8, 11, 15, 25, 31, 35, 40, 48, 55, 63, 72, 80, 91, 96, 103, 108, 118,
    129, 139, 145, 152, 157, 166, 171, 176, 181, 186, 194, 199, 206, 211, 219,
    228, 232, 236, 242, 250, 258, 267, 273, 283, 291, 297, 301, 305, 313, 321,
    326, 334, 340, 348, 355, 365, 375, 387, 395, 403, 412, 418, 424, 431, 438,
    444, 451, 455, 460, 470, 474, 481, 489, 496, 501, 505, 510, 514, 518, 522,
    526, 535, 541, 545, 553, 562, 570, 576, 583, 592, 599, 603, 607, 612, 618,
    625, 631, 635, 640, 645, 651, 658, 665, 671, 681, 685, 690, 699, 714, 723,
    738, 752, 757, 761, 766, 770, 782, 796, 806, 816, 826, 835, 847, 859, 871,
    884, 894, 906, 926, 943, 949, 954, 958, 965, 969, 973, 978, 989, 992, 999,
    1013, 1021, 1029, 1040, 1049, 1061, 1070, 1078, 1087, 1097, 1106, 1115, 1129
  };
  // @EnumStringEnd@

  return sb.append(feature_string_data + feature_string_index[Support::min(feature_id, uint32_t(CpuFeatures::X86::kMaxValue) + 1u)]);
}

// x86::FormatterInternal - Format Register
// ========================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::format_register(String& sb, FormatFlags format_flags, const BaseEmitter* emitter, Arch arch, RegType type, uint32_t id) noexcept {
  Support::maybe_unused(arch);
  const RegFormatInfo& info = reg_format_info;

#ifndef ASMJIT_NO_COMPILER
  if (Operand::is_virt_id(id)) {
    if (emitter && emitter->emitter_type() == EmitterType::kCompiler) {
      const BaseCompiler* cc = static_cast<const BaseCompiler*>(emitter);
      if (cc->is_virt_id_valid(id)) {
        VirtReg* virt_reg = cc->virt_reg_by_id(id);
        ASMJIT_ASSERT(virt_reg != nullptr);

        ASMJIT_PROPAGATE(Formatter::format_virt_reg_name(sb, virt_reg));

        bool format_type = (Support::test(format_flags, FormatFlags::kRegType)) ||
                           (Support::test(format_flags, FormatFlags::kRegCasts) && virt_reg->reg_type() != type);

        if (format_type && uint32_t(type) <= uint32_t(RegType::kMaxValue)) {
          const RegFormatInfo::TypeEntry& type_entry = info.type_entries[size_t(type)];
          if (type_entry.index) {
            ASMJIT_PROPAGATE(sb.append_format("@%s", info.type_strings + type_entry.index));
          }
        }

        return Error::kOk;
      }
    }
  }
#else
  Support::maybe_unused(emitter, format_flags);
#endif

  if (uint32_t(type) <= uint32_t(RegType::kMaxValue)) {
    const RegFormatInfo::NameEntry& name_entry = info.name_entries[size_t(type)];

    if (id < name_entry.special_count) {
      return sb.append(info.name_strings + name_entry.special_index + id * 4);
    }

    if (id < name_entry.count) {
      return sb.append_format(info.name_strings + name_entry.format_index, unsigned(id));
    }

    const RegFormatInfo::TypeEntry& type_entry = info.type_entries[size_t(type)];
    if (type_entry.index) {
      return sb.append_format("%s@%u", info.type_strings + type_entry.index, id);
    }
  }

  return sb.append_format("<Reg-%u>?%u", uint32_t(type), id);
}

// x86::FormatterInternal - Format Operand
// =======================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::format_operand(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept {

  if (op.is_reg()) {
    return format_register(sb, format_flags, emitter, arch, op.as<Reg>().reg_type(), op.as<Reg>().id());
  }

  if (op.is_mem()) {
    const Mem& m = op.as<Mem>();
    ASMJIT_PROPAGATE(sb.append(get_address_size_string(m.size())));

    // Segment override prefix.
    uint32_t seg = m.segment_id();
    if (seg != SReg::kIdNone && seg < SReg::kIdCount) {
      ASMJIT_PROPAGATE(sb.append_format("%s:", reg_format_info.name_strings + 224 + size_t(seg) * 4));
    }

    ASMJIT_PROPAGATE(sb.append('['));
    switch (m.addr_type()) {
      case Mem::AddrType::kDefault:
        break;
      case Mem::AddrType::kAbs:
        ASMJIT_PROPAGATE(sb.append("abs "));
        break;
      case Mem::AddrType::kRel:
        ASMJIT_PROPAGATE(sb.append("rel "));
        break;
    }

    char op_sign = '\0';
    if (m.has_base()) {
      op_sign = '+';
      if (m.has_base_label()) {
        ASMJIT_PROPAGATE(Formatter::format_label(sb, format_flags, emitter, m.base_id()));
      }
      else {
        FormatFlags modified_flags = format_flags;
        if (m.is_reg_home()) {
          ASMJIT_PROPAGATE(sb.append("&"));
          modified_flags &= ~FormatFlags::kRegCasts;
        }
        ASMJIT_PROPAGATE(format_register(sb, modified_flags, emitter, arch, m.base_type(), m.base_id()));
      }
    }

    if (m.has_index()) {
      if (op_sign) {
        ASMJIT_PROPAGATE(sb.append(op_sign));
      }

      op_sign = '+';
      ASMJIT_PROPAGATE(format_register(sb, format_flags, emitter, arch, m.index_type(), m.index_id()));
      if (m.has_shift())
        ASMJIT_PROPAGATE(sb.append_format("*%u", 1 << m.shift()));
    }

    uint64_t off = uint64_t(m.offset());
    if (off || !m.has_base_or_index()) {
      if (int64_t(off) < 0) {
        op_sign = '-';
        off = ~off + 1;
      }

      if (op_sign) {
        ASMJIT_PROPAGATE(sb.append(op_sign));
      }

      uint32_t base = 10;
      if (Support::test(format_flags, FormatFlags::kHexOffsets) && off > 9) {
        ASMJIT_PROPAGATE(sb.append("0x", 2));
        base = 16;
      }

      ASMJIT_PROPAGATE(sb.append_uint(off, base));
    }

    return sb.append(']');
  }

  if (op.is_imm()) {
    const Imm& i = op.as<Imm>();
    int64_t val = i.value();

    if (Support::test(format_flags, FormatFlags::kHexImms) && uint64_t(val) > 9) {
      ASMJIT_PROPAGATE(sb.append("0x", 2));
      return sb.append_uint(uint64_t(val), 16);
    }
    else {
      return sb.append_int(val, 10);
    }
  }

  if (op.is_label()) {
    return Formatter::format_label(sb, format_flags, emitter, op.id());
  }

  return sb.append("<None>");
}

// x86::FormatterInternal - Format Immediate (Extension)
// =====================================================

static constexpr char kImmCharStart = '{';
static constexpr char kImmCharEnd = '}';
static constexpr char kImmCharOr = '|';

struct ImmBits {
  enum Mode : uint32_t {
    kModeLookup = 0,
    kModeFormat = 1
  };

  uint8_t mask;
  uint8_t shift;
  uint8_t mode;
  char text[48 - 3];
};

ASMJIT_FAVOR_SIZE static Error FormatterInternal_format_imm_shuf(String& sb, uint32_t imm8, uint32_t bits, uint32_t count) noexcept {
  uint32_t mask = (1 << bits) - 1;
  uint32_t last_predicate_shift = bits * (count - 1u);

  for (uint32_t i = 0; i < count; i++, imm8 <<= bits) {
    uint32_t index = (imm8 >> last_predicate_shift) & mask;
    ASMJIT_PROPAGATE(sb.append(i == 0 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.append_uint(index));
  }

  ASMJIT_PROPAGATE(sb.append(kImmCharEnd));

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE static Error FormatterInternal_format_imm_bits(String& sb, uint32_t imm8, const ImmBits* bits, uint32_t count) noexcept {
  uint32_t n = 0;
  char buf[64];

  for (uint32_t i = 0; i < count; i++) {
    const ImmBits& spec = bits[i];

    uint32_t value = (imm8 & uint32_t(spec.mask)) >> spec.shift;
    const char* str = nullptr;

    switch (spec.mode) {
      case ImmBits::kModeLookup:
        str = Support::find_packed_string(spec.text, value);
        break;

      case ImmBits::kModeFormat:
        snprintf(buf, sizeof(buf), spec.text, unsigned(value));
        str = buf;
        break;

      default:
        return make_error(Error::kInvalidState);
    }

    if (!str[0]) {
      continue;
    }

    ASMJIT_PROPAGATE(sb.append(++n == 1 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.append(str));
  }

  if (n) {
    ASMJIT_PROPAGATE(sb.append(kImmCharEnd));
  }

  return Error::kOk;
}

ASMJIT_FAVOR_SIZE static Error FormatterInternal_format_imm_text(String& sb, uint32_t imm8, uint32_t bits, uint32_t advance, const char* text, uint32_t count = 1) noexcept {
  uint32_t mask = (1u << bits) - 1;
  uint32_t pos = 0;

  for (uint32_t i = 0; i < count; i++, imm8 >>= bits, pos += advance) {
    uint32_t value = (imm8 & mask) + pos;
    ASMJIT_PROPAGATE(sb.append(i == 0 ? kImmCharStart : kImmCharOr));
    ASMJIT_PROPAGATE(sb.append(Support::find_packed_string(text, value)));
  }

  return sb.append(kImmCharEnd);
}

ASMJIT_FAVOR_SIZE static Error FormatterInternal_explain_const(
  String& sb,
  FormatFlags format_flags,
  InstId inst_id,
  uint32_t vec_size,
  const Imm& imm
) noexcept {
  Support::maybe_unused(format_flags);

  static const char vcmpx[] =
    "EQ_OQ\0" "LT_OS\0"  "LE_OS\0"  "UNORD_Q\0"  "NEQ_UQ\0" "NLT_US\0" "NLE_US\0" "ORD_Q\0"
    "EQ_UQ\0" "NGE_US\0" "NGT_US\0" "FALSE_OQ\0" "NEQ_OQ\0" "GE_OS\0"  "GT_OS\0"  "TRUE_UQ\0"
    "EQ_OS\0" "LT_OQ\0"  "LE_OQ\0"  "UNORD_S\0"  "NEQ_US\0" "NLT_UQ\0" "NLE_UQ\0" "ORD_S\0"
    "EQ_US\0" "NGE_UQ\0" "NGT_UQ\0" "FALSE_OS\0" "NEQ_OS\0" "GE_OQ\0"  "GT_OQ\0"  "TRUE_US\0";

  // Why to make it compatible...
  static const char vpcmpx[] = "EQ\0" "LT\0" "LE\0" "FALSE\0" "NEQ\0" "GE\0"  "GT\0"    "TRUE\0";
  static const char vpcomx[] = "LT\0" "LE\0" "GT\0" "GE\0"    "EQ\0"  "NEQ\0" "FALSE\0" "TRUE\0";

  static const char vshufpd[] = "A0\0" "A1\0" "B0\0" "B1\0" "A2\0" "A3\0" "B2\0" "B3\0" "A4\0" "A5\0" "B4\0" "B5\0" "A6\0" "A7\0" "B6\0" "B7\0";
  static const char vshufps[] = "A0\0" "A1\0" "A2\0" "A3\0" "A0\0" "A1\0" "A2\0" "A3\0" "B0\0" "B1\0" "B2\0" "B3\0" "B0\0" "B1\0" "B2\0" "B3\0";

  static const ImmBits vfpclassxx[] = {
    { 0x07u, 0, ImmBits::kModeLookup, "QNAN\0" "+0\0" "-0\0" "+INF\0" "-INF\0" "DENORMAL\0" "-FINITE\0" "SNAN\0" }
  };

  static const ImmBits vfixupimmxx[] = {
    { 0x01u, 0, ImmBits::kModeLookup, "\0" "+INF_IE\0" },
    { 0x02u, 1, ImmBits::kModeLookup, "\0" "-VE_IE\0"  },
    { 0x04u, 2, ImmBits::kModeLookup, "\0" "-INF_IE\0" },
    { 0x08u, 3, ImmBits::kModeLookup, "\0" "SNAN_IE\0" },
    { 0x10u, 4, ImmBits::kModeLookup, "\0" "ONE_IE\0"  },
    { 0x20u, 5, ImmBits::kModeLookup, "\0" "ONE_ZE\0"  },
    { 0x40u, 6, ImmBits::kModeLookup, "\0" "ZERO_IE\0" },
    { 0x80u, 7, ImmBits::kModeLookup, "\0" "ZERO_ZE\0" }
  };

  static const ImmBits vgetmantxx[] = {
    { 0x03u, 0, ImmBits::kModeLookup, "[1, 2)\0" "[.5, 2)\0" "[.5, 1)\0" "[.75, 1.5)\0" },
    { 0x04u, 2, ImmBits::kModeLookup, "\0" "NO_SIGN\0" },
    { 0x08u, 3, ImmBits::kModeLookup, "\0" "QNAN_IF_SIGN\0" }
  };

  static const ImmBits vmpsadbw[] = {
    { 0x40u, 6, ImmBits::kModeLookup, "BLK1[4]\0" "BLK1[5]\0" },
    { 0x30u, 4, ImmBits::kModeLookup, "BLK2[4]\0" "BLK2[5]\0" "BLK2[6]\0" "BLK2[7]\0" },
    { 0x04u, 2, ImmBits::kModeLookup, "BLK1[0]\0" "BLK1[1]\0" },
    { 0x03u, 0, ImmBits::kModeLookup, "BLK2[0]\0" "BLK2[1]\0" "BLK2[2]\0" "BLK2[3]\0" }
  };

  static const ImmBits vpclmulqdq[] = {
    { 0x10u, 4, ImmBits::kModeLookup, "LQ\0" "HQ\0" },
    { 0x01u, 0, ImmBits::kModeLookup, "LQ\0" "HQ\0" }
  };

  static const ImmBits vperm2x128[] = {
    { 0xB0u, 4, ImmBits::kModeLookup, "A0\0" "A1\0" "B0\0" "B1\0" "\0" "\0" "\0" "\0" "0\0" "0\0" "0\0" "0\0" },
    { 0x0Bu, 0, ImmBits::kModeLookup, "A0\0" "A1\0" "B0\0" "B1\0" "\0" "\0" "\0" "\0" "0\0" "0\0" "0\0" "0\0" }
  };

  static const ImmBits vrangexx[] = {
    { 0x0Cu, 2, ImmBits::kModeLookup, "SIGN_A\0" "SIGN_B\0" "SIGN_0\0" "SIGN_1\0" },
    { 0x03u, 0, ImmBits::kModeLookup, "MIN\0" "MAX\0" "MIN_ABS\0" "MAX_ABS\0" }
  };

  static const ImmBits vreducexx_vrndscalexx[] = {
    { 0x07u, 0, ImmBits::kModeLookup, "\0" "\0" "\0" "\0" "ROUND\0" "FLOOR\0" "CEIL\0" "TRUNC\0" },
    { 0x08u, 3, ImmBits::kModeLookup, "\0" "SAE\0" },
    { 0xF0u, 4, ImmBits::kModeFormat, "LEN=%d" }
  };

  static const ImmBits vroundxx[] = {
    { 0x07u, 0, ImmBits::kModeLookup, "ROUND\0" "FLOOR\0" "CEIL\0" "TRUNC\0" "CURRENT\0" "\0" "\0" "\0" },
    { 0x08u, 3, ImmBits::kModeLookup, "\0" "SUPPRESS\0" }
  };

  uint32_t u8 = imm.value_as<uint8_t>();
  switch (inst_id) {
    case Inst::kIdVblendpd:
    case Inst::kIdBlendpd:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, vec_size / 8);

    case Inst::kIdVblendps:
    case Inst::kIdBlendps:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, vec_size / 4);

    case Inst::kIdVcmppd:
    case Inst::kIdVcmpps:
    case Inst::kIdVcmpsd:
    case Inst::kIdVcmpss:
      return FormatterInternal_format_imm_text(sb, u8, 5, 0, vcmpx);

    case Inst::kIdCmppd:
    case Inst::kIdCmpps:
    case Inst::kIdCmpsd:
    case Inst::kIdCmpss:
      return FormatterInternal_format_imm_text(sb, u8, 3, 0, vcmpx);

    case Inst::kIdVdbpsadbw:
      return FormatterInternal_format_imm_shuf(sb, u8, 2, 4);

    case Inst::kIdVdppd:
    case Inst::kIdVdpps:
    case Inst::kIdDppd:
    case Inst::kIdDpps:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, 8);

    case Inst::kIdVmpsadbw:
    case Inst::kIdMpsadbw:
      return FormatterInternal_format_imm_bits(sb, u8, vmpsadbw, Support::min<uint32_t>(vec_size / 8, 4));

    case Inst::kIdVpblendw:
    case Inst::kIdPblendw:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, 8);

    case Inst::kIdVpblendd:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, Support::min<uint32_t>(vec_size / 4, 8));

    case Inst::kIdVpclmulqdq:
    case Inst::kIdPclmulqdq:
      return FormatterInternal_format_imm_bits(sb, u8, vpclmulqdq, ASMJIT_ARRAY_SIZE(vpclmulqdq));

    case Inst::kIdVroundpd:
    case Inst::kIdVroundps:
    case Inst::kIdVroundsd:
    case Inst::kIdVroundss:
    case Inst::kIdRoundpd:
    case Inst::kIdRoundps:
    case Inst::kIdRoundsd:
    case Inst::kIdRoundss:
      return FormatterInternal_format_imm_bits(sb, u8, vroundxx, ASMJIT_ARRAY_SIZE(vroundxx));

    case Inst::kIdVshufpd:
    case Inst::kIdShufpd:
      return FormatterInternal_format_imm_text(sb, u8, 1, 2, vshufpd, Support::min<uint32_t>(vec_size / 8, 8));

    case Inst::kIdVshufps:
    case Inst::kIdShufps:
      return FormatterInternal_format_imm_text(sb, u8, 2, 4, vshufps, 4);

    case Inst::kIdVcvtps2ph:
      return FormatterInternal_format_imm_bits(sb, u8, vroundxx, 1);

    case Inst::kIdVperm2f128:
    case Inst::kIdVperm2i128:
      return FormatterInternal_format_imm_bits(sb, u8, vperm2x128, ASMJIT_ARRAY_SIZE(vperm2x128));

    case Inst::kIdVpermilpd:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, vec_size / 8);

    case Inst::kIdVpermilps:
      return FormatterInternal_format_imm_shuf(sb, u8, 2, 4);

    case Inst::kIdVpshufd:
    case Inst::kIdPshufd:
      return FormatterInternal_format_imm_shuf(sb, u8, 2, 4);

    case Inst::kIdVpshufhw:
    case Inst::kIdVpshuflw:
    case Inst::kIdPshufhw:
    case Inst::kIdPshuflw:
    case Inst::kIdPshufw:
      return FormatterInternal_format_imm_shuf(sb, u8, 2, 4);

    case Inst::kIdVfixupimmpd:
    case Inst::kIdVfixupimmps:
    case Inst::kIdVfixupimmsd:
    case Inst::kIdVfixupimmss:
      return FormatterInternal_format_imm_bits(sb, u8, vfixupimmxx, ASMJIT_ARRAY_SIZE(vfixupimmxx));

    case Inst::kIdVfpclasspd:
    case Inst::kIdVfpclassps:
    case Inst::kIdVfpclasssd:
    case Inst::kIdVfpclassss:
      return FormatterInternal_format_imm_bits(sb, u8, vfpclassxx, ASMJIT_ARRAY_SIZE(vfpclassxx));

    case Inst::kIdVgetmantpd:
    case Inst::kIdVgetmantps:
    case Inst::kIdVgetmantsd:
    case Inst::kIdVgetmantss:
      return FormatterInternal_format_imm_bits(sb, u8, vgetmantxx, ASMJIT_ARRAY_SIZE(vgetmantxx));

    case Inst::kIdVpcmpb:
    case Inst::kIdVpcmpd:
    case Inst::kIdVpcmpq:
    case Inst::kIdVpcmpw:
    case Inst::kIdVpcmpub:
    case Inst::kIdVpcmpud:
    case Inst::kIdVpcmpuq:
    case Inst::kIdVpcmpuw:
      return FormatterInternal_format_imm_text(sb, u8, 3, 0, vpcmpx);

    case Inst::kIdVpcomb:
    case Inst::kIdVpcomd:
    case Inst::kIdVpcomq:
    case Inst::kIdVpcomw:
    case Inst::kIdVpcomub:
    case Inst::kIdVpcomud:
    case Inst::kIdVpcomuq:
    case Inst::kIdVpcomuw:
      return FormatterInternal_format_imm_text(sb, u8, 3, 0, vpcomx);

    case Inst::kIdVpermq:
    case Inst::kIdVpermpd:
      return FormatterInternal_format_imm_shuf(sb, u8, 2, 4);

    case Inst::kIdVpternlogd:
    case Inst::kIdVpternlogq:
      return FormatterInternal_format_imm_shuf(sb, u8, 1, 8);

    case Inst::kIdVrangepd:
    case Inst::kIdVrangeps:
    case Inst::kIdVrangesd:
    case Inst::kIdVrangess:
      return FormatterInternal_format_imm_bits(sb, u8, vrangexx, ASMJIT_ARRAY_SIZE(vrangexx));

    case Inst::kIdVreducepd:
    case Inst::kIdVreduceps:
    case Inst::kIdVreducesd:
    case Inst::kIdVreducess:
    case Inst::kIdVrndscalepd:
    case Inst::kIdVrndscaleps:
    case Inst::kIdVrndscalesd:
    case Inst::kIdVrndscaless:
      return FormatterInternal_format_imm_bits(sb, u8, vreducexx_vrndscalexx, ASMJIT_ARRAY_SIZE(vreducexx_vrndscalexx));

    case Inst::kIdVshuff32x4:
    case Inst::kIdVshuff64x2:
    case Inst::kIdVshufi32x4:
    case Inst::kIdVshufi64x2: {
      uint32_t count = Support::max<uint32_t>(vec_size / 16, 2u);
      uint32_t bits = count <= 2 ? 1u : 2u;
      return FormatterInternal_format_imm_shuf(sb, u8, bits, count);
    }

    default:
      return Error::kOk;
  }
}

// x86::FormatterInternal - Format Instruction
// ===========================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept {

  InstId inst_id = inst.inst_id();
  InstOptions options = inst.options();

  // Format instruction options and instruction mnemonic.
  if (inst_id < Inst::_kIdCount) {
    // VEX|EVEX options.
    if (Support::test(options, InstOptions::kX86_Vex)) {
      ASMJIT_PROPAGATE(sb.append("{vex} "));
    }

    if (Support::test(options, InstOptions::kX86_Vex3)) {
      ASMJIT_PROPAGATE(sb.append("{vex3} "));
    }

    if (Support::test(options, InstOptions::kX86_Evex)) {
      ASMJIT_PROPAGATE(sb.append("{evex} "));
    }

    // MOD/RM and MOD/MR options
    if (Support::test(options, InstOptions::kX86_ModRM)) {
      ASMJIT_PROPAGATE(sb.append("{modrm} "));
    }
    else if (Support::test(options, InstOptions::kX86_ModMR)) {
      ASMJIT_PROPAGATE(sb.append("{modmr} "));
    }

    // SHORT|LONG options.
    if (Support::test(options, InstOptions::kShortForm)) {
      ASMJIT_PROPAGATE(sb.append("short "));
    }

    if (Support::test(options, InstOptions::kLongForm)) {
      ASMJIT_PROPAGATE(sb.append("long "));
    }

    // LOCK|XACQUIRE|XRELEASE options.
    if (Support::test(options, InstOptions::kX86_XAcquire)) {
      ASMJIT_PROPAGATE(sb.append("xacquire "));
    }

    if (Support::test(options, InstOptions::kX86_XRelease)) {
      ASMJIT_PROPAGATE(sb.append("xrelease "));
    }

    if (Support::test(options, InstOptions::kX86_Lock)) {
      ASMJIT_PROPAGATE(sb.append("lock "));
    }

    // REP|REPNE options.
    if (Support::test(options, InstOptions::kX86_Rep | InstOptions::kX86_Repne)) {
      ASMJIT_PROPAGATE(sb.append(Support::test(options, InstOptions::kX86_Rep) ? "rep " : "repnz "));
      if (inst.has_extra_reg()) {
        ASMJIT_PROPAGATE(sb.append("{"));
        ASMJIT_PROPAGATE(format_operand(sb, format_flags, emitter, arch, inst.extra_reg().to_reg<Reg>()));
        ASMJIT_PROPAGATE(sb.append("} "));
      }
    }

    // REX options.
    if (Support::test(options, InstOptions::kX86_Rex)) {
      ASMJIT_PROPAGATE(sb.append("rex "));
    }

    InstStringifyOptions stringify_options =
      Support::test(format_flags, FormatFlags::kShowAliases)
        ? InstStringifyOptions::kAliases
        : InstStringifyOptions::kNone;

    ASMJIT_PROPAGATE(InstInternal::inst_id_to_string(inst_id, stringify_options, sb));
  }
  else {
    ASMJIT_PROPAGATE(sb.append_format("[InstId=#%u]", unsigned(inst_id)));
  }

  for (size_t i = 0u; i < operands.size(); i++) {
    const Operand_& op = operands[i];

    if (op.is_none()) {
      break;
    }

    ASMJIT_PROPAGATE(sb.append(i == 0 ? " " : ", "));
    ASMJIT_PROPAGATE(format_operand(sb, format_flags, emitter, arch, op));

    if (op.is_imm() && uint32_t(format_flags & FormatFlags::kExplainImms)) {
      uint32_t vec_size = 16;
      for (size_t j = 0u; j < operands.size(); j++) {
        if (operands[j].is_reg()) {
          vec_size = Support::max<uint32_t>(vec_size, operands[j].as<Reg>().size());
        }
      }
      ASMJIT_PROPAGATE(FormatterInternal_explain_const(sb, format_flags, inst_id, vec_size, op.as<Imm>()));
    }

    // Support AVX-512 masking - {k}{z}.
    if (i == 0) {
      if (inst.extra_reg().group() == RegGroup::kMask) {
        ASMJIT_PROPAGATE(sb.append(" {"));
        ASMJIT_PROPAGATE(format_register(sb, format_flags, emitter, arch, inst.extra_reg().type(), inst.extra_reg().id()));
        ASMJIT_PROPAGATE(sb.append('}'));

        if (Support::test(options, InstOptions::kX86_ZMask)) {
          ASMJIT_PROPAGATE(sb.append("{z}"));
        }
      }
      else if (Support::test(options, InstOptions::kX86_ZMask)) {
        ASMJIT_PROPAGATE(sb.append(" {z}"));
      }
    }

    // Support AVX-512 broadcast - {1tox}.
    if (op.is_mem() && op.as<Mem>().has_broadcast()) {
      ASMJIT_PROPAGATE(sb.append_format(" {1to%u}", Support::bit_mask<uint32_t>(uint32_t(op.as<Mem>().get_broadcast()))));
    }
  }

  // Support AVX-512 embedded rounding and suppress-all-exceptions {sae}.
  if (inst.has_option(InstOptions::kX86_ER | InstOptions::kX86_SAE)) {
    if (inst.has_option(InstOptions::kX86_ER)) {
      uint32_t bits = uint32_t(inst.options() & InstOptions::kX86_ERMask) >> Support::ctz_const<InstOptions::kX86_ERMask>;

      const char rounding_modes[] = "rn\0rd\0ru\0rz";
      ASMJIT_PROPAGATE(sb.append_format(", {%s-sae}", rounding_modes + bits * 3));
    }
    else {
      ASMJIT_PROPAGATE(sb.append(", {sae}"));
    }
  }

  return Error::kOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_X86 && !ASMJIT_NO_LOGGING
