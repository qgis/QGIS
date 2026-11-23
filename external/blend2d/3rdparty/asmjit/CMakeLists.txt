# AsmJit
# ======

# AsmJit library provides a single alias that can use used to consume it as a dependency:
#
#   `asmjit::asmjit`
#
# Depending on options, the alias would be either a static or shared library (depending on
# `ASMJIT_STATIC=ON|OFF` options) or it would be an interface library if `ASMJIT_EMBED=ON`.
#
# NOTE: When `ASMJIT_EMBED=ON` AsmJit won't add any installation instructions of libraries
# or header files. Embed is literally for users that want to consume AsmJit as source code
# without creating an intermediate static library. C++ flags, which used by the consuming
# target will then be used to compile the embedded AsmJit source code as well. This option
# improves the control of how AsmJit is both compiled and consumed.
#
# Consuming AsmJit
# ----------------
#
# The recommended scenario to consume AsmJit is to clone it to a directory where third
# party libraries are placed (this can be done even if you use a package manager such as
# vcpkg or conan to manage other dependencies). Usually, the directory is called '3rdparty'
# and it resides in your project's root, but it's just a recommendation, not a convention.
# In such case, consuming a cloned AsmJit from your own project would look like this:
#
#   # == Configure AsmJit options ==
#   set(ASMJIT_EMBED  ON or OFF)                   # AsmJit uses ASMJIT_EMBED=OFF by default.
#   set(ASMJIT_STATIC ON or OFF)                   # AsmJit uses ASMJIT_STATIC=OFF by default.
#   add_subdirectory(3rdparty/asmjit asmjit)       # Adds AsmJit as a sub-project.
#
#   # == Consume AsmJit as a CMake dependency ==
#   add_executable(test_app main.cpp)              # Target executable having main.cpp
#   target_link_libraries(test_app asmjit::asmjit) # Adds AsmJit as a dependency to test_app.
#
# Developing & Testing AsmJit
# ---------------------------
#
# There are sample configure scripts provided, which reside in project root. In general, to
# develop AsmJit it must be configured with `ASMJIT_TEST=ON` to build AsmJit tests and they
# have to pass. Here is a simple cmake invocation to build AsmJit with tests enabled:
#
#   cmake . -B build/Debug -DCMAKE_BUILD_TYPE=Debug -DASMJIT_TEST=ON
#
# See <asmjit.com> for more details.

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# AsmJit - Project & Configuration
# ================================

# Don't create a project if it was already created by another CMakeLists.txt. This makes
# it possible to support both add_subdirectory() and include() ways of using AsmJit as a
# dependency.
if (NOT CMAKE_PROJECT_NAME OR "${CMAKE_PROJECT_NAME}" STREQUAL "asmjit")
  project(asmjit
    LANGUAGES CXX
    DESCRIPTION "Low-latency machine code generation"
    HOMEPAGE_URL "https://asmjit.com")
endif()

set(ASMJIT_SANITIZE            "" CACHE STRING "Sanitizers to use (development)")
set(ASMJIT_SANITIZE_OPTS       "" CACHE STRING "Extra flags for sanitizers (development)")

option(ASMJIT_EMBED            "Embed 'asmjit' as an interface library (no static/shared library)" OFF)
option(ASMJIT_STATIC           "Build 'asmjit' as a static library (implied by ASMJIT_EMBED)" ${ASMJIT_EMBED})
option(ASMJIT_TEST             "Build 'asmjit' tests and benchmarks (development)" OFF)

option(ASMJIT_NO_NATVIS        "Disable natvis support (embedding asmjit.natvis in PDB)" OFF)
option(ASMJIT_NO_CUSTOM_FLAGS  "Disable extra compilation flags added by AsmJit to its targets" OFF)
option(ASMJIT_NO_ABI_NAMESPACE "Disable the use of inline ABI namespace {asmjit::v...}" OFF)
option(ASMJIT_NO_SHM_OPEN      "Disable the use of shm_open() (some platforms have better options)" OFF)
option(ASMJIT_NO_X86           "Disable X86/X64 backend" OFF)
option(ASMJIT_NO_AARCH64       "Disable AArch64 backend" OFF)
option(ASMJIT_NO_FOREIGN       "Disable all foreign backends (enables only a backend that matches the target)" OFF)
option(ASMJIT_NO_JIT           "Disable VirtMem, JitAllocator, and JitRuntime at build time" OFF)
option(ASMJIT_NO_TEXT          "Disable textual representation of instructions, enums, cpu features, ..." OFF)
option(ASMJIT_NO_LOGGING       "Disable logging features (build feature)" ${ASMJIT_NO_TEXT})
option(ASMJIT_NO_INTROSPECTION "Disable instruction validation and introspection API (build feature)" OFF)
option(ASMJIT_NO_BUILDER       "Disable AsmJit's Builder emitter (build feature)" OFF)

