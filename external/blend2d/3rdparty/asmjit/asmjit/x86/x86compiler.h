// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86COMPILER_H_INCLUDED
#define ASMJIT_X86_X86COMPILER_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_COMPILER

#include <asmjit/core/compiler.h>
#include <asmjit/core/type.h>
#include <asmjit/x86/x86emitter.h>

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

//! X86/X64 compiler implementation.
//!
//! ### Compiler Basics
//!
//! The first \ref x86::Compiler example shows how to generate a function that simply returns an integer value. It's
//! an analogy to the first Assembler example:
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! int main() {
//!   using Func = int (*)(void);              // Signature of the generated function.
//!
//!   JitRuntime rt;                           // Runtime specialized for JIT code execution.
//!   CodeHolder code;                         // Holds code and relocation information.
//!
//!   code.init(rt.environment(),              // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Compiler cc(&code);                 // Create and attach x86::Compiler to code.
//!
//!   cc.add_func(FuncSignature::build<int>());// Begin a function of `int fn(void)` signature.
//!
//!   x86::Gp virt_reg = cc.new_gp32();        // Create a 32-bit general purpose register.
//!   cc.mov(virt_reg, 1);                     // Move one to our virtual register `virt_reg`.
//!   cc.ret(virt_reg);                        // Return `virt_reg` from the function.
//!
//!   cc.end_func();                           // End of the function body.
//!   cc.finalize();                           // Translate and assemble the whole 'cc' content.
//!   // ----> x86::Compiler is no longer needed from here and can be destroyed <----
//!
//!   Func fn;
//!   Error err = rt.add(&fn, &code);          // Add the generated code to the runtime.
//!   if (err != Error::kOk) {
//!     return 1;                              // Handle a possible error returned by AsmJit.
//!   }
//!   // ----> CodeHolder is no longer needed from here and can be destroyed <----
//!
//!   int result = fn();                       // Execute the generated code.
//!   printf("%d\n", result);                  // Print the resulting "1".
//!
//!   rt.release(fn);                          // Explicitly remove the function from the runtime.
//!   return 0;
//! }
//! ```
//!
//! The \ref BaseCompiler::add_func() and \ref BaseCompiler::end_func() functions are used to define the function and
//! its end. Both must be called per function, but the body doesn't have to be generated in sequence. An example of
//! generating two functions will be shown later. The next example shows more complicated code that contain a loop
//! and generates a simple memory copy function that uses `uint32_t` items:
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! int main() {
//!   // Signature of the generated function.
//!   using MemCpy32 = void (*)(uint32_t* dst, const uint32_t* src, size_t count);
//!
//!   JitRuntime rt;                           // Runtime specialized for JIT code execution.
//!   CodeHolder code;                         // Holds code and relocation information.
//!
//!   code.init(rt.environment(),              // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Compiler cc(&code);                 // Create and attach x86::Compiler to code.
//!
//!   FuncNode* func_node = cc.add_func(       // Begin the function of the following signature:
//!     FuncSignature::build<void,             //   Return value - void      (no return value).
//!       uint32_t*,                           //   1st argument - uint32_t* (machine reg-size).
//!       const uint32_t*,                     //   2nd argument - uint32_t* (machine reg-size).
//!       size_t>());                          //   3rd argument - size_t    (machine reg-size).
//!
//!   Label L_Loop = cc.new_label();           // Start of the loop.
//!   Label L_Exit = cc.new_label();           // Used to exit early.
//!
//!   x86::Gp dst = cc.new_gp_ptr("dst");      // Create `dst` register (destination pointer).
//!   x86::Gp src = cc.new_gp_ptr("src");      // Create `src` register (source pointer).
//!   x86::Gp i = cc.new_gp_ptr("i");          // Create `i` register (loop counter).
//!
//!   func_node->set_arg(0, dst);              // Assign `dst` argument.
//!   func_node->set_arg(1, src);              // Assign `src` argument.
//!   func_node->set_arg(2, i);                // Assign `i` argument.
//!
//!   cc.test(i, i);                           // Early exit if length is zero.
//!   cc.jz(L_Exit);
//!
//!   cc.bind(L_Loop);                         // Bind the beginning of the loop here.
//!
//!   x86::Gp tmp = cc.new_gp32("tmp");        // Copy a single dword (4 bytes).
//!   cc.mov(tmp, x86::dword_ptr(src));        // Load DWORD from [src] address.
//!   cc.mov(x86::dword_ptr(dst), tmp);        // Store DWORD to [dst] address.
//!
//!   cc.add(src, 4);                          // Increment `src`.
//!   cc.add(dst, 4);                          // Increment `dst`.
//!
//!   cc.dec(i);                               // Loop until `i` is non-zero.
//!   cc.jnz(L_Loop);
//!
//!   cc.bind(L_Exit);                         // Label used by early exit.
//!   cc.end_func();                           // End of the function body.
//!
//!   cc.finalize();                           // Translate and assemble the whole 'cc' content.
//!
//!   // ----> x86::Compiler is no longer needed from here and can be destroyed <----
//!
//!   // Add the generated code to the runtime.
//!   MemCpy32 memcpy32;
//!   Error err = rt.add(&memcpy32, &code);
//!
//!   if (err != Error::kOk) {
//!     return 1;                              // Handle a possible error returned by AsmJit.
//!   }
//!
//!   // ----> CodeHolder is no longer needed from here and can be destroyed <----
//!
//!   // Test the generated code.
//!   uint32_t input[6] = { 1, 2, 3, 5, 8, 13 };
//!   uint32_t output[6];
//!   memcpy32(output, input, 6);
//!
//!   for (uint32_t i = 0; i < 6; i++) {
//!     printf("%d\n", output[i]);
//!   }
//!
//!   rt.release(memcpy32);
//!   return 0;
//! }
//! ```
//!
//! ### AVX and AVX-512
//!
//! AVX and AVX-512 code generation must be explicitly enabled via \ref FuncFrame to work properly. If it's not setup
//! correctly then Prolog & Epilog would use SSE instead of AVX instructions to work with SIMD registers. In addition,
//! Compiler requires explicitly enable AVX-512 via \ref FuncFrame in order to use all 32 SIMD registers.
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! int main() {
//!   using Func = void (*)(void*);             // Signature of the generated function.
//!
//!   JitRuntime rt;                            // Runtime specialized for JIT code execution.
//!   CodeHolder code;                          // Holds code and relocation information.
//!
//!   code.init(rt.environment(),               // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Compiler cc(&code);                  // Create and attach x86::Compiler to code.
//!
//!   FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*>());
//!
//!   // Use the following to enable AVX and/or AVX-512.
//!   func_node->frame().set_avx_enabled();
//!   func_node->frame().set_avx512_enabled();
//!
//!   // Do something with the input pointer.
//!   x86::Gp addr = cc.new_gp_ptr("addr");
//!   x86::Vec vreg = cc.new_zmm("vreg");
//!
//!   func_node->set_arg(0, addr);
//!
//!   cc.vmovdqu32(vreg, x86::ptr(addr));
//!   cc.vpaddq(vreg, vreg, vreg);
//!   cc.vmovdqu32(x86::ptr(addr), vreg);
//!
//!   cc.end_func();                            // End of the function body.
//!   cc.finalize();                            // Translate and assemble the whole 'cc' content.
//!   // ----> x86::Compiler is no longer needed from here and can be destroyed <----
//!
//!   Func fn;
//!   Error err = rt.add(&fn, &code);           // Add the generated code to the runtime.
//!
//!   if (err != Error::kOk) {
//!     return 1;                               // Handle a possible error returned by AsmJit.
//!   }
//!   // ----> CodeHolder is no longer needed from here and can be destroyed <----
//!
//!   // Execute the generated code and print some output.
//!   uint64_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
//!   fn(data);
//!   printf("%llu\n", (unsigned long long)data[0]);
//!
//!   rt.release(fn);                           // Explicitly remove the function from the runtime.
//!   return 0;
//! }
//! ```
//!
//! ### Recursive Functions
//!
//! It's possible to create more functions by using the same \ref x86::Compiler instance and make links between them.
//! In such case it's important to keep the pointer to \ref FuncNode.
//!
//! The example below creates a simple Fibonacci function that calls itself recursively:
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! int main() {
//!   using FibFn = uint32_t (*)(uint32_t x);   // Signature of the generated function.
//!
//!   JitRuntime rt;                            // Runtime specialized for JIT code execution.
//!   CodeHolder code;                          // Holds code and relocation information.
//!
//!   code.init(rt.environment(),               // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Compiler cc(&code);                  // Create and attach x86::Compiler to code.
//!
//!   FuncNode* func_node = cc.add_func(        // Begin of the Fibonacci function, add_func()
//!     FuncSignature::build<int, int>());      // Returns a pointer to the FuncNode node.
//!
//!   Label L_Exit = cc.new_label();            // Exit label.
//!   x86::Gp x = cc.new_gp32();                // Function x argument.
//!   x86::Gp y = cc.new_gp32();                // Temporary.
//!
//!   func_node->set_arg(0, x);
//!
//!   cc.cmp(x, 3);                             // Return x if less than 3.
//!   cc.jb(L_Exit);
//!
//!   cc.mov(y, x);                             // Make copy of the original x.
//!   cc.dec(x);                                // Decrease x.
//!
//!   InvokeNode* invoke_node;                  // Function invocation:
//!   cc.invoke(Out(invoke_node),               //   - InvokeNode (output).
//!     func_node->label(),                     //   - Function address or Label.
//!     FuncSignature::build<int, int>());      //   - Function signature.
//!
//!   invoke_node->set_arg(0, x);               // Assign x as the first argument.
//!   invoke_node->set_ret(0, x);               // Assign x as a return value as well.
//!
//!   cc.add(x, y);                             // Combine the return value with y.
//!
//!   cc.bind(L_Exit);
//!   cc.ret(x);                                // Return x.
//!   cc.end_func();                            // End of the function body.
//!
//!   cc.finalize();                            // Translate and assemble the whole 'cc' content.
//!   // ----> x86::Compiler is no longer needed from here and can be destroyed <----
//!
//!   FibFn fib;
//!   Error err = rt.add(&fib, &code);          // Add the generated code to the runtime.
//!
//!   if (err != Error::kOk) {
//!     return 1;                               // Handle a possible error returned by AsmJit.
//!   }
//!   // ----> CodeHolder is no longer needed from here and can be destroyed <----
//!
//!   // Test the generated code.
//!   printf("Fib(%u) -> %u\n", 8, fib(8));
//!
//!   rt.release(fib);
//!   return 0;
//! }
//! ```
//!
//! ### Stack Management
//!
//! Function's stack-frame is managed automatically, which is used by the register allocator to spill virtual
//! registers. It also provides an interface to allocate user-defined block of the stack, which can be used as
//! a temporary storage by the generated function. In the following example a stack of 256 bytes size is allocated,
//! filled by bytes starting from 0 to 255 and then iterated again to sum all the values.
//!
//! ```
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! int main() {
//!   using Func = int (*)(void);               // Signature of the generated function.
//!
//!   JitRuntime rt;                            // Runtime specialized for JIT code execution.
//!   CodeHolder code;                          // Holds code and relocation information.
//!
//!   code.init(rt.environment(),               // Initialize code to match the JIT environment.
//!             rt.cpu_features());
//!   x86::Compiler cc(&code);                  // Create and attach x86::Compiler to code.
//!
//!   cc.add_func(FuncSignature::build<int>()); // Create a function that returns int.
//!
//!   x86::Gp p = cc.new_gp_ptr("p");
//!   x86::Gp i = cc.new_gp_ptr("i");
//!
//!   // Allocate 256 bytes on the stack aligned to 4 bytes.
//!   x86::Mem stack = cc.new_stack(256, 4);
//!
//!   x86::Mem stack_idx(stack);                // Copy of stack with i added.
//!   stack_idx.set_index(i);                   // stack_idx <- stack[i].
//!   stack_idx.set_size(1);                    // stack_idx <- byte ptr stack[i].
//!
//!   // Load a stack address to `p`. This step is purely optional and shows
//!   // that `lea` is useful to load a memory operands address (even absolute)
//!   // to a general purpose register.
//!   cc.lea(p, stack);
//!
//!   // Clear i (xor is a C++ keyword, hence 'xor_' is used instead).
//!   cc.xor_(i, i);
//!
//!   Label L1 = cc.new_label();
//!   Label L2 = cc.new_label();
//!
//!   cc.bind(L1);                              // First loop, fill the stack.
//!   cc.mov(stack_idx, i.r8());                // stack[i] = uint8_t(i).
//!
//!   cc.inc(i);                                // i++;
//!   cc.cmp(i, 256);                           // if (i < 256)
//!   cc.jb(L1);                                //   goto L1;
//!
//!   // Second loop, sum all bytes stored in `stack`.
//!   x86::Gp sum = cc.new_gp32("sum");
//!   x86::Gp val = cc.new_gp32("val");
//!
//!   cc.xor_(i, i);
//!   cc.xor_(sum, sum);
//!
//!   cc.bind(L2);
//!
//!   cc.movzx(val, stack_idx);                 // val = uint32_t(stack[i]);
//!   cc.add(sum, val);                         // sum += val;
//!
//!   cc.inc(i);                                // i++;
//!   cc.cmp(i, 256);                           // if (i < 256)
//!   cc.jb(L2);                                //   goto L2;
//!
//!   cc.ret(sum);                              // Return the `sum` of all values.
//!   cc.end_func();                            // End of the function body.
//!
//!   cc.finalize();                            // Translate and assemble the whole 'cc' content.
//!   // ----> x86::Compiler is no longer needed from here and can be destroyed <----
//!
//!   Func func;
//!   Error err = rt.add(&func, &code);         // Add the generated code to the runtime.
//!
//!   if (err != Error::kOk) {
//!     return 1;                               // Handle a possible error returned by AsmJit.
//!   }
//!   // ----> CodeHolder is no longer needed from here and can be destroyed <----
//!
//!   printf("Func() -> %d\n", func());         // Test the generated code.
//!
//!   rt.release(func);
//!   return 0;
//! }
//! ```
//!
//! ### Constant Pool
//!
//! Compiler provides two constant pools for a general purpose code generation:
//!
//!   - Local constant pool - Part of \ref FuncNode, can be only used by a single function and added after the
//!     function epilog sequence (after `ret` instruction).
//!
//!   - Global constant pool - Part of \ref BaseCompiler, flushed at the end of the generated code by \ref
//!     BaseEmitter::finalize().
//!
//! The example below illustrates how a built-in constant pool can be used:
//!
//! ```
//! #include <asmjit/x86.h>
//!
//! using namespace asmjit;
//!
//! static void example_use_of_const_pool(x86::Compiler& cc) {
//!   cc.add_func(FuncSignature::build<int>());
//!
//!   x86::Gp v0 = cc.new_gp32("v0");
//!   x86::Gp v1 = cc.new_gp32("v1");
//!
//!   x86::Mem c0 = cc.new_int32_const(ConstPoolScope::kLocal, 200);
//!   x86::Mem c1 = cc.new_int32_const(ConstPoolScope::kLocal, 33);
//!
//!   cc.mov(v0, c0);
//!   cc.mov(v1, c1);
//!   cc.add(v0, v1);
//!
//!   cc.ret(v0);
//!   cc.end_func();
//! }
//! ```
//!
//! ### Jump Tables
//!
//! x86::Compiler supports `jmp` instruction with reg/mem operand, which is a commonly used pattern to implement
//! indirect jumps within a function, for example to implement `switch()` statement in a programming languages.
//! By default AsmJit assumes that every basic block can be a possible jump target as it's unable to deduce targets
//! from instruction's operands. This is a very pessimistic default that should be avoided if possible as it's costly
//! and very unfriendly to liveness analysis and register allocation.
//!
//! Instead of relying on such pessimistic default behavior, let's use \ref JumpAnnotation to annotate a jump where
//! all targets are known:
//!
//! ```
//! #include <asmjit/x86.h>
//!
//! using namespace asmjit;
//!
//! static void example_use_of_indirect_jump(x86::Compiler& cc) {
//!   FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float, uint32_t>());
//!
//!   // Function arguments
//!   x86::Vec a = cc.new_xmm_ss("a");
//!   x86::Vec b = cc.new_xmm_ss("b");
//!   x86::Gp op = cc.new_gp32("op");
//!
//!   x86::Gp target = cc.new_gp_ptr("target");
//!   x86::Gp offset = cc.new_gp_ptr("offset");
//!
//!   Label L_Table = cc.new_label();
//!   Label L_Add = cc.new_label();
//!   Label L_Sub = cc.new_label();
//!   Label L_Mul = cc.new_label();
//!   Label L_Div = cc.new_label();
//!   Label L_End = cc.new_label();
//!
//!   func_node->set_arg(0, a);
//!   func_node->set_arg(1, b);
//!   func_node->set_arg(2, op);
//!
//!   // Jump annotation is a building block that allows to annotate all possible targets where `jmp()` can
//!   // jump. It then drives the CFG construction and liveness analysis, which impacts register allocation.
//!   JumpAnnotation* annotation = cc.new_jump_annotation();
//!   annotation->add_label(L_Add);
//!   annotation->add_label(L_Sub);
//!   annotation->add_label(L_Mul);
//!   annotation->add_label(L_Div);
//!
//!   // Most likely not the common indirect jump approach, but it
//!   // doesn't really matter how final address is calculated. The
//!   // most important path using JumpAnnotation with `jmp()`.
//!   cc.lea(offset, x86::ptr(L_Table));
//!   if (cc.is_64bit())
//!     cc.movsxd(target, x86::dword_ptr(offset, op.clone_as(offset), 2));
//!   else
//!     cc.mov(target, x86::dword_ptr(offset, op.clone_as(offset), 2));
//!   cc.add(target, offset);
//!   cc.jmp(target, annotation);
//!
//!   // Acts like a switch() statement in C.
//!   cc.bind(L_Add);
//!   cc.addss(a, b);
//!   cc.jmp(L_End);
//!
//!   cc.bind(L_Sub);
//!   cc.subss(a, b);
//!   cc.jmp(L_End);
//!
//!   cc.bind(L_Mul);
//!   cc.mulss(a, b);
//!   cc.jmp(L_End);
//!
//!   cc.bind(L_Div);
//!   cc.divss(a, b);
//!
//!   cc.bind(L_End);
//!   cc.ret(a);
//!
//!   cc.end_func();
//!
//!   // Relative int32_t offsets of `L_XXX - L_Table`.
//!   cc.bind(L_Table);
//!   cc.embed_label_delta(L_Add, L_Table, 4);
//!   cc.embed_label_delta(L_Sub, L_Table, 4);
//!   cc.embed_label_delta(L_Mul, L_Table, 4);
//!   cc.embed_label_delta(L_Div, L_Table, 4);
//! }
//! ```
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! Creates a new general-purpose register with `type_id` type and optional name passed via `args`.
  //!
  //! \note Using \ref TypeId is too generic. In general it's recommended to use \ref new_gp8(),
  //! \ref new_gp16(), \ref new_gp32(), \ref new_gp64(), and \ref new_gpz() or \ref new_gp_ptr().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp(TypeId type_id, Args&&... args) { return new_reg<Gp>(type_id, std::forward<Args>(args)...); }

  //! Creates a new vector register with `type_id` type and optional name passed via `args`.
  //!
  //! \note Using \ref TypeId is too generic. In general it's recommended to use \ref new_vec128(),
  //! \ref new_vec256(), \ref new_vec512(), or alternatively \ref new_xmm(), \ref new_ymm(), and \ref new_zmm().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec(TypeId type_id, Args&&... args) { return new_reg<Vec>(type_id, std::forward<Args>(args)...); }

  //! Creates a new mask register with `type_id` type and optional name passed via `args`.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k(TypeId type_id, Args&&... args) { return new_reg<KReg>(type_id, std::forward<Args>(args)...); }

  //! Creates a new 8-bit general purpose register mapped to low 8 bits of a full register.
  //!
  //! \note Using 8-bit registers is not recommended, use at least 32-bit registers in portable code.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp8(Args&&... args) { return new_reg<Gp>(TypeId::kUInt8, std::forward<Args>(args)...); }

  //! Creates a new 16-bit general purpose register mapped to low 16 bits of a full register.
  //!
  //! \note Using 16-bit registers is not recommended, use at least 32-bit registers in portable code.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp16(Args&&... args) { return new_reg<Gp>(TypeId::kUInt16, std::forward<Args>(args)...); }

  //! Creates a new 32-bit general purpose register mapped to low 32 bits of a full register (on 64-bit targets).
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp32(Args&&... args) { return new_reg<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }

  //! Creates a new 64-bit general purpose register.
  //!
  //! \warning The target must be 64-bit in order to create 64-bit registers.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp64(Args&&... args) { return new_reg<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  //! Creates a new 32-bit or 64-bit general purpose register depending on the target register width.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpz(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 32-bit or 64-bit general purpose register depending on the target register width.
  //!
  //! \note This is just an alternative name that maps more closely to C's `uintptr_t`, it's the same function as
  //! \ref new_gpz().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp_ptr(Args&&... args) { return new_reg<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register (XMM).
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register (XMM) that will be used for scalar 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x1, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register (XMM) that will be used for scalar 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x1(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x1, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register (XMM) that will be used for packed 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f32x4(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x4, std::forward<Args>(args)...); }

  //! Creates a new 128-bit vector register (XMM) that will be used for packed 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec128_f64x2(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x2, std::forward<Args>(args)...); }

  //! Creates a new 256-bit vector register (YMM).
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x8, std::forward<Args>(args)...); }

  //! Creates a new 256-bit vector register (YMM) that will be used for packed 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256_f32x8(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x8, std::forward<Args>(args)...); }

  //! Creates a new 256-bit vector register (YMM) that will be used for packed 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec256_f64x4(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x4, std::forward<Args>(args)...); }

  //! Creates a new 512-bit vector register (ZMM).
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x16, std::forward<Args>(args)...); }

  //! Creates a new 512-bit vector register (ZMM) that will be used for packed 32-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512_f32x16(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x16, std::forward<Args>(args)...); }

  //! Creates a new 512-bit vector register (ZMM) that will be used for packed 64-bit floating point operation.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec512_f64x8(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x8, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x4, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128_f32x1() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_ss(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x1, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128_f64x1() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_sd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x1, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128_f32x4() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x4, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec128_f64x2() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_xmm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x2, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec256() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x8, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec256_f32x8() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x8, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec256_f64x4() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_ymm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x4, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec512() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm(Args&&... args) { return new_reg<Vec>(TypeId::kInt32x16, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec512_f32x16() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm_ps(Args&&... args) { return new_reg<Vec>(TypeId::kFloat32x16, std::forward<Args>(args)...); }

  //! Alias of \ref new_vec512_f64x8() that matches x86 architecture terminology.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_zmm_pd(Args&&... args) { return new_reg<Vec>(TypeId::kFloat64x8, std::forward<Args>(args)...); }

  //! Creates a new 64-bit MMX register.
  //!
  //! \note MMX ISA is generally deprecated by the X86 architecture.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Mm new_mm(Args&&... args) { return new_reg<Mm>(TypeId::kMmx64, std::forward<Args>(args)...); }

  //! Creates a new 8-bit mask (K) register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k8(Args&&... args) { return new_reg<KReg>(TypeId::kMask8, std::forward<Args>(args)...); }

  //! Creates a new 16-bit mask (K) register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k16(Args&&... args) { return new_reg<KReg>(TypeId::kMask16, std::forward<Args>(args)...); }

  //! Creates a new 32-bit mask (K) register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k32(Args&&... args) { return new_reg<KReg>(TypeId::kMask32, std::forward<Args>(args)...); }

  //! Creates a new 64-bit mask (K) register.
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_k64(Args&&... args) { return new_reg<KReg>(TypeId::kMask64, std::forward<Args>(args)...); }

  //! Creates a new 8-bit mask (K) register, alias of \ref new_k8().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kb(Args&&... args) { return new_reg<KReg>(TypeId::kMask8, std::forward<Args>(args)...); }

  //! Creates a new 16-bit mask (K) register, alias of \ref new_k16().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kw(Args&&... args) { return new_reg<KReg>(TypeId::kMask16, std::forward<Args>(args)...); }

  //! Creates a new 32-bit mask (K) register, alias of \ref new_k32().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kd(Args&&... args) { return new_reg<KReg>(TypeId::kMask32, std::forward<Args>(args)...); }

  //! Creates a new 64-bit mask (K) register, alias of \ref new_k64().
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG KReg new_kq(Args&&... args) { return new_reg<KReg>(TypeId::kMask64, std::forward<Args>(args)...); }

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new stack and returns a \ref Mem operand that can be used to address it.
  ASMJIT_INLINE_NODEBUG Mem new_stack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _new_stack(Out<BaseMem>{m}, size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE_NODEBUG Mem new_const(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _new_const(Out<BaseMem>(m), scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_byte_const(ConstPoolScope scope, uint8_t val) noexcept { return new_const(scope, &val, 1); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_word_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_dword_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_qword_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int16_const(ConstPoolScope scope, int16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint16_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int32_const(ConstPoolScope scope, int32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint32_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int64_const(ConstPoolScope scope, int64_t val) noexcept { return new_const(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint64_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_float_const(ConstPoolScope scope, float val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_double_const(ConstPoolScope scope, double val) noexcept { return new_const(scope, &val, 8); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  ASMJIT_INLINE_NODEBUG Compiler& unfollow() noexcept { add_inst_options(InstOptions::kUnfollow); return *this; }
  //! Tell the compiler that the destination variable will be overwritten.
  ASMJIT_INLINE_NODEBUG Compiler& overwrite() noexcept { add_inst_options(InstOptions::kOverwrite); return *this; }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Invoke a function call without `target` type enforcement.
  ASMJIT_INLINE_NODEBUG Error invoke_(Out<InvokeNode*> out, const Operand_& target, const FuncSignature& signature) {
    return add_invoke_node(out, Inst::kIdCall, target, signature);
  }

  //! Invoke a function call of the given `target` and `signature` and store the added node to `out`.
  //!
  //! Creates a new \ref InvokeNode, initializes all the necessary members to match the given function `signature`,
  //! adds the node to the compiler, and stores its pointer to `out`. The operation is atomic, if anything fails
  //! nullptr is stored in `out` and error code is returned.
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }

  //! Return from function.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret() { return add_ret(Operand(), Operand()); }

  //! Return from function - one value.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return add_ret(o0, Operand()); }

  //! Return from function - two values / register pair.
  //!
  //! \note This doesn't end the function - it just emits a return.
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0, const Reg& o1) { return add_ret(o0, o1); }

  //! \}

  //! \name Jump Tables Support
  //! \{

  using EmitterExplicitT<Compiler>::jmp;

  //! Adds a jump to the given `target` with the provided jump `annotation`.
  ASMJIT_INLINE_NODEBUG Error jmp(const Reg& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdJmp, target, annotation); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error jmp(const BaseMem& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdJmp, target, annotation); }

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_X86_X86COMPILER_H_INCLUDED
