# Contributing to Blend2D

The following is a set of guidelines for contributing to Blend2D and other repositories hosted in the [Blend2D Organization](https://github.com/blend2d) on GitHub.


## Table of Contents

* [Asking Questions](#asking-questions)
* [Reporting Bugs](#reporting-bugs)
* [Website Enhancement](#website-enhancement)
* [Coding Style](#coding-style)


## Asking Questions

We prefer GitHub issues to be used for reporting bugs and similar problems. So please consider using our [Gitter Channel](https://gitter.im/blend2d/blend2d) to ask questions. It is active and may provide you quick help.

* **Generic questions** should be asked on [Gitter](https://gitter.im/blend2d/blend2d) if possible
* **Feature Suggestions** may be discussed on [Gitter](https://gitter.im/blend2d/blend2d), we will add features that match our future plans to the roadmap
* For **C/C++ API Questions** use either [Gitter](https://gitter.im/blend2d/blend2d) or open a new issue on GitHub (see pinned issues first, we prefer to write a documentation regarding API functions that are not clear)
* Read our [FAQ](https://blend2d.com/about.html#FAQ) as well, we would gladly improve it if you found something missing or can improve the existing content


## Reporting Bugs

A good bug report should contain the following information:

* Brief and detailed bug description
* Steps to reproduce the bug (isolated test-case is preferred but not required)
* Build and system information (use `<blend2d-debug.h>` to obtain it, see below)

To make the reporting bugs as easy as possible Blend2D provides a header file called `<blend2d-debug.h>`. It contains functions for C and C++ API users that can be used to query useful information from Blend2D runtime and to dump the content of some Blend2D objects.

Use the snippet below in your own code and include its output in your bug report. It will help us to identify the issue quicker. Debug API is provided for both C and C++ users.

```c
#include <blend2d/blend2d.h>
#include <blend2d-debug.h>

int main(int argc, char* argv[]) {
  // This will query and dump Blend2D build and system information. We
  // need this information to make the initial guess of where the problem
  // could be. Some JIT and SIMD bugs can depend on this information.
  bl_debug_runtime();

  // Now dump everything you have by using the following functions.
  bl_debug_array(&arr);          // Dumps the content of BLArray
  bl_debug_context(&ctx);        // Dumps the state of BLContext
  bl_debug_matrix2d(&mat);       // Dumps the content of BLMatrix2D
  bl_debug_image(&img);          // Dumps the content of BLImage (without pixels)
  bl_debug_path(&path);          // Dumps the content of BLPath
  bl_debug_stroke_options(&opt); // Dumps the content of BLStrokeOptions
}
```

When reporting bugs related to incorrect renderings it is important to also include the rendered image together with the bug report as it will help us to understand the problem better.


## Website Enhancement

If you found a typo or have suggestions about the content on Blend2D website you can either contact us directly via email or discuss it on Gitter. Please don't open GitHub issues that are related to the website.


## Coding Style

If you plan to contribute to Blend2D make sure you follow the guidelines described in this section.


### Source Files

* Source files contain a header, which contains a project name `[Blend2D]`, description, and license that refers to `LICENSE.md` file (when you are creating a new file just copy the header from any other file)
* Source files that include other Blend2D headers use relative paths starting with `./` or `../`
* Header guard format is `BLEND2D_FILE_H` for root headers and `BLEND2D_PATH_FILE_H` for headers in a subdirectory
* Source files (.cpp) must first include `api-build_p.h` and then other headers
* Source files that use compiler intrinsics (SSE, AVX, NEON) must have the following suffix:
  * X86/X64
    * `*_sse2.cpp`         - SSE2
    * `*_sse3.cpp`         - SSE3
    * `*_ssse3.cpp`        - SSSE3
    * `*_sse4_1.cpp`       - SSE4.1
    * `*_sse4_2.cpp`       - SSE4.2 + POPCNT + PCLMULQDQ
    * `*_avx.cpp`          - AVX
    * `*_avx2.cpp`         - AVX2   + POPCNT + BMI + BMI2
    * `*_avx2fma.cpp`      - AVX2   + POPCNT + BMI + BMI2 + FMA
    * `*_avx512.cpp`       - AVX512 + POPCNT + BMI + BMI2
  * ARM/AArch64
    * `*_asimd.cpp`        - ASIMD
    * `*_asimd_crypto.cpp` - ASIMD + CRYPTO


### API Design

#### Namespaces

* Do not use namespaces in public API if possible

Namespaces are used mostly internally. There are few exceptions like `BLRuntime` and `BLFileSystem` though.


#### C vs C++ API

* Public headers must place C++ specific functionality between `#ifdef __cplusplus` and `#endif`
* Private headers use C++ by default (no C compatibility)

Since Blend2D exports only C API and can be compiled without linking to standard C++ library it cannot use functions that depend on it. Blend2D at the moment only uses atomics and some other utilities like `std::numeric_limits<>` that don't require linking to C++ standard library and it must stay like that.


#### Exception Safety and Error Handling

* Do not use exceptions or RTTI

Exceptions and RTTI are never used by Blend2D. In general every public function is marked `noexcept` (or `BL_NOEXCEPT_C` for C API) that should guarantee that the compiler won't emit unwind tables even when C++ exceptions are enabled at build time. Blend2D users can use exceptions in their code, but Blend2D would never throw them.

* Use error codes for error handling and propagation
* Use `BLResult` as a return value in functions that can fail

Every function that can fail must return `BLResult`. Use `BL_PROPAGATE(<expression>)` to return on failure, but be careful and check how it's used first.


#### Default Constructed State

  * Always offer a defined default constructed state.

Default constructed state guarantees that no dynamic memory is allocated when creating a default constructed instance. Only initialization like `create()` or `begin()` and using setters would turn default initialized instance into an instance that could use dynamically allocated memory. Use `.reset()` to set the state of any class back to its default constructed state and to release all resources it holds.


### Coding Conventions

If you are planning to contribute to Blend2D, please read our coding conventions carefully.

* Indent by 2 spaces and never use TABs
* Class and struct names use *Upper Camel Case* convention and always start with `BL` prefix (`BLClassName`)
* Global functions and variables use *snake_case* convention and always start with `bl_` prefix (`bl_function_name`)
* Structs are used for everything that doesn't have initialization and must be compatible with C API
* Structs can have utility member functions available in C++ mode like `.reset()`, but cannot have constructors, destructors, or assignment operators
* Classes are only used for implementing C++ API that is based on C API
* Pointer `*` or reference `&` is part of the type, for example `BLImage* image` and not ~~`BLImage *image`~~
* Namespaces are not indented, use the following in public headers:

```c++
namespace BLSomeNamespace {
[...]
} // {BLSomeNamespace}
```

* No line between a statement and opening bracket `{`:

```c++
class BLSomeClass {
  void some_function() {
    for (size_t i = 0; i < 10; i++) {
      if (i & 0x1) {
        [...]
      }
    }
  }
};
```

* No spaces in a conditional expression, function declaration, and call:

```c++
// Right:
if (x) {
  some_function(x);
}

/* WRONG:
if( x )
  some_function ( x );
*/
```

* If obvious, omit `== nullptr` and `!= nullptr` from conditional expressions, except asserts:

```c++
void* ptr;

// Right:
BL_ASSERT(ptr != nullptr);
if (ptr) {
  [...]
}

/* WRONG:
BL_ASSERT(ptr);
if (ptr != nullptr) {
  [...]
}
*/
```

* Blend2D transitions to always use blocks in branches (**if**/**else**):

```c++
// Right:
if (something) {
  do_something()
}

if (x) {
  first();
}
else {
  second();
}

/* WRONG:
if (something)
  do_something()

if (x) {
  first();
}
else
  second();
*/
```

* There is a space between a **switch** statement and its expression; **case** statement and its content are indented:

```c++
switch (expression) {
  case 0:
    do_something();
    break;

  // Case that requires a block.
  case 1: {
    int var = something;
    do_something_else_with_var(var);
    break;
  }

  // If the default should never be reached, mark it so:
  default:
    BL_NOT_REACHED();
}
```

* Sometimes **case** and its content can be inlined:

```c++
switch (condition) {
  case 0: do_something(); break;
  case 1: do_something_else(); break;
}
```

* Public enum values are *UPPER_CASED* and use `BL_` prefix
* Public enums usually end with `_MAX_VALUE`, which should be separated by an empty line
* Public enums are always `uint32_t`, use `BL_DEFINE_ENUM` to make sure they are properly defined in both C and C++ modes:

```c++
BL_DEFINE_ENUM(BLSomeEnum) {
  BL_SOME_ENUM_A = 0,
  BL_SOME_ENUM_B = 1,

  BL_SOME_ENUM_MAX_VALUE = 1
};
```
