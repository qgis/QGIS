vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    pyogrio
    VERSION         ${VERSION}
    SHA512          c687c8d1f13639e1e8bdfe618c9a9f1b4ee492effc089674bf23318e7b2e02dbe88541bf7ff0b0ad5fb913cc7c0b7b188280bcc618e6a13305446b21a2988264
)

set(ENV{GDAL_INCLUDE_PATH} "${CURRENT_INSTALLED_DIR}/include")
set(ENV{GDAL_LIBRARY_PATH} "${CURRENT_INSTALLED_DIR}/lib")

set(PKGCONFIG "${CURRENT_HOST_INSTALLED_DIR}/tools/pkgconf/pkgconf${VCPKG_HOST_EXECUTABLE_SUFFIX}")
set(backup_PKG_CONFIG_PATH "$ENV{PKG_CONFIG_PATH}")
set(ENV{PKG_CONFIG_PATH} "${CURRENT_INSTALLED_DIR}/lib/pkgconfig${VCPKG_HOST_PATH_SEPARATOR}${CURRENT_PACKAGES_DIR}/lib/pkgconfig${VCPKG_HOST_PATH_SEPARATOR}${backup_PKG_CONFIG_PATH}")
execute_process(
                COMMAND "${PKGCONFIG}" "--modversion" "gdal"
                OUTPUT_VARIABLE GDAL_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY
            )
set(ENV{GDAL_VERSION} "${GDAL_VERSION}")
set(ENV{PKG_CONFIG_PATH} "${backup_PKG_CONFIG_PATH}")

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
vcpkg_python_test_import(MODULE "pyogrio")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)