if (NOT ASMJIT_NO_BUILDER AND NOT ASMJIT_NO_INTROSPECTION)
  option(ASMJIT_NO_COMPILER    "Disable AsmJit's Compiler emitter (build feature)" OFF)
else()
  if (NOT ASMJIT_NO_COMPILER)
    unset(ASMJIT_NO_COMPILER CACHE)
  endif()
  option(ASMJIT_NO_COMPILER    "Disable AsmJit's Compiler emitter (build feature)" ON)
endif()

if (NOT ASMJIT_NO_COMPILER)
  option(ASMJIT_NO_UJIT        "Disable AsmJit's UniCompiler backend (build feature)" OFF)
else()
  if (NOT ASMJIT_NO_UJIT)
    unset(ASMJIT_NO_UJIT CACHE)
  endif()
  option(ASMJIT_NO_UJIT        "Disable AsmJit's UniCompiler backend (build feature)" ON)
endif()

# AsmJit - Build Definitions and Source Files
# ===========================================

set(ASMJIT_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}") # AsmJit include directory is the same as <root> directory.
set(ASMJIT_LIBS "")                                  # AsmJit library dependencies for the linker.
set(ASMJIT_CFLAGS "")                                # Public compiler flags.
set(ASMJIT_PRIVATE_CFLAGS "")                        # Private compiler flags independent of build type.
set(ASMJIT_PRIVATE_CFLAGS_DBG "")                    # Private compiler flags used by debug builds.
set(ASMJIT_PRIVATE_CFLAGS_REL "")                    # Private compiler flags used by release builds.
set(ASMJIT_SANITIZE_CFLAGS "")                       # Compiler flags required by currently enabled sanitizers.
set(ASMJIT_SANITIZE_LFLAGS "")                       # Linker flags required by currently enabled sanitizers.

if (ASMJIT_EMBED)
  set(ASMJIT_TARGET_TYPE "EMBED")
elseif (ASMJIT_STATIC)
  set(ASMJIT_TARGET_TYPE "STATIC")
else()
  set(ASMJIT_TARGET_TYPE "SHARED")
endif()
message(STATUS "[asmjit] == Configuring AsmJit ('ASMJIT_TARGET_TYPE=${ASMJIT_TARGET_TYPE}') ==")

