// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CODEHOLDER_H_INCLUDED
#define ASMJIT_CORE_CODEHOLDER_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/codebuffer.h>
#include <asmjit/core/errorhandler.h>
#include <asmjit/core/fixup.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/string.h>
#include <asmjit/core/target.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenahash.h>
#include <asmjit/support/arenapool.h>
#include <asmjit/support/arenastring.h>
#include <asmjit/support/arenatree.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/span.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

class BaseEmitter;
class CodeHolder;
class LabelEntry;
class Logger;

//! Operator type that can be used within an \ref Expression.
enum class ExpressionOpType : uint8_t {
  //! Addition.
  kAdd = 0,
  //! Subtraction.
  kSub = 1,
  //! Multiplication
  kMul = 2,
  //! Logical left shift.
  kSll = 3,
  //! Logical right shift.
  kSrl = 4,
  //! Arithmetic right shift.
  kSra = 5
};

//! Value type that can be used within an \ref Expression.
enum class ExpressionValueType : uint8_t {
  //! No value or invalid.
  kNone = 0,
  //! Value is 64-bit unsigned integer (constant).
  kConstant = 1,
  //! Value is \ref LabelEntry, which references a \ref Label.
  kLabel = 2,
  //! Value is \ref Expression
  kExpression = 3
};

//! Expression node that can reference constants, labels, and another expressions.
struct Expression {
  //! Expression value.
  union Value {
    //! Constant.
    uint64_t constant;
    //! Pointer to another expression.
    Expression* expression;
    //! Label identifier
    uint32_t label_id;
  };

  //! \name Members
  //! \{

  //! Operation type.
  ExpressionOpType op_type;
  //! Value types of \ref value.
  ExpressionValueType value_type[2];
  //! Reserved for future use, should be initialized to zero.
  uint8_t reserved[5];
  //! Expression left and right values.
  Value value[2];

  //! \}

  //! \name Accessors
  //! \{

  //! Resets the whole expression.
  //!
  //! Changes both values to \ref ExpressionValueType::kNone.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Expression{}; }

  //! Sets the value type at `index` to \ref ExpressionValueType::kConstant and its content to `constant`.
  ASMJIT_INLINE_NODEBUG void set_value_as_constant(size_t index, uint64_t constant) noexcept {
    value_type[index] = ExpressionValueType::kConstant;
    value[index].constant = constant;
  }

  //! Sets the value type at `index` to \ref ExpressionValueType::kLabel and its content to `label_entry`.
  ASMJIT_INLINE_NODEBUG void set_value_as_label_id(size_t index, uint32_t label_id) noexcept {
    value_type[index] = ExpressionValueType::kLabel;
    value[index].label_id = label_id;
  }

  //! Sets the value type at `index` to \ref ExpressionValueType::kExpression and its content to `expression`.
  ASMJIT_INLINE_NODEBUG void set_value_as_expression(size_t index, Expression* expression) noexcept {
    value_type[index] = ExpressionValueType::kExpression;
    value[index].expression = expression;
  }

  //! \}
};

//! Relocation type.
enum class RelocType : uint32_t {
  //! None/deleted (no relocation).
  kNone = 0,
  //! Expression evaluation, `_payload` is pointer to `Expression`.
  kExpression = 1,
  //! Relative relocation from one section to another.
  kSectionRelative = 2,
  //! Relocate absolute to absolute.
  kAbsToAbs = 3,
  //! Relocate relative to absolute.
  kRelToAbs = 4,
  //! Relocate absolute to relative.
  kAbsToRel = 5,
  //! Relocate absolute to relative or use trampoline.
  kX64AddressEntry = 6
};

//! Type of the \ref Label.
enum class LabelType : uint8_t {
  //! Anonymous label that can optionally have a name, which is only used for debugging purposes.
  kAnonymous = 0u,
  //! Local label (always has parent_id).
  kLocal = 1u,
  //! Global label (never has parent_id).
  kGlobal = 2u,
  //! External label (references an external symbol).
  kExternal = 3u,

  //! Maximum value of `LabelType`.
  kMaxValue = kExternal
};

//! Label flags describe some details about labels used by \ref LabelEntry, mostly for AsmJit's own use.
enum class LabelFlags : uint8_t {
  //! No flags.
  kNone = 0x00u,
  //! Label has associated extra data with it that it owns.
  kHasOwnExtraData = 0x01u,
  //! Label has a name.
  kHasName = 0x02u,
  //! Label has a parent (only a local label can have a parent).
  kHasParent = 0x04u
};
ASMJIT_DEFINE_ENUM_FLAGS(LabelFlags)

//! Section flags, used by \ref Section.
enum class SectionFlags : uint32_t {
  //! No flags.
  kNone = 0,
  //! Executable (.text sections).
  kExecutable = 0x0001u,
  //! Read-only (.text and .data sections).
  kReadOnly = 0x0002u,
  //! Zero initialized by the loader (BSS).
  kZeroInitialized = 0x0004u,
  //! Info / comment flag.
  kComment = 0x0008u,
  //! Section is built in and created by default (.text section).
  kBuiltIn = 0x4000u,
  //! Section created implicitly, can be deleted by \ref Target.
  kImplicit = 0x8000u
};
ASMJIT_DEFINE_ENUM_FLAGS(SectionFlags)

//! Flags that can be used with \ref CodeHolder::copy_section_data() and \ref CodeHolder::copy_flattened_data().
enum class CopySectionFlags : uint32_t {
  //! No flags.
  kNone = 0,

  //! If virtual size of a section is greater than the size of its \ref CodeBuffer then all bytes between the buffer
  //! size and virtual size will be zeroed. If this option is not set then those bytes would be left as is, which
  //! means that if the user didn't initialize them they would have a previous content, which may be unwanted.
  kPadSectionBuffer = 0x00000001u,

