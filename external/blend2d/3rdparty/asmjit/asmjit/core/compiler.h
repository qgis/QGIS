// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_COMPILER_H_INCLUDED
#define ASMJIT_CORE_COMPILER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/assembler.h>
#include <asmjit/core/builder.h>
#include <asmjit/core/constpool.h>
#include <asmjit/core/compilerdefs.h>
#include <asmjit/core/func.h>
#include <asmjit/core/inst.h>
#include <asmjit/core/operand.h>
#include <asmjit/support/arena.h>
#include <asmjit/support/arenavector.h>
#include <asmjit/support/support.h>

ASMJIT_BEGIN_NAMESPACE

class JumpAnnotation;
class JumpNode;
class FuncNode;
class FuncRetNode;
class InvokeNode;

//! \addtogroup asmjit_compiler
//! \{

//! Code emitter that uses virtual registers and performs register allocation.
//!
//! Compiler is a high-level code-generation tool that provides register allocation and automatic handling of function
//! calling conventions. It was primarily designed for merging multiple parts of code into a function without worrying
//! about registers and function calling conventions.
//!
//! BaseCompiler can be used, with a minimum effort, to handle 32-bit and 64-bit code generation within a single code
//! base.
//!
//! BaseCompiler is based on BaseBuilder and contains all the features it provides. It means that the code it stores
//! can be modified (removed, added, injected) and analyzed. When the code is finalized the compiler can emit the code
//! into an Assembler to translate the abstract representation into a machine code.
//!
//! Check out architecture specific compilers for more details and examples:
//!
//!   - \ref x86::Compiler - X86/X64 compiler implementation.
//!   - \ref a64::Compiler - AArch64 compiler implementation.
class ASMJIT_VIRTAPI BaseCompiler : public BaseBuilder {
public:
  ASMJIT_NONCOPYABLE(BaseCompiler)
  using Base = BaseBuilder;

  //! \name Members
  //! \{

  //! Current function.
  FuncNode* _func;
  //! Stores array of `VirtReg` pointers.
  ArenaVector<VirtReg*> _virt_regs;
  //! Stores jump annotations.
  ArenaVector<JumpAnnotation*> _jump_annotations;

  //! Local and global constant pools.
  //!
  //! Local constant pool is flushed with each function, global constant pool is flushed only by \ref finalize().
  ConstPoolNode* _const_pools[2];

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `BaseCompiler` instance.
  ASMJIT_API BaseCompiler() noexcept;
  //! Destroys the `BaseCompiler` instance.
  ASMJIT_API ~BaseCompiler() noexcept override;

  //! \}

  //! \name Passes
  //! \{

  //! \overload
  template<typename PassT, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE PassT* new_pass(Args&&... args) noexcept { return _builder_arena.new_oneshot<PassT>(*this, std::forward<Args>(args)...); }

  template<typename T, typename... Args>
  ASMJIT_INLINE Error add_pass(Args&&... args) { return _add_pass(new_pass<T, Args...>(std::forward<Args>(args)...)); }

  //! \}

  //! \name Function Management
  //! \{

  //! Returns the function being generated.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncNode* func() const noexcept { return _func; }

  //! Creates a new \ref FuncNode.
  ASMJIT_API Error new_func_node(Out<FuncNode*> out, const FuncSignature& signature);
  //! Creates a new \ref FuncNode adds it to the instruction stream.
  ASMJIT_API Error add_func_node(Out<FuncNode*> out, const FuncSignature& signature);