set(ASMJIT_SRC
  asmjit/asmjit.h
  asmjit/asmjit-scope-begin.h
  asmjit/asmjit-scope-end.h

  asmjit/core.h
  asmjit/core/api-build_p.h
  asmjit/core/api-config.h
  asmjit/core/archtraits.cpp
  asmjit/core/archtraits.h
  asmjit/core/archcommons.h
  asmjit/core/assembler.cpp
  asmjit/core/assembler.h
  asmjit/core/builder.cpp
  asmjit/core/builder.h
  asmjit/core/codebuffer.h
  asmjit/core/codeholder.cpp
  asmjit/core/codeholder.h
  asmjit/core/codewriter.cpp
  asmjit/core/codewriter_p.h
  asmjit/core/compiler.cpp
  asmjit/core/compiler.h
  asmjit/core/compilerdefs.h
  asmjit/core/constpool.cpp
  asmjit/core/constpool.h
  asmjit/core/cpuinfo.cpp
  asmjit/core/cpuinfo.h
  asmjit/core/emithelper.cpp
  asmjit/core/emithelper_p.h
  asmjit/core/emitter.cpp
  asmjit/core/emitter.h
  asmjit/core/emitterutils.cpp
  asmjit/core/emitterutils_p.h
  asmjit/core/environment.cpp
  asmjit/core/environment.h
  asmjit/core/errorhandler.cpp
  asmjit/core/errorhandler.h
  asmjit/core/fixup.h
  asmjit/core/formatter.cpp
  asmjit/core/formatter.h
  asmjit/core/func.cpp
  asmjit/core/func.h
  asmjit/core/funcargscontext.cpp
  asmjit/core/funcargscontext_p.h
  asmjit/core/globals.cpp
  asmjit/core/globals.h
  asmjit/core/inst.cpp
  asmjit/core/inst.h
  asmjit/core/instdb.cpp
  asmjit/core/instdb_p.h
  asmjit/core/jitallocator.cpp
  asmjit/core/jitallocator.h
  asmjit/core/jitruntime.cpp
  asmjit/core/jitruntime.h
  asmjit/core/logger.cpp
  asmjit/core/logger.h
  asmjit/core/misc_p.h
  asmjit/core/operand.cpp
  asmjit/core/operand.h
  asmjit/core/osutils.cpp
  asmjit/core/osutils.h
  asmjit/core/osutils_p.h
  asmjit/core/raassignment_p.h
  asmjit/core/racfgblock_p.h
  asmjit/core/racfgbuilder_p.h
  asmjit/core/raconstraints_p.h
  asmjit/core/radefs_p.h
  asmjit/core/rainst_p.h
  asmjit/core/ralocal.cpp
  asmjit/core/ralocal_p.h
  asmjit/core/rapass.cpp
  asmjit/core/rapass_p.h
  asmjit/core/rastack.cpp
  asmjit/core/rastack_p.h
  asmjit/core/string.cpp
  asmjit/core/string.h
  asmjit/core/target.cpp
  asmjit/core/target.h
  asmjit/core/type.cpp
  asmjit/core/type.h
  asmjit/core/virtmem.cpp
  asmjit/core/virtmem.h

  asmjit/support/arena.cpp
  asmjit/support/arena.h
  asmjit/support/arenabitset.cpp
  asmjit/support/arenabitset_p.h
  asmjit/support/arenahash.cpp
  asmjit/support/arenahash.h
  asmjit/support/arenalist.cpp
  asmjit/support/arenalist.h
  asmjit/support/arenapool.h
  asmjit/support/arenastring.h
  asmjit/support/arenatree.cpp
  asmjit/support/arenatree.h
  asmjit/support/arenavector.cpp
  asmjit/support/arenavector.h
  asmjit/support/span.h
  asmjit/support/support.cpp
  asmjit/support/support.h
  asmjit/support/support_p.h

  asmjit/a64.h
  asmjit/arm.h
  asmjit/arm/armformatter.cpp
  asmjit/arm/armformatter_p.h
  asmjit/arm/armglobals.h
  asmjit/arm/armutils.h
  asmjit/arm/a64archtraits_p.h
  asmjit/arm/a64assembler.cpp
  asmjit/arm/a64assembler.h
  asmjit/arm/a64builder.cpp
  asmjit/arm/a64builder.h
  asmjit/arm/a64compiler.cpp
  asmjit/arm/a64compiler.h
  asmjit/arm/a64emithelper.cpp
  asmjit/arm/a64emithelper_p.h
  asmjit/arm/a64emitter.h
  asmjit/arm/a64formatter.cpp
  asmjit/arm/a64formatter_p.h
  asmjit/arm/a64func.cpp
  asmjit/arm/a64func_p.h
  asmjit/arm/a64globals.h
  asmjit/arm/a64instapi.cpp
  asmjit/arm/a64instapi_p.h
  asmjit/arm/a64instdb.cpp
  asmjit/arm/a64instdb.h
  asmjit/arm/a64operand.cpp
  asmjit/arm/a64operand.h
  asmjit/arm/a64rapass.cpp
  asmjit/arm/a64rapass_p.h

  asmjit/x86.h
  asmjit/x86/x86archtraits_p.h
  asmjit/x86/x86assembler.cpp
  asmjit/x86/x86assembler.h
  asmjit/x86/x86builder.cpp
  asmjit/x86/x86builder.h
  asmjit/x86/x86compiler.cpp
  asmjit/x86/x86compiler.h
  asmjit/x86/x86emithelper.cpp
  asmjit/x86/x86emithelper_p.h
  asmjit/x86/x86emitter.h
  asmjit/x86/x86formatter.cpp
  asmjit/x86/x86formatter_p.h
  asmjit/x86/x86func.cpp
  asmjit/x86/x86func_p.h
  asmjit/x86/x86globals.h
  asmjit/x86/x86instdb.cpp
  asmjit/x86/x86instdb.h
  asmjit/x86/x86instdb_p.h
  asmjit/x86/x86instapi.cpp
  asmjit/x86/x86instapi_p.h
  asmjit/x86/x86operand.cpp
  asmjit/x86/x86operand.h
  asmjit/x86/x86rapass.cpp
  asmjit/x86/x86rapass_p.h

  asmjit/ujit/ujitbase.h
  asmjit/ujit/unicompiler.h
  asmjit/ujit/unicompiler_a64.cpp
  asmjit/ujit/unicompiler_x86.cpp
  asmjit/ujit/unicompiler_utils_p.h
  asmjit/ujit/uniop.h
  asmjit/ujit/vecconsttable.cpp
  asmjit/ujit/vecconsttable.h
)

