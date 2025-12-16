vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    beniget
    VERSION         ${VERSION}
    SHA512          7094e5c0759d54738aa10923e96e3b20a50dc7736311e36fda757d16eb47838eae8372da53fb794a6871d0f92a38726889665b78b364a430e253b44de795cb6e
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "beniget")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)