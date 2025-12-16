vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    llvmlite
    VERSION         ${VERSION}
    SHA512          b691ee5139c74a39d75f2cd8633916cd35f6d4c62b5b54f25df9068cef829f8ba6b552ddf8596b2fe756c245eff84c63156c8d93cc85faab851c9212a65cc2d5
)

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "llvmlite")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