  //! Clears the target buffer if the flattened data is less than the destination size. This option works
  //! only with \ref CodeHolder::copy_flattened_data() as it processes multiple sections. It is ignored by
  //! \ref CodeHolder::copy_section_data().
  kPadTargetBuffer = 0x00000002u
};
ASMJIT_DEFINE_ENUM_FLAGS(CopySectionFlags)

//! Base class for both \ref Section and \ref LabelEntry::ExtraData.
class SectionOrLabelEntryExtraHeader {
public:
  //! \name Members
  //! \{

  //! Section id - describes either a section where a \ref Label is bound or it's a real section id of \ref Section.
  uint32_t _section_id;

  //! Internal label type is only used by \ref LabelEntry::ExtraData. \ref Section always leaves this field zero,
  //! which describes an anonymous label. Anonymous labels are default and always used when there is no
  //! \ref LabelEntry::ExtraData
  LabelType _internal_label_type;

  //! Internal label flags, used by \ref LabelEntry::ExtraData. \ref Section doesn't use these flags and sets them
  //! to zero.
  LabelFlags _internal_label_flags;

  //! Internal data used freely by \ref Section and \ref LabelEntry::ExtraData.
  uint16_t _internal_uint16_data;

  //! \}
};

//! Section entry.
class Section : public SectionOrLabelEntryExtraHeader {
public:
  //! \name Members
  //! \{

  //! Section alignment requirements (0 if no requirements).
  uint32_t _alignment;
  //! Order (lower value means higher priority).
  int32_t _order;
  //! Offset of this section from base-address.
  uint64_t _offset;
  //! Virtual size of the section (zero initialized sections).
  uint64_t _virtual_size;
  //! Section name (max 35 characters, PE allows max 8).
  FixedString<Globals::kMaxSectionNameSize + 1> _name;
  //! Code or data buffer.
  CodeBuffer _buffer;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _section_id; }

  //! Returns the section name, as a null terminated string.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return _name.str; }

  //! Returns the section data.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* data() noexcept { return _buffer.data(); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const uint8_t* data() const noexcept { return _buffer.data(); }

  //! Returns the section flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SectionFlags flags() const noexcept { return SectionFlags(_internal_uint16_data); }

  //! Tests whether the section has the given `flag`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_flag(SectionFlags flag) const noexcept { return Support::test(_internal_uint16_data, uint32_t(flag)); }

  //! Assigns `flags` to the section (replaces all existing flags).
  ASMJIT_INLINE_NODEBUG void assign_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(flags); }

  //! Adds `flags` to the section flags.
  ASMJIT_INLINE_NODEBUG void add_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(_internal_uint16_data | uint32_t(flags)); }

  //! Removes `flags` from the section flags.
  ASMJIT_INLINE_NODEBUG void clear_flags(SectionFlags flags) noexcept { _internal_uint16_data = uint16_t(_internal_uint16_data | ~uint32_t(flags)); }

  //! Returns the minimum section alignment
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t alignment() const noexcept { return _alignment; }

  //! Sets the minimum section alignment
  ASMJIT_INLINE_NODEBUG void set_alignment(uint32_t alignment) noexcept { _alignment = alignment; }

  //! Returns the section order, which has a higher priority than section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG int32_t order() const noexcept { return _order; }

  //! Returns the section offset, relative to base.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t offset() const noexcept { return _offset; }

  //! Set the section offset.
  ASMJIT_INLINE_NODEBUG void set_offset(uint64_t offset) noexcept { _offset = offset; }

  //! Returns the virtual size of the section.
  //!
  //! Virtual size is initially zero and is never changed by AsmJit. It's normal if virtual size is smaller than
  //! size returned by `buffer_size()` as the buffer stores real data emitted by assemblers or appended by users.
  //!
  //! Use `real_size()` to get the real and final size of this section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t virtual_size() const noexcept { return _virtual_size; }

  //! Sets the virtual size of the section.
  ASMJIT_INLINE_NODEBUG void set_virtual_size(uint64_t virtual_size) noexcept { _virtual_size = virtual_size; }

  //! Returns the buffer size of the section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t buffer_size() const noexcept { return _buffer.size(); }

  //! Returns the real size of the section calculated from virtual and buffer sizes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t real_size() const noexcept { return Support::max<uint64_t>(virtual_size(), buffer_size()); }

  //! Returns the `CodeBuffer` used by this section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeBuffer& buffer() noexcept { return _buffer; }

  //! Returns the `CodeBuffer` used by this section (const).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CodeBuffer& buffer() const noexcept { return _buffer; }

  //! \}
};

//! Entry in an address table.
class AddressTableEntry : public ArenaTreeNodeT<AddressTableEntry> {
public:
  ASMJIT_NONCOPYABLE(AddressTableEntry)

  //! \name Members
  //! \{

