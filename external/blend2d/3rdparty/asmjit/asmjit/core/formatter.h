// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FORMATTER_H_INCLUDED
#define ASMJIT_CORE_FORMATTER_H_INCLUDED

#include <asmjit/core/globals.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/string.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_logging
//! \{

class BaseBuilder;
class BaseEmitter;
class BaseNode;
struct Operand_;

//! Format flags used by \ref Logger and \ref FormatOptions.
enum class FormatFlags : uint32_t {
  //! No formatting flags.
  kNone = 0u,

  //! Show also a binary representation of each logged instruction (Assembler).
  kMachineCode = 0x00000001u,
  //! Show aliases of some instructions that have them.
  //!
  //! This option is now mostly for x86/x64 to show aliases of instructions such as `cmov<cc>`, `j<cc>`, `set<cc>`,
  //! etc...
  kShowAliases = 0x00000008u,
  //! Show a text explanation of some immediate values.
  kExplainImms = 0x00000010u,
  //! Use hexadecimal notation of immediate values.
  kHexImms = 0x00000020u,
  //! Use hexadecimal notation of addresses and offsets in addresses.
  kHexOffsets = 0x00000040u,
  //! Show casts between virtual register types (Compiler).
  kRegCasts = 0x00000100u,
  //! Show positions associated with nodes (Compiler).
  kPositions = 0x00000200u,
  //! Always format a register type (Compiler).
  kRegType = 0x00000400u
};
ASMJIT_DEFINE_ENUM_FLAGS(FormatFlags)

//! Format indentation group, used by \ref FormatOptions.
enum class FormatIndentationGroup : uint32_t {
  //! Indentation used for instructions and directives.
  kCode = 0u,
  //! Indentation used for labels and function nodes.
  kLabel = 1u,
  //! Indentation used for comments (not inline comments).
  kComment = 2u,

  //! \cond INTERNAL
  //! Reserved for future use.
  kReserved = 3u,
  //! \endcond

  //! Maximum value of `FormatIndentationGroup`.
  kMaxValue = kReserved
};

//! Format padding group, used by \ref FormatOptions.
enum class FormatPaddingGroup : uint32_t {
  //! Describes padding of a regular line, which can represent instruction, data, or assembler directives.
  kRegularLine = 0,
  //! Describes padding of machine code dump that is visible next to the instruction, if enabled.
  kMachineCode = 1,

  //! Maximum value of `FormatPaddingGroup`.
  kMaxValue = kMachineCode
};

//! Formatting options used by \ref Logger and \ref Formatter.
class FormatOptions {
public:
  //! \name Members
  //! \{

  //! Format flags.
  FormatFlags _flags = FormatFlags::kNone;
  //! Indentations for each indentation group.
  Support::Array<uint8_t, uint32_t(FormatIndentationGroup::kMaxValue) + 1> _indentation {};
  //! Paddings for each padding group.
  Support::Array<uint16_t, uint32_t(FormatPaddingGroup::kMaxValue) + 1> _padding {};

  //! \}

  //! \name Reset
  //! \{

  //! Resets FormatOptions to its default initialized state.
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _flags = FormatFlags::kNone;
    _indentation.fill(uint8_t(0));
    _padding.fill(uint16_t(0));
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns format flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FormatFlags flags() const noexcept { return _flags; }

  //! Tests whether the given `flag` is set in format flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(FormatFlags flag) const noexcept { return Support::test(_flags, flag); }

  //! Resets all format flags to `flags`.
  ASMJIT_INLINE_NODEBUG void set_flags(FormatFlags flags) noexcept { _flags = flags; }

  //! Adds `flags` to format flags.
  ASMJIT_INLINE_NODEBUG void add_flags(FormatFlags flags) noexcept { _flags |= flags; }

  //! Removes `flags` from format flags.
  ASMJIT_INLINE_NODEBUG void clear_flags(FormatFlags flags) noexcept { _flags &= ~flags; }