  //! Creates a new \ref FuncRetNode.
  ASMJIT_API Error new_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1);
  //! Creates a new \ref FuncRetNode and adds it to the instruction stream.
  ASMJIT_API Error add_func_ret_node(Out<FuncRetNode*> out, const Operand_& o0, const Operand_& o1);

  //! Creates a new \ref FuncNode with the given `signature` and returns it.
  ASMJIT_INLINE FuncNode* new_func(const FuncSignature& signature) {
    FuncNode* node;
    new_func_node(Out(node), signature);
    return node;
  }

  //! Creates a new \ref FuncNode with the given `signature`, adds it to the instruction stream by using
  //! `add_func(FuncNode* func)` overload, and returns the node.
  ASMJIT_INLINE FuncNode* add_func(const FuncSignature& signature) {
    FuncNode* node;
    add_func_node(Out(node), signature);
    return node;
  }

  //! Adds a function `node` to the instruction stream.
  ASMJIT_API FuncNode* add_func(FuncNode* ASMJIT_NONNULL(func));

  //! Ends the current function by emitting a sentinel that marks the end of it.
  //!
  //! This would close the context for generating the current function. After calling \ref end_func() the active
  //! function node is reset and \ref func() would return `nullptr` unless another function is being started via
  //! \ref add_func().
  ASMJIT_API Error end_func();

  ASMJIT_INLINE Error add_ret(const Operand_& o0, const Operand_& o1) {
    FuncRetNode* node;
    return add_func_ret_node(Out(node), o0, o1);
  }

  //! \}

  //! \name Function Invocation
  //! \{

  //! Creates a new \ref InvokeNode.
  ASMJIT_API Error new_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature);
  //! Creates a new \ref InvokeNode and adds it to the instruction stream.
  ASMJIT_API Error add_invoke_node(Out<InvokeNode*> out, InstId inst_id, const Operand_& o0, const FuncSignature& signature);

  //! \}

  //! \name Virtual Registers
  //! \{

  //! Creates a new virtual register representing the given `type_id` and `signature`.
  //!
  //! \note This function is public, but it's not generally recommended to be used by AsmJit users, use `new_reg()`,
  //! `new_similar_reg()`, and architecture specific functions like \ref x86::Compiler::new_gp32(), etc...
  ASMJIT_API Error new_virt_reg(Out<VirtReg*> out, TypeId type_id, OperandSignature signature, const char* name);

  //! \cond INTERNAL

  //! Creates a new virtual register of the given `type_id` and stores it to `out` operand.
  ASMJIT_API Error _new_reg_with_name(Out<Reg> out, TypeId type_id, const char* name);

  //! Creates a new virtual register compatible with the provided reference register `ref`.
  ASMJIT_API Error _new_reg_with_name(Out<Reg> out, const Reg& ref, const char* name);

  //! Creates a new virtual register of the given `type_id` and stores it to `out` operand.
  //!
  //! \note This version accepts a snprintf() format `fmt` followed by variadic arguments.
  ASMJIT_API Error _new_reg_with_vfmt(Out<Reg> out, TypeId type_id, const char* fmt, ...);

  //! Creates a new virtual register compatible with the provided reference register `ref`.
  //!
  //! \note This version accepts a snprintf() format `fmt` followed by variadic arguments.
  ASMJIT_API Error _new_reg_with_vfmt(Out<Reg> out, const Reg& ref, const char* fmt, ...);

  template<typename RegT>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, TypeId type_id) {
    return _new_reg_with_name(Out<Reg>(out.value()), type_id, nullptr);
  }

  template<typename RegT, typename... Args>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, TypeId type_id, const char* name_or_fmt, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    if constexpr (sizeof...(args) == 0u) {
      return _new_reg_with_name(Out<Reg>(out.value()), type_id, name_or_fmt);
    }
    else {
      return _new_reg_with_vfmt(Out<Reg>(out.value()), type_id, name_or_fmt, std::forward<Args>(args)...);
    }
#else
    Support::maybe_unused(name_or_fmt, std::forward<Args>(args)...);
    return _new_reg_with_name(Out<Reg>(out.value()), type_id, nullptr);
