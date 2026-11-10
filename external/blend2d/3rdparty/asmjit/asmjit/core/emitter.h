// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_EMITTER_H_INCLUDED
#define ASMJIT_CORE_EMITTER_H_INCLUDED

#include <asmjit/core/archtraits.h>
#include <asmjit/core/codeholder.h>
#include <asmjit/core/formatter.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/core/type.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

class ConstPool;
class FuncFrame;
class FuncArgsAssignment;

//! Align mode, used by \ref BaseEmitter::align().
enum class AlignMode : uint8_t {
  //! Align executable code.
  kCode = 0,
  //! Align non-executable code.
  kData = 1,
  //! Align by a sequence of zeros.
  kZero = 2,

  //! Maximum value of `AlignMode`.
  kMaxValue = kZero
};

//! Emitter type used by \ref BaseEmitter.
enum class EmitterType : uint8_t {
  //! Unknown or uninitialized.
  kNone = 0,
  //! Emitter inherits from \ref BaseAssembler.
  kAssembler = 1,
  //! Emitter inherits from \ref BaseBuilder.
  kBuilder = 2,
  //! Emitter inherits from \ref BaseCompiler.
  kCompiler = 3,

  //! Maximum value of `EmitterType`.
  kMaxValue = kCompiler
};

//! Emitter flags, used by \ref BaseEmitter.
enum class EmitterFlags : uint8_t {
  //! No flags.
  kNone = 0u,
  //! Emitter is attached to CodeHolder.
  kAttached = 0x01u,
  //! The emitter must emit comments.
  kLogComments = 0x08u,
  //! The emitter has its own \ref Logger (not propagated from \ref CodeHolder).
  kOwnLogger = 0x10u,
  //! The emitter has its own \ref ErrorHandler (not propagated from \ref CodeHolder).
  kOwnErrorHandler = 0x20u,
  //! The emitter was finalized.
  kFinalized = 0x40u,
  //! The emitter was destroyed.
  //!
  //! This flag is used for a very short time when an emitter is being destroyed by
  //! CodeHolder.
  kDestroyed = 0x80u
};
ASMJIT_DEFINE_ENUM_FLAGS(EmitterFlags)

//! Encoding options.
enum class EncodingOptions : uint32_t {
  //! No encoding options.
  kNone = 0,

  //! Emit instructions that are optimized for size, if possible.
  //!
  //! Default: false.
  //!
  //! X86 Specific
  //! ------------
  //!
  //! When this option is set it the assembler will try to fix instructions if possible into operation equivalent
  //! instructions that take less bytes by taking advantage of implicit zero extension. For example instruction
  //! like `mov r64, imm` and `and r64, imm` can be translated to `mov r32, imm` and `and r32, imm` when the
  //! immediate constant is lesser than `2^31`.
  kOptimizeForSize = 0x00000001u,

  //! Emit optimized code-alignment sequences.
  //!
  //! Default: false.
  //!
  //! X86 Specific
  //! ------------
  //!
  //! Default align sequence used by X86 architecture is one-byte (0x90) opcode that is often shown by disassemblers
  //! as NOP. However there are more optimized align sequences for 2-11 bytes that may execute faster on certain CPUs.
  //! If this feature is enabled AsmJit will generate specialized sequences for alignment between 2 to 11 bytes.
  kOptimizedAlign = 0x00000002u,

  //! Emit jump-prediction hints.
  //!
  //! Default: false.
  //!
  //! X86 Specific
  //! ------------
  //!
  //! Jump prediction is usually based on the direction of the jump. If the jump is backward it is usually predicted as
  //! taken; and if the jump is forward it is usually predicted as not-taken. The reason is that loops generally use
  //! backward jumps and conditions usually use forward jumps. However this behavior can be overridden by using
  //! instruction prefixes. If this option is enabled these hints will be emitted.
  //!
  //! This feature is disabled by default, because the only processor that used to take into consideration prediction
  //! hints was P4. Newer processors implement heuristics for branch prediction and ignore static hints. This means
  //! that this feature can be only used for annotation purposes.
  kPredictedJumps = 0x00000010u
};
ASMJIT_DEFINE_ENUM_FLAGS(EncodingOptions)

