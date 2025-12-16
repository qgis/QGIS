vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    adbc_driver_sqlite
    VERSION         ${VERSION}
    SHA512          08c1418e40aa58f26648dff49d35b5dd8f30b4a8888d1aad01ca415d296971837b10b89e2329070df7a09d322ca5fd824dfda58f43a772e9abfb10655197c6b5
)

set(ENV{ADBC_SQLITE_LIBRARY} "${CURRENT_INSTALLED_DIR}/lib/libadbc_driver_sqlite${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX}")

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
vcpkg_python_test_import(MODULE "adbc_driver_sqlite")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)