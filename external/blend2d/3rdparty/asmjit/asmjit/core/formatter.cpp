// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#ifndef ASMJIT_NO_LOGGING

#include <asmjit/core/archtraits.h>
#include <asmjit/core/builder.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/compiler.h>
#include <asmjit/core/emitter.h>
#include <asmjit/core/formatter_p.h>
#include <asmjit/core/string.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86/x86formatter_p.h>
#endif

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/arm/a64formatter_p.h>
#endif

ASMJIT_BEGIN_NAMESPACE

#if defined(ASMJIT_NO_COMPILER)
class VirtReg;
#endif

namespace Formatter {

Error format_virt_reg_name(String& sb, const VirtReg* virt_reg) noexcept {
  if (virt_reg->name_size()) {
    return sb.append(virt_reg->name(), virt_reg->name_size());
  }
  else {
    return sb.append_format("%%%u", unsigned(Operand::virt_id_to_index(virt_reg->id())));
  }
}

Error format_virt_reg_name_with_prefix(String& sb, const char* prefix, size_t prefix_size, const VirtReg* v_reg) noexcept {
  ASMJIT_PROPAGATE(sb.append(prefix, prefix_size));

  if (v_reg->name_size()) {
    return sb.append(v_reg->name(), v_reg->name_size());
  }
  else {
    return sb.append_format("%%%u", unsigned(Operand::virt_id_to_index(v_reg->id())));
  }
}

static const char word_name_table[][8] = {
  "db",
  "dw",
  "dd",
  "dq",
  "byte",
  "half",
  "word",
  "hword",
  "dword",
  "qword",
  "xword",
  "short",
  "long",
  "quad"
};


Error format_type_id(String& sb, TypeId type_id) noexcept {
  if (type_id == TypeId::kVoid) {
    return sb.append("void");
  }

  if (!TypeUtils::is_valid(type_id)) {
    return sb.append("unknown");
  }

  const char* type_name = nullptr;
  uint32_t type_size = TypeUtils::size_of(type_id);
  TypeId scalar_type = TypeUtils::scalar_of(type_id);

  switch (scalar_type) {
    case TypeId::kIntPtr : type_name = "intptr" ; break;
    case TypeId::kUIntPtr: type_name = "uintptr"; break;
    case TypeId::kInt8   : type_name = "int8"   ; break;
    case TypeId::kUInt8  : type_name = "uint8"  ; break;
    case TypeId::kInt16  : type_name = "int16"  ; break;
    case TypeId::kUInt16 : type_name = "uint16" ; break;
    case TypeId::kInt32  : type_name = "int32"  ; break;
    case TypeId::kUInt32 : type_name = "uint32" ; break;
    case TypeId::kInt64  : type_name = "int64"  ; break;
    case TypeId::kUInt64 : type_name = "uint64" ; break;
    case TypeId::kFloat32: type_name = "float32"; break;
    case TypeId::kFloat64: type_name = "float64"; break;
    case TypeId::kFloat80: type_name = "float80"; break;
    case TypeId::kMask8  : type_name = "mask8"  ; break;
    case TypeId::kMask16 : type_name = "mask16" ; break;
    case TypeId::kMask32 : type_name = "mask32" ; break;
    case TypeId::kMask64 : type_name = "mask64" ; break;
    case TypeId::kMmx32  : type_name = "mmx32"  ; break;
    case TypeId::kMmx64  : type_name = "mmx64"  ; break;

    default:
      type_name = "unknown";
      break;
  }

  uint32_t base_size = TypeUtils::size_of(scalar_type);
  if (type_size > base_size) {
    uint32_t count = type_size / base_size;
    return sb.append_format("%sx%u", type_name, unsigned(count));
  }
  else {
    return sb.append(type_name);
  }
}

Error format_feature(String& sb, Arch arch, uint32_t feature_id) noexcept {
#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::FormatterInternal::format_feature(sb, feature_id);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_arm(arch)) {
    return arm::FormatterInternal::format_feature(sb, feature_id);
  }
#endif

