# Blend2D
# =======

# To consume blend2d as a dependency, use `blend2d::blend2d` alias.

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# Blend2D - Integration
# =====================

# This is a hack for CMakePresets.json, which cannot conditionally include this when building Blend2D demos.
# So instead, it passes BLEND2D_SETUP_VCPKG_TOOLCHAIN_FILE set to ON and the logic of setting up the toolchain
# is here.
if (BLEND2D_SETUP_VCPKG_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  message(STATUS "[blend2d] Setting CMake toolchain file to $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

# Blend2D - Project & Configuration
# =================================

project(blend2d
  LANGUAGES C CXX
  DESCRIPTION "High-performance 2D vector graphics engine with JIT compiler"
  HOMEPAGE_URL "https://blend2d.com")

# BLEND2D_XXX are cmake variables whereas BL_XXX are build-time macros passed to compiler via flags.
set(BLEND2D_SANITIZE           "" CACHE STRING "Sanitizers to use (development)")
set(BLEND2D_SANITIZE_OPTS      "" CACHE STRING "Extra flags for sanitizers (development)")

option(BLEND2D_STATIC          "Build 'blend2d' as a static library instead" OFF)
option(BLEND2D_TEST            "Build 'blend2d' tests and samples (development)" OFF)
option(BLEND2D_DEMOS           "Build 'blend2d' interactive demos that use Qt library (development)" OFF)

option(BLEND2D_NO_NATVIS       "Disable 'blend2d' natvis support (development)" OFF)
option(BLEND2D_NO_JIT          "Disable 'blend2d' JIT pipelines generation support (not recommended)" OFF)
option(BLEND2D_NO_JIT_LOGGING  "Disable 'blend2d' JIT pipelines logging support (deployment)" OFF)
option(BLEND2D_NO_TLS          "Disable 'blend2d' use of TLS (development)" OFF)
option(BLEND2D_NO_FUTEX        "Disable 'blend2d' use of futexes (development)" OFF)
option(BLEND2D_NO_INSTALL      "Disable 'blend2d' install support for cmake (development/deployment)" OFF)
option(BLEND2D_EXTERNAL_ASMJIT "Enable support for external asmjit library (no support by upstream)" OFF)

# Avoid linking to the C++ stdlib by default if blend2d is being built as a shared library with bundled asmjit.
if (NOT BLEND2D_STATIC AND NOT BLEND2D_EXTERNAL_ASMJIT)
  option(BLEND2D_NO_STDCXX     "Disable 'blend2d' linking to a standard C++ library (deployment)" ON)
else()
  option(BLEND2D_NO_STDCXX     "Disable 'blend2d' linking to a standard C++ library (deployment)" OFF)
endif()

# Blend2D - Build Definitions and Source Files
# ============================================

set(BLEND2D_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}")   # Blend2D include directory is the same as <root> directory.
set(BLEND2D_LIBS "")                                    # Blend2D library dependencies for the linker.
set(BLEND2D_CFLAGS "")                                  # Public compiler flags.
set(BLEND2D_WARNING_CFLAGS "")                          # Flags that enable compiler warnings (we add them to various targets).
set(BLEND2D_PRIVATE_CFLAGS "")                          # Private compiler flags independent of build type.
set(BLEND2D_PRIVATE_CFLAGS_DBG "")                      # Private compiler flags used by debug builds.
set(BLEND2D_PRIVATE_CFLAGS_REL "")                      # Private compiler flags used by release builds.
set(BLEND2D_PRIVATE_LFLAGS "")                          # Private linker flags.
set(BLEND2D_SANITIZE_CFLAGS "${BLEND2D_SANITIZE_OPTS}") # Compiler flags required by currently enabled sanitizers.
set(BLEND2D_SANITIZE_LFLAGS "")                         # Linker flags required by currently enabled sanitizers.
set(BLEND2D_NO_STDCXX_CFLAGS "")                        # Private compiler flags to disable linking to a standard C++ library.
set(BLEND2D_NO_STDCXX_LFLAGS "")                        # Private linker flags to disable linking to a standard C++ library.

if (BLEND2D_STATIC)
  set(BLEND2D_TARGET_TYPE "STATIC")
  list(APPEND BLEND2D_CFLAGS -DBL_STATIC)
else()
  set(BLEND2D_TARGET_TYPE "SHARED")
endif()
message(STATUS "[blend2d] == Configuring Blend2D ('BLEND2D_TARGET_TYPE=${BLEND2D_TARGET_TYPE}') ==")

set(BLEND2D_SRC_LIST
  blend2d/blend2d.h
  blend2d/blend2d-debug.h
  blend2d/blend2d-impl.h

  blend2d/core/api.h
  blend2d/core/api-build_p.h
  blend2d/core/api-build_test_p.h
  blend2d/core/api-globals.cpp
  blend2d/core/api-impl.h
  blend2d/core/api-internal_p.h
  blend2d/core/api-nocxx.cpp
  blend2d/core/array.cpp
  blend2d/core/array_test.cpp
  blend2d/core/array.h
  blend2d/core/array_p.h
  blend2d/core/bitarray.cpp
  blend2d/core/bitarray_test.cpp
  blend2d/core/bitarray.h
  blend2d/core/bitarray_p.h
  blend2d/core/bitset.cpp
  blend2d/core/bitset_test.cpp
  blend2d/core/bitset.h
  blend2d/core/bitset_p.h
  blend2d/core/compop_p.h
  blend2d/core/compopinfo_p.h
  blend2d/core/compopinfo.cpp
  blend2d/core/context.cpp
  blend2d/core/context_test.cpp
  blend2d/core/context.h
  blend2d/core/context_p.h
  blend2d/core/filesystem.cpp
  blend2d/core/filesystem.h
  blend2d/core/filesystem_p.h
  blend2d/core/font.cpp
  blend2d/core/font.h
  blend2d/core/font_p.h
  blend2d/core/font_test.cpp
  blend2d/core/fontdata.cpp
  blend2d/core/fontdata.h
  blend2d/core/fontdata_p.h
  blend2d/core/fontdefs.h
  blend2d/core/fontface.cpp
  blend2d/core/fontface.h
  blend2d/core/fontface_p.h
  blend2d/core/fontfeaturesettings.cpp
  blend2d/core/fontfeaturesettings_test.cpp
  blend2d/core/fontfeaturesettings.h
  blend2d/core/fontfeaturesettings_p.h
  blend2d/core/fontmanager.cpp
  blend2d/core/fontmanager.h
  blend2d/core/fontmanager_p.h
  blend2d/core/fonttagdata_p.h
  blend2d/core/fonttagdataids.cpp
  blend2d/core/fonttagdataids_test.cpp
  blend2d/core/fonttagdataids_p.h
  blend2d/core/fonttagdatainfo.cpp
  blend2d/core/fonttagdatainfo_test.cpp
  blend2d/core/fonttagdatainfo_p.h
  blend2d/core/fonttagset.cpp
  blend2d/core/fonttagset_p.h
  blend2d/core/fontvariationsettings.cpp
  blend2d/core/fontvariationsettings_test.cpp
  blend2d/core/fontvariationsettings.h
  blend2d/core/fontvariationsettings_p.h
  blend2d/core/format.cpp
  blend2d/core/format.h
  blend2d/core/format_p.h
  blend2d/core/geometry.h
  blend2d/core/glyphbuffer.cpp
  blend2d/core/glyphbuffer.h
  blend2d/core/glyphbuffer_p.h
  blend2d/core/glyphrun.h
  blend2d/core/gradient.cpp
  blend2d/core/gradient_test.cpp
  blend2d/core/gradient.h
  blend2d/core/gradient_p.h
  blend2d/core/image.cpp
  blend2d/core/image_test.cpp
  blend2d/core/image.h
  blend2d/core/image_p.h
  blend2d/core/imagecodec.cpp
  blend2d/core/imagecodec_test.cpp
  blend2d/core/imagecodec.h
  blend2d/core/imagedecoder.cpp
  blend2d/core/imagedecoder.h
  blend2d/core/imageencoder.cpp
  blend2d/core/imageencoder.h
  blend2d/core/imagescale.cpp
  blend2d/core/imagescale_p.h
  blend2d/core/matrix.cpp
  blend2d/core/matrix_avx.cpp
  blend2d/core/matrix_sse2.cpp
  blend2d/core/matrix_test.cpp
  blend2d/core/matrix.h
  blend2d/core/matrix_p.h
  blend2d/core/object.cpp
  blend2d/core/object.h
  blend2d/core/object_p.h
  blend2d/core/path.cpp
  blend2d/core/path_test.cpp
  blend2d/core/path.h
  blend2d/core/path_p.h
  blend2d/core/pathstroke.cpp
  blend2d/core/pathstroke_p.h
  blend2d/core/pattern.cpp
  blend2d/core/pattern.h
  blend2d/core/pattern_p.h
  blend2d/core/pixelconverter.cpp
  blend2d/core/pixelconverter_avx2.cpp
  blend2d/core/pixelconverter_sse2.cpp
  blend2d/core/pixelconverter_ssse3.cpp
  blend2d/core/pixelconverter_test.cpp
  blend2d/core/pixelconverter.h
  blend2d/core/pixelconverter_p.h
  blend2d/core/random.cpp
  blend2d/core/random_test.cpp
  blend2d/core/random.h
  blend2d/core/random_p.h
  blend2d/core/rgba_test.cpp
  blend2d/core/rgba.h
  blend2d/core/runtime.cpp
  blend2d/core/runtime.h
  blend2d/core/runtime_p.h
  blend2d/core/runtimescope.cpp
  blend2d/core/runtimescope.h
  blend2d/core/string.cpp
  blend2d/core/string_test.cpp
  blend2d/core/string.h
  blend2d/core/string_p.h
  blend2d/core/trace.cpp
  blend2d/core/trace_p.h
  blend2d/core/var.cpp
  blend2d/core/var_test.cpp
  blend2d/core/var.h
  blend2d/core/var_p.h

  blend2d/codec/bmpcodec.cpp
  blend2d/codec/bmpcodec_p.h
  blend2d/codec/codec_test.cpp
  blend2d/codec/jpegcodec.cpp
  blend2d/codec/jpegcodec_p.h
  blend2d/codec/jpeghuffman.cpp
  blend2d/codec/jpeghuffman_p.h
  blend2d/codec/jpegops.cpp
  blend2d/codec/jpegops_sse2.cpp
  blend2d/codec/jpegops_p.h
  blend2d/codec/pngcodec.cpp
  blend2d/codec/pngcodec_p.h
  blend2d/codec/pngops.cpp
  blend2d/codec/pngops_asimd.cpp
  blend2d/codec/pngops_avx.cpp
  blend2d/codec/pngops_sse2.cpp
  blend2d/codec/pngops_p.h
  blend2d/codec/pngopssimdimpl_p.h
  blend2d/codec/pngopssimdimpl_test.cpp
  blend2d/codec/qoicodec.cpp
  blend2d/codec/qoicodec_p.h

  blend2d/compression/checksum.cpp
  blend2d/compression/checksum_asimd.cpp
  blend2d/compression/checksum_asimd_crypto.cpp
  blend2d/compression/checksum_sse2.cpp
  blend2d/compression/checksum_sse4_2.cpp
  blend2d/compression/checksum_test.cpp
  blend2d/compression/checksum_p.h
  blend2d/compression/checksumadler32simdimpl_p.h
  blend2d/compression/checksumcrc32simdimpl_p.h
  blend2d/compression/deflatedecoder.cpp
  blend2d/compression/deflatedecoder_p.h
  blend2d/compression/deflatedecoderfast.cpp
  blend2d/compression/deflatedecoderfast_avx2.cpp
  blend2d/compression/deflatedecoderfast_p.h
  blend2d/compression/deflatedecoderfastimpl_p.h
  blend2d/compression/deflatedecoderutils.cpp
  blend2d/compression/deflatedecoderutils_p.h
  blend2d/compression/deflatedefs_p.h
  blend2d/compression/deflatedefs.cpp
  blend2d/compression/deflateencoder.cpp
  blend2d/compression/deflateencoder_p.h
  blend2d/compression/deflateencoderutils_p.h
  blend2d/compression/deflate_test.cpp
  blend2d/compression/matchfinder_p.h

  blend2d/geometry/bezier_p.h
  blend2d/geometry/bezier_test.cpp
  blend2d/geometry/commons_p.h
  blend2d/geometry/sizetable.cpp
  blend2d/geometry/sizetable_p.h
  blend2d/geometry/tolerance_p.h

  blend2d/opentype/otcff.cpp
  blend2d/opentype/otcff_test.cpp
  blend2d/opentype/otcff_p.h
  blend2d/opentype/otcmap.cpp
  blend2d/opentype/otcmap_p.h
  blend2d/opentype/otcore.cpp
  blend2d/opentype/otcore_p.h
  blend2d/opentype/otdefs_p.h
  blend2d/opentype/otface.cpp
  blend2d/opentype/otface_p.h
  blend2d/opentype/otglyf.cpp
  blend2d/opentype/otglyf_asimd.cpp
  blend2d/opentype/otglyf_avx2.cpp
  blend2d/opentype/otglyf_sse4_2.cpp
  blend2d/opentype/otglyf_p.h
  blend2d/opentype/otglyfsimddata_p.h
  blend2d/opentype/otglyfsimddata.cpp
  blend2d/opentype/otglyfsimdimpl_p.h
  blend2d/opentype/otkern.cpp
  blend2d/opentype/otkern_p.h
  blend2d/opentype/otlayout.cpp
  blend2d/opentype/otlayout_p.h
  blend2d/opentype/otlayoutcontext_p.h
  blend2d/opentype/otlayouttables_p.h
  blend2d/opentype/otmetrics.cpp
  blend2d/opentype/otmetrics_p.h
  blend2d/opentype/otname.cpp
  blend2d/opentype/otname_p.h
  blend2d/opentype/otplatform_p.h

  blend2d/pipeline/pipedefs.cpp
  blend2d/pipeline/pipedefs_p.h
  blend2d/pipeline/piperuntime.cpp
  blend2d/pipeline/piperuntime_p.h

  blend2d/pipeline/jit/compoppart.cpp
  blend2d/pipeline/jit/compoppart_p.h
  blend2d/pipeline/jit/compoputils_p.h
  blend2d/pipeline/jit/fetchgradientpart.cpp
  blend2d/pipeline/jit/fetchgradientpart_p.h
  blend2d/pipeline/jit/fetchpart.cpp
  blend2d/pipeline/jit/fetchpart_p.h
  blend2d/pipeline/jit/fetchpatternpart.cpp
  blend2d/pipeline/jit/fetchpatternpart_p.h
  blend2d/pipeline/jit/fetchpixelptrpart.cpp
  blend2d/pipeline/jit/fetchpixelptrpart_p.h
  blend2d/pipeline/jit/fetchsolidpart.cpp
  blend2d/pipeline/jit/fetchsolidpart_p.h
  blend2d/pipeline/jit/fetchutilsbilinear_p.h
  blend2d/pipeline/jit/fetchutilscoverage.cpp
  blend2d/pipeline/jit/fetchutilscoverage_p.h
  blend2d/pipeline/jit/fetchutilsinlineloops.cpp
  blend2d/pipeline/jit/fetchutilsinlineloops_p.h
  blend2d/pipeline/jit/fetchutilspixelaccess.cpp
  blend2d/pipeline/jit/fetchutilspixelaccess_p.h
  blend2d/pipeline/jit/fetchutilspixelgather.cpp
  blend2d/pipeline/jit/fetchutilspixelgather_p.h
  blend2d/pipeline/jit/fillpart.cpp
  blend2d/pipeline/jit/fillpart_p.h
  blend2d/pipeline/jit/jitbase_p.h
  blend2d/pipeline/jit/pipecompiler.cpp
  blend2d/pipeline/jit/pipecompiler_p.h
  blend2d/pipeline/jit/pipecomposer.cpp
  blend2d/pipeline/jit/pipecomposer_p.h
  blend2d/pipeline/jit/pipedebug_p.h
  blend2d/pipeline/jit/pipefunction_p.h
  blend2d/pipeline/jit/pipefunction.cpp
  blend2d/pipeline/jit/pipeprimitives.cpp
  blend2d/pipeline/jit/pipeprimitives_p.h
  blend2d/pipeline/jit/pipegenruntime_p.h
  blend2d/pipeline/jit/pipegenruntime.cpp
  blend2d/pipeline/jit/pipepart.cpp
  blend2d/pipeline/jit/pipepart_p.h

  blend2d/pipeline/reference/compopgeneric_p.h
  blend2d/pipeline/reference/fetchgeneric_p.h
  blend2d/pipeline/reference/fillgeneric_p.h
  blend2d/pipeline/reference/fixedpiperuntime_p.h
  blend2d/pipeline/reference/fixedpiperuntime.cpp
  blend2d/pipeline/reference/pixelbufferptr_p.h
  blend2d/pipeline/reference/pixelgeneric_p.h

  blend2d/pixelops/interpolation.cpp
  blend2d/pixelops/interpolation_avx2.cpp
  blend2d/pixelops/interpolation_sse2.cpp
  blend2d/pixelops/funcs.cpp
  blend2d/pixelops/funcs_p.h
  blend2d/pixelops/scalar_test.cpp
  blend2d/pixelops/scalar_p.h

  blend2d/raster/analyticrasterizer_test.cpp
  blend2d/raster/analyticrasterizer_p.h
  blend2d/raster/debugging_p.h
  blend2d/raster/edgebuilder_p.h
  blend2d/raster/edgestorage_p.h
  blend2d/raster/rastercontext.cpp
  blend2d/raster/rastercontext_p.h
  blend2d/raster/rastercontextops.cpp
  blend2d/raster/rastercontextops_p.h
  blend2d/raster/rasterdefs_p.h
  blend2d/raster/renderbatch_p.h
  blend2d/raster/rendercommand_p.h
  blend2d/raster/rendercommandprocasync_p.h
  blend2d/raster/rendercommandprocsync_p.h
  blend2d/raster/renderfetchdata.cpp
  blend2d/raster/renderfetchdata_p.h
  blend2d/raster/renderjob_p.h
  blend2d/raster/renderjobproc_p.h
  blend2d/raster/renderqueue_p.h
  blend2d/raster/rendertargetinfo.cpp
  blend2d/raster/rendertargetinfo_p.h
  blend2d/raster/statedata_p.h
  blend2d/raster/styledata_p.h
  blend2d/raster/workdata.cpp
  blend2d/raster/workdata_p.h
  blend2d/raster/workermanager.cpp
  blend2d/raster/workermanager_p.h
  blend2d/raster/workerproc.cpp
  blend2d/raster/workerproc_p.h
  blend2d/raster/workersynchronization.cpp
  blend2d/raster/workersynchronization_p.h

  blend2d/simd/simd_p.h
  blend2d/simd/simd_test.cpp
  blend2d/simd/simd_test_p.h
  blend2d/simd/simdbase_p.h
  blend2d/simd/simdarm_p.h
  blend2d/simd/simdarm_test_asimd.cpp
  blend2d/simd/simdx86_p.h
  blend2d/simd/simdx86_test_avx.cpp
  blend2d/simd/simdx86_test_avx2.cpp
  blend2d/simd/simdx86_test_avx512.cpp
  blend2d/simd/simdx86_test_sse2.cpp
  blend2d/simd/simdx86_test_sse4_1.cpp
  blend2d/simd/simdx86_test_sse4_2.cpp
  blend2d/simd/simdx86_test_ssse3.cpp

  blend2d/support/algorithm_test.cpp
  blend2d/support/algorithm_p.h
  blend2d/support/arenaallocator.cpp
  blend2d/support/arenaallocator_p.h
  blend2d/support/arenabitarray_test.cpp
  blend2d/support/arenabitarray_p.h
  blend2d/support/arenahashmap.cpp
  blend2d/support/arenahashmap_test.cpp
  blend2d/support/arenahashmap_p.h
  blend2d/support/arenalist_test.cpp
  blend2d/support/arenalist_p.h
  blend2d/support/arenatree_test.cpp
  blend2d/support/arenatree_p.h
  blend2d/support/bitops_test.cpp
  blend2d/support/bitops_p.h
  blend2d/support/fixedbitarray_p.h
  blend2d/support/hashops_p.h
  blend2d/support/intops_test.cpp
  blend2d/support/intops_p.h
  blend2d/support/lookuptable_p.h
  blend2d/support/math.cpp
  blend2d/support/math_test.cpp
  blend2d/support/math_p.h
  blend2d/support/memops_test.cpp
  blend2d/support/memops_p.h
  blend2d/support/ptrops_test.cpp
  blend2d/support/ptrops_p.h
  blend2d/support/scopedallocator.cpp
  blend2d/support/scopedallocator_p.h
  blend2d/support/scopedbuffer_p.h
  blend2d/support/stringops_p.h
  blend2d/support/wrap_p.h
  blend2d/support/zeroallocator.cpp
  blend2d/support/zeroallocator_test.cpp
  blend2d/support/zeroallocator_p.h

  blend2d/tables/tables.cpp
  blend2d/tables/tables_test.cpp
  blend2d/tables/tables_p.h

  blend2d/threading/atomic_p.h
  blend2d/threading/conditionvariable_p.h
  blend2d/threading/futex.cpp
  blend2d/threading/futex_p.h
  blend2d/threading/mutex_p.h
  blend2d/threading/thread.cpp
  blend2d/threading/thread_p.h
  blend2d/threading/threadingutils_p.h
  blend2d/threading/threadpool.cpp
  blend2d/threading/threadpool_test.cpp
  blend2d/threading/threadpool_p.h
  blend2d/threading/tsanutils_p.h
  blend2d/threading/uniqueidgenerator.cpp
  blend2d/threading/uniqueidgenerator_p.h

  blend2d/unicode/unicode.cpp
  blend2d/unicode/unicode_test.cpp
  blend2d/unicode/unicode_p.h
)

set(BLEND2D_BENCH_SRC
  blend2d-testing/bench/bl_bench_app.cpp
  blend2d-testing/bench/bl_bench_app.h
  blend2d-testing/bench/bl_bench_backend.cpp
  blend2d-testing/bench/bl_bench_backend.h
  blend2d-testing/bench/bl_bench_backend_agg.cpp
  blend2d-testing/bench/bl_bench_backend_blend2d.cpp
  blend2d-testing/bench/bl_bench_backend_cairo.cpp
  blend2d-testing/bench/bl_bench_backend_coregraphics.cpp
  blend2d-testing/bench/bl_bench_backend_juce.cpp
  blend2d-testing/bench/bl_bench_backend_qt.cpp
  blend2d-testing/bench/bl_bench_backend_skia.cpp
  blend2d-testing/bench/images_data.h
  blend2d-testing/bench/shape_data.cpp
  blend2d-testing/bench/shape_data.h
  blend2d-testing/commons/jsonbuilder.cpp
  blend2d-testing/commons/jsonbuilder.h
)

# Blend2D - CMake Utilities
# =========================

include(CheckCXXCompilerFlag)
include(CheckCSourceCompiles)
include(CheckLinkerFlag)
include(GNUInstallDirs)

# Detects target architecture - since the code doesn't run it works also for cross-compilation.
function(blend2d_detect_target_arch out)
  set(${out} "unknown" PARENT_SCOPE)
  foreach(target_arch "X86_64" "AARCH64" "X86" "AARCH32")
    check_c_source_compiles("
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
function(blend2d_detect_cflags out)
  set(_out_array ${${out}})
  string(REGEX REPLACE "[+]" "x" _flag_signature "${ARGN}")
  string(REGEX REPLACE "[-= :;/.\]" "_" _flag_signature "${_flag_signature}")

  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
    check_cxx_compiler_flag("${ARGN} -Werror" "__CxxFlag_${_flag_signature}")
  else()
    check_cxx_compiler_flag("${ARGN}" "__CxxFlag_${_flag_signature}")
  endif()

  if (${__CxxFlag_${_flag_signature}})
    list(APPEND _out_array ${ARGN})
  endif()
  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

# Detects C++ link flags and appends all detected ones to `out`.
function(blend2d_detect_lflags out)
  set(_out_array ${${out}})

  foreach(_flag ${ARGN})
    string(REGEX REPLACE "[+]" "x" _flag_signature "${_flag}")
    string(REGEX REPLACE "[-= :;/.\]" "_" _flag_signature "${_flag_signature}")
    check_linker_flag(CXX "${_flag}" "__LinkFlag_${_flag_signature}")
    if (${__LinkFlag_${_flag_signature}})
      list(APPEND _out_array ${_flag})
    endif()
  endforeach()

  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

# Support for various sanitizers provided by C/C++ compilers.
function(blend2d_detect_sanitizers out)
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
    blend2d_detect_cflags(_out_array ${_flag})
    unset(CMAKE_REQUIRED_FLAGS)
  endforeach()

  set(${out} "${_out_array}" PARENT_SCOPE)
endfunction()

function(blend2d_add_target target target_type)
  set(multi_value_args SOURCES INCLUDE_DIRS CFLAGS CFLAGS_DBG CFLAGS_REL LFLAGS LIBRARIES)
  cmake_parse_arguments("ARG" "" "" "${multi_value_args}" ${ARGN})

  if ("${target_type}" MATCHES "^(EXECUTABLE|EXECUTABLE_WIN32|TEST)$")
    if ("${target_type}" MATCHES "^(EXECUTABLE_WIN32)$")
      add_executable(${target} WIN32 ${ARG_SOURCES})
    else()
      add_executable(${target} ${ARG_SOURCES})
    endif()
    set_target_properties(${target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(TargetDir).")
  else()
    add_library(${target} ${target_type} ${ARG_SOURCES})
  endif()

  set_target_properties(${target} PROPERTIES DEFINE_SYMBOL "" CXX_VISIBILITY_PRESET hidden)
  target_include_directories(${target} BEFORE PRIVATE ${ARG_INCLUDE_DIRS})
  target_compile_options(${target} PRIVATE ${ARG_CFLAGS} ${BLEND2D_SANITIZE_CFLAGS} $<IF:$<CONFIG:Debug>,${ARG_CFLAGS_DBG},${ARG_CFLAGS_REL}>)
  target_compile_features(${target} PUBLIC cxx_std_17)
  target_link_options(${target} PRIVATE ${ARG_LFLAGS} ${BLEND2D_SANITIZE_LFLAGS})
  target_link_libraries(${target} PRIVATE ${ARG_LIBRARIES})

  if ("${target_type}" STREQUAL "TEST")
    add_test(NAME ${target} COMMAND ${target})
  endif()
endfunction()

# Blend2D - Features
# ==================

blend2d_detect_target_arch(BLEND2D_TARGET_ARCH)

if (BLEND2D_NO_JIT)
  message(STATUS "[blend2d] Disabling JIT compiler support ('BLEND2D_TARGET_ARCH=${BLEND2D_TARGET_ARCH}', but 'BLEND2D_NO_JIT=${BLEND2D_NO_JIT}')")
  set(BLEND2D_BUILD_WITH_JIT FALSE)
elseif ("${BLEND2D_TARGET_ARCH}" MATCHES "^(X86|X86_64|AARCH64)$")
  message(STATUS "[blend2d] Enabling JIT compiler support ('BLEND2D_TARGET_ARCH=${BLEND2D_TARGET_ARCH}' is supported)")
  set(BLEND2D_BUILD_WITH_JIT TRUE)
else()
  message(STATUS "[blend2d] Disabling JIT compiler support ('BLEND2D_TARGET_ARCH=${BLEND2D_TARGET_ARCH}' isn't supported)")
  set(BLEND2D_BUILD_WITH_JIT FALSE)
endif()

if (BLEND2D_NO_TLS)
  message(STATUS "[blend2d] Disabling use of TLS ('BLEND2D_NO_TLS=${BLEND2D_NO_TLS}')")
  list(APPEND BLEND2D_PRIVATE_CFLAGS -DBL_BUILD_NO_TLS)
endif()

if (BLEND2D_NO_FUTEX)
  message(STATUS "[blend2d] Disabling futex support ('BLEND2D_NO_FUTEX=${BLEND2D_NO_FUTEX}')")
  list(APPEND BLEND2D_PRIVATE_CFLAGS -DBL_BUILD_NO_FUTEX)
endif()

# Blend2D - Compiler Support - Detection
# ======================================

if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
  list(APPEND BLEND2D_WARNING_CFLAGS -W4)                                       # [+] Warning level 4.

  list(APPEND BLEND2D_PRIVATE_CFLAGS -MP)                                       # [+] Multi-Process Compilation.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -GR-)                                      # [-] Runtime type information.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -GF)                                       # [+] Eliminate duplicate strings.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -Zc:__cplusplus)                           # [+] Conforming __cplusplus definition.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -Zc:inline)                                # [+] Remove unreferenced COMDAT.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -Zc:strictStrings)                         # [+] Strict const qualification of string literals.
  list(APPEND BLEND2D_PRIVATE_CFLAGS -Zc:threadSafeInit-)                       # [-] Thread-safe statics.

  list(APPEND BLEND2D_PRIVATE_CFLAGS_DBG -GS)                                   # [+] Buffer security-check.
  list(APPEND BLEND2D_PRIVATE_CFLAGS_REL -GS-)                                  # [-] Buffer security-check.
  list(APPEND BLEND2D_PRIVATE_CFLAGS_REL -O2)                                   # [+] Favor speed over size.
  list(APPEND BLEND2D_PRIVATE_CFLAGS_REL -Oi)                                   # [+] Generate Intrinsic Functions.

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND BLEND2D_PRIVATE_CFLAGS -clang:-fno-rtti -clang:-fno-math-errno -clang:-fno-trapping-math)
  endif()
  blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS -Zc:arm64-aliased-neon-types-)   # [-] ARM64 aliased neon types
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
  list(APPEND BLEND2D_WARNING_CFLAGS -Wall -Wextra -Wconversion)                # [+] Add baseline warnings that can be used safely even with system headers.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wdouble-promotion)              # [+] Warn about double promotions.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wduplicated-cond)               # [+] Warn about duplicate conditions.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wduplicated-branches)           # [+] Warn about duplicate branches.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wlogical-op)                    # [+] Warn about suspicious uses of logical operators in expressions.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wlogical-not-parentheses)       # [+] Warn about logical not used on the left hand side operand of a comparison.
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wrestrict)
  blend2d_detect_cflags(BLEND2D_WARNING_CFLAGS -Wbidi-chars=any)                # [+] Warn about using BIDI characters in source code (we don't use them).

  list(APPEND BLEND2D_PRIVATE_CFLAGS -fno-math-errno)                           # [-] Disable math functions setting errno (performance reasons).
  list(APPEND BLEND2D_PRIVATE_CFLAGS -fno-threadsafe-statics)                   # [-] Don't add guards when initializing statics (we don't need it).
  list(APPEND BLEND2D_PRIVATE_CFLAGS -fno-exceptions -fno-rtti)                 # [-] Disable exceptions and RTTI (we never throw, we don't need RTTI).
  list(APPEND BLEND2D_PRIVATE_CFLAGS_REL -O2)                                   # [+] Compiling with -O2 in release mode is what we generally want.
  list(APPEND BLEND2D_PRIVATE_CFLAGS_REL -fmerge-all-constants)                 # [+] We don't need unique address per constant (merging improves library size).

  # -fno-semantic-interposition is not available on apple - the compiler issues a warning, which is not detected.
  if (NOT APPLE)
    blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS -fno-semantic-interposition)
  endif()

  if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
    list(APPEND BLEND2D_PRIVATE_CFLAGS -fno-trapping-math -fno-finite-math-only)# [-] Disable these flags in case somebody uses `-ffast-math` globally.
    blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS_REL -fno-enforce-eh-specs)     # [-] We never throw, so we don't need strict exception handling enforcement.
  endif()

  blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS -mllvm --disable-loop-idiom-all) # [-] Don't use memset/memcpy instead of our own loops (critical in some cases).
  blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS_REL -ftree-vectorize)            # [+] Auto-vectorize by default in release builds (sometimes helps).

  # Building Blend2D without C++ library support requires these to be setup.
  if (BLEND2D_NO_STDCXX)
    message(STATUS "[blend2d] Enabling build without linking to the C++ standard library ('BLEND2D_NO_STDCXX=${BLEND2D_NO_STDCXX}')")
    list(APPEND BLEND2D_NO_STDCXX_CFLAGS -DBL_BUILD_NO_STDCXX)
    blend2d_detect_lflags(BLEND2D_NO_STDCXX_LFLAGS -static-libgcc -static-libstdc++)
  endif()
