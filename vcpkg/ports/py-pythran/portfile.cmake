vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    pythran
    VERSION         ${VERSION}
    SHA512          f80770b62490c0acb03953ad8b7f4dbfc715e0653b27f6ef0cbae47f7fd088e5f959e1f74a88d310975135a7c35b1aabd2d727443dcbc12a61960d2db78d0fef
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "pythran")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)