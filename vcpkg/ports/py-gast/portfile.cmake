vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    gast
    VERSION         ${VERSION}
    SHA512          ee7c17f3f890edf8e32845bee9b616cd0ed4a0b235833b11ce6716f15675a03fc835b2df1ac6417d5332e2c162ade0fdc0533914c014537ab0526f98dd7339aa
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "gast")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)