# AsmJit - CMake Utilities
# ========================

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)

# Detects target architecture - since the code doesn't run it works also for cross-compilation.
function(asmjit_detect_target_arch out)
  set(${out} "unknown" PARENT_SCOPE)
  foreach(target_arch "X86_64" "AARCH64" "X86" "AARCH32")
    check_cxx_source_compiles("
      #if defined(_M_X64) || defined(__x86_64__)
        static int target_arch_is_X86_64() { return 1; }
      #elif defined(_M_IX86) || defined(__X86__) || defined(__i386__)
        static int target_arch_is_X86() { return 1; }
      #elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
        static int target_arch_is_AARCH64() { return 1; }
      #elif defined(_M_ARM) || defined(_M_ARMT) || defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
        static int target_arch_is_AARCH32() { return 1; }
      #endif
      int main(void) { return target_arch_is_${target_arch}(); }" "${out}_${target_arch}")
    if (${${out}_${target_arch}})
      set(${out} "${target_arch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

# Detects C++ compile flags and appends all detected ones to `out`.
function(asmjit_detect_cflags out)
  set(out_array ${${out}})
  foreach(flag ${ARGN})
    string(REGEX REPLACE "[+]" "x" flag_signature "${flag}")
    string(REGEX REPLACE "[-=:;/.\]" "_" flag_signature "${flag_signature}")
    check_cxx_compiler_flag(${flag} "__CxxFlag_${flag_signature}")
    if (${__CxxFlag_${flag_signature}})
      list(APPEND out_array "${flag}")
    endif()
  endforeach()
  set(${out} "${out_array}" PARENT_SCOPE)
endfunction()

# Support for various sanitizers provided by C/C++ compilers.
function(asmjit_detect_sanitizers out)
  set(_out_array ${${out}})
  set(_flags "")

  foreach(_arg ${ARGN})
    string(REPLACE "," ";" _arg "${_arg}")
    list(APPEND _flags ${_arg})
  endforeach()

  foreach(_flag ${_flags})
    if (NOT "${_flag}" MATCHES "^-fsanitize=")
      SET(_flag "-fsanitize=${_flag}")
    endif()

    # Sanitizers also require link flags, see CMAKE_REQUIRED_FLAGS.
    set(CMAKE_REQUIRED_FLAGS "${_flag}")
    asmjit_detect_cflags(_out_array ${_flag})
    unset(CMAKE_REQUIRED_FLAGS)
  endforeach()

  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

function(asmjit_add_target target target_type)
  set(single_val "")
  set(multi_val SOURCES LIBRARIES CFLAGS CFLAGS_DBG CFLAGS_REL)
  cmake_parse_arguments("X" "" "${single_val}" "${multi_val}" ${ARGN})

  if ("${target_type}" MATCHES "^(EXECUTABLE|TEST)$")
    add_executable(${target} ${X_SOURCES})
  else()
    add_library(${target} ${target_type} ${X_SOURCES})
  endif()

  set_target_properties(${target}
    PROPERTIES
      DEFINE_SYMBOL ""
      CXX_VISIBILITY_PRESET hidden)
  target_compile_options(${target} PRIVATE ${X_CFLAGS} ${ASMJIT_SANITIZE_CFLAGS} $<$<CONFIG:Debug>:${X_CFLAGS_DBG}> $<$<NOT:$<CONFIG:Debug>>:${X_CFLAGS_REL}>)
  target_compile_features(${target} PUBLIC cxx_std_17)
  target_link_options(${target} PRIVATE ${ASMJIT_PRIVATE_LFLAGS})
  target_link_libraries(${target} PRIVATE ${X_LIBRARIES})

  if ("${target_type}" STREQUAL "TEST")
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

# AsmJit - Compiler Support
# =========================

if (NOT ASMJIT_EMBED)
  if (NOT ASMJIT_NO_CUSTOM_FLAGS)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
      list(APPEND ASMJIT_PRIVATE_CFLAGS -W4)                                      # [+] Warning level 4.

      list(APPEND ASMJIT_PRIVATE_CFLAGS -MP)                                      # [+] Multi-Process Compilation.
      list(APPEND ASMJIT_PRIVATE_CFLAGS -GF)                                      # [+] Eliminate duplicate strings.
      list(APPEND ASMJIT_PRIVATE_CFLAGS -Zc:__cplusplus)                          # [+] Conforming __cplusplus definition.
      list(APPEND ASMJIT_PRIVATE_CFLAGS -Zc:inline)                               # [+] Remove unreferenced COMDAT.
      list(APPEND ASMJIT_PRIVATE_CFLAGS -Zc:strictStrings)                        # [+] Strict const qualification of string literals.
      list(APPEND ASMJIT_PRIVATE_CFLAGS -Zc:threadSafeInit-)                      # [-] Thread-safe statics.

      list(APPEND ASMJIT_PRIVATE_CFLAGS_DBG -GS)                                  # [+] Buffer security-check.
      list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -GS-)                                 # [-] Buffer security-check.
      list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -O2)                                  # [+] Favor speed over size.
      list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -Oi)                                  # [+] Generate intrinsic functions.
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
      list(APPEND ASMJIT_PRIVATE_CFLAGS -Wall -Wextra -Wconversion)               # [+] Add baseline warnings that can be used safely even with system headers.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wdouble-promotion)              # [+] Warn about double promotions.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wduplicated-cond)               # [+] Warn about duplicate conditions.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wduplicated-branches)           # [+] Warn about duplicate branches.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wlogical-op)                    # [+] Warn about suspicious uses of logical operators in expressions.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wlogical-not-parentheses)       # [+] Warn about logical not used on the left hand side operand of a comparison.
      asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -Wrestrict)

      list(APPEND ASMJIT_PRIVATE_CFLAGS -fno-math-errno)                          # [-] Disable math functions setting errno (performance reasons).
      list(APPEND ASMJIT_PRIVATE_CFLAGS -fno-threadsafe-statics)                  # [-] Don't add guards when initializing statics (we don't need it).
      list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -O2)                                  # [+] Compiling with -O2 in release mode is what we generally want.
      list(APPEND ASMJIT_PRIVATE_CFLAGS_REL -fmerge-all-constants)                # [+] We don't need unique address per constant (merging improves library size).

      # -fno-semantic-interposition is not available on apple - the compiler issues a warning, which is not detected.
      if (NOT APPLE)
        asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS -fno-semantic-interposition)
      endif()

      if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
        asmjit_detect_cflags(ASMJIT_PRIVATE_CFLAGS_REL -fno-enforce-eh-specs)     # [-] Don't enforce termination if noexcept function throws.
      endif()
    endif()
  endif()

  # Support for sanitizers.
  if (ASMJIT_SANITIZE)
    asmjit_detect_sanitizers(ASMJIT_SANITIZE_CFLAGS ${ASMJIT_SANITIZE})
    if (ASMJIT_SANITIZE_CFLAGS)
      message(STATUS "[asmjit] Enabling sanitizers: '${ASMJIT_SANITIZE_CFLAGS}'")

      # Linker must receive the same flags as the compiler when it comes to sanitizers.
      set(ASMJIT_SANITIZE_LFLAGS ${ASMJIT_SANITIZE_CFLAGS})

      # Don't omit frame pointer if sanitizers are enabled.
      if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
        list(APPEND ASMJIT_SANITIZE_CFLAGS -Oy-)
      else()
        list(APPEND ASMJIT_SANITIZE_CFLAGS -fno-omit-frame-pointer -g)
      endif()

      list(APPEND ASMJIT_PRIVATE_CFLAGS ${ASMJIT_SANITIZE_CFLAGS})
      list(APPEND ASMJIT_PRIVATE_LFLAGS ${ASMJIT_SANITIZE_LFLAGS})
    endif()
  endif()
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "^Windows")
  # Dependency: Windows target needs no extra dependencies, however, we have to check for .natvis support.
  if(CMAKE_LINKER MATCHES "^(lld-link|link)\\.exe$")
    set(ASMJIT_LINKER_SUPPORTS_NATVIS TRUE)
  endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  # Dependency: libc is the only required library on Android as it also provides libpthread.
  message(STATUS "[asmjit] Adding 'libc' dependency (Android)")
  list(APPEND ASMJIT_LIBS c)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
  # Dependency: libroot is used by Haiku instead of libc, so link to libroot and libpthread.
  message(STATUS "[asmjit] Adding 'libroot' and 'libpthread' dependencies (Haiku)")
  list(APPEND ASMJIT_LIBS root pthread)
