// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ASSEMBLER_H_INCLUDED
#define ASMJIT_CORE_ASSEMBLER_H_INCLUDED

#include <asmjit/core/codeholder.h>
#include <asmjit/core/emitter.h>
#include <asmjit/core/operand.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_assembler
//! \{

//! Base assembler.
//!
//! This is a base class that provides interface used by architecture specific assembler implementations. Assembler
//! doesn't hold any data, instead it's attached to \ref CodeHolder, which provides all the data that Assembler needs
//! and which can be altered by it.
//!
//! Check out architecture specific assemblers for more details and examples:
//!
//!   - \ref x86::Assembler - X86/X64 assembler implementation.
//!   - \ref a64::Assembler - AArch64 assembler implementation.
class ASMJIT_VIRTAPI BaseAssembler : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseAssembler)
  using Base = BaseEmitter;

  //! \name Members
  //! \{

  //! Current section where the assembling happens.
  Section* _section = nullptr;
  //! Start of the CodeBuffer of the current section.
  uint8_t* _buffer_data = nullptr;
  //! End (first invalid byte) of the current section.
  uint8_t* _buffer_end = nullptr;
  //! Pointer in the CodeBuffer of the current section.
  uint8_t* _buffer_ptr = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `BaseAssembler` instance.
  ASMJIT_API BaseAssembler() noexcept;
  //! Destroys the `BaseAssembler` instance.
  ASMJIT_API ~BaseAssembler() noexcept override;

  //! \}

  //! \name Code-Buffer Management
  //! \{

  //! Returns the capacity of the current CodeBuffer.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t buffer_capacity() const noexcept { return (size_t)(_buffer_end - _buffer_data); }

  //! Returns the number of remaining bytes in the current CodeBuffer.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remaining_space() const noexcept { return (size_t)(_buffer_end - _buffer_ptr); }

  //! Returns the current position in the CodeBuffer.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t offset() const noexcept { return (size_t)(_buffer_ptr - _buffer_data); }

  //! Sets the current position in the CodeBuffer to `offset`.
  //!
  //! \note The `offset` cannot be greater than buffer size even if it's within the buffer's capacity.
  ASMJIT_API Error set_offset(size_t offset);

  //! Returns the start of the CodeBuffer in the current section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_data() const noexcept { return _buffer_data; }

  //! Returns the end (first invalid byte) in the current section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_end() const noexcept { return _buffer_end; }

  //! Returns the current pointer in the CodeBuffer in the current section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t* buffer_ptr() const noexcept { return _buffer_ptr; }

  //! \}

  //! \name Section Management
  //! \{

  //! Returns the current section.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Section* current_section() const noexcept { return _section; }

  ASMJIT_API Error section(Section* section) override;

  //! \}

  //! \name Label Management
  //! \{

  ASMJIT_API Label new_label() override;
  ASMJIT_API Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) override;

  ASMJIT_API Error bind(const Label& label) override;

  //! \}

  //! \name Embed
  //! \{

  ASMJIT_API Error embed(const void* data, size_t data_size) override;
  ASMJIT_API Error embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1) override;
  ASMJIT_API Error embed_const_pool(const Label& label, const ConstPool& pool) override;

  ASMJIT_API Error embed_label(const Label& label, size_t data_size = 0) override;
  ASMJIT_API Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0) override;

  //! \}

  //! \name Comment
  //! \{

  ASMJIT_API Error comment(const char* data, size_t size = SIZE_MAX) override;

  ASMJIT_INLINE Error comment(Span<const char> data) { return comment(data.data(), data.size()); }

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ASSEMBLER_H_INCLUDED
