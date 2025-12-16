vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    bottleneck
    VERSION         ${VERSION}
    SHA512          e83d470b3380d579966855eef18c50069ca7db7a789ce0e8c39b707d0a8ad7cd2179121bad0c8bce41da5095ea1dea3dbfd59d31968ab21724d66baad694d978
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "bottleneck")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)