  return make_error(Error::kInvalidArch);
}

Error format_label(String& sb, FormatFlags format_flags, const BaseEmitter* emitter, uint32_t label_id) noexcept {
  Support::maybe_unused(format_flags);

  if (emitter && emitter->code()) {
    CodeHolder* code = emitter->code();
    if (ASMJIT_UNLIKELY(!code->is_label_valid(label_id))) {
      return sb.append_format("<InvalidLabel:%u>", label_id);
    }

    const LabelEntry& le = code->label_entry_of(label_id);
    if (le.has_name()) {
      if (le.has_parent()) {
        uint32_t parent_id = le.parent_id();
        const LabelEntry& pe = code->label_entry_of(parent_id);

        if (pe.has_name()) {
          ASMJIT_PROPAGATE(sb.append(pe.name()));
        }
        else {
          ASMJIT_PROPAGATE(sb.append_format("L%u", parent_id));
        }

        ASMJIT_PROPAGATE(sb.append('.'));
      }

      if (le.label_type() == LabelType::kAnonymous) {
        ASMJIT_PROPAGATE(sb.append_format("L%u@", label_id));
      }
      return sb.append(le.name());
    }
  }

  return sb.append_format("L%u", label_id);
}

Error format_register(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_id) noexcept {

#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::FormatterInternal::format_register(sb, format_flags, emitter, arch, reg_type, reg_id);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_arm(arch)) {
    return arm::FormatterInternal::format_register(sb, format_flags, emitter, arch, reg_type, reg_id);
  }
#endif

  return make_error(Error::kInvalidArch);
}

Error format_operand(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept {

#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::FormatterInternal::format_operand(sb, format_flags, emitter, arch, op);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_arm(arch)) {
    return arm::FormatterInternal::format_operand(sb, format_flags, emitter, arch, op);
  }
#endif

  return make_error(Error::kInvalidArch);
}

ASMJIT_API Error format_data_type(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id) noexcept
{
  Support::maybe_unused(format_flags);

  if (ASMJIT_UNLIKELY(uint32_t(arch) > uint32_t(Arch::kMaxValue))) {
    return make_error(Error::kInvalidArch);
  }

  uint32_t type_size = TypeUtils::size_of(type_id);
  if (type_size == 0 || type_size > 8) {
    return make_error(Error::kInvalidState);
  }

  uint32_t type_size_log2 = Support::ctz(type_size);
  return sb.append(word_name_table[size_t(ArchTraits::by_arch(arch).type_name_id_by_index(type_size_log2))]);
}

static Error format_data_helper(String& sb, const char* type_name, uint32_t type_size, const uint8_t* data, size_t item_count) noexcept {
  sb.append('.');
  sb.append(type_name);
  sb.append(' ');

  for (size_t i = 0; i < item_count; i++) {
    uint64_t v = 0;

    if (i != 0) {
      ASMJIT_PROPAGATE(sb.append(", ", 2));
    }

    switch (type_size) {
      case 1: v = data[0]; break;
      case 2: v = Support::loadu_u16(data); break;
      case 4: v = Support::loadu_u32(data); break;
      case 8: v = Support::loadu_u64(data); break;
    }

    ASMJIT_PROPAGATE(sb.append_uint(v, 16, type_size * 2, StringFormatFlags::kAlternate));
    data += type_size;
  }

  return Error::kOk;
}

