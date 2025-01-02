set(NUGET_SOURCE "https://nuget.pkg.github.com/qgis/index.json" CACHE STRING "Nuget source")
set(NUGET_USERNAME "qgis" CACHE STRING "Nuget user")

# Setup features (dependencies) based on cmake configuration
if(WITH_BINDINGS)
  list(APPEND VCPKG_MANIFEST_FEATURES "bindings")
endif()
if(WITH_3D)
  list(APPEND VCPKG_MANIFEST_FEATURES "3d")
endif()
if(WITH_GUI)
  list(APPEND VCPKG_MANIFEST_FEATURES "gui")
endif()

# Binarycache can only be used on Windows or if mono is available.
find_program(_VCPKG_MONO mono)
if(NOT "${NUGET_TOKEN}" STREQUAL "" AND (CMAKE_HOST_WIN32 OR EXISTS "${_VCPKG_MONO}"))
  if(CMAKE_HOST_WIN32)
    set(_VCPKG_EXECUTABLE "$ENV{VCPKG_ROOT}/vcpkg.exe")
  else()
    set(_VCPKG_EXECUTABLE "$ENV{VCPKG_ROOT}/vcpkg")
  endif()

  execute_process(
    COMMAND ${_VCPKG_EXECUTABLE} fetch nuget
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE _FETCH_NUGET_OUTPUT)

  STRING(REGEX REPLACE "\n" ";" _FETCH_NUGET_OUTPUT "${_FETCH_NUGET_OUTPUT}")
  list(GET _FETCH_NUGET_OUTPUT -1 _NUGET_PATH)

  if(CMAKE_HOST_WIN32)
    set(_NUGET_EXE ${_NUGET_PATH})
  else()
    set(_NUGET_EXE ${_VCPKG_MONO} ${_NUGET_PATH})
  endif()

  set(_CONFIG_PATH "${CMAKE_BINARY_DIR}/github-NuGet.Config")

  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/NuGet.Config.in"
    "${_CONFIG_PATH}"
    @ONLY)
  execute_process(
    COMMAND ${_NUGET_EXE} setapikey "${NUGET_TOKEN}" -src ${NUGET_SOURCE} -configfile ${_CONFIG_PATH}
    OUTPUT_VARIABLE _OUTPUT
    ERROR_VARIABLE _ERROR
    RESULT_VARIABLE _RESULT)
  if(_RESULT EQUAL 0)
    message(STATUS "Setup nuget api key - done")
  else()
    message(STATUS "Setup nuget api key - failed")
    message(STATUS "Output:")
    message(STATUS ${_OUTPUT})
    message(STATUS "Error:")
    message(STATUS ${_ERROR})
  endif()

  file(TO_NATIVE_PATH "${_CONFIG_PATH}" _CONFIG_PATH_NATIVE)
  set(ENV{VCPKG_BINARY_SOURCES} "$ENV{VCPKG_BINARY_SOURCES};nugetconfig,${_CONFIG_PATH_NATIVE},readwrite")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}/vcpkg")
# Copies DLLs built by vcpkg when an install() command is run.
# Only works on Windows and even there not reliably ...
# set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON CACHE BOOL "Copy dependency DLLs on install")