//! Diagnostic options are used to tell emitters and their passes to perform diagnostics when emitting or processing
//! user code. These options control validation and extra diagnostics that can be performed by higher level emitters.
//!
//! Instruction Validation
//! ----------------------
//!
//! \ref BaseAssembler implementation perform by default only basic checks that are necessary to identify all
//! variations of an instruction so the correct encoding can be selected. This is fine for production-ready code
//! as the assembler doesn't have to perform checks that would slow it down. However, sometimes these checks are
//! beneficial especially when the project that uses AsmJit is in a development phase, in which mistakes happen
//! often. To make the experience of using AsmJit seamless it offers validation features that can be controlled
//! by \ref DiagnosticOptions.
//!
//! Compiler Diagnostics
//! --------------------
//!
//! Diagnostic options work with \ref BaseCompiler passes (precisely with its register allocation pass). These options
//! can be used to enable logging of all operations that the Compiler does.
enum class DiagnosticOptions : uint32_t {
  //! No validation options.
  kNone = 0,

  //! Perform strict validation in \ref BaseAssembler::emit() implementations.
  //!
  //! This flag ensures that each instruction is checked before it's encoded into a binary representation. This flag
  //! is only relevant for \ref BaseAssembler implementations, but can be set in any other emitter type, in that case
  //! if that emitter needs to create an assembler on its own, for the purpose of \ref BaseEmitter::finalize() it
  //! would propagate this flag to such assembler so all instructions passed to it are explicitly validated.
  //!
  //! Default: false.
  kValidateAssembler = 0x00000001u,

  //! Perform strict validation in \ref BaseBuilder::emit() and \ref BaseCompiler::emit() implementations.
  //!
  //! This flag ensures that each instruction is checked before an \ref InstNode representing the instruction is
  //! created by \ref BaseBuilder or \ref BaseCompiler. This option could be more useful than \ref kValidateAssembler
  //! in cases in which there is an invalid instruction passed to an assembler, which was invalid much earlier, most
  //! likely when such instruction was passed to Builder/Compiler.
  //!
  //! This is a separate option that was introduced, because it's possible to manipulate the instruction stream
  //! emitted by \ref BaseBuilder and \ref BaseCompiler - this means that it's allowed to emit invalid instructions
  //! (for example with missing operands) that will be fixed later before finalizing it.
  //!
  //! Default: false.
  kValidateIntermediate = 0x00000002u,

  //! Annotate all nodes processed by register allocator (Compiler/RA).
  //!
  //! \note Annotations don't need debug options, however, some debug options like `kRADebugLiveness` may influence
  //! their output (for example the mentioned option would add liveness information to per-instruction annotation).
  kRAAnnotate = 0x00000080u,

  //! Debug CFG generation and other related algorithms / operations (Compiler/RA).
  kRADebugCFG = 0x00000100u,

  //! Debug liveness analysis (Compiler/RA).
  kRADebugLiveness = 0x00000200u,

  //! Debug register allocation assignment (Compiler/RA).
  kRADebugAssignment = 0x00000400u,

  //! Debug the removal of code part of unreachable blocks.
  kRADebugUnreachable = 0x00000800u,

  //! Enable all debug options (Compiler/RA).
  kRADebugAll = 0x0000FF00u
};
ASMJIT_DEFINE_ENUM_FLAGS(DiagnosticOptions)

//! Provides a base foundation to emitting code - specialized by \ref BaseAssembler and \ref BaseBuilder.
class ASMJIT_VIRTAPI BaseEmitter {
public:
  ASMJIT_BASE_CLASS(BaseEmitter)
  ASMJIT_NONCOPYABLE(BaseEmitter)

  //! \name Types
  //! \{

  //! Emitter state that can be used to specify options and inline comment of a next node or instruction.
  struct State {
    InstOptions options;
    RegOnly extra_reg;
    const char* comment;
  };