#endif
  }

  template<typename RegT>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, const Reg& ref) {
    return _new_reg_with_name(Out<Reg>(out.value()), ref, nullptr);
  }

  template<typename RegT, typename... Args>
  ASMJIT_INLINE Error _new_reg(Out<RegT> out, const Reg& ref, const char* name_or_fmt, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    if constexpr (sizeof...(args) == 0u) {
      return _new_reg_with_name(Out<Reg>(out.value()), ref, name_or_fmt);
    }
    else {
      return _new_reg_with_vfmt(Out<Reg>(out.value()), ref, name_or_fmt, std::forward<Args>(args)...);
    }
#else
    Support::maybe_unused(name_or_fmt, std::forward<Args>(args)...);
    return _new_reg_with_name(Out<Reg>(out.value()), ref, nullptr);
#endif
  }

  //! \endcond

  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT new_reg(TypeId type_id, Args&&... args) {
    RegT reg(Globals::NoInit);
    (void)_new_reg<RegT>(Out(reg), type_id, std::forward<Args>(args)...);
    return reg;
  }

  //! Creates and returns a new register, which is similar to `ref` in terms of size and type.
  //!
  //! \note Optionally you can provide a name and format parameters via `args`.
  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT new_similar_reg(const RegT& ref, Args&&... args) {
    RegT reg(Globals::NoInit);
    (void)_new_reg<RegT>(Out(reg), ref, std::forward<Args>(args)...);
    return reg;
  }

  //! Tests whether the given `virt_id` is a valid virtual register id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_virt_id_valid(uint32_t virt_id) const noexcept {
    uint32_t index = Operand::virt_id_to_index(virt_id);
    return index < _virt_regs.size();
  }

  //! Tests whether the given `reg` is a virtual register having a valid id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool is_virt_reg_valid(const Reg& reg) const noexcept {
    return is_virt_id_valid(reg.id());
  }

  //! Returns \ref VirtReg associated with the given `virt_id`.
  [[nodiscard]]
  ASMJIT_INLINE VirtReg* virt_reg_by_id(uint32_t virt_id) const noexcept {
    ASMJIT_ASSERT(is_virt_id_valid(virt_id));
    return _virt_regs[Operand::virt_id_to_index(virt_id)];
  }

  //! Returns \ref VirtReg associated with the given `reg`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virt_reg_by_reg(const Reg& reg) const noexcept { return virt_reg_by_id(reg.id()); }

  //! Returns \ref VirtReg associated with the given virtual register `index`.
  //!
  //! \note This is not the same as virtual register id. The conversion between id and its index is implemented
  //! by \ref Operand_::virt_id_to_index() and \ref Operand_::virt_index_to_virt_id() functions.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG VirtReg* virt_reg_by_index(uint32_t index) const noexcept { return _virt_regs[index]; }

  //! Returns an array of all virtual registers managed by the Compiler.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<VirtReg*> virt_regs() const noexcept { return _virt_regs.as_span(); }

  //! \name Stack
  //! \{

  //! Creates a new stack of the given `size` and `alignment` and stores it to `out`.
  //!
  //! \note `name` can be used to give the stack a name, for debugging purposes.
  ASMJIT_API Error _new_stack(Out<BaseMem> out, uint32_t size, uint32_t alignment, const char* name = nullptr);

  //! Updates the stack size of a stack created by `_new_stack()` by its `virt_id`.
  ASMJIT_API Error set_stack_size(uint32_t virt_id, uint32_t new_size, uint32_t new_alignment = 0);

  //! Updates the stack size of a stack created by `_new_stack()`.
  ASMJIT_INLINE_NODEBUG Error set_stack_size(const BaseMem& mem, uint32_t new_size, uint32_t new_alignment = 0) {
    return set_stack_size(mem.id(), new_size, new_alignment);
  }

  //! \}

  //! \name Constants
  //! \{

  //! Creates a new constant of the given `scope` (see \ref ConstPoolScope).
  //!
  //! This function adds a constant of the given `size` to the built-in \ref ConstPool and stores the reference to that
  //! constant to the `out` operand.
  ASMJIT_API Error _new_const(Out<BaseMem> out, ConstPoolScope scope, const void* data, size_t size);

  //! \}

  //! \name Miscellaneous
  //! \{

  //! Rename the given virtual register `reg` to a formatted string `fmt`.
  ASMJIT_API void rename(const Reg& reg, const char* fmt, ...);

  //! \}

  //! \name Jump Annotations
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<JumpAnnotation*> jump_annotations() const noexcept { return _jump_annotations.as_span(); }

  ASMJIT_API Error new_jump_node(Out<JumpNode*> out, InstId inst_id, InstOptions inst_options, const Operand_& o0, JumpAnnotation* annotation);
  ASMJIT_API Error emit_annotated_jump(InstId inst_id, const Operand_& o0, JumpAnnotation* annotation);

  //! Returns a new `JumpAnnotation` instance, which can be used to aggregate possible targets of a jump where the
  //! target is not a label, for example to implement jump tables.
  [[nodiscard]]
  ASMJIT_API JumpAnnotation* new_jump_annotation();

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}
};

