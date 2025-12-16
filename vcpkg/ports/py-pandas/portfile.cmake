set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled) # pandas includes are stored in the module itself
set(VCPKG_POLICY_MISMATCHED_NUMBER_OF_BINARIES enabled)
set(VCPKG_BUILD_TYPE release) # No debug builds required for pure python modules since vcpkg does not install a debug python executable. 

set(VCPKG_PYTHON3_BASEDIR "${CURRENT_HOST_INSTALLED_DIR}/tools/python3")
find_program(VCPKG_PYTHON3 NAMES python${PYTHON3_VERSION_MAJOR}.${PYTHON3_VERSION_MINOR} python${PYTHON3_VERSION_MAJOR} python PATHS "${VCPKG_PYTHON3_BASEDIR}" NO_DEFAULT_PATH)
find_program(VCPKG_CYTHON NAMES cython PATHS "${VCPKG_PYTHON3_BASEDIR}" "${VCPKG_PYTHON3_BASEDIR}/Scripts" NO_DEFAULT_PATH)

set(ENV{PYTHON3} "${VCPKG_PYTHON3}")
set(PYTHON3 "${VCPKG_PYTHON3}")

vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}")
if(VCPKG_TARGET_IS_WINDOWS)
  vcpkg_add_to_path(PREPEND "${VCPKG_PYTHON3_BASEDIR}/Scripts")
endif()

cmake_path(GET SCRIPT_MESON PARENT_PATH MESON_DIR)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pandas-dev/pandas
    REF v${VERSION}
    SHA512 c9d1416c7d13b6cb479b26bcaeda29b638fbe24d9f45e906c51fabe4918c1dd16e5178720f7388e698d3703c33d5620a649ece70bdec4d6af9d108f38c2650bb
    HEAD_REF main
)

vcpkg_configure_meson(
    SOURCE_PATH "${SOURCE_PATH}"
    ADDITIONAL_BINARIES
      cython=['${VCPKG_CYTHON}']
      python3=['${VCPKG_PYTHON3}']
#      python=['${VCPKG_PYTHON3}']
    ${opts}
    )
vcpkg_install_meson()
vcpkg_fixup_pkgconfig()


if(VCPKG_TARGET_IS_WINDOWS)
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}")
    file(RENAME "${CURRENT_PACKAGES_DIR}/lib/site-packages/pandas" "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/pandas")
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib")
endif()

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

# Add required Metadata for some python build plugins
file(WRITE "${CURRENT_PACKAGES_DIR}/${PYTHON3_SITE}/pandas-${VERSION}.dist-info/METADATA"
"Metadata-Version: 2.1\n\
Name: pandas\n\
Version: ${VERSION}"
)

vcpkg_python_test_import(MODULE "pandas")