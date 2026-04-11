vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO scikit-build/scikit-build
    REF ${VERSION}
    SHA512 962d6c77d7393229edebcc4dd4105a3d36dd5f5b2cb63522614f04b560961f038c1c17136f70878aa497aad9a32656c0fdd29fba2a0603dafe895135fffa32e8
    HEAD_REF main
)

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

vcpkg_python_test_import(MODULE "skbuild")
