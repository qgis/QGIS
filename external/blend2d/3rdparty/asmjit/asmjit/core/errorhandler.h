// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ERRORHANDLER_H_INCLUDED
#define ASMJIT_CORE_ERRORHANDLER_H_INCLUDED

#include <asmjit/core/globals.h>

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_error_handling
//! \{

class BaseEmitter;

//! Error handler can be used to override the default behavior of error handling.
//!
//! It's available to all classes that inherit `BaseEmitter`. Override \ref ErrorHandler::handle_error() to implement
//! your own error handler.
//!
//! The following use-cases are supported:
//!
//!   - Record the error and continue code generation. This is the simplest approach that can be used to at least log
//!     possible errors.
//!   - Throw an exception. AsmJit doesn't use exceptions and is completely exception-safe, but it's perfectly legal
//!     to throw an exception from the error handler.
//!   - Use plain old C's `setjmp()` and `longjmp()`. Asmjit always puts Assembler, Builder and Compiler to
//!     a consistent state before calling \ref handle_error(), so `longjmp()` can be used without issues to cancel the
//!     code generation if an error occurred. This method can be used if exception handling in your project is turned
//!     off and you still want some comfort. In most cases it should be safe as AsmJit uses \ref Arena allocator and
//!     the ownership of allocated memory it allocates always ends with the instance that allocated it. If using this
//!     approach please never jump outside the life-time of \ref CodeHolder and \ref BaseEmitter.
//!
//! \ref ErrorHandler can be attached to \ref CodeHolder or \ref BaseEmitter, which has a priority. The example below
//! uses error handler that just prints the error, but lets AsmJit continue:
//!
//! ```
//! // Error Handling #1 - Logging and returning Error.
//! #include <asmjit/x86.h>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! // Error handler that just prints the error and lets AsmJit ignore it.
//! class SimpleErrorHandler : public ErrorHandler {
//! public:
//!   Error err;
//!
//!   inline SimpleErrorHandler() : err(Error::kOk) {}
//!
//!   void handle_error(Error err, const char* message, BaseEmitter* origin) override {
//!     this->err = err;
//!     fprintf(stderr, "ERROR: %s\n", message);
//!   }
//! };
//!
//! int main() {
//!   JitRuntime rt;
//!   SimpleErrorHandler eh;
//!
//!   CodeHolder code;
//!   code.init(rt.environment(), rt.cpu_features());
//!   code.set_error_handler(&eh);
//!
//!   // Try to emit instruction that doesn't exist.
//!   x86::Assembler a(&code);
//!   a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);
//!
//!   if (eh.err) {
//!     // Assembler failed!
//!     return 1;
//!   }
//!
//!   return 0;
//! }
//! ```
//!
//! If error happens during instruction emitting / encoding the assembler behaves transactionally - the output buffer
//! won't advance if encoding failed, thus either a fully encoded instruction or nothing is emitted. The error handling
//! shown above is useful, but it's still not the best way of dealing with errors in AsmJit. The following example
//! shows how to use exception handling to handle errors in a more C++ way:
//!
//! ```
//! // Error Handling #2 - Throwing an exception.
//! #include <asmjit/x86.h>
//! #include <exception>
//! #include <string>
//! #include <stdio.h>
//!
//! using namespace asmjit;
//!
//! // Error handler that throws a user-defined `AsmJitException`.
//! class AsmJitException : public std::exception {
//! public:
//!   Error err;
//!   std::string message;
//!
//!   AsmJitException(Error err, const char* message) noexcept
//!     : err(err),
//!       message(message) {}
//!
//!   const char* what() const noexcept override { return message.c_str(); }
//! };
//!
//! class ThrowableErrorHandler : public ErrorHandler {
//! public:
//!   // Throw is possible, functions that use ErrorHandler are never 'noexcept'.
//!   void handle_error(Error err, const char* message, BaseEmitter* origin) override {
//!     throw AsmJitException(err, message);
//!   }
//! };
//!
//! int main() {
//!   JitRuntime rt;
//!   ThrowableErrorHandler eh;
//!
//!   CodeHolder code;
//!   code.init(rt.environment(), rt.cpu_features());
//!   code.set_error_handler(&eh);
//!
//!   x86::Assembler a(&code);
//!
//!   // Try to emit instruction that doesn't exist.
//!   try {
//!     a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);
//!   }
//!   catch (const AsmJitException& ex) {
//!     printf("EXCEPTION THROWN: %s\n", ex.what());
//!     return 1;
//!   }
//!
//!   return 0;
//! }
//! ```
//!
//! If C++ exceptions are not what you like or your project turns off them completely there is still a way of reducing
//! the error handling to a minimum by using a standard setjmp/longjmp approach. AsmJit is exception-safe and cleans
//! up everything before calling the ErrorHandler, so any approach is safe. You can simply jump from the error handler
//! without causing any side-effects or memory leaks. The following example demonstrates how it could be done:
//!
//! ```
//! // Error Handling #3 - Using setjmp/longjmp if exceptions are not allowed.
//! #include <asmjit/x86.h>
//! #include <setjmp.h>
//! #include <stdio.h>
//!
//! class LongJmpErrorHandler : public asmjit::ErrorHandler {
//! public:
//!   inline LongJmpErrorHandler() : err(asmjit::Error::kOk) {}
//!
//!   void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
//!     this->err = err;
//!     longjmp(state, 1);
//!   }
//!
//!   jmp_buf state;
//!   asmjit::Error err;
//! };
//!
//! int main(int argc, char* argv[]) {
//!   using namespace asmjit;
//!
//!   JitRuntime rt;
//!   LongJmpErrorHandler eh;
//!
//!   CodeHolder code;
//!   code.init(rt.environment(), rt.cpu_features());
//!   code.set_error_handler(&eh);
//!
//!   x86::Assembler a(&code);
//!
//!   if (!setjmp(eh.state)) {
//!     // Try to emit instruction that doesn't exist.
//!     a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);
//!   }
//!   else {
//!     Error err = eh.err;
//!     printf("ASMJIT ERROR: 0x%08X [%s]\n", err, DebugUtils::error_as_string(err));
//!   }
//!
//!   return 0;
//! }
//! ```
class ASMJIT_VIRTAPI ErrorHandler {
public:
  ASMJIT_BASE_CLASS(ErrorHandler)

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `ErrorHandler` instance.
  ASMJIT_API ErrorHandler() noexcept;
  //! Destroys the `ErrorHandler` instance.
  ASMJIT_API virtual ~ErrorHandler() noexcept;

  //! \}

  //! \name Interface
  //! \{

  //! Error handler (must be reimplemented).
  //!
  //! Error handler is called after an error happened and before it's propagated to the caller. There are multiple
  //! ways how the error handler can be used:
  //!
  //! 1. User-based error handling without throwing exception or using C's`longjmp()`. This is for users that don't
  //!     use exceptions and want customized error handling.
  //!
  //! 2. Throwing an exception. AsmJit doesn't use exceptions and is completely exception-safe, but you can throw
  //!     exception from your error handler if this way is the preferred way of handling errors in your project.
  //!
  //! 3. Using plain old C's `setjmp()` and `longjmp()`. Asmjit always puts `BaseEmitter` to a consistent state before
  //!    calling `handle_error()`  so `longjmp()` can be used without any issues to cancel the code generation if an
  //!    error occurred. There is no difference between exceptions and `longjmp()` from AsmJit's perspective, however,
  //!    never jump outside of `CodeHolder` and `BaseEmitter` scope as you would leak memory.
  ASMJIT_API virtual void handle_error(Error err, const char* message, BaseEmitter* origin);

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ERRORHANDLER_H_INCLUDED

