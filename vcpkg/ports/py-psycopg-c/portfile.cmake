vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    psycopg-c
    VERSION         ${VERSION}
    SHA512          ab3299f090dc594a2036398fee10a9b63548b7de62dbcbd4041b806a105212b62a7aff7b7a6003a1bf041bc4846ea8a9e2c25e432234059f26c55e603190d608
    FILENAME        psycopg_c
)

if(VCPKG_TARGET_IS_WINDOWS)
  vcpkg_add_to_path("${CURRENT_INSTALLED_DIR}/tools/libpq")
  set(ENV{PG_CONFIG} "${CURRENT_INSTALLED_DIR}/tools/libpq/pg_config.exe")
else()
  # libpq's vcpkg_copy_tools() call has no DESTINATION argument, so pg_config
  # lands at tools/libpq/pg_config (NOT tools/libpq/bin/pg_config). Match the
  # Windows branch above.
  vcpkg_add_to_path("${CURRENT_INSTALLED_DIR}/tools/libpq")
  set(ENV{PG_CONFIG} "${CURRENT_INSTALLED_DIR}/tools/libpq/pg_config")
endif()

set(ENV{INCLUDE} "${CURRENT_INSTALLED_DIR}/include;$ENV{INCLUDE}")

vcpkg_python_build_and_install_wheel(
  SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