  //! Functions used by backend-specific emitter implementation.
  //!
  //! These are typically shared between Assembler/Builder/Compiler of a single backend.
  struct Funcs {
    using EmitProlog = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame);
    using EmitEpilog = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame);
    using EmitArgsAssignment = Error (ASMJIT_CDECL*)(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args);

    using FormatInstruction = Error (ASMJIT_CDECL*)(
      String& sb,
      FormatFlags format_flags,
      const BaseEmitter* emitter,
      Arch arch,
      const BaseInst& inst, Span<const Operand_> operands) noexcept;

    using ValidateFunc = Error (ASMJIT_CDECL*)(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept;

    //! Emit prolog implementation.
    EmitProlog emit_prolog;
    //! Emit epilog implementation.
    EmitEpilog emit_epilog;
    //! Emit arguments assignment implementation.
    EmitArgsAssignment emit_args_assignment;
    //! Instruction formatter implementation.
    FormatInstruction format_instruction;
    //! Instruction validation implementation.
    ValidateFunc validate;

    //! Resets all functions to nullptr.
    ASMJIT_INLINE_NODEBUG void reset() noexcept { *this = Funcs{}; }
  };

  //! \}

  //! \name Members
  //! \{

  //! See \ref EmitterType.
  EmitterType _emitter_type = EmitterType::kNone;

  //! See \ref EmitterFlags.
  EmitterFlags _emitter_flags = EmitterFlags::kNone;

  //! Instruction alignment.
  uint8_t _instruction_alignment = 0u;

  //! Validation flags in case validation is used.
  //!
  //! \note Validation flags are specific to the emitter and they are setup at construction time and then never
  //! changed.
  ValidationFlags _validation_flags = ValidationFlags::kNone;

  //! Validation options.
  DiagnosticOptions _diagnostic_options = DiagnosticOptions::kNone;

  //! Encoding options.
  EncodingOptions _encoding_options = EncodingOptions::kNone;

  //! Forced instruction options, combined with \ref _inst_options by \ref emit().
  InstOptions _forced_inst_options = InstOptions::kReserved;

  //! All supported architectures in a bit-mask, where LSB is the bit with a zero index.
  uint64_t _arch_mask = 0;

  //! CodeHolder the emitter is attached to.
  CodeHolder* _code = nullptr;

  //! Attached \ref Logger.
  Logger* _logger = nullptr;

  //! Attached \ref ErrorHandler.
  ErrorHandler* _error_handler = nullptr;

  //! Describes the target environment, matches \ref CodeHolder::environment().
  Environment _environment {};

  //! Native GP register signature (either a 32-bit or 64-bit GP register signature).
  OperandSignature _gp_signature {};
  //! Internal private data used freely by any emitter.
  uint32_t _private_data = 0;

  //! Next instruction options (affects the next instruction).
  InstOptions _inst_options = InstOptions::kNone;
  //! Extra register (op-mask {k} on AVX-512) (affects the next instruction).
  RegOnly _extra_reg {};
  //! Inline comment of the next instruction (affects the next instruction).
  const char* _inline_comment = nullptr;

  //! Pointer to functions used by backend-specific emitter implementation.
  Funcs _funcs {};

  //! Emitter attached before this emitter in \ref CodeHolder, otherwise nullptr if there is no emitter before.
  BaseEmitter* _attached_prev = nullptr;
  //! Emitter attached after this emitter in \ref CodeHolder, otherwise nullptr if there is no emitter after.
  BaseEmitter* _attached_next = nullptr;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit BaseEmitter(EmitterType emitter_type) noexcept;
  ASMJIT_API virtual ~BaseEmitter() noexcept;

  //! \}

  //! \name Cast
  //! \{

  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* as() noexcept { return reinterpret_cast<T*>(this); }

  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* as() const noexcept { return reinterpret_cast<const T*>(this); }

  //! \}

  //! \name Emitter Type & Flags
  //! \{

  //! Returns the type of this emitter, see `EmitterType`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitterType emitter_type() const noexcept { return _emitter_type; }

  //! Returns emitter flags , see `Flags`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EmitterFlags emitter_flags() const noexcept { return _emitter_flags; }

  //! Tests whether the emitter inherits from `BaseAssembler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_assembler() const noexcept { return _emitter_type == EmitterType::kAssembler; }

  //! Tests whether the emitter inherits from `BaseBuilder`.
  //!
  //! \note Both Builder and Compiler emitters would return `true`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_builder() const noexcept { return uint32_t(_emitter_type) >= uint32_t(EmitterType::kBuilder); }

  //! Tests whether the emitter inherits from `BaseCompiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_compiler() const noexcept { return _emitter_type == EmitterType::kCompiler; }

  //! Tests whether the emitter has the given `flag` enabled.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_emitter_flag(EmitterFlags flag) const noexcept { return Support::test(_emitter_flags, flag); }

  //! Tests whether the emitter is finalized.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_finalized() const noexcept { return has_emitter_flag(EmitterFlags::kFinalized); }

  //! Tests whether the emitter is destroyed (only used during destruction).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_destroyed() const noexcept { return has_emitter_flag(EmitterFlags::kDestroyed); }

  //! \}

  //! \cond INTERNAL
  //! \name Internal Functions
  //! \{

  ASMJIT_INLINE_NODEBUG void _add_emitter_flags(EmitterFlags flags) noexcept { _emitter_flags |= flags; }
  ASMJIT_INLINE_NODEBUG void _clear_emitter_flags(EmitterFlags flags) noexcept { _emitter_flags &= _emitter_flags & ~flags; }

  //! \}
  //! \endcond

  //! \name Target Information
  //! \{

  //! Returns the CodeHolder this emitter is attached to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG CodeHolder* code() const noexcept { return _code; }

  //! Returns the target environment.
  //!
  //! The returned \ref Environment reference matches \ref CodeHolder::environment().
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Environment& environment() const noexcept { return _environment; }

  //! Tests whether the target architecture is 32-bit.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_32bit() const noexcept { return environment().is_32bit(); }

  //! Tests whether the target architecture is 64-bit.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_64bit() const noexcept { return environment().is_64bit(); }

  //! Returns the target architecture type.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Arch arch() const noexcept { return environment().arch(); }

  //! Returns the target architecture sub-type.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SubArch sub_arch() const noexcept { return environment().sub_arch(); }

  //! Returns the target architecture's GP register size (4 or 8 bytes).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t register_size() const noexcept { return environment().register_size(); }

  //! Returns a signature of a native general purpose register (either 32-bit or 64-bit depending on the architecture).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandSignature gp_signature() const noexcept { return _gp_signature; }

  //! Returns instruction alignment.
  //!
  //! The following values are returned based on the target architecture:
  //!   - X86 and X86_64 - instruction alignment is 1
  //!   - AArch32 - instruction alignment is 4 in A32 mode and 2 in THUMB mode.
  //!   - AArch64 - instruction alignment is 4
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t instruction_alignment() const noexcept { return _instruction_alignment; }

  //! \}

  //! \name Initialization & Finalization
  //! \{

  //! Tests whether the emitter is initialized (i.e. attached to \ref CodeHolder).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_initialized() const noexcept { return _code != nullptr; }

  //! Finalizes this emitter.
  //!
  //! Materializes the content of the emitter by serializing it to the attached \ref CodeHolder through an architecture
  //! specific \ref BaseAssembler. This function won't do anything if the emitter inherits from \ref BaseAssembler as
  //! assemblers emit directly to a \ref CodeBuffer held by \ref CodeHolder. However, if this is an emitter that
  //! inherits from \ref BaseBuilder or \ref BaseCompiler then these emitters need the materialization phase as they
  //! store their content in a representation not visible to \ref CodeHolder.
  ASMJIT_API virtual Error finalize();

  //! \}

  //! \name Logging
  //! \{

  //! Tests whether the emitter has a logger.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_logger() const noexcept { return _logger != nullptr; }

  //! Tests whether the emitter has its own logger.
  //!
  //! Own logger means that it overrides the possible logger that may be used by \ref CodeHolder this emitter is
  //! attached to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_own_logger() const noexcept { return has_emitter_flag(EmitterFlags::kOwnLogger); }

  //! Returns the logger this emitter uses.
  //!
  //! The returned logger is either the emitter's own logger or it's logger used by \ref CodeHolder this emitter
  //! is attached to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Logger* logger() const noexcept { return _logger; }

  //! Sets or resets the logger of the emitter.
  //!
  //! If the `logger` argument is non-null then the logger will be considered emitter's own logger, see \ref
  //! has_own_logger() for more details. If the given `logger` is null then the emitter will automatically use logger
  //! that is attached to the \ref CodeHolder this emitter is attached to.
  ASMJIT_API void set_logger(Logger* logger) noexcept;

  //! Resets the logger of this emitter.
  //!
  //! The emitter will bail to using a logger attached to \ref CodeHolder this emitter is attached to, or no logger
  //! at all if \ref CodeHolder doesn't have one.
  ASMJIT_INLINE_NODEBUG void reset_logger() noexcept { return set_logger(nullptr); }

  //! \}

  //! \name Error Handling
  //! \{

  //! Tests whether the emitter has an error handler attached.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_error_handler() const noexcept { return _error_handler != nullptr; }

  //! Tests whether the emitter has its own error handler.
  //!
  //! Own error handler means that it overrides the possible error handler that may be used by \ref CodeHolder this
  //! emitter is attached to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_own_error_handler() const noexcept { return has_emitter_flag(EmitterFlags::kOwnErrorHandler); }

  //! Returns the error handler this emitter uses.
  //!
  //! The returned error handler is either the emitter's own error handler or it's error handler used by
  //! \ref CodeHolder this emitter is attached to.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ErrorHandler* error_handler() const noexcept { return _error_handler; }

  //! Sets or resets the error handler of the emitter.
  ASMJIT_API void set_error_handler(ErrorHandler* error_handler) noexcept;

  //! Resets the error handler.
  ASMJIT_INLINE_NODEBUG void reset_error_handler() noexcept { set_error_handler(nullptr); }

  //! \cond INTERNAL
  ASMJIT_API Error _report_error(Error err, const char* message = nullptr);
  //! \endcond

  //! Handles the given error in the following way:
  //!   1. If the emitter has \ref ErrorHandler attached, it calls its \ref ErrorHandler::handle_error() member function
  //!      first, and then returns the error. The `handle_error()` function may throw.
  //!   2. if the emitter doesn't have \ref ErrorHandler, the error is simply returned.
  ASMJIT_INLINE Error report_error(Error err, const char* message = nullptr) {
    Error e = _report_error(err, message);

    // Static analysis is not working properly without these assumptions.
    ASMJIT_ASSUME(e == err);
    ASMJIT_ASSUME(e != Error::kOk);

    return e;
  }

  //! \}

  //! \name Encoding Options
  //! \{

  //! Returns encoding options.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG EncodingOptions encoding_options() const noexcept { return _encoding_options; }

  //! Tests whether the encoding `option` is set.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_encoding_option(EncodingOptions option) const noexcept { return Support::test(_encoding_options, option); }

  //! Enables the given encoding `options`.
  ASMJIT_INLINE_NODEBUG void add_encoding_options(EncodingOptions options) noexcept { _encoding_options |= options; }
  //! Disables the given encoding `options`.
  ASMJIT_INLINE_NODEBUG void clear_encoding_options(EncodingOptions options) noexcept { _encoding_options &= ~options; }

  //! \}

  //! \name Diagnostic Options
  //! \{

  //! Returns the emitter's diagnostic options.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG DiagnosticOptions diagnostic_options() const noexcept { return _diagnostic_options; }

  //! Tests whether the given `option` is present in the emitter's diagnostic options.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_diagnostic_option(DiagnosticOptions option) const noexcept { return Support::test(_diagnostic_options, option); }

  //! Activates the given diagnostic `options`.
  //!
  //! This function is used to activate explicit validation options that will be then used by all emitter
  //! implementations. There are in general two possibilities:
  //!
  //!   - Architecture specific assembler is used. In this case a \ref DiagnosticOptions::kValidateAssembler can be
  //!     used to turn on explicit validation that will be used before an instruction is emitted. This means that
  //!     internally an extra step will be performed to make sure that the instruction is correct. This is needed,
  //!     because by default assemblers prefer speed over strictness.
  //!
  //!     This option should be used in debug builds as it's pretty expensive.
  //!
  //!   - Architecture specific builder or compiler is used. In this case the user can turn on
  //!     \ref DiagnosticOptions::kValidateIntermediate option that adds explicit validation step before the Builder
  //!     or Compiler creates an \ref InstNode to represent an emitted instruction. Error will be returned if the
  //!     instruction is ill-formed. In addition, also \ref DiagnosticOptions::kValidateAssembler can be used, which
  //!     would not be consumed by Builder / Compiler directly, but it would be propagated to an architecture specific
  //!     \ref BaseAssembler implementation it creates during \ref BaseEmitter::finalize().
  ASMJIT_API void add_diagnostic_options(DiagnosticOptions options) noexcept;

  //! Deactivates the given validation `options`.
  //!
  //! See \ref add_diagnostic_options() and \ref DiagnosticOptions for more details.
  ASMJIT_API void clear_diagnostic_options(DiagnosticOptions options) noexcept;

  //! \}

  //! \name Instruction Options
  //! \{

  //! Returns forced instruction options.
  //!
  //! Forced instruction options are merged with next instruction options before the instruction is encoded. These
  //! options have some bits reserved that are used by error handling, logging, and instruction validation purposes.
  //! Other options are globals that affect each instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions forced_inst_options() const noexcept { return _forced_inst_options; }

  //! Returns options of the next instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG InstOptions inst_options() const noexcept { return _inst_options; }

  //! Returns options of the next instruction.
  ASMJIT_INLINE_NODEBUG void set_inst_options(InstOptions options) noexcept { _inst_options = options; }

  //! Adds options of the next instruction.
  ASMJIT_INLINE_NODEBUG void add_inst_options(InstOptions options) noexcept { _inst_options |= options; }

  //! Resets options of the next instruction.
  ASMJIT_INLINE_NODEBUG void reset_inst_options() noexcept { _inst_options = InstOptions::kNone; }

  //! Tests whether the extra register operand is valid.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_extra_reg() const noexcept { return _extra_reg.is_reg(); }

  //! Returns an extra operand that will be used by the next instruction (architecture specific).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const RegOnly& extra_reg() const noexcept { return _extra_reg; }

  //! Sets an extra operand that will be used by the next instruction (architecture specific).
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const Reg& reg) noexcept { _extra_reg.init(reg); }

  //! Sets an extra operand that will be used by the next instruction (architecture specific).
  ASMJIT_INLINE_NODEBUG void set_extra_reg(const RegOnly& reg) noexcept { _extra_reg.init(reg); }

  //! Resets an extra operand that will be used by the next instruction (architecture specific).
  ASMJIT_INLINE_NODEBUG void reset_extra_reg() noexcept { _extra_reg.reset(); }

  //! Returns comment/annotation of the next instruction.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const char* inline_comment() const noexcept { return _inline_comment; }

  //! Sets comment/annotation of the next instruction.
  //!
  //! \note This string is set back to null by `_emit()`, but until that it has to remain valid as the Emitter is not
  //! required to make a copy of it (and it would be slow to do that for each instruction).
  ASMJIT_INLINE_NODEBUG void set_inline_comment(const char* s) noexcept { _inline_comment = s; }

  //! Resets the comment/annotation to nullptr.
  ASMJIT_INLINE_NODEBUG void reset_inline_comment() noexcept { _inline_comment = nullptr; }

  //! \}

  //! \name Emitter State
  //! \{

  //! Resets the emitter state, which contains instruction options, extra register, and inline comment.
  //!
  //! Emitter can have a state that describes instruction options and extra register used by the instruction. Most
  //! instructions don't need nor use the state, however, if an instruction uses a prefix such as REX or REP prefix,
  //! which is set explicitly, then the state would contain it. This allows to mimic the syntax of assemblers such
  //! as X86. For example `rep().movs(...)` would map to a `REP MOVS` instuction on X86. The same applies to various
  //! hints and the use of a mask register in AVX-512 mode.
  ASMJIT_INLINE_NODEBUG void reset_state() noexcept {
    reset_inst_options();
    reset_extra_reg();
    reset_inline_comment();
  }

  //! \cond INTERNAL

  //! Grabs the current emitter state and resets the emitter state at the same time, returning the state the emitter
  //! had before the state was reset.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG State _grab_state() noexcept {
    State s{_inst_options | _forced_inst_options, _extra_reg, _inline_comment};
    reset_state();
    return s;
  }
  //! \endcond

  //! \}

  //! \name Sections
  //! \{

  //! Switches to the given `section`.
  //!
  //! Once switched, everything is emitted to `section`.
  ASMJIT_API virtual Error section(Section* section);

  //! \}

  //! \name Labels
  //! \{

  //! Creates a new anonymous label.
  [[nodiscard]]
  ASMJIT_API virtual Label new_label();

  //! Creates a new named label.
  [[nodiscard]]
  ASMJIT_API virtual Label new_named_label(const char* name, size_t name_size = SIZE_MAX, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId);

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_named_label(Span<const char> name, LabelType type = LabelType::kGlobal, uint32_t parent_id = Globals::kInvalidId) {
    return new_named_label(name.data(), name.size(), type, parent_id);
  }

  //! Creates a new anonymous label with a name, which can only be used for debugging purposes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_anonymous_label(const char* name, size_t name_size = SIZE_MAX) {
    return new_named_label(name, name_size, LabelType::kAnonymous);
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_anonymous_label(Span<const char> name) { return new_anonymous_label(name.data(), name.size()); }

  //! Creates a new external label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label new_external_label(const char* name, size_t name_size = SIZE_MAX) {
    return new_named_label(name, name_size, LabelType::kExternal);
  }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE Label new_external_label(Span<const char> name) { return new_external_label(name.data(), name.size()); }

  //! Returns `Label` by `name`.
  //!
  //! Returns invalid Label in case that the name is invalid or label was not found.
  //!
  //! \note This function doesn't trigger ErrorHandler in case the name is invalid or no such label exist. You must
  //! always check the validity of the `Label` returned.
  [[nodiscard]]
  ASMJIT_API Label label_by_name(const char* name, size_t name_size = SIZE_MAX, uint32_t parent_id = Globals::kInvalidId) noexcept;

  //! \overload
  [[nodiscard]]
  ASMJIT_API Label label_by_name(Span<const char> name, uint32_t parent_id = Globals::kInvalidId) noexcept {
    return label_by_name(name.data(), name.size(), parent_id);
  }

  //! Binds the `label` to the current position of the current section.
  //!
  //! \note Attempt to bind the same label multiple times will return an error.
  ASMJIT_API virtual Error bind(const Label& label);

  //! Tests whether the label `id` is valid (i.e. registered).
  [[nodiscard]]
  ASMJIT_API bool is_label_valid(uint32_t label_id) const noexcept;

  //! Tests whether the `label` is valid (i.e. registered).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_label_valid(const Label& label) const noexcept { return is_label_valid(label.id()); }

  //! \}

  //! \name Emit
  //! \{

  // NOTE: These `emit()` helpers are designed to address a code-bloat generated by C++ compilers to call a function
  // having many arguments. Each parameter to `_emit()` requires some code to pass it, which means that if we default
  // to 5 arguments in `_emit()` and inst_id the C++ compiler would have to generate a virtual function call having 5
  // parameters and additional `this` argument, which is quite a lot. Since by default most instructions have 2 to 3
  // operands it's better to introduce helpers that pass from 0 to 6 operands that help to reduce the size of emit(...)
  // function call.

  //! Emits an instruction (internal).
  ASMJIT_API Error _emitI(InstId inst_id);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4);
  //! \overload
  ASMJIT_API Error _emitI(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5);

  //! Emits an instruction `inst_id` with the given `operands`.
  //!
  //! This is the most universal way of emitting code, which accepts an instruction identifier and instruction
  //! operands. This is called an "unchecked" API as emit doesn't provide any type checks at compile-time. This
  //! allows to emit instruction with just \ref Operand instances, which could be handy in some cases - for
  //! example emitting generic code where you don't know whether some operand is register, memory, or immediate.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Error emit(InstId inst_id, Args&&... operands) {
    return _emitI(inst_id, Support::ForwardOp<Args>::forward(operands)...);
  }

  //! Similar to \ref emit(), but uses array of `operands` instead.
  ASMJIT_INLINE_NODEBUG Error emit_op_array(InstId inst_id, const Operand_* operands, size_t op_count) {
    return _emit_op_array(inst_id, operands, op_count);
  }

  //! Similar to \ref emit(), but emits instruction with both instruction options and extra register, followed
  //! by an array of `operands`.
  ASMJIT_INLINE Error emit_inst(const BaseInst& inst, const Operand_* operands, size_t op_count) {
    set_inst_options(inst.options());
    set_extra_reg(inst.extra_reg());
    return _emit_op_array(inst.inst_id(), operands, op_count);
  }

  //! \}

  //! \cond INTERNAL
  //! \name Emit Internals
  //! \{

  //! Emits an instruction - all 6 operands must be defined.
  ASMJIT_API virtual Error _emit(InstId inst_id, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* op_ext);
  //! Emits instruction having operands stored in array.
  ASMJIT_API virtual Error _emit_op_array(InstId inst_id, const Operand_* operands, size_t op_count);

  //! \}
  //! \endcond

  //! \name Emit Utilities
  //! \{

  //! Emits a function prolog described by the given function `frame`.
  ASMJIT_API Error emit_prolog(const FuncFrame& frame);
  //! Emits a function epilog described by the given function `frame`.
  ASMJIT_API Error emit_epilog(const FuncFrame& frame);
  //! Emits code that reassigns function `frame` arguments to the given `args`.
  ASMJIT_API Error emit_args_assignment(const FuncFrame& frame, const FuncArgsAssignment& args);

  //! \}

  //! \name Align
  //! \{

  //! Aligns the current CodeBuffer position to the `alignment` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location and the current location depends on the
  //! align `mode`, see \ref AlignMode. The `alignment` argument specifies alignment in bytes, so for example when
  //! it's `32` it means that the code buffer will be aligned to `32` bytes.
  ASMJIT_API virtual Error align(AlignMode align_mode, uint32_t alignment);

  //! \}

  //! \name Embed
  //! \{

  //! Embeds raw data into the \ref CodeBuffer.
  ASMJIT_API virtual Error embed(const void* data, size_t data_size);

  //! Embeds a typed data array.
  //!
  //! This is the most flexible function for embedding data as it allows to:
  //!
  //!   - Assign a `type_id` to the data, so the emitter knows the type of items stored in `data`. Binary data should
  //!     use \ref TypeId::kUInt8.
  //!
  //!   - Repeat the given data `repeat_count` times, so the data can be used as a fill pattern for example, or as a
  //!     pattern used by SIMD instructions.
  ASMJIT_API virtual Error embed_data_array(TypeId type_id, const void* data, size_t item_count, size_t repeat_count = 1);

  //! Embeds int8_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_int8(int8_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt8, &value, 1, repeat_count); }
  //! Embeds uint8_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_uint8(uint8_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt8, &value, 1, repeat_count); }
  //! Embeds int16_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_int16(int16_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt16, &value, 1, repeat_count); }
  //! Embeds uint16_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_uint16(uint16_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt16, &value, 1, repeat_count); }
  //! Embeds int32_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_int32(int32_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt32, &value, 1, repeat_count); }
  //! Embeds uint32_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_uint32(uint32_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt32, &value, 1, repeat_count); }
  //! Embeds int64_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_int64(int64_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kInt64, &value, 1, repeat_count); }
  //! Embeds uint64_t `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_uint64(uint64_t value, size_t repeat_count = 1) { return embed_data_array(TypeId::kUInt64, &value, 1, repeat_count); }
  //! Embeds a floating point `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_float(float value, size_t repeat_count = 1) { return embed_data_array(TypeId(TypeUtils::TypeIdOfT<float>::kTypeId), &value, 1, repeat_count); }
  //! Embeds a floating point `value` repeated by `repeat_count`.
  ASMJIT_INLINE_NODEBUG Error embed_double(double value, size_t repeat_count = 1) { return embed_data_array(TypeId(TypeUtils::TypeIdOfT<double>::kTypeId), &value, 1, repeat_count); }

  //! Embeds a constant pool at the current offset by performing the following:
  //!   1. Aligns by using AlignMode::kData to the minimum `pool` alignment.
  //!   2. Binds the ConstPool label so it's bound to an aligned location.
  //!   3. Emits ConstPool content.
  ASMJIT_API virtual Error embed_const_pool(const Label& label, const ConstPool& pool);

  //! Embeds an absolute `label` address as data.
  //!
  //! The `data_size` is an optional argument that can be used to specify the size of the address data. If it's zero
  //! (default) the address size is deduced from the target architecture (either 4 or 8 bytes).
  ASMJIT_API virtual Error embed_label(const Label& label, size_t data_size = 0);

  //! Embeds a delta (distance) between the `label` and `base` calculating it as `label - base`. This function was
  //! designed to make it easier to embed lookup tables where each index is a relative distance of two labels.
  ASMJIT_API virtual Error embed_label_delta(const Label& label, const Label& base, size_t data_size = 0);

  //! \}

  //! \name Comment
  //! \{

  //! Emits a comment stored in `data` with an optional `size` parameter.
  ASMJIT_API virtual Error comment(const char* data, size_t size = SIZE_MAX);

  //! Emits a comment passed via a `data` span.
  ASMJIT_INLINE Error comment(Span<const char> data) {
    return comment(data.data(), data.size());
  }

  //! Emits a formatted comment specified by `fmt` and variable number of arguments.
  ASMJIT_API Error commentf(const char* fmt, ...);
  //! Emits a formatted comment specified by `fmt` and `ap`.
  ASMJIT_API Error commentv(const char* fmt, va_list ap);

  //! \}

  //! \name Events
  //! \{

  //! Called after the emitter was attached to `CodeHolder`.
  ASMJIT_API virtual Error on_attach(CodeHolder& code) noexcept;

  //! Called after the emitter was detached from `CodeHolder`.
  ASMJIT_API virtual Error on_detach(CodeHolder& code) noexcept;

  //! Called when CodeHolder is reinitialized when the emitter is attached.
  ASMJIT_API virtual Error on_reinit(CodeHolder& code) noexcept;

  //! Called when \ref CodeHolder has updated an important setting, which involves the following:
  //!
  //!   - \ref Logger has been changed (\ref CodeHolder::set_logger() has been called).
  //!
  //!   - \ref ErrorHandler has been changed (\ref CodeHolder::set_error_handler() has been called).
  //!
  //! This function ensures that the settings are properly propagated from \ref CodeHolder to the emitter.
  //!
  //! \note This function is virtual and can be overridden, however, if you do so, always call \ref
  //! BaseEmitter::on_settings_updated() within your own implementation to ensure that the emitter is
  //! in a consistent state.
  ASMJIT_API virtual void on_settings_updated() noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_EMITTER_H_INCLUDED