endif()

if (BLEND2D_TARGET_ARCH MATCHES "^(X86|X86_64)$")
  set(BLEND2D_PRIVATE_CFLAGS_SSE2 "")
  if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
    if ("${BLEND2D_TARGET_ARCH}" STREQUAL "X86")
      set(BLEND2D_PRIVATE_CFLAGS_SSE2 -arch:SSE2) # 32-bit target requires -arch:SSE2.
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      # To enable certain optimizations for clang-cl additional flags have to be passed (MSVC incompatibility).
      set(BLEND2D_PRIVATE_CFLAGS_SSE3    ${BLEND2D_PRIVATE_CFLAGS_SSE2} -msse3)
      set(BLEND2D_PRIVATE_CFLAGS_SSSE3   ${BLEND2D_PRIVATE_CFLAGS_SSE2} -mssse3)
      set(BLEND2D_PRIVATE_CFLAGS_SSE4_1  ${BLEND2D_PRIVATE_CFLAGS_SSE2} -msse4.1)
      set(BLEND2D_PRIVATE_CFLAGS_SSE4_2  ${BLEND2D_PRIVATE_CFLAGS_SSE2} -msse4.2 -mpopcnt -mpclmul)
      set(BLEND2D_PRIVATE_CFLAGS_AVX     -arch:AVX    -mpopcnt -mpclmul)
      set(BLEND2D_PRIVATE_CFLAGS_AVX2    -arch:AVX2   -mpopcnt -mpclmul -mbmi -mbmi2)
      set(BLEND2D_PRIVATE_CFLAGS_AVX2FMA -arch:AVX2   -mpopcnt -mpclmul -mbmi -mbmi2 -mfma)
      set(BLEND2D_PRIVATE_CFLAGS_AVX512  -arch:AVX512 -mpopcnt -mpclmul -mbmi -mbmi2)
    else()
      # MSVC doesn't distinguish between some optimization levels - so add some definitions manually.
      set(BLEND2D_PRIVATE_CFLAGS_SSE3    ${BLEND2D_PRIVATE_CFLAGS_SSE2} -D__SSE3__)
      set(BLEND2D_PRIVATE_CFLAGS_SSSE3   ${BLEND2D_PRIVATE_CFLAGS_SSE2} -D__SSSE3__)
      set(BLEND2D_PRIVATE_CFLAGS_SSE4_1  ${BLEND2D_PRIVATE_CFLAGS_SSE2} -D__SSE4_1__)
      set(BLEND2D_PRIVATE_CFLAGS_SSE4_2  ${BLEND2D_PRIVATE_CFLAGS_SSE2} -D__SSE4_2__)
      set(BLEND2D_PRIVATE_CFLAGS_AVX     -arch:AVX)
      set(BLEND2D_PRIVATE_CFLAGS_AVX2    -arch:AVX2)
      set(BLEND2D_PRIVATE_CFLAGS_AVX2FMA -arch:AVX2)
      set(BLEND2D_PRIVATE_CFLAGS_AVX512  -arch:AVX512)
    endif()
  else()
    if ("${BLEND2D_TARGET_ARCH}" STREQUAL "X86")
      if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(BLEND2D_PRIVATE_CFLAGS_SSE2 -msse2 -mfpmath=sse)
      else()
        set(BLEND2D_PRIVATE_CFLAGS_SSE2 -msse2)
      endif()
    endif()
    set(BLEND2D_PRIVATE_CFLAGS_SSE3    -msse3)
    set(BLEND2D_PRIVATE_CFLAGS_SSSE3   -mssse3)
    set(BLEND2D_PRIVATE_CFLAGS_SSE4_1  -msse4.1)
    set(BLEND2D_PRIVATE_CFLAGS_SSE4_2  -mpopcnt -mpclmul -msse4.2)
    set(BLEND2D_PRIVATE_CFLAGS_AVX     -mpopcnt -mpclmul -mavx)
    set(BLEND2D_PRIVATE_CFLAGS_AVX2    -mpopcnt -mpclmul -mbmi -mbmi2 -mavx2)
    set(BLEND2D_PRIVATE_CFLAGS_AVX2FMA -mpopcnt -mpclmul -mbmi -mbmi2 -mavx2 -mfma)
    set(BLEND2D_PRIVATE_CFLAGS_AVX512  -mpopcnt -mpclmul -mbmi -mbmi2 -mavx512f  -mavx512bw -mavx512dq -mavx512cd -mavx512vl)
  endif()

  list(APPEND BLEND2D_PRIVATE_CFLAGS_AVX2    -DBL_TARGET_OPT_POPCNT -DBL_TARGET_OPT_BMI2)
  list(APPEND BLEND2D_PRIVATE_CFLAGS_AVX2FMA -DBL_TARGET_OPT_POPCNT -DBL_TARGET_OPT_BMI2 -DBL_TARGET_OPT_FMA)
  list(APPEND BLEND2D_PRIVATE_CFLAGS_AVX512  -DBL_TARGET_OPT_POPCNT -DBL_TARGET_OPT_BMI2)

  # SSE2 is our baseline if no other baseline is specified; AVX512 is our maximum.
  list(APPEND BLEND2D_PRIVATE_CFLAGS ${BLEND2D_PRIVATE_CFLAGS_SSE2} -DBL_BUILD_OPT_AVX512)