//! Jump annotation used to annotate jumps.
//!
//! \ref BaseCompiler allows to emit jumps where the target is either register or memory operand. Such jumps cannot be
//! trivially inspected, so instead of doing heuristics AsmJit allows to annotate such jumps with possible targets.
//! Register allocator then uses the annotation to construct control-flow, which is then used by liveness analysis and
//! other tools to prepare ground for register allocation.
class JumpAnnotation {
public:
  ASMJIT_NONCOPYABLE(JumpAnnotation)

  //! \name Members
  //! \{

  //! Compiler that owns this JumpAnnotation.
  BaseCompiler* _compiler;
  //! Annotation identifier.
  uint32_t _annotation_id;
  //! Vector of label identifiers, see \ref label_ids().
  ArenaVector<uint32_t> _label_ids;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG JumpAnnotation(BaseCompiler* ASMJIT_NONNULL(compiler), uint32_t annotation_id) noexcept
    : _compiler(compiler),
      _annotation_id(annotation_id) {}

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the compiler that owns this JumpAnnotation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler* compiler() const noexcept { return _compiler; }

  //! Returns the annotation id.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t annotation_id() const noexcept { return _annotation_id; }

  //! Returns a vector of label identifiers that lists all targets of the jump.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Span<uint32_t> label_ids() const noexcept { return _label_ids.as_span(); }

  //! Tests whether the given `label` is a target of this JumpAnnotation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label(const Label& label) const noexcept { return has_label_id(label.id()); }

  //! Tests whether the given `label_id` is a target of this JumpAnnotation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_label_id(uint32_t label_id) const noexcept { return _label_ids.contains(label_id); }

  //! \}

  //! \name Annotation Building API
  //! \{

  //! Adds the `label` to the list of targets of this JumpAnnotation.
  ASMJIT_INLINE_NODEBUG Error add_label(const Label& label) noexcept { return add_label_id(label.id()); }
  //! Adds the `label_id` to the list of targets of this JumpAnnotation.
  ASMJIT_INLINE_NODEBUG Error add_label_id(uint32_t label_id) noexcept { return _label_ids.append(_compiler->_builder_arena, label_id); }

  //! \}
};

//! Jump instruction with \ref JumpAnnotation.
//!
//! \note This node should be only used to represent jump where the jump target cannot be deduced by examining
//! instruction operands. For example if the jump target is register or memory location. This pattern is often
//! used to perform indirect jumps that use jump table, e.g. to implement `switch{}` statement.
class JumpNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(JumpNode)

  //! \name Members
  //! \{

  JumpAnnotation* _annotation;

  //! \}

  //! \name Construction & Destruction
  //! \{

  inline JumpNode(InstId inst_id, InstOptions options, uint32_t op_count, JumpAnnotation* annotation) noexcept
    : InstNodeWithOperands(inst_id, options, op_count),
      _annotation(annotation) {
    _set_type(NodeType::kJump);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether this JumpNode has associated a \ref JumpAnnotation.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_annotation() const noexcept { return _annotation != nullptr; }

  //! Returns the \ref JumpAnnotation associated with this jump, or `nullptr`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG JumpAnnotation* annotation() const noexcept { return _annotation; }

  //! Sets the \ref JumpAnnotation associated with this jump to `annotation`.
  ASMJIT_INLINE_NODEBUG void set_annotation(JumpAnnotation* annotation) noexcept { _annotation = annotation; }

  //! \}
};

