set(VCPKG_ENV_PASSTHROUGH_UNTRACKED EMSCRIPTEN_ROOT EMSDK PATH)

if(NOT DEFINED ENV{EMSCRIPTEN_ROOT})
   find_path(EMSCRIPTEN_ROOT "emcc")
else()
   set(EMSCRIPTEN_ROOT "$ENV{EMSCRIPTEN_ROOT}")
endif()

if(NOT EMSCRIPTEN_ROOT)
   if(NOT DEFINED ENV{EMSDK})
      message(FATAL_ERROR "The emcc compiler not found in PATH")
   endif()
   set(EMSCRIPTEN_ROOT "$ENV{EMSDK}/upstream/emscripten")
endif()

if(NOT EXISTS "${EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake")
   message(FATAL_ERROR "Emscripten.cmake toolchain file not found")
endif()

set(VCPKG_TARGET_ARCHITECTURE wasm32)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Emscripten)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../toolchains/wasm.cmake")
set(ENV{EMSCRIPTEN_ROOT} "${EMSCRIPTEN_ROOT}")

# Build both debug and release - CMake will select based on CMAKE_BUILD_TYPE
# (VCPKG_BUILD_TYPE not set = builds both)

# TODO:
#  -msimd128 would be nice but requires patching gdal
set(QGIS_JS_FLAGS "-pthread -fwasm-exceptions")

set(VCPKG_C_FLAGS "${QGIS_JS_FLAGS}")
set(VCPKG_CXX_FLAGS "${QGIS_JS_FLAGS}")

# Qt6 feature configuration for Emscripten
if(PORT MATCHES "qtbase")
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
        "-DFEATURE_wasm_exceptions=ON"
        "-DFEATURE_png=ON"
    )
endif()