elseif ("${BLEND2D_TARGET_ARCH}" MATCHES "^(AARCH32|AARCH64)$")
  if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
    # FIXME: Not sure whether it makes sense to detect anything in this case.
  else()
    blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS_ASIMD -mfpu=neon-vfpv4)
    if ("${BLEND2D_TARGET_ARCH}" STREQUAL "AARCH64")
      blend2d_detect_cflags(BLEND2D_PRIVATE_CFLAGS_ASIMD_CRYPTO -march=armv8-a+aes+crc+crypto)
    endif()
  endif()

  if (BLEND2D_PRIVATE_CFLAGS_ASIMD)
    list(APPEND BLEND2D_PRIVATE_CFLAGS -DBL_BUILD_OPT_ASIMD)
    list(APPEND BLEND2D_PRIVATE_CFLAGS_ASIMD -DBL_TARGET_OPT_ASIMD)
  endif()

  if (BLEND2D_PRIVATE_CFLAGS_ASIMD_CRYPTO)
    list(APPEND BLEND2D_PRIVATE_CFLAGS -DBL_BUILD_OPT_ASIMD_CRYPTO)
    list(APPEND BLEND2D_PRIVATE_CFLAGS_ASIMD_CRYPTO -DBL_TARGET_OPT_ASIMD_CRYPTO)
  endif()