//! Function node represents a function used by \ref BaseCompiler.
//!
//! A function is composed of the following:
//!
//!   - Function entry, \ref FuncNode acts as a label, so the entry is implicit. To get the entry, simply use
//!     \ref FuncNode::label(), which is the same as \ref LabelNode::label().
//!
//!   - Function exit, which is represented by \ref FuncNode::exit_node(). A helper function
//!     \ref FuncNode::exit_label() exists and returns an exit label instead of node.
//!
//!   - Function \ref FuncNode::end_node() sentinel. This node marks the end of a function - there should be no
//!     code that belongs to the function after this node, but the Compiler doesn't enforce that at the moment.
//!
//!   - Function detail, see \ref FuncNode::detail().
//!
//!   - Function frame, see \ref FuncNode::frame().
//!
//!   - Function arguments mapped to virtual registers, see \ref FuncNode::arg_packs().
//!
//! In a node list, the function and its body looks like the following:
//!
//! \code{.unparsed}
//! [...]       - Anything before the function.
//!
//! [FuncNode]  - Entry point of the function, acts as a label as well.
//!   <Prolog>  - Prolog inserted by the register allocator.
//!   {...}     - Function body - user code basically.
//! [ExitLabel] - Exit label
//!   <Epilog>  - Epilog inserted by the register allocator.
//!   <Return>  - Return inserted by the register allocator.
//!   {...}     - Can contain data or user code (error handling, special cases, ...).
//! [FuncEnd]   - End sentinel
//!
//! [...]       - Anything after the function.
//! \endcode
//!
//! When a function is added to the instruction stream by \ref BaseCompiler::add_func() it actually inserts 3 nodes
//! (FuncNode, ExitLabel, and FuncEnd) and sets the current cursor to be FuncNode. When \ref BaseCompiler::end_func()
//! is called the cursor is set to FuncEnd. This guarantees that user can use ExitLabel as a marker after additional
//! code or data can be placed, which is a common practice.
class FuncNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(FuncNode)

  //! Arguments pack.
  struct ArgPack {
    RegOnly _data[Globals::kMaxValuePack];

    ASMJIT_INLINE void reset() noexcept {
      for (RegOnly& v : _data) {
        v.reset();
      }
    }

    ASMJIT_INLINE RegOnly& operator[](size_t value_index) noexcept { return _data[value_index]; }
    ASMJIT_INLINE const RegOnly& operator[](size_t value_index) const noexcept { return _data[value_index]; }
  };

  //! \name Members
  //! \{

  //! Function detail.
  FuncDetail _func_detail;
  //! Function frame.
  FuncFrame _frame;
  //! Function exit label.
  LabelNode* _exit_node;
  //! Function end (sentinel).
  SentinelNode* _end;
  //! Argument packs.
  ArgPack* _args;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FuncNode` instance.
  //!
  //! Always use `BaseCompiler::add_func()` to create a new `FuncNode`.
  inline explicit FuncNode(uint32_t label_id = Globals::kInvalidId) noexcept
    : LabelNode(label_id),
      _func_detail(),
      _frame(),
      _exit_node(nullptr),
      _end(nullptr),
      _args(nullptr) {
    _set_type(NodeType::kFunc);
  }

  //! \}

  //! \{
  //! \name Accessors

  //! Returns function exit `LabelNode`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG LabelNode* exit_node() const noexcept { return _exit_node; }

  //! Returns function exit label.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Label exit_label() const noexcept { return _exit_node->label(); }

  //! Returns "End of Func" sentinel node.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG SentinelNode* end_node() const noexcept { return _end; }

  //! Returns function detail.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncDetail& detail() noexcept { return _func_detail; }

  //! Returns function detail.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail& detail() const noexcept { return _func_detail; }

  //! Returns function frame.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncFrame& frame() noexcept { return _frame; }

  //! Returns function frame.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncFrame& frame() const noexcept { return _frame; }

  //! Returns function attributes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncAttributes attributes() const noexcept { return _frame.attributes(); }

  //! Adds `attrs` to the function attributes.
  ASMJIT_INLINE_NODEBUG void add_attributes(FuncAttributes attrs) noexcept { _frame.add_attributes(attrs); }

  //! Returns arguments count.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _func_detail.arg_count(); }

  //! Returns argument packs.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG ArgPack* arg_packs() const noexcept { return _args; }

  //! Tests whether the function has a return value.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return _func_detail.has_ret(); }

  //! Returns argument pack at `arg_index`.
  [[nodiscard]]
  inline ArgPack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }

  //! Sets argument at `arg_index`.
  inline void set_arg(size_t arg_index, const Reg& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][0].init(virt_reg);
  }

  //! \overload
  inline void set_arg(size_t arg_index, const RegOnly& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][0].init(virt_reg);
  }

  //! Sets argument at `arg_index` and `value_index`.
  inline void set_arg(size_t arg_index, size_t value_index, const Reg& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].init(virt_reg);
  }

  //! \overload
  inline void set_arg(size_t arg_index, size_t value_index, const RegOnly& virt_reg) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].init(virt_reg);
  }

  //! Resets argument pack at `arg_index`.
  inline void reset_arg(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index].reset();
  }

  //! Resets argument pack at `arg_index`.
  inline void reset_arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index].reset();
  }

  //! \}
};

//! Function return, used by \ref BaseCompiler.
class FuncRetNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(FuncRetNode)

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FuncRetNode` instance.
  inline FuncRetNode() noexcept
    : InstNodeWithOperands(BaseInst::kIdAbstract, InstOptions::kNone, 0) {
    _node_type = NodeType::kFuncRet;
  }

  //! \}
};