  //! Returns indentation for the given indentation `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t indentation(FormatIndentationGroup group) const noexcept { return _indentation[group]; }

  //! Sets indentation for the given indentation `group`.
  ASMJIT_INLINE_NODEBUG void set_indentation(FormatIndentationGroup group, uint32_t n) noexcept { _indentation[group] = uint8_t(n); }

  //! Resets indentation for the given indentation `group` to zero.
  ASMJIT_INLINE_NODEBUG void reset_indentation(FormatIndentationGroup group) noexcept { _indentation[group] = uint8_t(0); }

  //! Returns padding for the given padding `group`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t padding(FormatPaddingGroup group) const noexcept { return _padding[group]; }

  //! Sets padding for the given padding `group`.
  ASMJIT_INLINE_NODEBUG void set_padding(FormatPaddingGroup group, size_t n) noexcept { _padding[group] = uint16_t(n); }

  //! Resets padding for the given padding `group` to zero, which means that a default padding will be used
  //! based on the target architecture properties.
  ASMJIT_INLINE_NODEBUG void reset_padding(FormatPaddingGroup group) noexcept { _padding[group] = uint16_t(0); }

  //! \}
};

//! Provides formatting functionality to format operands, instructions, and nodes.
namespace Formatter {

#ifndef ASMJIT_NO_LOGGING

//! Appends a formatted `type_id` to the output string `sb`.
ASMJIT_API Error format_type_id(
  String& sb,
  TypeId type_id) noexcept;

//! Appends a formatted `feature_id` to the output string `sb`.
//!
//! See \ref CpuFeatures.
ASMJIT_API Error format_feature(
  String& sb,
  Arch arch,
  uint32_t feature_id) noexcept;

//! Appends a formatted register to the output string `sb`.
//!
//! \note Emitter is optional, but it's required to format virtual registers, which won't be formatted properly
//! if the `emitter` is not provided.
ASMJIT_API Error format_register(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  RegType reg_type,
  uint32_t reg_id) noexcept;

//! Appends a formatted label to the output string `sb`.
//!
//! \note Emitter is optional, but it's required to format named labels properly, otherwise the formatted as
//! it is an anonymous label.
ASMJIT_API Error format_label(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  uint32_t label_id) noexcept;

//! Appends a formatted operand to the output string `sb`.
//!
//! \note Emitter is optional, but it's required to format named labels and virtual registers. See
//! \ref format_register() and \ref format_label() for more details.
ASMJIT_API Error format_operand(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const Operand_& op) noexcept;

//! Appends a formatted data-type to the output string `sb`.
ASMJIT_API Error format_data_type(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id) noexcept;

//! Appends a formatted data to the output string `sb`.
ASMJIT_API Error format_data(
  String& sb,
  FormatFlags format_flags,
  Arch arch,
  TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1) noexcept;

//! Appends a formatted instruction to the output string `sb`.
//!
//! \note Emitter is optional, but it's required to format named labels and virtual registers. See
//! \ref format_register() and \ref format_label() for more details.
ASMJIT_API Error format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept;

#ifndef ASMJIT_NO_BUILDER
//! Appends a formatted node to the output string `sb`.
//!
//! The `node` must belong to the provided `builder`.
ASMJIT_API Error format_node(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* node) noexcept;

//! Appends formatted nodes to the output string `sb`.
//!
//! All nodes that are part of the given `builder` will be appended.
ASMJIT_API Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder) noexcept;

//! Appends formatted nodes to the output string `sb`.
//!
//! This function works the same as \ref format_node(), but appends more nodes to the output string,
//! separating each node with a newline '\n' character.
ASMJIT_API Error format_node_list(
  String& sb,
  const FormatOptions& format_options,
  const BaseBuilder* builder,
  const BaseNode* begin,
  const BaseNode* end) noexcept;
#endif

#endif

} // {Formatter}

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FORMATTER_H_INCLUDED
