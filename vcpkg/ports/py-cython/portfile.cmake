vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO  cython/cython
    REF ${VERSION}
    SHA512 16a7fae5cfebfe76015de817d66d1bad8e39f922916338015f0f2209fe4a64fc3c5ff7f1fdc8f78304ffe854052fbe96e667a3ebdf94d2b554d322947d8c1e67
    HEAD_REF main
)

# Disable optimizations to fix ARM64 build
if(MSVC)
  set(ENV{_LINK_} "/LTCG:OFF")
endif()

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")

if(NOT VCPKG_TARGET_IS_WINDOWS)
  vcpkg_copy_tools(TOOL_NAMES cygdb cython cythonize DESTINATION "${CURRENT_PACKAGES_DIR}/tools/python3" AUTO_CLEAN)
endif()

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)
