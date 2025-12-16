vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    adbc_driver_postgresql
    VERSION         ${VERSION}
    SHA512          088698277e559cf834b68e60647d680a90efaed78fc8f8b0c061013aef5ba641a56f8b96fa03426b328b85b9bceba89218ce4fd1b797191707b41332e793c68d
)

set(ENV{ADBC_POSTGRESQL_LIBRARY} "${CURRENT_INSTALLED_DIR}/lib/libadbc_driver_postgresql${VCPKG_TARGET_SHARED_LIBRARY_SUFFIX}")

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
vcpkg_python_test_import(MODULE "adbc_driver_sqlite")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)