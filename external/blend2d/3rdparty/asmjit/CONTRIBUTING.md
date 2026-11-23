## How to Contribute to AsmJit

### Did you find a bug or something isn't working as expected?

  * Please use [Issues](https://github.com/asmjit/asmjit/issues) page to report bugs or create a [pull request](https://github.com/asmjit/asmjit/pulls) if you have already fixed it.

  * Make sure that when a bug is reported it provides as much information as possible to make it easy to either reproduce it locally or to at least guess where the problem could be. AsmJit is a low-level tool, which makes it very easy to emit code that would crash or not work as intended when executed. Always use AsmJit's [Logging](https://asmjit.com/doc/group__asmjit__logging.html) and [Error Handling](https://asmjit.com/doc/group__asmjit__error__handling.html) features first to analyze whether there is not a simple to catch bug in your own code.

  * Don't be afraid to ask for help if you don't know how to solve a particular problem or in case it's unclear how to do it. The community would help if the problem is well described and has a solution. In general we always try to at least improve the documentation in case it doesn't provide enough information and users must ask for help.

### Asking questions

  * We prefer GitHub issues to be used for reporting bugs or feature requests, but it's still okay to ask questions there as well. However, please consider joining our [Gitter Chat](https://app.gitter.im/#/room/#asmjit:gitter.im) to ask questions; it has an active community that can quickly respond.

### Suggesting feature requests

  * It's very likely that when using AsmJit you have found something that AsmJit doesn't provide, which would be handy to have as a built-in. The [Issues](https://github.com/asmjit/asmjit/issues) page can be used to submit feature requests, but please keep in mind that AsmJit is a relatively small project and not all requested features will be accepted, especially if they are non-trivial, time consuming to implement, or the scope of the feature doesn't match AsmJit goals.

  * If you have already implemented the feature you are suggesting, please open a [pull request](https://github.com/asmjit/asmjit/pulls).

  * Ports (requesting new AsmJit backends) can be reported as feature requests, but only by people that are willing to work on them as creating new ports takes a lot of time.

### Suggesting a documentation enhancement

  * [AsmJit's documentation](https://asmjit.com/doc/index.html) is auto-generated from source code, so if you would like to improve it just open a [pull request](https://github.com/asmjit/asmjit/pulls) with your changes. The documentation uses [Doxygen](https://www.doxygen.nl/) as a front-end, so you can use `\ref` keyword to create links and other Doxygen keywords to enhance the documentation.

### Suggesting a website content enhancement

  * [AsmJit's website](https://asmjit.com) is also generated, but not from public sources at the moment. If you did find an issue on the website you can either use contact information on the [support page](https://asmjit.com/support.html) or to discuss the change on our [Gitter Chat](https://app.gitter.im/#/room/#asmjit:gitter.im). Alternatively, opening a regular issue is also okay.


## Coding Style & Consistency

  * If you decide to open a pull request, make sure that the code you submit uses the same convention as the rest of the code. We prefer keeping the code consistent.

  * [.editorconfig](./.editorconfig) should help with basic settings.

  * Initially, AsmJit coding style was based on Google C++ Style Guide, but it has diverged from it.

  * Include guards use `<PATH_TO_SRC>_H_INCLUDED` format.

  * `asmjit` namespace must be open by `ASMJIT_BEGIN_NAMESPACE` and closed by `ASMJIT_END_NAMESPACE`

  * `asmjit::xxx` (backend specific) nested namespace must be open by `ASMJIT_BEGIN_SUB_NAMESPACE(xxx)` and closed by `ASMJIT_END_SUB_NAMESPACE`.

  * Opening bracket is on the same line, like `struct Something {`, `if (condition) {`, etc...

  * The code uses a soft limit of 120 characters per line (including documentation), but it's not enforced and it's okay to use more when it makes sense (for example defining tables, etc...).

  * Since AsmJit doesn't use Exceptions nor RTTI the code cannot use containers provided by the C++ standard library. In general, we try to only use a bare minimum from the C++ standard library to make it viable to use AsmJit even in C code bases where JIT complier is implemented in C++ ([Erlang](https://www.erlang.org/) can be seen as a great example).

## Testing

  * AsmJit uses a minimalist unit testing framework to write unit tests to avoid third-party dependencies.

  * At the moment tests are in the same file as the implementation and are only compiled when `ASMJIT_TEST` macro is defined.

  * Use `-DASMJIT_TEST=1` when invoking [CMake](https://cmake.org/) to compile AsmJit tests.

  * Unit tests are compiled to a single `asmjit_test_unit[.exe]` executable.

  * Other tests have their own executables based on what is tested.

  * Always add assembler tests when adding new instructions, see [asmjit_test_assembler_x64.cpp](./test/asmjit_test_assembler_x64.cpp) and [asmjit_test_assembler_a64.cpp](./test/asmjit_test_assembler_a64.cpp) for more details.

## Pull Request Messages

  * If a change fixes a bug the message should should start with `[bug]`.

  * If a change fixes or enhances documentation it should start with `[doc]`.

  * If a change fixes or enhances our CI it should start with `[ci]`.

  * If a change breaks ABI it must start with `[abi]`.

  * Otherwise there is no suggested prefix.

## ABI Changes

  * ABI changes happen, but they are usually accumulated and committed within a short time window to not break it often. In general we prefer to break ABI once a year, or once 6 months if there is something that has a high priority. There are no hard rules though.

  * AsmJit uses an `inline namespace`, which should make it impossible to link to AsmJit library that is ABI incompatible. When ABI break happens both AsmJit version and ABI namespace are changed, see [asmjit/core/api-config.h](./src/asmjit/core/api-config.h) for more details.

  * What is an ABI break?

    * Modifying a public struct/class in a way that its functionality is altered and/or its size is changed

    * Adding/removing virtual functions to/from classes, respectively

    * Changing a signature of a public function or a class member function (for example adding a parameter).

    * Changing the value of an enum or global constant (for example instructions are now sorted by name, so adding a new instruction breaks ABI)

    * Possibly more, but these were the most common...

  * What is not ABI break?

    * Extending the functionality by using reserved members of a struct/class

    * Adding new API including new structs and classes

    * Changing anything that is internal and that doesn't leak to public headers
