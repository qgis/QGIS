vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    numexpr
    VERSION         ${VERSION}
    SHA512          6602afcab2085f875c20883358956f3f813c6dbe9274654868d3d28d4fa07fd77cd1291b33d56afd1f91955af9118ebdcc39b19202638b6ee6559cc4f0e67f64
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
vcpkg_python_test_import(MODULE "numexpr")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)