//! Function invocation, used by \ref BaseCompiler.
class InvokeNode : public InstNodeWithOperands<InstNode::kBaseOpCapacity> {
public:
  ASMJIT_NONCOPYABLE(InvokeNode)

  //! Operand pack provides multiple operands that can be associated with a single return value of function
  //! argument. Sometimes this is necessary to express an argument or return value that requires multiple
  //! registers, for example 64-bit value in 32-bit mode or passing / returning homogeneous data structures.
  struct OperandPack {
    //! Operands.
    Operand_ _data[Globals::kMaxValuePack];

    //! Reset the pack by resetting all operands in the pack.
    ASMJIT_INLINE void reset() noexcept {
      for (Operand_& op : _data) {
        op.reset();
      }
    }

    //! Returns an operand at the given `value_index`.
    [[nodiscard]]
    ASMJIT_INLINE Operand& operator[](size_t value_index) noexcept {
      ASMJIT_ASSERT(value_index < Globals::kMaxValuePack);

      return _data[value_index].as<Operand>();
    }

    //! Returns an operand at the given `value_index` (const).
    [[nodiscard]]
    ASMJIT_INLINE const Operand& operator[](size_t value_index) const noexcept {
      ASMJIT_ASSERT(value_index < Globals::kMaxValuePack);

      return _data[value_index].as<Operand>();
    }
  };

  //! \name Members
  //! \{

