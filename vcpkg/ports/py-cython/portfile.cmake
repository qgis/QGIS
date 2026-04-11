vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO  cython/cython
    REF ${VERSION}
    SHA512 5ab8e39739a79debbe89b2ff5b6d88e1b7aafd5ad34460f14f3fc2bd90c10bdc9d6aa5b5844a48e30402dde9790cb9abcba3dc2b4e8cde24c4eeee818a180099
    HEAD_REF main
)

# Disable optimizations to speed up ARM64 build
set(ENV{CFLAGS} "/O1")
set(ENV{CXXFLAGS} "/O1")
set(ENV{CL} "/Od /GL-")
set(ENV{_LINK_} "/LTCG:OFF")

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")

if(NOT VCPKG_TARGET_IS_WINDOWS)
  vcpkg_copy_tools(TOOL_NAMES cygdb cython cythonize DESTINATION "${CURRENT_PACKAGES_DIR}/tools/python3" AUTO_CLEAN)
endif()

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)