endif()

list(APPEND BLEND2D_PRIVATE_CFLAGS ${BLEND2D_WARNING_CFLAGS})

# Blend2D - Compiler Support - Sanitizers
# =======================================

if (BLEND2D_SANITIZE)
  blend2d_detect_sanitizers(BLEND2D_SANITIZE_CFLAGS ${BLEND2D_SANITIZE})
  if (BLEND2D_SANITIZE_CFLAGS)
    message(STATUS "[blend2d] Enabling sanitizers: '${BLEND2D_SANITIZE_CFLAGS}'")

    # Linker must receive the same flags as the compiler when it comes to sanitizers.
    set(BLEND2D_SANITIZE_LFLAGS ${BLEND2D_SANITIZE_CFLAGS})

    # Don't omit frame pointer if sanitizers are enabled.
    if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC" OR "x${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "xMSVC")
      list(APPEND BLEND2D_SANITIZE_CFLAGS -Oy-)
    else()
      list(APPEND BLEND2D_SANITIZE_CFLAGS -fno-omit-frame-pointer -g)
    endif()

    list(APPEND BLEND2D_PRIVATE_CFLAGS ${BLEND2D_SANITIZE_CFLAGS})
    list(APPEND BLEND2D_PRIVATE_LFLAGS ${BLEND2D_SANITIZE_LFLAGS})
  endif()