  //! Function detail.
  FuncDetail _func_detail;
  //! Function return value(s).
  OperandPack _rets;
  //! Function arguments.
  OperandPack* _args;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `InvokeNode` instance.
  inline InvokeNode(InstId inst_id, InstOptions options) noexcept
    : InstNodeWithOperands(inst_id, options, 0),
      _func_detail(),
      _args(nullptr) {
    _set_type(NodeType::kInvoke);
    _reset_ops();
    _rets.reset();
    _add_flags(NodeFlags::kIsRemovable);
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Sets the function signature.
  [[nodiscard]]
  inline Error init(const FuncSignature& signature, const Environment& environment) noexcept {
    return _func_detail.init(signature, environment);
  }

  //! Returns the function detail.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG FuncDetail& detail() noexcept { return _func_detail; }

  //! Returns the function detail.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const FuncDetail& detail() const noexcept { return _func_detail; }

  //! Returns the target operand.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand& target() noexcept { return op(0); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand& target() const noexcept { return op(0); }

  //! Returns the number of function return values.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_ret() const noexcept { return _func_detail.has_ret(); }

  //! Returns the number of function arguments.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t arg_count() const noexcept { return _func_detail.arg_count(); }

  //! Returns operand pack representing function return value(s).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG OperandPack& ret_pack() noexcept { return _rets; }

  //! Returns operand pack representing function return value(s).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const OperandPack& ret_pack() const noexcept { return _rets; }

  //! Returns the return value at the given `value_index`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Operand& ret(size_t value_index = 0) noexcept { return _rets[value_index]; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const Operand& ret(size_t value_index = 0) const noexcept { return _rets[value_index]; }

  //! Returns operand pack representing function return value(s).
  [[nodiscard]]
  inline OperandPack& arg_pack(size_t arg_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }

  //! \overload
  [[nodiscard]]
  inline const OperandPack& arg_pack(size_t arg_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index];
  }

  //! Returns a function argument at the given `arg_index`.
  [[nodiscard]]
  inline Operand& arg(size_t arg_index, size_t value_index) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index][value_index];
  }

  //! \overload
  [[nodiscard]]
  inline const Operand& arg(size_t arg_index, size_t value_index) const noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    return _args[arg_index][value_index];
  }

  //! Sets the function return value at `i` to `op`.
  inline void _set_ret(size_t value_index, const Operand_& op) noexcept { _rets[value_index] = op; }
  //! Sets the function argument at `i` to `op`.
  inline void _set_arg(size_t arg_index, size_t value_index, const Operand_& op) noexcept {
    ASMJIT_ASSERT(arg_index < arg_count());
    _args[arg_index][value_index] = op;
  }

  //! Sets the function return value at `value_index` to `reg`.
  ASMJIT_INLINE_NODEBUG void set_ret(size_t value_index, const Reg& reg) noexcept { _set_ret(value_index, reg); }

  //! Sets the first function argument in a value-pack at `arg_index` to `reg`.
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, const Reg& reg) noexcept { _set_arg(arg_index, 0, reg); }
  //! Sets the first function argument in a value-pack at `arg_index` to `imm`.
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, const Imm& imm) noexcept { _set_arg(arg_index, 0, imm); }

  //! Sets the function argument at `arg_index` and `value_index` to `reg`.
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, size_t value_index, const Reg& reg) noexcept { _set_arg(arg_index, value_index, reg); }
  //! Sets the function argument at `arg_index` and `value_index` to `imm`.
  ASMJIT_INLINE_NODEBUG void set_arg(size_t arg_index, size_t value_index, const Imm& imm) noexcept { _set_arg(arg_index, value_index, imm); }

  //! \}
};

//! Function pass extends \ref Pass with \ref FuncPass::run_on_function().
class ASMJIT_VIRTAPI FuncPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(FuncPass)
  using Base = Pass;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API FuncPass(BaseCompiler& cc, const char* name) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated `BaseCompiler`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BaseCompiler& cc() const noexcept { return static_cast<BaseCompiler&>(_cb); }

  //! \}

  //! \name Pass Interface
  //! \{

  //! Calls `run_on_function()` on each `FuncNode` node found.
  ASMJIT_API Error run(Arena& arena, Logger* logger) override;

  //! Called once per `FuncNode`.
  ASMJIT_API virtual Error run_on_function(Arena& arena, Logger* logger, FuncNode* func);

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_CORE_COMPILER_H_INCLUDED