Error format_data(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id, const void* data, size_t item_count, size_t repeat_count
) noexcept {
  Support::maybe_unused(format_flags);

  if (ASMJIT_UNLIKELY(!Environment::is_defined_arch(arch))) {
    return make_error(Error::kInvalidArch);
  }

  uint32_t type_size = TypeUtils::size_of(type_id);
  if (type_size == 0) {
    return make_error(Error::kInvalidState);
  }

  if (!Support::is_power_of_2(type_size)) {
    item_count *= type_size;
    type_size = 1;
  }

  while (type_size > 8u) {
    type_size >>= 1;
    item_count <<= 1;
  }

  uint32_t type_size_log2 = Support::ctz(type_size);
  const char* word_name = word_name_table[size_t(ArchTraits::by_arch(arch).type_name_id_by_index(type_size_log2))];

  if (repeat_count > 1) {
    ASMJIT_PROPAGATE(sb.append_format(".repeat %zu ", repeat_count));
  }

  return format_data_helper(sb, word_name, type_size, static_cast<const uint8_t*>(data), item_count);
}

Error format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept {

#if !defined(ASMJIT_NO_X86)
  if (Environment::is_family_x86(arch)) {
    return x86::FormatterInternal::format_instruction(sb, format_flags, emitter, arch, inst, operands);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (Environment::is_family_aarch64(arch)) {
    return a64::FormatterInternal::format_instruction(sb, format_flags, emitter, arch, inst, operands);
  }
#endif

  return make_error(Error::kInvalidArch);
}

#ifndef ASMJIT_NO_BUILDER

#ifndef ASMJIT_NO_COMPILER
static Error format_func_value(String& sb, FormatFlags format_flags, const BaseEmitter* emitter, FuncValue value) noexcept {
  TypeId type_id = value.type_id();
  ASMJIT_PROPAGATE(format_type_id(sb, type_id));

  if (value.is_assigned()) {
    ASMJIT_PROPAGATE(sb.append('@'));

    if (value.is_indirect()) {
      ASMJIT_PROPAGATE(sb.append('['));
    }

    // NOTE: It should be either reg or stack, but never both. We
    // use two IFs on purpose so if the FuncValue is both it would
    // show in logs.
    if (value.is_reg()) {
      ASMJIT_PROPAGATE(format_register(sb, format_flags, emitter, emitter->arch(), value.reg_type(), value.reg_id()));
    }

    if (value.is_stack()) {
      ASMJIT_PROPAGATE(sb.append_format("[%d]", int(value.stack_offset())));
    }

    if (value.is_indirect()) {
      ASMJIT_PROPAGATE(sb.append(']'));
    }
  }

  return Error::kOk;
}

static Error format_func_value_pack(
  String& sb,
  FormatFlags format_flags,
  const BaseCompiler* cc,
  const FuncValuePack& pack,
  const RegOnly* virt_regs) noexcept {

  size_t count = pack.count();
  if (!count) {
    return sb.append("void");
  }

  if (count > 1) {
    ASMJIT_PROPAGATE(sb.append('['));
  }

  for (uint32_t value_index = 0; value_index < count; value_index++) {
    const FuncValue& value = pack[value_index];
    if (!value) {
      break;
    }

    if (value_index) {
      ASMJIT_PROPAGATE(sb.append(", "));
    }

    ASMJIT_PROPAGATE(format_func_value(sb, format_flags, cc, value));

    if (virt_regs) {
      const VirtReg* virt_reg = nullptr;
      static const char null_reg[] = "<none>";

      if (virt_regs[value_index].is_reg() && cc->is_virt_id_valid(virt_regs[value_index].id())) {
        virt_reg = cc->virt_reg_by_id(virt_regs[value_index].id());
      }

      ASMJIT_PROPAGATE(sb.append(' '));

      if (virt_reg) {
        ASMJIT_PROPAGATE(Formatter::format_virt_reg_name(sb, virt_reg));
      }
      else {
        ASMJIT_PROPAGATE(sb.append(null_reg, sizeof(null_reg) - 1));
      }
    }
  }

  if (count > 1) {
    ASMJIT_PROPAGATE(sb.append(']'));
  }

  return Error::kOk;
}

static Error format_func_rets(
  String& sb,
  FormatFlags format_flags,
  const BaseCompiler* cc,
  const FuncDetail& fd) noexcept {

  return format_func_value_pack(sb, format_flags, cc, fd.ret_pack(), nullptr);
}

static Error format_func_args(
  String& sb,
  FormatFlags format_flags,
  const BaseCompiler* cc,
  const FuncDetail& fd,
  const FuncNode::ArgPack* arg_packs) noexcept {

  uint32_t arg_count = fd.arg_count();
  if (!arg_count) {
    return sb.append("void");
  }

  for (uint32_t arg_index = 0; arg_index < arg_count; arg_index++) {
    if (arg_index) {
      ASMJIT_PROPAGATE(sb.append(", "));
    }
    ASMJIT_PROPAGATE(format_func_value_pack(sb, format_flags, cc, fd.arg_pack(arg_index), arg_packs[arg_index]._data));
  }

  return Error::kOk;
}
#endif

Error format_node(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* node) noexcept {

  if (node->has_position() && format_options.has_flag(FormatFlags::kPositions)) {
    ASMJIT_PROPAGATE(sb.append_format("<%05u> ", uint32_t(node->position())));
  }

  size_t start_line_index = sb.size();

  switch (node->type()) {
    case NodeType::kInst:
    case NodeType::kJump: {
      const InstNode* inst_node = node->as<InstNode>();
      ASMJIT_PROPAGATE(builder->_funcs.format_instruction(sb, format_options.flags(), builder, builder->arch(), inst_node->baseInst(), inst_node->operands()));
      break;
    }

    case NodeType::kSection: {
      const SectionNode* section_node = node->as<SectionNode>();
      if (builder->_code->is_section_valid(section_node->section_id())) {
        const Section* section = builder->_code->section_by_id(section_node->section_id());
        ASMJIT_PROPAGATE(sb.append_format(".section %s", section->name()));
      }
      break;
    }

    case NodeType::kLabel: {
      const LabelNode* label_node = node->as<LabelNode>();
      ASMJIT_PROPAGATE(format_label(sb, format_options.flags(), builder, label_node->label_id()));
      ASMJIT_PROPAGATE(sb.append(":"));
      break;
    }

    case NodeType::kAlign: {
      const AlignNode* align_node = node->as<AlignNode>();
      ASMJIT_PROPAGATE(sb.append_format(".align %u (%s)",
        align_node->alignment(),
        align_node->align_mode() == AlignMode::kCode ? "code" : "data"));
      break;
    }

    case NodeType::kEmbedData: {
      const EmbedDataNode* embed_node = node->as<EmbedDataNode>();
      ASMJIT_PROPAGATE(sb.append('.'));
      ASMJIT_PROPAGATE(format_data_type(sb, format_options.flags(), builder->arch(), embed_node->type_id()));
      ASMJIT_PROPAGATE(sb.append_format(" {Count=%zu Repeat=%zu TotalSize=%zu}", embed_node->item_count(), embed_node->repeat_count(), embed_node->data_size()));
      break;
    }

    case NodeType::kEmbedLabel: {
      const EmbedLabelNode* embed_node = node->as<EmbedLabelNode>();
      ASMJIT_PROPAGATE(sb.append(".label "));
      ASMJIT_PROPAGATE(format_label(sb, format_options.flags(), builder, embed_node->label_id()));
      break;
    }

    case NodeType::kEmbedLabelDelta: {
      const EmbedLabelDeltaNode* embed_node = node->as<EmbedLabelDeltaNode>();
      ASMJIT_PROPAGATE(sb.append(".label ("));
      ASMJIT_PROPAGATE(format_label(sb, format_options.flags(), builder, embed_node->label_id()));
      ASMJIT_PROPAGATE(sb.append(" - "));
      ASMJIT_PROPAGATE(format_label(sb, format_options.flags(), builder, embed_node->base_label_id()));
      ASMJIT_PROPAGATE(sb.append(")"));
      break;
    }

    case NodeType::kConstPool: {
      const ConstPoolNode* const_pool_node = node->as<ConstPoolNode>();
      ASMJIT_PROPAGATE(sb.append_format("[ConstPool Size=%zu Alignment=%zu]", const_pool_node->size(), const_pool_node->alignment()));
      break;
    };

    case NodeType::kComment: {
      const CommentNode* comment_node = node->as<CommentNode>();
      return sb.append_format("; %s", comment_node->inline_comment());
    }

    case NodeType::kSentinel: {
      const SentinelNode* sentinel_node = node->as<SentinelNode>();
      const char* sentinel_name = nullptr;

      switch (sentinel_node->sentinel_type()) {
        case SentinelType::kFuncEnd:
          sentinel_name = "[FuncEnd]";
          break;

        default:
          sentinel_name = "[Sentinel]";
          break;
      }

      ASMJIT_PROPAGATE(sb.append(sentinel_name));
      break;
    }

#ifndef ASMJIT_NO_COMPILER
    case NodeType::kFunc: {
      const FuncNode* func_node = node->as<FuncNode>();

      if (builder->is_compiler()) {
        ASMJIT_PROPAGATE(format_label(sb, format_options.flags(), builder, func_node->label_id()));
        ASMJIT_PROPAGATE(sb.append(": "));

        ASMJIT_PROPAGATE(format_func_rets(sb, format_options.flags(), static_cast<const BaseCompiler*>(builder), func_node->detail()));
        ASMJIT_PROPAGATE(sb.append(" Func("));
        ASMJIT_PROPAGATE(format_func_args(sb, format_options.flags(), static_cast<const BaseCompiler*>(builder), func_node->detail(), func_node->arg_packs()));
        ASMJIT_PROPAGATE(sb.append(")"));
      }
      break;
    }

    case NodeType::kFuncRet: {
      const FuncRetNode* ret_node = node->as<FuncRetNode>();
      ASMJIT_PROPAGATE(sb.append("[FuncRet]"));

      for (uint32_t i = 0; i < 2; i++) {
        const Operand_& op = ret_node->op(i);
        if (!op.is_none()) {
          ASMJIT_PROPAGATE(sb.append(i == 0 ? " " : ", "));
          ASMJIT_PROPAGATE(format_operand(sb, format_options.flags(), builder, builder->arch(), op));
        }
      }
      break;
    }

    case NodeType::kInvoke: {
      const InvokeNode* invoke_node = node->as<InvokeNode>();
      ASMJIT_PROPAGATE(builder->_funcs.format_instruction(sb, format_options.flags(), builder, builder->arch(), invoke_node->baseInst(), invoke_node->operands()));
      break;
    }
#endif

    default: {
      ASMJIT_PROPAGATE(sb.append_format("[UserNode:%u]", node->type()));
      break;
    }
  }

  if (node->has_inline_comment()) {
    size_t required_padding = padding_from_options(format_options, FormatPaddingGroup::kRegularLine);
    size_t current_padding = sb.size() - start_line_index;

    if (current_padding < required_padding) {
      ASMJIT_PROPAGATE(sb.append_chars(' ', required_padding - current_padding));
    }

    ASMJIT_PROPAGATE(sb.append("; "));
    ASMJIT_PROPAGATE(sb.append(node->inline_comment()));
  }

  return Error::kOk;
}

Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder) noexcept {

  return format_node_list(sb, format_options, builder, builder->first_node(), nullptr);
}

Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* begin,
  const BaseNode* end) noexcept {

  const BaseNode* node = begin;
  while (node != end) {
    ASMJIT_PROPAGATE(format_node(sb, format_options, builder, node));
    ASMJIT_PROPAGATE(sb.append('\n'));
    node = node->next();
  }
  return Error::kOk;
}
#endif

} // {Formatter}

ASMJIT_END_NAMESPACE

#endif
