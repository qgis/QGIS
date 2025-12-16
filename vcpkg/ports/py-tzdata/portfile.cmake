vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    tzdata
    VERSION         ${VERSION}
    SHA512          f525eb24719ec7e060989f03bb39c612ff0792f3469b31badb8011863cf1e6cc115c932fc4382b4a5b648f91f2f84631e18b404d627ea19b7bfe44355b1ab3f1
)
vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "tzdata")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)