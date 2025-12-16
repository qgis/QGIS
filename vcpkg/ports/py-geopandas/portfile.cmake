vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    geopandas
    VERSION         ${VERSION}
    SHA512          41225eb7d51da9e6cf8314c18ef6d710eff976c358e80dd7f85c21ed04062525ccecaa3f6090919c963d9622892520ddfde553e049a9e8503173798737da53ae
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
vcpkg_python_test_import(MODULE "geopandas")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)