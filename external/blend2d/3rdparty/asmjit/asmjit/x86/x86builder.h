// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86BUILDER_H_INCLUDED
#define ASMJIT_X86_X86BUILDER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_BUILDER

#include <asmjit/core/builder.h>
#include <asmjit/x86/x86emitter.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

//! X86/X64 builder implementation.
//!
//! The code representation used by \ref BaseBuilder is compatible with everything AsmJit provides. Each instruction
//! is stored as \ref InstNode, which contains instruction id, options, and operands. Each instruction emitted will
//! create a new \ref InstNode instance and add it to the current cursor in the double-linked list of nodes. Since
//! the instruction stream used by \ref BaseBuilder can be manipulated, we can rewrite the SumInts example from
//! \ref asmjit_assembler into the following:
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! // Small helper function to print the current content of `cb`.
//! static void dump_code(BaseBuilder& builder, const char* phase) {
//!   String sb;
//!   format_options format_options {};
//!
//!   Formatter::format_node_list(sb, format_options, &builder);
//!   printf("%s:\n%s\n", phase, sb.data());
//! }
//!
//! int main() {
//!   using SumIntsFunc = void (*)(int* dst, const int* a, const int* b);
//!
//!   JitRuntime rt;                      // Create JIT Runtime.
//!   CodeHolder code;                    // Create a CodeHolder.
//!
//!   code.init(rt.environment(),         // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Builder cb(&code);             // Create and attach x86::Builder to `code`.
//!
//!   // Decide which registers will be mapped to function arguments. Try changing registers
//!   // of `dst`, `src_a`, and `src_b` and see what happens in function's prolog and epilog.
//!   x86::Gp dst = cb.zax();
//!   x86::Gp src_a = cb.zcx();
//!   x86::Gp src_b = cb.zdx();
//!
//!   X86::Vec vec0 = x86::xmm0;
//!   X86::Vec vec1 = x86::xmm1;
//!
//!   // Create and initialize `FuncDetail`.
//!   FuncDetail func;
//!   func.init(FuncSignature::build<void, int*, const int*, const int*>());
//!
//!   // Remember prolog insertion point.
//!   BaseNode* prolog_insertion_point = cb.cursor();
//!
//!   // Emit function body:
//!   cb.movdqu(vec0, x86::ptr(src_a));   // Load 4 ints from [src_a] to XMM0.
//!   cb.movdqu(vec1, x86::ptr(src_b));   // Load 4 ints from [src_b] to XMM1.
//!   cb.paddd(vec0, vec1);               // Add 4 ints in XMM1 to XMM0.
//!   cb.movdqu(x86::ptr(dst), vec0);     // Store the result to [dst].
//!
//!   // Remember epilog insertion point.
//!   BaseNode* epilog_insertion_point = cb.cursor();
//!
//!   // Let's see what we have now.
//!   dump_code(cb, "Raw Function");
//!
//!   // Now, after we emitted the function body, we can insert the prolog, arguments
//!   // allocation, and epilog. This is not possible with using pure x86::Assembler.
//!   FuncFrame frame;
//!   frame.init(func);
//!
//!   // Make XMM0 and XMM1 dirty; RegGroup::kVec describes XMM|YMM|ZMM registers.
//!   frame.set_dirty_regs(RegGroup::kVec, IntUtils::mask(0, 1));
//!
//!   FuncArgsAssignment args(&func);     // Create arguments assignment context.
//!   args.assign_all(dst, src_a, src_b); // Assign our registers to arguments.
//!   args.update_func_frame(frame);      // Reflect our args in FuncFrame.
//!   frame.finalize();                   // Finalize the FuncFrame (updates it).
//!
//!   // Insert function prolog and allocate arguments to registers.
//!   cb.set_cursor(prolog_insertion_point);
//!   cb.emit_prolog(frame);
//!   cb.emit_args_assignment(frame, args);
//!
//!   // Insert function epilog.
//!   cb.set_cursor(epilog_insertion_point);
//!   cb.emit_epilog(frame);
//!
//!   // Let's see how the function's prolog and epilog looks.
//!   dump_code(cb, "Prolog & Epilog");
//!
//!   // IMPORTANT: Builder requires finalize() to be called to serialize its
//!   // code to the Assembler (it automatically creates one if not attached).
//!   cb.finalize();
//!
//!   SumIntsFunc fn;
//!   Error err = rt.add(&fn, &code);     // Add the generated code to the runtime.
//!
//!   if (err != Error::kOk) {
//!     return 1;                         // Handle a possible error case.
//!   }
//!
//!   // Execute the generated function.
//!   int in_a[4] = { 4, 3, 2, 1 };
//!   int in_b[4] = { 1, 5, 2, 8 };
//!   int out[4];
//!   fn(out, in_a, in_b);
//!
//!   // Prints {5 8 4 9}
//!   printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);
//!
//!   rt.release(fn);                     // Explicitly remove the function from the runtime.
//!   return 0;
//! }
//! ```
//!
//! When the example is executed it should output the following (this one using AMD64-SystemV ABI):
//!
//! ```
//! Raw Function:
//! movdqu xmm0, [rcx]
//! movdqu xmm1, [rdx]
//! paddd xmm0, xmm1
//! movdqu [rax], xmm0
//!
//! Prolog & Epilog:
//! mov rax, rdi
//! mov rcx, rsi
//! movdqu xmm0, [rcx]
//! movdqu xmm1, [rdx]
//! paddd xmm0, xmm1
//! movdqu [rax], xmm0
//! ret
//!
//! {5 8 4 9}
//! ```
//!
//! The number of use-cases of \ref BaseBuilder is not limited and highly depends on your creativity and experience.
//! The previous example can be easily improved to collect all dirty registers inside the function programmatically
//! and to pass them to \ref FuncFrame::set_dirty_regs().
//!
//! ```
//! #include <asmjit/x86.h>
//!
//! using namespace asmjit;
//!
//! // NOTE: This function doesn't cover all possible constructs. It ignores instructions that write
//! // to implicit registers that are not part of the operand list. It also counts read-only registers.
//! // Real implementation would be a bit more complicated, but still relatively easy to implement.
//! static void collect_dirty_regs(const BaseNode* first,
//!                                const BaseNode* last,
//!                                Support::Array<RegMask, Globals::kNumVirtGroups>& reg_mask) {
//!   const BaseNode* node = first;
//!   while (node) {
//!     if (node->is_inst()) {
//!       for (Operand& op : node->as<InstNode>()->operands()) {
//!         if (op.is_reg()) {
//!           const Reg& reg = op.as<Reg>();
//!           if (reg.group() <= RegGroup::kMaxVirt) {
//!             reg_mask[reg.group()] |= 1u << reg.id();
//!           }
//!         }
//!       }
//!     }
//!
//!     if (node == last) {
//!       break;
//!     }
//!     node = node->next();
//!   }
//! }
//!
//! static void set_dirty_regs_of_func_frame(const x86::Builder& builder, FuncFrame& frame) {
//!   Support::Array<RegMask, Globals::kNumVirtGroups> reg_mask {};
//!   collect_dirty_regs(builder.first_node(), builder.last_node(), reg_mask);
//!
//!   // X86/X64 ABIs only require to save GP/XMM registers:
//!   frame.set_dirty_regs(RegGroup::kGp, reg_mask[RegGroup::kGp]);
//!   frame.set_dirty_regs(RegGroup::kVec, reg_mask[RegGroup::kVec]);
//! }
//! ```
//!
//! ### Casting Between Various Emitters
//!
//! Even when \ref BaseAssembler and \ref BaseBuilder provide the same interface as defined by \ref BaseEmitter their
//! platform dependent variants like \ref x86::Assembler and \ref x86::Builder cannot be interchanged or casted to each
//! other by using a C++ `static_cast<>`. The main reason is the inheritance graph of these classes is different and
//! cast-incompatible, as illustrated below:
//!
//! ```
//!                                             +--------------+      +=========================+
//!                    +----------------------->| x86::Emitter |<--+--# x86::EmitterImplicitT<> #<--+
//!                    |                        +--------------+   |  +=========================+   |
//!                    |                           (abstract)      |           (mixin)              |
//!                    |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
//!                    +-->| BaseAssembler|---->|x86::Assembler|<--+                                |
//!                    |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
//!                    |      (abstract)            (final)        |                                |
//! +===============+  |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
//! #  BaseEmitter  #--+-->|  BaseBuilder |--+->| x86::Builder |<--+                                |
//! +===============+      +--------------+  |  +~~~~~~~~~~~~~~+                                    |
//!    (abstract)             (abstract)     |      (final)                                         |
//!                    +---------------------+                                                      |
//!                    |                                                                            |
//!                    |   +--------------+     +~~~~~~~~~~~~~~+      +=========================+   |
//!                    +-->| BaseCompiler |---->| x86::Compiler|<-----# x86::EmitterExplicitT<> #---+
//!                        +--------------+     +~~~~~~~~~~~~~~+      +=========================+
//!                           (abstract)            (final)                   (mixin)
//! ```
//!
//! The graph basically shows that it's not possible to cast between \ref x86::Assembler and \ref x86::Builder.
//! However, since both share the base interface (\ref BaseEmitter) it's possible to cast them to a class that
//! cannot be instantiated, but defines the same interface - the class is called \ref x86::Emitter and was
//! introduced to make it possible to write a function that can emit to both \ref x86::Assembler and \ref
//! x86::Builder. Note that \ref x86::Emitter cannot be created, it's abstract and has private constructors and
//! destructors; it was only designed to be casted to and used as an interface.
//!
//! Each architecture-specific emitter implements a member function called
//! `as<arch::Emitter>()`, which casts the instance to the architecture
//! specific emitter as illustrated below:
//!
//! ```
//! #include <asmjit/x86.h>
//!
//! using namespace asmjit;
//!
//! static void emit_something(x86::Emitter* e) {
//!   e->mov(x86::eax, x86::ebx);
//! }
//!
//! static void assemble(CodeHolder& code, bool use_asm) {
//!   if (use_asm) {
//!     x86::Assembler assembler(&code);
//!     emit_something(assembler.as<x86::Emitter>());
//!   }
//!   else {
//!     x86::Builder builder(&code);
//!     emit_something(builder.as<x86::Emitter>());
//!
//!     // NOTE: Builder requires `finalize()` to be called to serialize its
//!     // content to Assembler (it automatically creates one if not attached).
//!     builder.finalize();
//!   }
//! }
//! ```
//!
//! The example above shows how to create a function that can emit code to either \ref x86::Assembler or \ref
//! x86::Builder through \ref x86::Emitter, which provides emitter-neutral functionality. \ref x86::Emitter,
//! however, doesn't provide any emitter-specific functionality like `set_cursor()`.
//!
//! ### Code Injection and Manipulation
//!
//! \ref BaseBuilder emitter stores its nodes in a double-linked list, which makes it easy to manipulate that
//! list during the code generation or afterwards. Each node is always emitted next to the current cursor and
//! the cursor is advanced to that newly emitted node. The cursor can be retrieved and changed by \ref
//! BaseBuilder::cursor() and \ref BaseBuilder::set_cursor(), respectively.
//!
//! The example below demonstrates how to remember a node and inject something
//! next to it.
//!
//! ```
//! static void example(x86::Builder& builder) {
//!   // Emit something, after it returns the cursor would point at the last
//!   // emitted node.
//!   builder.mov(x86::rax, x86::rdx); // [1]
//!
//!   // We can retrieve the node.
//!   BaseNode* node = builder.cursor();
//!
//!   // Change the instruction we just emitted, just for fun...
//!   if (node->is_inst()) {
//!     InstNode* inst = node->as<InstNode>();
//!     // Changes the operands at index [1] to RCX.
//!     inst->set_op(1, x86::rcx);
//!   }
//!
//!   // ------------------------- Generate Some Code -------------------------
//!   builder.add(x86::rax, x86::rdx); // [2]
//!   builder.shr(x86::rax, 3);        // [3]
//!   // ----------------------------------------------------------------------
//!
//!   // Now, we know where our node is, and we can simply change the cursor
//!   // and start emitting something after it. The set_cursor() function
//!   // returns the previous cursor, and it's always a good practice to remember
//!   // it, because you never know if you are not already injecting the code
//!   // somewhere else...
//!   BaseNode* old_cursor = builder.set_cursor(node);
//!
//!   builder.mul(x86::rax, 8);        // [4]
//!
//!   // Restore the cursor
//!   builder.set_cursor(old_cursor);
//! }
//! ```
//!
//! The function above would actually emit the following:
//!
//! ```
//! mov rax, rcx ; [1] Patched at the beginning.
//! mul rax, 8   ; [4] Injected.
//! add rax, rdx ; [2] Followed [1] initially.
//! shr rax, 3   ; [3] Follows [2].
//! ```
class ASMJIT_VIRTAPI Builder
  : public BaseBuilder,
    public EmitterImplicitT<Builder> {
public:
  ASMJIT_NONCOPYABLE(Builder)
  using Base = BaseBuilder;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Builder(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Builder() noexcept override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
#endif // ASMJIT_X86_X86BUILDER_H_INCLUDED