  //! Address.
  uint64_t _address;
  //! Slot.
  uint32_t _slot;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG explicit AddressTableEntry(uint64_t address) noexcept
    : _address(address),
      _slot(0xFFFFFFFFu) {}

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t address() const noexcept { return _address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t slot() const noexcept { return _slot; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_assigned_slot() const noexcept { return _slot != 0xFFFFFFFFu; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(const AddressTableEntry& other) const noexcept { return _address < other._address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(const AddressTableEntry& other) const noexcept { return _address > other._address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator<(uint64_t query_address) const noexcept { return _address < query_address; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator>(uint64_t query_address) const noexcept { return _address > query_address; }

  //! \}
};

//! Relocation entry.
struct RelocEntry {
  //! \name Members
  //! \{

  //! Relocation id.
  uint32_t _id;
  //! Type of the relocation.
  RelocType _reloc_type;
  //! Format of the relocated value.
  OffsetFormat _format;
  //! Source section id.
  uint32_t _source_section_id;
  //! Target section id.
  uint32_t _target_section_id;
  //! Source offset (relative to start of the section).
  uint64_t _source_offset;
  //! Payload (target offset, target address, expression, etc).
  uint64_t _payload;

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t id() const noexcept { return _id; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocType reloc_type() const noexcept { return _reloc_type; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OffsetFormat& format() const noexcept { return _format; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t source_section_id() const noexcept { return _source_section_id; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t target_section_id() const noexcept { return _target_section_id; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t source_offset() const noexcept { return _source_offset; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t payload() const noexcept { return _payload; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Expression* payload_as_expression() const noexcept {
    return reinterpret_cast<Expression*>(uintptr_t(_payload));
  }

  //! \}
};

//! Label entry provides data stored by \ref CodeHolder for each \ref Label.
//!
//! Label entry is used mostly internall by AsmJit, but it's possibly to use it to query various information about
//! a label. For example to get its type, flags, name, and fixups (if the label is not bound) or offset (if the label
//! is bound).
//!
//! To make the entry small, it's currently split into two data structures - \ref LabelEntry, which is stored in an
//! array as a value, and \ref LabelEntry::ExtraData, which can be pointed to via \ref LabelEntry::_object_data. Extra
//! data of unnamed anonymous labels is shared (and immutable), thus all unnamed anonymous labels would only use
//! \ref LabelEntry (16 bytes per label).
class LabelEntry {
public:
  //! Contains extra data that is only created when the label is not anonymous or has a name.
  struct ExtraData : public SectionOrLabelEntryExtraHeader {
    //! Label parent id or zero.
    uint32_t _parent_id;
    //! Label name length.
    uint32_t _name_size;

    //! Returns a name associated with this extra data - a valid pointer is only returned when the label has a name, which
    //! is marked by \ref LabelFlags::kHasName flag.
    ASMJIT_INLINE_NODEBUG const char* name() const noexcept { return Support::offset_ptr<char>(this, sizeof(ExtraData)); }
  };

  //! \name Members
  //! \{

  //! Either references a \ref Section where the label is bound or \ref ExtraData.
  SectionOrLabelEntryExtraHeader* _object_data;

  //! Label entry payload.
  //!
  //! When a Label is bound, `_offset_or_fixups` is the relative offset from the start of the section where
  //! the \ref Label has been bound, otherwise `_offset_or_fixups` is a pointer to the first \ref Fixup.
  uint64_t _offset_or_fixups;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the label.
  //!
  //! The type of the label depends on how it was created. Most JIT code uses unnamed anonymous labels created by
  //! emitters, for example \ref BaseEmitter::new_label() returns a \ref Label instance having id that was created
  //! by \ref CodeHolder::new_label_id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelType label_type() const noexcept { return _object_data->_internal_label_type; }

  //! Returns label flags.
  //!
  //! \note Label flags are mostly for internal use, there is probably no reason to use them in user code.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelFlags label_flags() const noexcept { return _object_data->_internal_label_flags; }

  //! Tests whether the label has the given `flag` set.
  //!
  //! \note Using other getters instead is advised, for example using \ref has_name() and \ref has_parent() is better
  //! (and shorter) than checking label flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label_flag(LabelFlags flag) const noexcept { return Support::test(_object_data->_internal_label_flags, flag); }

  //! Tests whether the LabelEntry has own extra data (see \ref LabelEntry::ExtraData).
  //!
  //! \note This should only be used by AsmJit for internal purposes. Own extra data means that the LabelEntry has
  //! a mutable extra data separately allocated. This information should not be necessary to users as LabelEntry
  //! getters should encapsulate label introspection.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool _has_own_extra_data() const noexcept { return has_label_flag(LabelFlags::kHasOwnExtraData); }

  //! Tests whether the Label represented by this LabelEntry has a name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_name() const noexcept { return has_label_flag(LabelFlags::kHasName); }

  //! Tests whether the Label represented by this LabelEntry has a parent label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_parent() const noexcept { return has_label_flag(LabelFlags::kHasParent); }

  //! Tests whether the label represented by this LabelEntry is bound.
  //!
  //! Bound label means that it has an associated \ref Section and a position in such section. Labels are bound by
  //! calling \ref BaseEmitter::bind() method with \ref Label operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound() const noexcept { return _object_data->_section_id != Globals::kInvalidId; }

  //! Tests whether the label is bound to a the given `section`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound_to(const Section* section) const noexcept { return _object_data->_section_id == section->section_id(); }

  //! Tests whether the label is bound to a the given `section_id`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_bound_to(uint32_t section_id) const noexcept { return _object_data->_section_id == section_id; }

  //! Returns the section where the label was bound.
  //!
  //! If the label was not yet bound the return value is `nullptr`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t section_id() const noexcept { return _object_data->_section_id; }

  [[nodiscard]]
  ASMJIT_INLINE ExtraData* _own_extra_data() const noexcept {
    ASMJIT_ASSERT(_has_own_extra_data());
    return static_cast<ExtraData*>(_object_data);
  }

  //! Returns label's parent id or \ref Globals::kInvalidId if the label has no parent.
  [[nodiscard]]
  ASMJIT_INLINE uint32_t parent_id() const noexcept {
    return _has_own_extra_data() ? _own_extra_data()->_parent_id : Globals::kInvalidId;
  }

  //! Returns the label's name.
  //!
  //! \note Local labels will return their local name without their parent part, for example ".L1".
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* name() const noexcept {
    return has_name() ? _own_extra_data()->name() : nullptr;
  }

  //! Returns size of label's name.
  //!
  //! \note Label name is always null terminated, so you can use `strlen()` to get it, however, it's also cached in
  //! `LabelEntry` itself, so if you want to know the size the fastest way is to call `LabelEntry::name_size()`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t name_size() const noexcept {
    return has_name() ? _own_extra_data()->_name_size : uint32_t(0);
  }

  //! Returns unresolved fixups associated with this label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_fixups() const noexcept {
    return Support::bool_and(!is_bound(), _offset_or_fixups != 0u);
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* _get_fixups() const noexcept { return reinterpret_cast<Fixup*>(uintptr_t(_offset_or_fixups)); }

  ASMJIT_INLINE_NODEBUG void _set_fixups(Fixup* first) noexcept { _offset_or_fixups = reinterpret_cast<uintptr_t>(first); }

  //! Returns unresolved fixups associated with this label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Fixup* unresolved_fixups() const noexcept { return !is_bound() ? _get_fixups() : nullptr; }

  //! Returns the label offset (can only be used after the label is bound).
  //!
  //! \note This would trigger an assertion failure in debug builds when called on an unbound label. When accessing
  //! offsets, always check whether the label is bound. Unbound labels don't have offsets.
  [[nodiscard]]
  ASMJIT_INLINE uint64_t offset() const noexcept {
    ASMJIT_ASSERT(is_bound());
    return _offset_or_fixups;
  }

  //! \}
};

//! Holds assembled code and data (including sections, labels, and relocation information).
//!
//! CodeHolder connects emitters with their targets. It provides them interface that can be used to query information
//! about the target environment (architecture, etc...) and API to create labels, sections, relocations, and to write
//! data to a \ref CodeBuffer, which is always part of \ref Section. More than one emitter can be attached to a single
//! CodeHolder instance at a time, which is used in practice
//!
//! CodeHolder provides interface for all emitter types. Assemblers use CodeHolder to write into \ref CodeBuffer, and
//! higher level emitters like Builder and Compiler use CodeHolder to manage labels and sections so higher level code
//! can be serialized to Assembler by \ref BaseEmitter::finalize() and \ref BaseBuilder::serialize_to().
//!
//! In order to use CodeHolder, it must be first initialized by \ref init(). After the CodeHolder has been successfully
//! initialized it can be used to hold assembled code, sections, labels, relocations, and to attach / detach code
//! emitters. After the end of code generation it can be used to query physical locations of labels and to relocate
//! the assembled code into the right address. Please not that calling \ref init() twice doesn't work and would return
//! an error - to reuse CodeHolder it has to be first \ref reset() or reinitialized by calling \ref reinit().
//!
//! Multiple Functions
//! ------------------
//!
//! CodeHolder can be used to hold a single function or multiple functions - when it's holding multiple functions it's
//! considered like a module (or library, or something that provides more than just a single function). When a code is
//! relocated and moved into executable memory, you typically get a single pointer back. When CodeHolder holds a single
//! function, it's the pointer to such function. However, when CodeHolder holds multiple functions, that pointer is
//! basically start of the code, which is usually the first function.
//!
//! In order to get a pointer to more functions, it's necessary to use \ref Label for each function and then to get the
//! offset to each such function via \ref CodeHolder::label_offset_from_base() - which returns an offset, which is relative
//! to the start of the assembled code. When using higher level emitters such as \ref asmjit_compiler labels are created
//! automatically - \ref FuncNode inherits from \ref LabelNode, so a function is a label at the same time.
//!
//! To query and apply an offset, consider the following code, which uses \ref x86::Compiler to create two functions:
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//! #include <string.h>
//!
//! int main(int argc, char* argv[]) {
//!   using namespace asmjit;
//!
//!   JitRuntime rt;
//!   CodeHolder code;
//!   code.init(rt.environment());
//!
//!   x86::Compiler cc(&code);
//!
//!   // Generate first function.
//!   FuncNode* func1_node = cc.add_func(FuncSignature::build<uint32_t>());
//!   Label func1_label = func1_node->label();
//!
//!   {
//!     x86::Gp r = cc.new_gp32("r0");
//!     cc.mov(r, 0);
//!     cc.ret(r);
//!     cc.end_func();
//!   }
//!
//!   // Generate second function.
//!   FuncNode* func2_node = cc.add_func(FuncSignature::build<uint32_t>());
//!   Label func2_label = func2_node->label();
//!
//!   {
//!     x86::Gp r = cc.new_gp32("r1");
//!     cc.mov(r, 1);
//!     cc.ret(r);
//!     cc.end_func();
//!   }
//!
//!   // Finalize the generated code - this would also call `serialize_to()`.
//!   Error err = cc.finalize();
//!   if (err != Error::kOk) {
//!     printf("ERROR during finalization: %s\n", DebugUtils::error_as_string(err));
//!     return 1;
//!   }
//!
//!   // We have deliberately used void* as a pointer type as it's start of an assembled module.
//!   void* module;
//!   err = rt.add(&module, &code);
//!
//!   if (err != Error::kOk) {
//!     printf("ERROR during allocation/relocation: %s\n", DebugUtils::error_as_string(err));
//!     return 1;
//!   }
//!
//!   // Normally both CodeHolder and Compiler are not needed after the code has been finalized
//!   // and allocated/relocated into an executable memory. However, in order to get the required
//!   // offsets it's necessary to query CodeHolder for positions in code, and to get these it's
//!   // required to either have `FuncNode` or `Label`.
//!   size_t func1_offset = code.label_offset_from_base(func1_label);
//!   size_t func2_offset = code.label_offset_from_base(func2_label);
//!
//!   using Fn = uint32_t(*)(void);
//!
//!   Fn fn1 = ptr_as_func<Fn>(module, func1_offset);
//!   Fn fn2 = ptr_as_func<Fn>(module, func2_offset);
//!
//!   printf("fn1()=%u fn2()=%u\n", fn1(), fn2());
//!
//!   // The module has to be released at once - individual functions cannot be released.
//!   rt.release(module);
//!
//!   return 0;
//! }
//! ```
//!
//! CodeHolder Reusability
//! ----------------------
//!
//! If you intend to generate a lot of code, or tiny code, it's advised to reuse CodeHolder and emitter instances.
//! There are currently two ways of reusing CodeHolder and emitters - one is using \ref CodeHolder::init() followed
//! by \ref CodeHolder::reset(), and another is initializing once by \ref CodeHolder::init() and then reinitializing
//! by \ref CodeHolder::reinit(). The first strategy is shown below:
//!
//! ```
//! // All of them will be reused for code generation by using an 'init()/reset()' strategy.
//! Environment env = ...; // Environment to use, for example from JitRuntime.
//! CodeHolder code;       // CodeHolder to reuse (all allocated memory will be held by it until it's destroyed).
//! x86::Compiler cc;      // Emitter to reuse (for example x86::Compiler).
//!
//! for (size_t i = 0; i < ...; i++) {
//!   // Initialize the CodeHolder first.
//!   code.init(env);
//!   code.attach(&emitter);
//!
//!   [[code generation as usual]]
//!
//!   code.reset();
//! }
//! ```
//!
//! While this approach is good for many use-cases, there is even a faster strategy called reinitialization, which is
//! provided by \ref CodeHolder::reinit(). The idea of reinit is to reinitialize the CodeHolder into a state, which
//! was achieved by initializing it by \ref CodeHolder::init(), by optionally attaching \ref Logger, \ref ErrorHandler,
//! and emitters of any kind. See an example below:
//!
//! ```
//! // All of them will be reused for code generation by using a 'reinit()' strategy.
//! Environment env = ...; // Environment to use, for example from JitRuntime.
//! CodeHolder code;       // CodeHolder to reuse (all allocated memory will be held by it until it's destroyed).
//! x86::Compiler cc;      // Emitter to reuse (for example x86::Compiler).
//!
//! // Initialize the CodeHolder and attach emitters to it (attaching ErrorHandler is advised!)
//! code.init(env);
//! code.attach(&emitter);
//!
//! for (size_t i = 0; i < ...; i++) {
//!   [[code generation as usual]]
//!
//!   // Optionally you can start the loop with 'code.reinit()', but this is cleaner as it wipes out all intermediate
//!   // states of CodeHolder and the attached emitters. It won't detach Logger, ErrorHandler, nor attached emitters.
//!   code.reinit();
//! }
//! ```
//!
//! \note \ref CodeHolder has an ability to attach an \ref ErrorHandler, however, the error handler is not triggered
//! by \ref CodeHolder itself, it's instead propagated to all emitters that attach to it.
class CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)

  //! \name Types
  //! \{

  //! \cond INTERNAL
  struct NamedLabelExtraData : public ArenaHashNode {
    LabelEntry::ExtraData extra_data;

    ASMJIT_INLINE_NODEBUG uint32_t label_id() const noexcept { return _custom_data; }
  };
  //! \endcond

  //! An informative data structure that is filled with some details that happened during \ref relocate_to_base().
  struct RelocationSummary {
    //! The number of bytes the final code has been reduced by.
    //!
    //! At the moment this is the same as the number of bytes that the address table was shrunk, because it was
    //! possible to avoid certain entries during relocation - the functions that would be otherwise present were
    //! close enough to avoid them in the .addrtab section.
    size_t code_size_reduction;
  };

  //! \}

  //! \name Members
  //! \{

  //! Environment information.
  Environment _environment;
  //! CPU features of the target architecture.
  CpuFeatures _cpu_features;
  //! Base address or \ref Globals::kNoBaseAddress.
  uint64_t _base_address;

  //! Attached `Logger`, used by all consumers.
  Logger* _logger;
  //! Attached `ErrorHandler`.
  ErrorHandler* _error_handler;

  //! Arena allocator used to allocate core structures.
  Arena _arena;

  //! First emitter attached to this CodeHolder (double-linked list).
  BaseEmitter* _attached_first;
  //! Last emitter attached to this CodeHolder (double-linked list).
  BaseEmitter* _attached_last;

  //! Section entries.
  ArenaVector<Section*> _sections;
  //! Section entries sorted by section order and then section id.
  ArenaVector<Section*> _sections_by_order;

  //! Label entries.
  ArenaVector<LabelEntry> _label_entries;
  //! Relocation entries.
  ArenaVector<RelocEntry*> _relocations;
  //! Label name -> LabelEntry::ExtraData (only used by labels that have a name and are not anonymous).
  ArenaHash<NamedLabelExtraData> _named_labels;
  //! Unresolved fixups that are most likely references across sections.
  Fixup* _fixups;
  //! Pool containing \ref Fixup instances for quickly recycling them.
  ArenaPool<Fixup> _fixup_data_pool;
  //! Count of unresolved fixups of unbound labels (at the end of assembling this should be zero).
  size_t _unresolved_fixup_count;

  //! Text section - always one part of a CodeHolder itself.
  Section _text_section;

  //! Pointer to an address table section (or null if this section doesn't exist).
  Section* _address_table_section;
  //! Address table entries.
  ArenaTree<AddressTableEntry> _address_table_entries;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates an uninitialized CodeHolder (you must init() it before it can be used).
  //!
  //! An optional `temporary` argument can be used to initialize the first block of \ref Arena
  //! that \ref CodeHolder uses into a temporary memory provided by the user.
  ASMJIT_API explicit CodeHolder(Span<uint8_t> static_arena_memory = Span<uint8_t>{}) noexcept;

  //! Destroys the CodeHolder and frees all resources it has allocated.
  ASMJIT_API ~CodeHolder() noexcept;

  //! Tests whether the `CodeHolder` has been initialized.
  //!
  //! Emitters can be only attached to initialized `CodeHolder` instances.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _environment.is_initialized(); }

  //! Initializes CodeHolder to hold code described by the given `environment` and `base_address`.
  ASMJIT_API Error init(const Environment& environment, uint64_t base_address = Globals::kNoBaseAddress) noexcept;
  //! Initializes CodeHolder to hold code described by the given `environment`, `cpu_features`, and `base_address`.
  ASMJIT_API Error init(const Environment& environment, const CpuFeatures& cpu_features, uint64_t base_address = Globals::kNoBaseAddress) noexcept;

  //! Reinitializes CodeHolder with the same environment, cpu features, and base address as it had, and notifies
  //! all attached emitters of reinitialization. If the \ref CodeHolder was not initialized, \ref Error::kNotInitialized
  //! is returned.
  //!
  //! Reinitialization is designed to be a faster alternative compared to \ref reset() followed by \ref init() chain.
  //! The purpose of reinitialization is a very quick reuse of \ref CodeHolder and all attached emitters (most likely
  //! Assembler or Compiler) without paying the cost of complete initialization and then assignment of all the loggers,
  //! error handlers, and emitters.
  //!
  //! \note Semantically reinit() is the same as using \ref reset() with \ref ResetPolicy::kSoft parameter followed by
  //! \ref init(), and then by attaching loggers, error handlers, and emitters that were attached previously. This
  //! means that after reinitialization you will get a clean and ready for use \ref CodeHolder, which was initialized
  //! the same way as before.
  ASMJIT_API Error reinit() noexcept;

  //! Detaches all code-generators attached and resets the `CodeHolder`.
  ASMJIT_API void reset(ResetPolicy reset_policy = ResetPolicy::kSoft) noexcept;

  //! \}

  //! \name Attach & Detach
  //! \{

  //! Attaches an emitter to this `CodeHolder`.
  ASMJIT_API Error attach(BaseEmitter* emitter) noexcept;
  //! Detaches an emitter from this `CodeHolder`.
  ASMJIT_API Error detach(BaseEmitter* emitter) noexcept;

  //! \}

  //! \name Memory Allocators
  //! \{

  //! Returns the allocator that the `CodeHolder` uses.
  //!
  //! \note This should be only used for AsmJit's purposes. Code holder uses arena allocator to allocate everything,
  //! so anything allocated through this allocator will be invalidated by \ref CodeHolder::reset() or by CodeHolder's
  //! destructor.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arena& arena() const noexcept { return const_cast<Arena&>(_arena); }

  //! \}

  //! \name Code & Architecture
  //! \{

  //! Returns the target environment information.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }

  //! Returns the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return environment().arch(); }

  //! Returns the target sub-architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return environment().sub_arch(); }

  //! Returns the minimum CPU features of the target architecture.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const CpuFeatures& cpu_features() const noexcept { return _cpu_features; }

  //! Tests whether a static base-address is set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_base_address() const noexcept { return _base_address != Globals::kNoBaseAddress; }

  //! Returns a static base-address or \ref Globals::kNoBaseAddress, if not set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t base_address() const noexcept { return _base_address; }

  //! \}

  //! \name Attached Emitters
  //! \{

  //! Returns a vector of attached emitters.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attached_first() noexcept { return _attached_first; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseEmitter* attached_last() noexcept { return _attached_last; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attached_first() const noexcept { return _attached_first; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BaseEmitter* attached_last() const noexcept { return _attached_last; }

  //! \}

  //! \name Logging
  //! \{

  //! Returns the attached logger.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }

  //! Attaches a `logger` to CodeHolder and propagates it to all attached emitters.
  ASMJIT_API void set_logger(Logger* logger) noexcept;

  //! Resets the logger to none.
  ASMJIT_INLINE_NODEBUG void reset_logger() noexcept { set_logger(nullptr); }

  //! \name Error Handling
  //! \{

  //! Tests whether the CodeHolder has an attached error handler, see \ref ErrorHandler.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_error_handler() const noexcept { return _error_handler != nullptr; }

  //! Returns the attached error handler.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ErrorHandler* error_handler() const noexcept { return _error_handler; }

  //! Attach an error handler to this `CodeHolder`.
  ASMJIT_API void set_error_handler(ErrorHandler* error_handler) noexcept;

  //! Resets the error handler to none.
  ASMJIT_INLINE_NODEBUG void reset_error_handler() noexcept { set_error_handler(nullptr); }

  //! \}

  //! \name Code Buffer
  //! \{

  //! Makes sure that at least `n` bytes can be added to CodeHolder's buffer `cb`.
  //!
  //! \note The buffer `cb` must be managed by `CodeHolder` - otherwise the behavior of the function is undefined.
  ASMJIT_API Error grow_buffer(CodeBuffer* cb, size_t n) noexcept;

  //! Reserves the size of `cb` to at least `n` bytes.
  //!
  //! \note The buffer `cb` must be managed by `CodeHolder` - otherwise the behavior of the function is undefined.
  ASMJIT_API Error reserve_buffer(CodeBuffer* cb, size_t n) noexcept;

  //! \}

  //! \name Sections
  //! \{

  //! Returns an array of `Section*` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Section*> sections() const noexcept { return _sections.as_span(); }

  //! Returns an array of `Section*` records sorted according to section order first, then section id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<Section*> sections_by_order() const noexcept { return _sections_by_order.as_span(); }

  //! Returns the number of sections.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t section_count() const noexcept { return _sections.size(); }

  //! Tests whether the given `section_id` is valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_section_valid(uint32_t section_id) const noexcept { return section_id < _sections.size(); }

  //! Creates a new section and return its pointer in `section_out`.
  //!
  //! Returns `Error`, does not report a possible error to `ErrorHandler`.
  ASMJIT_API Error new_section(Out<Section*> section_out, const char* name, size_t name_size = SIZE_MAX, SectionFlags flags = SectionFlags::kNone, uint32_t alignment = 1, int32_t order = 0) noexcept;

  //! Returns a section entry of the given index.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* section_by_id(uint32_t section_id) const noexcept { return _sections[section_id]; }

  //! Returns section-id that matches the given `name`.
  //!
  //! If there is no such section `Section::kInvalidId` is returned.
  [[nodiscard]]
  ASMJIT_API Section* section_by_name(const char* name, size_t name_size = SIZE_MAX) const noexcept;

  //! Returns '.text' section (section that commonly represents code).
  //!
  //! \note Text section is always the first section in \ref CodeHolder::sections() array.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* text_section() const noexcept { return _sections[0]; }

  //! Tests whether '.addrtab' section exists.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_address_table_section() const noexcept { return _address_table_section != nullptr; }

  //! Returns '.addrtab' section.
  //!
  //! This section is used exclusively by AsmJit to store absolute 64-bit
  //! addresses that cannot be encoded in instructions like 'jmp' or 'call'.
  //!
  //! \note This section is created on demand, the returned pointer can be null.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* address_table_section() const noexcept { return _address_table_section; }

  //! Ensures that '.addrtab' section exists (creates it if it doesn't) and
  //! returns it. Can return `nullptr` on out of memory condition.
  [[nodiscard]]
  ASMJIT_API Section* ensure_address_table_section() noexcept;

  //! Used to add an address to an address table.
  //!
  //! This implicitly calls `ensure_address_table_section()` and then creates `AddressTableEntry` that is inserted
  //! to `_address_table_entries`. If the address already exists this operation does nothing as the same addresses
  //! use the same slot.
  //!
  //! This function should be considered internal as it's used by assemblers to insert an absolute address into the
  //! address table. Inserting address into address table without creating a particular relocation entry makes no sense.
  ASMJIT_API Error add_address_to_address_table(uint64_t address) noexcept;

  //! \}

  //! \name Labels & Symbols
  //! \{

  //! Returns array of `LabelEntry` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<LabelEntry> label_entries() const noexcept { return _label_entries.as_span(); }

  //! Returns number of labels created.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t label_count() const noexcept { return _label_entries.size(); }

  //! Tests whether the label having `label_id` is valid (i.e. created by `new_label_id()`).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(uint32_t label_id) const noexcept {
    return label_id < _label_entries.size();
  }

  //! Tests whether the `label` is valid (i.e. created by `new_label_id()`).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(const Label& label) const noexcept {
    return is_label_valid(label.id());
  }

  //! Tests whether a label having `label_id` is already bound.
  //!
  //! Returns `false` if the `label_id` is not valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_bound(uint32_t label_id) const noexcept {
    return is_label_valid(label_id) && _label_entries[label_id].is_bound();
  }

  //! Tests whether the `label` is already bound.
  //!
  //! Returns `false` if the `label` is not valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_bound(const Label& label) const noexcept {
    return is_label_bound(label.id());
  }

  //! Returns LabelEntry of the given label identifier `label_id` (or `label` if you are using overloads).
  //!
  //! \attention The passed `label_id` must be valid as it's used as an index to `_label_entries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::is_label_valid() function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& label_entry_of(uint32_t label_id) noexcept {
    return _label_entries[label_id];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& label_entry_of(uint32_t label_id) const noexcept {
    return _label_entries[label_id];
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelEntry& label_entry_of(const Label& label) noexcept {
    return label_entry_of(label.id());
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const LabelEntry& label_entry_of(const Label& label) const noexcept {
    return label_entry_of(label.id());
  }

  //! Returns offset of a `Label` by its `label_id`.
  //!
  //! The offset returned is relative to the start of the section where the label is bound. Zero offset is returned
  //! for unbound labels, which is their initial offset value.
  //!
  //! \attention The passed `label_id` must be valid as it's used as an index to `_label_entries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::is_label_valid() function.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t label_offset(uint32_t label_id) const noexcept {
    ASMJIT_ASSERT(is_label_valid(label_id));
    return _label_entries[label_id].offset();
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint64_t label_offset(const Label& label) const noexcept {
    return label_offset(label.id());
  }

  //! Returns offset of a label by it's `label_id` relative to the base offset.
  //!
  //! \attention The passed `label_id` must be valid as it's used as an index to `_label_entries[]` array. In debug
  //! builds the array access uses an assertion, but such assertion is not present in release builds. To get whether
  //! a label is valid, check out \ref CodeHolder::is_label_valid() function.
  //!
  //! \note The offset of the section where the label is bound must be valid in order to use this function, otherwise
  //! the value returned will not be reliable. Typically, sections have offsets when they are flattened, see \ref
  //! CodeHolder::flatten() function for more details.
  [[nodiscard]]
  inline uint64_t label_offset_from_base(uint32_t label_id) const noexcept {
    ASMJIT_ASSERT(is_label_valid(label_id));

    const LabelEntry& le = _label_entries[label_id];
    return (le.is_bound() ? _sections[le.section_id()]->offset() : uint64_t(0)) + le.offset();
  }

  //! \overload
  [[nodiscard]]
  inline uint64_t label_offset_from_base(const Label& label) const noexcept {
    return label_offset_from_base(label.id());
  }

  //! Creates a new anonymous label and return its id in `label_id_out`.
  //!
  //! Returns `Error`, does not report error to `ErrorHandler`.
  [[nodiscard]]
  ASMJIT_API Error new_label_id(Out<uint32_t> label_id_out) noexcept;

  //! Creates a new named \ref LabelEntry of the given label `type`.
  //!
  //! \param label_id_out Where to store the created \ref Label id.
  //! \param name The name of the label.
  //! \param name_size The length of `name` argument, or `SIZE_MAX` if `name` is a null terminated string, which
  //!        means that the `CodeHolder` will use `strlen()` to determine the length.
  //! \param type The type of the label to create, see \ref LabelType.
  //! \param parent_id Parent id of a local label, otherwise it must be \ref Globals::kInvalidId.
  //! \retval Always returns \ref Error, does not report a possible error to the attached \ref ErrorHandler.
  //!
  //! AsmJit has a support for local labels (\ref LabelType::kLocal) which require a parent label id (parent_id).
  //! The names of local labels can conflict with names of other local labels that have a different parent. In
  //! addition, AsmJit supports named anonymous labels, which are useful only for debugging purposes as the
  //! anonymous name will have a name, which will be formatted, but the label itself cannot be queried by such
  //! name.
  [[nodiscard]]
  ASMJIT_API Error new_named_label_id(Out<uint32_t> label_id_out, const char* name, size_t name_size, LabelType type, uint32_t parent_id = Globals::kInvalidId) noexcept;

  //! Returns a label by name.
  //!
  //! \remarks If the named label doesn't exist a default constructed \ref Label is returned, which has its id set
  //! to \ref Globals::kInvalidId. In other words, this function doesn't create new labels, it can only be used to
  //! query an existing \ref Label by name.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label label_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return Label(label_id_by_name(name, name_size, parent_id));
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_API Label label_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_by_name(name.data(), name.size(), parent_id);
  }

  //! Returns a label id by name.
  //!
  //! \remarks If the named label doesn't exist \ref Globals::kInvalidId is returned. In other words, this function
  //! doesn't create new labels, it can only be used to query an existing label identifier by name.
  [[nodiscard]]
  ASMJIT_API uint32_t label_id_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept;

  //! \overload
  [[nodiscard]]
  ASMJIT_API uint32_t label_id_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_id_by_name(name.data(), name.size(), parent_id);
  }

  //! Tests whether there are any unresolved fixups related to unbound labels.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_unresolved_fixups() const noexcept { return _unresolved_fixup_count != 0u; }

  //! Returns the number of unresolved fixups.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t unresolved_fixup_count() const noexcept { return _unresolved_fixup_count; }

  //! Creates a new label-link used to store information about yet unbound labels.
  //!
  //! Returns `null` if the allocation failed.
  [[nodiscard]]
  ASMJIT_API Fixup* new_fixup(LabelEntry& le, uint32_t section_id, size_t offset, intptr_t rel, const OffsetFormat& format) noexcept;

  //! Resolves cross-section fixups associated with each label that was used as a destination in code of a different
  //! section. It's only useful to people that use multiple sections as it will do nothing if the code only contains
  //! a single section in which cross-section fixups are not possible.
  ASMJIT_API Error resolve_cross_section_fixups() noexcept;

  //! Binds a label to a given `section_id` and `offset` (relative to start of the section).
  //!
  //! This function is generally used by `BaseAssembler::bind()` to do the heavy lifting.
  ASMJIT_API Error bind_label(const Label& label, uint32_t section_id, uint64_t offset) noexcept;

  //! \}

  //! \name Relocations
  //! \{

  //! Tests whether the code contains relocation entries.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_reloc_entries() const noexcept { return !_relocations.is_empty(); }

  //! Returns array of `RelocEntry*` records.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<RelocEntry*> reloc_entries() const noexcept { return _relocations.as_span(); }

  //! Returns a RelocEntry of the given `id`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG RelocEntry* reloc_entry_of(uint32_t id) const noexcept { return _relocations[id]; }

  //! Creates a new relocation entry of type `reloc_type`.
  //!
  //! Additional fields can be set after the relocation entry was created.
  [[nodiscard]]
  ASMJIT_API Error new_reloc_entry(Out<RelocEntry*> dst, RelocType reloc_type) noexcept;

  //! \}

  //! \name Utilities
  //! \{

  //! Flattens all sections by recalculating their offsets, starting at 0.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error flatten() noexcept;

  //! Returns computed the size of code & data of all sections.
  //!
  //! \note All sections will be iterated over and the code size returned would represent the minimum code size of
  //! all combined sections after applying minimum alignment. Code size may decrease after calling `flatten()` and
  //! `relocate_to_base()`.
  [[nodiscard]]
  ASMJIT_API size_t code_size() const noexcept;

  //! Relocates the code to the given `base_address`.
  //!
  //! \param base_address Absolute base address where the code will be relocated to. Please note that nothing is
  //! copied to such base address, it's just an absolute value used by the relocation code to resolve all stored
  //! relocations.
  //!
  //! \param summary_out Optional argument that can be used to get back information about the relocation.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error relocate_to_base(uint64_t base_address, RelocationSummary* summary_out = nullptr) noexcept;

  //! Copies a single section into `dst`.
  ASMJIT_API Error copy_section_data(void* dst, size_t dst_size, uint32_t section_id, CopySectionFlags copy_flags = CopySectionFlags::kNone) noexcept;

  //! Copies all sections into `dst`.
  //!
  //! This should only be used if the data was flattened and there are no gaps between the sections. The `dst_size`
  //! is always checked and the copy will never write anything outside the provided buffer.
  ASMJIT_API Error copy_flattened_data(void* dst, size_t dst_size, CopySectionFlags copy_flags = CopySectionFlags::kNone) noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CODEHOLDER_H_INCLUDED