endif()

# Blend2D - Dependencies
# ======================

if ("${CMAKE_SYSTEM_NAME}" MATCHES "^Windows")
  # Dependency: Windows target needs no extra dependencies, however, we have to check for .natvis support.
  if(CMAKE_LINKER MATCHES "^(lld-link|link)\\.exe$")
    set(BLEND2D_LINKER_SUPPORTS_NATVIS TRUE)
  endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  # Dependency: libc is the only required library on Android as it also provides libpthread.
  message(STATUS "[blend2d] Adding libc dependency (Android)")
  list(APPEND BLEND2D_LIBS c)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
  # Dependency: libroot is used by Haiku instead of libc, so link to libroot and libpthread.
  message(STATUS "[blend2d] Adding libroot, libm, and libpthread dependencies (Haiku)")
  list(APPEND BLEND2D_LIBS root m pthread)
else()
  # Dependency: libc is always required.
  message(STATUS "[blend2d] Adding libc dependency (Linux, BSD, or other UNIX/POSIX environment)")
  list(APPEND BLEND2D_LIBS c)

  # Dependency: pthread (required so Blend2D can use pthread_rwlock, pthread_create, ...).
  check_c_source_compiles("
    #include <pthread.h>

    static void* thread_entry_point(void* arg) { return arg; }
    int main() {
      pthread_rwlock_t m;
      pthread_rwlock_init(&m, (void*)0);
      pthread_rwlock_wrlock(&m);
      pthread_rwlock_unlock(&m);

      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      pthread_attr_setstacksize(&attr, 1024*1024);

      pthread_t handle;
      return pthread_create(&handle, &attr, thread_entry_point, (void*)0);
    }
    " BLEND2D_LIBC_HAS_LIBPTHREAD)
  if (BLEND2D_LIBC_HAS_LIBPTHREAD)
    message(STATUS "[blend2d] Library 'libpthread' provided by 'libc' (not linking to libpthread)")
  else()
    message(STATUS "[blend2d] Library 'libpthread' not provided by 'libc' (linking to libpthread)")
    list(APPEND BLEND2D_LIBS pthread)
  endif()

  check_c_source_compiles("
    #include <math.h>
    int main(int argc, char* argv[]) {
      (void)argv; return (int)sin((double)argc * 0.1234);
    }
    " BLEND2D_LIBC_HAS_LIBM)
  if (BLEND2D_LIBC_HAS_LIBM)
    message(STATUS "[blend2d] Library 'libm' provided by 'libc' (not linking to libm)")
  else()
    message(STATUS "[blend2d] Library 'libm' not provided by 'libc' (linking to libm)")
    list(APPEND BLEND2D_LIBS m)
  endif()
endif()

# Find asmjit dependency if building with JIT.
if (BLEND2D_BUILD_WITH_JIT)
  if (BLEND2D_EXTERNAL_ASMJIT)
    message(STATUS "[blend2d] Trying to find external asmjit ('BLEND2D_EXTERNAL_ASMJIT=${BLEND2D_EXTERNAL_ASMJIT}')\n")
    message(WARNING
      "Using external asmjit library means you are opting out of using the\n"
      "bundled one, which is the preferred way especially if Blend2D is compiled\n"
      "as a shared library. In that case asmjit is configured by Blend2D to only\n"
      "include the target architecture that is actually used (all foreign backends\n"
      "are disabled) and is built the same way as Blend2D (without exceptions/rtti).\n"
      "\n"
      "In general, using external asmjit option was introduced for package managers\n"
      "that prefer each dependency to be built separately. In this case the burden\n"
      "of configuring the dependency is on you.\n"
      "\n"
      "Do you want a smaller binary? Use 'ASMJIT_NO_FOREIGN' option to disable foreign\n"
      "architectures.\n"
      "\n"
      "If you are concerned about security (not using anything bundled) you can\n"
      "just git checkout https://github.com/asmjit/asmjit into [root]/3rdparty/asmjit\n"
      "or set ASMJIT_DIR variable to point to a previously checked out asmjit.\n")
    find_package(asmjit REQUIRED)
    list(APPEND BLEND2D_LIBS asmjit::asmjit)
  else()
    if (NOT DEFINED ASMJIT_DIR)
      foreach(dir "${CMAKE_CURRENT_LIST_DIR}/3rdparty/asmjit" "${CMAKE_CURRENT_LIST_DIR}/../asmjit")
        if (EXISTS ${dir}/CMakeLists.txt)
          set(ASMJIT_DIR "${dir}" CACHE PATH "Location of 'asmjit'")
          message(STATUS "[blend2d] AsmJit found ('BLEND2D_EXTERNAL_ASMJIT=OFF' => 'ASMJIT_DIR=${ASMJIT_DIR}')")
          break()
        endif()
      endforeach()
      if (NOT DEFINED ASMJIT_DIR)
        message(FATAL
          "[blend2d] Unable to find asmjit, please see <https://blend2d.com/doc/build-instructions.html>\n"
          "\n"
          "Blend2D expects asmjit to be in the following directories:\n"
          "\n"
          "  - ASMJIT_DIR               - CMake variable specifying asmjit directory\n"
          "  - '<root>/3rdparty/asmjit' - asmjit placed into 3rdparty subdirectory (dev & releases)\n"
          "  - '<root>/../asmjit'       - asmjit placed outside of blend2d project (development)\n"
          "\n"
          "Alternatively, if you use C/C++ package managers and would like to use\n"
          "a packaged asmjit instead, set 'BLEND2D_EXTERNAL_ASMJIT' cmake variable to ON.\n"
          "This way Blend2D would not try to auto-detect the location of asmjit and would\n"
          "try to find it by using find_package() instead. Please note that using extenal\n"
          "asmjit means that Blend2D would not be able to avoid linking to a C++ standard\n"
          "library, which it avoids by default when building a shared Blend2D library with\n"
          "bundled asmjit."
        )
      endif()
    endif()

    if (NOT DEFINED ASMJIT_EMBED)
      set(ASMJIT_EMBED TRUE CACHE BOOL "")
    endif()

    add_subdirectory("${ASMJIT_DIR}" asmjit)
    list(APPEND BLEND2D_LIBS $<BUILD_INTERFACE:asmjit::asmjit>)
    list(APPEND BLEND2D_PRIVATE_CFLAGS -DASMJIT_NO_FOREIGN -DASMJIT_NO_STDCXX -DASMJIT_ABI_NAMESPACE=abi_bl)

    # A possibility to reduce the resulting binary size by disabling asmjit logging.
    if (BLEND2D_NO_JIT_LOGGING)
      message(STATUS "[blend2d] Disabling asmjit logging functionality (JIT)")
      list(APPEND BLEND2D_PRIVATE_CFLAGS -DASMJIT_NO_LOGGING -DASMJIT_NO_TEXT)
    endif()
  endif()
else()
  list(APPEND BLEND2D_PRIVATE_CFLAGS -DBL_BUILD_NO_JIT)
endif()

# Blend2D - Finalize Build Options
# ================================

list(REMOVE_DUPLICATES BLEND2D_LIBS)
list(REMOVE_DUPLICATES BLEND2D_PRIVATE_CFLAGS)

# Blend2D - Source Files Processing
# =================================

if (MSVC AND NOT BLEND2D_NO_NATVIS)
  list(APPEND BLEND2D_SRC_LIST blend2d/blend2d.natvis)
endif()

set(BLEND2D_SRC "")
set(BLEND2D_TEST_SRC "")
set(BLEND2D_OPT_EXTENSIONS "sse2|sse3|ssse3|sse4_1|sse4_2|avx|avx2|avx2fma|avx512|asimd|asimd_crypto")

foreach(src_file ${BLEND2D_SRC_LIST})
  set(src_file "${CMAKE_CURRENT_LIST_DIR}/${src_file}")

  if ("${src_file}" MATCHES "_test(_(${BLEND2D_OPT_EXTENSIONS}))?\\.cpp$")
    list(APPEND BLEND2D_TEST_SRC ${src_file})
  else()
    list(APPEND BLEND2D_SRC ${src_file})
  endif()

  string(REGEX MATCH "_(${BLEND2D_OPT_EXTENSIONS})\\.(c|cc|cxx|cpp|m|mm)$" FEATURE ${src_file})
  if (FEATURE)
    # HACK 1: Cmake uses global variables everywhere, `CMAKE_MATCH_1` is the first capture...
    string(TOUPPER "${CMAKE_MATCH_1}" FEATURE)
    set_property(SOURCE "${src_file}" APPEND PROPERTY COMPILE_OPTIONS ${BLEND2D_PRIVATE_CFLAGS_${FEATURE}})
  endif()

  if ("${src_file}" MATCHES "\\.natvis")
    if (BLEND2D_LINKER_SUPPORTS_NATVIS)
      list(APPEND BLEND2D_PRIVATE_LFLAGS "-natvis:${src_file}")
    endif()
  endif()
endforeach()

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${BLEND2D_SRC})

# Blend2D - Targets - Core Library & 'blend2d::blend2d' Alias
# ===========================================================

message(STATUS "[blend2d] Adding 'blend2d::blend2d' target ('BLEND2D_TARGET_TYPE=${BLEND2D_TARGET_TYPE}')")
foreach(v "BLEND2D_LIBS"
          "BLEND2D_CFLAGS"
          "BLEND2D_WARNING_CFLAGS"
          "BLEND2D_PRIVATE_CFLAGS"
          "BLEND2D_PRIVATE_CFLAGS_DBG"
          "BLEND2D_PRIVATE_CFLAGS_REL"
          "BLEND2D_PRIVATE_CFLAGS_SSE3"
          "BLEND2D_PRIVATE_CFLAGS_SSSE3"
          "BLEND2D_PRIVATE_CFLAGS_SSE4_1"
          "BLEND2D_PRIVATE_CFLAGS_SSE4_2"
          "BLEND2D_PRIVATE_CFLAGS_AVX"
          "BLEND2D_PRIVATE_CFLAGS_AVX2"
          "BLEND2D_PRIVATE_CFLAGS_AVX2FMA"
          "BLEND2D_PRIVATE_CFLAGS_AVX512"
          "BLEND2D_PRIVATE_CFLAGS_ASIMD"
          "BLEND2D_PRIVATE_CFLAGS_ASIMD_CRYPTO"
          "BLEND2D_SANITIZE_CFLAGS"
          "BLEND2D_SANITIZE_LFLAGS")
  if (NOT "${${v}}" STREQUAL "")
    message("   ${v}=${${v}}")
  endif()
endforeach()

blend2d_add_target(blend2d ${BLEND2D_TARGET_TYPE}
  SOURCES      ${BLEND2D_SRC}
  LIBRARIES    ${BLEND2D_LIBS}
  CFLAGS       ${BLEND2D_PRIVATE_CFLAGS} ${BLEND2D_NO_STDCXX_CFLAGS}
  CFLAGS_DBG   ${BLEND2D_PRIVATE_CFLAGS_DBG}
  CFLAGS_REL   ${BLEND2D_PRIVATE_CFLAGS_REL}
  LFLAGS       ${BLEND2D_NO_STDCXX_LFLAGS})

target_compile_options(blend2d PUBLIC ${BLEND2D_CFLAGS})
target_include_directories(blend2d BEFORE PUBLIC $<BUILD_INTERFACE:${BLEND2D_INCLUDE_DIRS}>
                                                 $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(blend2d::blend2d ALIAS blend2d)

# Add Blend2D install instructions (library and public headers).
if (NOT BLEND2D_NO_INSTALL)
  message(STATUS "[blend2d] Enabling install support ('BLEND2D_NO_INSTALL=OFF')")

  include(CMakePackageConfigHelpers)

  install(TARGETS blend2d
          EXPORT blend2d-targets
          RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
          ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  install(EXPORT blend2d-targets
          NAMESPACE blend2d::
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/blend2d")

  configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/blend2d-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/blend2d-config.cmake"
          INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/blend2d"
          NO_SET_AND_CHECK_MACRO
          NO_CHECK_REQUIRED_COMPONENTS_MACRO)

  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blend2d-config.cmake"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/blend2d")

  foreach(src_file ${BLEND2D_SRC_LIST})
    if ("${src_file}" MATCHES "\\.h$" AND NOT "${src_file}" MATCHES "_p\\.h$")
      get_filename_component(_src_dir ${src_file} PATH)
      install(FILES "${CMAKE_CURRENT_LIST_DIR}/${src_file}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_src_dir}")
    endif()
  endforeach()
else()
  message(STATUS "[blend2d] Disabling install support ('BLEND2D_NO_INSTALL=${BLEND2D_NO_INSTALL}')")
endif()

# Blend2D - Targets - Tests
# =========================

if (BLEND2D_TEST)
  enable_testing()
  message(STATUS "[blend2d] Enabling Blend2D tests ('BLEND2D_TEST=${BLEND2D_TEST}')")

  # Blend2D Tests
  # -------------

  # bl_test_runner - optimize static builds (no need to build Blend2D twice).
  if (BLEND2D_STATIC)
    blend2d_add_target(bl_test_runner TEST
      SOURCES      ${BLEND2D_TEST_SRC}
                   blend2d-testing/tests/bl_test_runner.cpp
                   blend2d-testing/tests/broken.cpp
                   blend2d-testing/tests/broken.h
      LIBRARIES    ${BLEND2D_LIBS} blend2d::blend2d
      CFLAGS       ${BLEND2D_PRIVATE_CFLAGS} -DBL_TEST -DBL_STATIC
      CFLAGS_DBG   ${BLEND2D_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL   ${BLEND2D_PRIVATE_CFLAGS_REL}
      INCLUDE_DIRS ${ASMJIT_INCLUDE_DIRS})
  else()
    blend2d_add_target(bl_test_runner TEST
      SOURCES      ${BLEND2D_TEST_SRC}
                   ${BLEND2D_SRC}
                   blend2d-testing/tests/bl_test_runner.cpp
                   blend2d-testing/tests/broken.cpp
                   blend2d-testing/tests/broken.h
      LIBRARIES    ${BLEND2D_LIBS}
      CFLAGS       ${BLEND2D_PRIVATE_CFLAGS} -DBL_TEST -DBL_STATIC
      CFLAGS_DBG   ${BLEND2D_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL   ${BLEND2D_PRIVATE_CFLAGS_REL}
      INCLUDE_DIRS ${ASMJIT_INCLUDE_DIRS} ${BLEND2D_INCLUDE_DIRS})
  endif()

  # bl_test_context_xxx are all based on the same base-app.
  foreach(app bl_test_context_simple bl_test_context_mt bl_test_context_jit)
    blend2d_add_target(${app} TEST
      SOURCES    blend2d-testing/tests/${app}.cpp
                 blend2d-testing/tests/bl_test_context_baseapp.cpp
                 blend2d-testing/tests/bl_test_context_baseapp.h
                 blend2d-testing/tests/bl_test_context_utilities.h
      LIBRARIES  blend2d::blend2d
      CFLAGS     ${BLEND2D_PRIVATE_CFLAGS}
      CFLAGS_DBG ${BLEND2D_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL ${BLEND2D_PRIVATE_CFLAGS_REL})
  endforeach()

  blend2d_add_target(bl_test_image_io EXECUTABLE
    SOURCES    blend2d-testing/tests/bl_test_image_io.cpp
    LIBRARIES  blend2d::blend2d
    CFLAGS     ${BLEND2D_PRIVATE_CFLAGS}
    CFLAGS_DBG ${BLEND2D_PRIVATE_CFLAGS_DBG}
    CFLAGS_REL ${BLEND2D_PRIVATE_CFLAGS_REL})

  # Add a limited version of 'bl_bench' in case we are building tests, but not demos.
  if (NOT BLEND2D_DEMOS)
    blend2d_add_target(bl_bench EXECUTABLE
      SOURCES    ${BLEND2D_BENCH_SRC}
      LIBRARIES  blend2d::blend2d
      CFLAGS     ${BLEND2D_PRIVATE_CFLAGS}
      CFLAGS_DBG ${BLEND2D_PRIVATE_CFLAGS_DBG}
      CFLAGS_REL ${BLEND2D_PRIVATE_CFLAGS_REL})
  endif()

  # Blend2D C & C++ Samples
  # -----------------------

  foreach(app bl_sample_1 bl_sample_2 bl_sample_3 bl_sample_4 bl_sample_5 bl_sample_6 bl_sample_7 bl_sample_8)
    blend2d_add_target(${app} EXECUTABLE
      SOURCES   blend2d-testing/samples/${app}.cpp
      LIBRARIES blend2d::blend2d
      CFLAGS    ${BLEND2D_WARNING_CFLAGS})
  endforeach()

  foreach(app bl_sample_capi)
    blend2d_add_target(${app} EXECUTABLE
      SOURCES   blend2d-testing/samples/${app}.c
      LIBRARIES blend2d::blend2d
      CFLAGS    ${BLEND2D_WARNING_CFLAGS})

    # Sanitizers usually require a C++ linker and library support.
    if (BLEND2D_SANITIZE_CFLAGS)
      set_target_properties(${app} PROPERTIES LINKER_LANGUAGE CXX)
    endif()
  endforeach()

  # Blend2D Generator
  # -----------------

  blend2d_add_target(bl_generator EXECUTABLE
    SOURCES      blend2d-testing/generator/bl_generator.cpp
                 blend2d-testing/generator/bl_generator.h
    INCLUDE_DIRS ${BLEND2D_INCLUDE_DIRS}
    CFLAGS       ${BLEND2D_WARNING_CFLAGS})
  if (NOT WIN32)
    target_link_libraries(bl_generator "pthread")
  endif()

  # TODO: Not sure this is the best way of copying resources.
  foreach(resource ABeeZee-Regular.ttf Leaves.jpeg)
    if (NOT CMAKE_BUILD_TYPE)
      add_custom_command(TARGET blend2d POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/blend2d-testing/resources/${resource}"
          "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/${resource}"
        COMMENT "Resources")
    else()
      add_custom_command(TARGET blend2d POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/blend2d-testing/resources/${resource}"
          "${CMAKE_CURRENT_BINARY_DIR}/${resource}"
        COMMENT "Resources")
    endif()
  endforeach()
else()
  message(STATUS "[blend2d] Disabling Blend2D tests ('BLEND2D_TEST=OFF')")
endif()

# Blend2D - Targets - Demos + Benchmarking
# ========================================

if (BLEND2D_DEMOS)
  include(FindPkgConfig)
  message(STATUS "[blend2d] Enabling Blend2D demos ('BLEND2D_DEMOS=${BLEND2D_DEMOS}')")

  set(BLEND2D_BENCH_SRCS "")
  set(BLEND2D_BENCH_LIBS "")
  set(BLEND2D_BENCH_CFLAGS "")
  set(BLEND2D_BENCH_INCLUDE_DIRS "")
  set(BLEND2D_BENCH_LIBRARY_DIRS "")

  # Demos - Probe Dependencies
  find_package(Qt6 COMPONENTS Core Gui Widgets)
  if(Qt6Gui_FOUND)
    message(STATUS "[blend2d] Qt6 found - adding support for Qt6 (benchmarking + demo apps)")

    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_QT)
    list(APPEND BLEND2D_BENCH_LIBS Qt6::Gui Qt6::Widgets)
  else()
    message(STATUS "[blend2d] Qt6 not found - cannot build Qt6 benchmark backend and demo apps")
  endif()

  pkg_check_modules(CAIRO cairo)
  if(CAIRO_FOUND)
    message(STATUS "[blend2d] Cairo found - adding support for cairo (benchmarking)")

    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_CAIRO)
    list(APPEND BLEND2D_BENCH_LIBS ${CAIRO_LIBRARIES})
    list(APPEND BLEND2D_BENCH_LIBRARY_DIRS ${CAIRO_LIBRARY_DIRS})
    list(APPEND BLEND2D_BENCH_INCLUDE_DIRS ${CAIRO_INCLUDE_DIRS})
  else()
    message(STATUS "[blend2d] Cairo not found - cannot build cairo benchmark backend")
  endif()

  find_package(unofficial-skia CONFIG)
  if(TARGET unofficial::skia::skia)
    message(STATUS "[blend2d] Skia found - adding support for SKIA (benchmarking)")

    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_SKIA)
    list(APPEND BLEND2D_BENCH_LIBS unofficial::skia::skia)
  else()
    message(STATUS "[blend2d] Skia not found - cannot build Skia benchmark backend")
  endif()

  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/3rdparty/agg/include/agg_curves.h")
    message(STATUS "[blend2d] AGG found (./3rdparty/agg) - adding support for AGG (benchmarking)")

    file(GLOB_RECURSE
      BLEND2D_BENCH_SRCS
      LIST_DIRECTORIES FALSE
      RELATIVE "${CMAKE_CURRENT_LIST_DIR}"
      CONFIGURE_DEPENDS
      "3rdparty/agg/*.h" "3rdparty/agg/*.cpp")
    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_AGG)
    list(APPEND BLEND2D_BENCH_INCLUDE_DIRS 3rdparty/agg/include)
  else()
    message(STATUS "[blend2d] AGG not found (./3rdparty/agg) - cannot build AGG benchmark backend")
  endif()

  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/3rdparty/JUCE/CMakeLists.txt")
    message(STATUS "[blend2d] JUCE found (./3rdparty/JUCE) - adding support for JUCE (benchmarking)")

    add_definitions(-DJUCE_USE_CURL=0 -DUSE_COREGRAPHICS_RENDERING=0)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/3rdparty/JUCE" juce)

    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_JUCE)
    list(APPEND BLEND2D_BENCH_LIBS juce::juce_core juce::juce_events juce::juce_graphics)
  else()
    message(STATUS "[blend2d] JUCE not found (./3rdparty/JUCE) - cannot build JUCE benchmark backend")
  endif()

  if (APPLE)
    message(STATUS "[blend2d] CoreGraphics (system) - adding support for CoreGraphics (benchmarking)")

    list(APPEND BLEND2D_BENCH_CFLAGS -DBL_BENCH_ENABLE_COREGRAPHICS)
    list(APPEND BLEND2D_BENCH_LIBS "-framework CoreGraphics")
  endif()

  if(Qt6Gui_FOUND)
    message(STATUS "[blend2d] Adding demos that use Qt framework")

    foreach(app bl_demo_bounces
                bl_demo_bubbles
                bl_demo_circles
                bl_demo_elliptic_arc
                bl_demo_gradients
                bl_demo_image_view
                bl_demo_particles
                bl_demo_paths
                bl_demo_pattern
                bl_demo_rects
                bl_demo_sprites
                bl_demo_stroke
                bl_demo_text
                bl_demo_tiger)
      blend2d_add_target(${app} EXECUTABLE_WIN32
        SOURCES
          blend2d-testing/demos/${app}.cpp
          blend2d-testing/demos/bl_qt_canvas.cpp
          blend2d-testing/demos/bl_qt_canvas.h
          blend2d-testing/demos/bl_qt_headers.h
        LIBRARIES
          blend2d::blend2d Qt6::Gui Qt6::Widgets)
      set_property(TARGET ${app} PROPERTY AUTOMOC TRUE)
    endforeach()
  endif()

  blend2d_add_target(bl_bench EXECUTABLE
    SOURCES      ${BLEND2D_BENCH_SRC} ${BLEND2D_BENCH_SRCS}
    LIBRARIES    blend2d::blend2d ${BLEND2D_BENCH_LIBS}
    CFLAGS       ${BLEND2D_BENCH_CFLAGS}
    INCLUDE_DIRS ${BLEND2D_BENCH_INCLUDE_DIRS})
  target_link_directories(bl_bench PRIVATE ${BLEND2D_BENCH_LIBRARY_DIRS})
else()
  message(STATUS "[blend2d] Disabling Blend2D demos ('BLEND2D_DEMOS=OFF')")
endif()

message(STATUS "[blend2d] == Configuring done ==")