else()
  # Dependency: libc is always required.
  message(STATUS "[asmjit] Adding 'libc' dependency (Linux, BSD, or other UNIX/POSIX environment)")
  list(APPEND ASMJIT_LIBS c)

  # Dependency: pthread (required so AsmJit can use pthread_lock).
  check_cxx_source_compiles("
    #include <pthread.h>
    int main() {
      pthread_mutex_t m;
      pthread_mutex_init(&m, nullptr);
      return pthread_mutex_destroy(&m);
    }
    " ASMJIT_LIBC_HAS_LIBPTHREAD)
  if (ASMJIT_LIBC_HAS_LIBPTHREAD)
    message(STATUS "[asmjit] Library 'libpthread' provided by 'libc' (not linking to libpthread)")
  else()
    message(STATUS "[asmjit] Library 'libpthread' not provided by 'libc' (linking to libpthread)")
    list(APPEND ASMJIT_LIBS pthread)
  endif()

  # Dependency: shm_open (required so AsmJit can use shm_open on supported platforms).
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "^(Linux|NetBSD)$" AND NOT ASMJIT_NO_SHM_OPEN)
    check_cxx_source_compiles("
      #include <sys/mman.h>
      int main() {
        const char file_name[1] {};
        return shm_open(file_name, 0, 0);
      }
      " ASMJIT_LIBC_HAS_LIBRT)
    if (ASMJIT_LIBC_HAS_LIBRT)
      message(STATUS "[asmjit] Symbol 'shm_open' provided by libc (not linking to librt)")
    else()
      message(STATUS "[asmjit] Symbol 'shm_open' not provided by libc, linking to librt")
      list(APPEND ASMJIT_LIBS rt)
    endif()
  endif()
endif()

if (ASMJIT_STATIC OR ASMJIT_EMBED)
  list(APPEND ASMJIT_CFLAGS -DASMJIT_STATIC)
endif()

if (ASMJIT_NO_SHM_OPEN)
  list(APPEND ASMJIT_PRIVATE_CFLAGS -DASMJIT_NO_SHM_OPEN)
endif()

foreach(build_option ASMJIT_NO_X86
                     ASMJIT_NO_AARCH64
                     ASMJIT_NO_FOREIGN
                     ASMJIT_NO_ABI_NAMESPACE
                     ASMJIT_NO_JIT
                     ASMJIT_NO_TEXT
                     ASMJIT_NO_LOGGING
                     ASMJIT_NO_INTROSPECTION
                     ASMJIT_NO_BUILDER
                     ASMJIT_NO_COMPILER
                     ASMJIT_NO_UJIT)
  if (${build_option})
    list(APPEND ASMJIT_CFLAGS "-D${build_option}")
  endif()
endforeach()

if (ASMJIT_LINKER_SUPPORTS_NATVIS AND NOT ASMJIT_NO_NATVIS)
  list(APPEND ASMJIT_SRC asmjit/asmjit.natvis)
  list(APPEND ASMJIT_PRIVATE_LFLAGS "-natvis:${CMAKE_CURRENT_LIST_DIR}/asmjit/asmjit.natvis")
endif()

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${ASMJIT_SRC})

