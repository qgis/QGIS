vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    adbc_driver_manager
    VERSION         ${VERSION}
    SHA512          a0981f69fb5b81c3ebb82fc01ac86bcf26f013592261b65acdf7b55f9044fb0a2b37fdcf187f30c053ce3bb409147e0c6df5b030faee98923c90d703bb10d1b8
)

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
vcpkg_python_test_import(MODULE "adbc_driver_manager")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)