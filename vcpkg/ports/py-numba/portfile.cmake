vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    numba
    VERSION         ${VERSION}
    SHA512          fd0a32d79153ab71002f56e756823ea5fbb1098e7772eb4fb2f0f465d21a9511e799f657fe1b96e287dcb17d3ca06c909f1f54468bad5761812ada4f7dfc5d5f
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "numba")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)