# AsmJit - Targets
# ================

message(STATUS "[asmjit] Adding 'asmjit::asmjit' target ('ASMJIT_TARGET_TYPE=${ASMJIT_TARGET_TYPE}')")

if (ASMJIT_EMBED)
  add_library(asmjit_embed INTERFACE)
  target_sources(asmjit_embed INTERFACE ${ASMJIT_SRC})
  target_link_libraries(asmjit_embed INTERFACE ${ASMJIT_LIBS})
  target_compile_options(asmjit_embed INTERFACE ${ASMJIT_CFLAGS})
  target_include_directories(asmjit_embed BEFORE INTERFACE ${ASMJIT_INCLUDE_DIRS})
  add_library(asmjit::asmjit ALIAS asmjit_embed)
else()
  foreach(v "ASMJIT_LIBS"
            "ASMJIT_CFLAGS"
            "ASMJIT_PRIVATE_CFLAGS"
            "ASMJIT_PRIVATE_CFLAGS_DBG"
            "ASMJIT_PRIVATE_CFLAGS_REL"
            "ASMJIT_SANITIZE_CFLAGS"
            "ASMJIT_SANITIZE_LFLAGS")
    if (NOT "${${v}}" STREQUAL "")
      message("   ${v}=${${v}}")
    endif()
  endforeach()

  asmjit_add_target(asmjit "${ASMJIT_TARGET_TYPE}"
    SOURCES    ${ASMJIT_SRC}
    LIBRARIES  ${ASMJIT_LIBS}
    CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
    CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
    CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

  target_compile_options(asmjit PUBLIC ${ASMJIT_CFLAGS})
  target_include_directories(asmjit BEFORE PUBLIC $<BUILD_INTERFACE:${ASMJIT_INCLUDE_DIRS}>
                                                  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  add_library(asmjit::asmjit ALIAS asmjit)

  # AsmJit - Install Instructions
  # -----------------------------

  if (NOT ASMJIT_NO_INSTALL)
    message(STATUS "[asmjit] Enabling install support ('ASMJIT_NO_INSTALL=OFF')")

    install(TARGETS asmjit
            EXPORT asmjit-config
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    install(EXPORT asmjit-config
            NAMESPACE asmjit::
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/asmjit")

    foreach(_src_file ${ASMJIT_SRC})
      if ("${_src_file}" MATCHES "\\.h$" AND NOT "${_src_file}" MATCHES "_p\\.h$")
        get_filename_component(_src_dir ${_src_file} PATH)
        install(FILES "${CMAKE_CURRENT_LIST_DIR}/${_src_file}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_src_dir}")
      endif()
    endforeach()
  endif()

  # AsmJit - Tests & Benchmarks
  # ---------------------------

  if (ASMJIT_TEST)
    enable_testing()
    message(STATUS "[asmjit] Enabling AsmJit tests ('ASMJIT_TEST=${ASMJIT_TEST}')")

    # Special target that always uses embedded AsmJit.
    asmjit_add_target(asmjit_test_runner TEST
      SOURCES    ${ASMJIT_SRC}
                 asmjit-testing/tests/asmjit_test_runner.cpp
                 asmjit-testing/tests/broken.cpp
                 asmjit-testing/tests/broken.h
      LIBRARIES  ${ASMJIT_LIBS}
      CFLAGS     ${ASMJIT_CFLAGS}
                 ${ASMJIT_PRIVATE_CFLAGS}
                 -DASMJIT_TEST
                 -DASMJIT_STATIC
      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    target_include_directories(asmjit_test_runner BEFORE PRIVATE ${ASMJIT_INCLUDE_DIRS})

    asmjit_add_target(asmjit_test_assembler TEST
      SOURCES    asmjit-testing/tests/asmjit_test_assembler.cpp
                 asmjit-testing/tests/asmjit_test_assembler.h
                 asmjit-testing/tests/asmjit_test_assembler_a64.cpp
                 asmjit-testing/tests/asmjit_test_assembler_x64.cpp
                 asmjit-testing/tests/asmjit_test_assembler_x86.cpp
      LIBRARIES  asmjit::asmjit
      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

    foreach(app asmjit_test_environment asmjit_test_emitters asmjit_test_x86_sections)
      asmjit_add_target(${app} TEST
        SOURCES    asmjit-testing/tests/${app}.cpp
        LIBRARIES  asmjit::asmjit
        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endforeach()

    if (NOT ASMJIT_NO_INTROSPECTION)
      asmjit_add_target(asmjit_test_instinfo TEST
        SOURCES    asmjit-testing/tests/asmjit_test_instinfo.cpp
        LIBRARIES  asmjit::asmjit
        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endif()

    if (NOT (ASMJIT_NO_BUILDER OR ASMJIT_NO_COMPILER))
      # Vectorcall tests and XMM tests require at least SSE2 in 32-bit mode (in 64-bit mode it's implicit).
      # Some compilers don't like passing -msse2 for 64-bit targets, and some compilers targeting non-x86
      # would pass "-msse2" compile flag check, but with a warning not detected by CMake. Thus, verify that
      # our target is really 32-bit X86 and only use -msse2 or -arch:SSE2 flags when necessary.
      asmjit_detect_target_arch(ASMJIT_TARGET_ARCH)

      set(ASMJIT_SSE2_CFLAGS "")
      if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
        if (ASMJIT_TARGET_ARCH STREQUAL "X86")
          asmjit_detect_cflags(ASMJIT_SSE2_CFLAGS -arch:SSE2)
        endif()
        if (ASMJIT_TARGET_ARCH MATCHES "^(X86|X86_64)$")
          asmjit_detect_cflags(ASMJIT_AVX2FMA_CFLAGS -arch:AVX2)
        endif()
      else()
        if (ASMJIT_TARGET_ARCH STREQUAL "X86")
          asmjit_detect_cflags(ASMJIT_SSE2_CFLAGS -msse2)
        endif()
        if (ASMJIT_TARGET_ARCH MATCHES "^(X86|X86_64)$")
          asmjit_detect_cflags(ASMJIT_AVX2FMA_CFLAGS -mavx2 -mfma)
        endif()
      endif()

      asmjit_add_target(asmjit_test_compiler TEST
        SOURCES    asmjit-testing/tests/asmjit_test_compiler.cpp
                   asmjit-testing/tests/asmjit_test_compiler.h
                   asmjit-testing/tests/asmjit_test_compiler_a64.cpp
                   asmjit-testing/tests/asmjit_test_compiler_x86.cpp
        LIBRARIES  asmjit::asmjit
        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS} ${ASMJIT_SSE2_CFLAGS}
        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endif()

    if (NOT ASMJIT_NO_UJIT)
      asmjit_add_target(asmjit_test_unicompiler TEST
        SOURCES    asmjit-testing/tests/asmjit_test_unicompiler.cpp
                   asmjit-testing/tests/asmjit_test_unicompiler_sse2.cpp
                   asmjit-testing/tests/asmjit_test_unicompiler_avx2fma.cpp
                   asmjit-testing/tests/broken.cpp
        LIBRARIES  asmjit::asmjit
        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS} ${ASMJIT_SSE2_CFLAGS}
        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
      set_property(SOURCE asmjit-testing/tests/asmjit_test_unicompiler_avx2fma.cpp APPEND PROPERTY COMPILE_OPTIONS ${ASMJIT_AVX2FMA_CFLAGS})
    endif()

    asmjit_add_target(asmjit_bench_codegen EXECUTABLE
      SOURCES    asmjit-testing/bench/asmjit_bench_codegen.cpp
                 asmjit-testing/bench/asmjit_bench_codegen_a64.cpp
                 asmjit-testing/bench/asmjit_bench_codegen_x86.cpp
      SOURCES    asmjit-testing/bench/asmjit_bench_codegen.h
      LIBRARIES  asmjit::asmjit
      CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
      CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})

    foreach(app asmjit_bench_overhead asmjit_bench_regalloc)
      asmjit_add_target(${app} TEST
        SOURCES    asmjit-testing/bench/${app}.cpp
        LIBRARIES  asmjit::asmjit
        CFLAGS     ${ASMJIT_PRIVATE_CFLAGS}
        CFLAGS_DBG ${ASMJIT_PRIVATE_CFLAGS_DBG}
        CFLAGS_REL ${ASMJIT_PRIVATE_CFLAGS_REL})
    endforeach()

  endif()
endif()

message(STATUS "[asmjit] == Configuring done ==")
