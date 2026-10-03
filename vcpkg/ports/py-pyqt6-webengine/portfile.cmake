set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_BUILD_TYPE release)

vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME PyQt6-WebEngine
    VERSION ${VERSION}
    SHA512 9c1602a97721f91640b2479e80662b3c342c28ed156ccafc6d1d0fef2c57beba6d8cb466dc770ca445cd2fd19361f03302e5d0bb1bd554cea5209a45319ce38c
    FILENAME pyqt6_webengine
)

# The target directory is a staging package, so resolve imported SIP files
# from the installed PyQt6 rather than from the empty staging directory.
vcpkg_replace_string("${SOURCE_PATH}/pyproject.toml"
    "[tool.sip.project]"
    "[tool.sip.project]\nsip-include-dirs = [\"${CURRENT_INSTALLED_DIR}/lib/python3.12/site-packages/PyQt6/bindings\"]")

# Build against the same Qt and PyQt as QGIS, without downloading Qt wheels.
set(SIPBUILD_ARGS
    --qmake "${CURRENT_INSTALLED_DIR}/tools/Qt6/bin/qmake${VCPKG_HOST_EXECUTABLE_SUFFIX}"
    --verbose --no-make --pep484-pyi
    --build-dir "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel"
    --target-dir "${PYTHON3_SITEPACKAGES}"
)
if(DEFINED VCPKG_OSX_DEPLOYMENT_TARGET)
    list(APPEND SIPBUILD_ARGS --qmake-setting "QMAKE_MACOSX_DEPLOYMENT_TARGET=${VCPKG_OSX_DEPLOYMENT_TARGET}")
endif()

vcpkg_backup_env_variables(VARS PATH)
vcpkg_add_to_path(PREPEND "${CURRENT_HOST_INSTALLED_DIR}/tools/python3/Scripts/" "${CURRENT_HOST_INSTALLED_DIR}/tools/Qt6/bin/" "${CURRENT_HOST_INSTALLED_DIR}/bin")
vcpkg_execute_required_process(
    COMMAND "${PYTHON3}" -m sipbuild.tools.build ${SIPBUILD_ARGS}
    WORKING_DIRECTORY "${SOURCE_PATH}"
    LOGNAME "sipbuild-${TARGET_TRIPLET}"
)

# The dist-info inventory must refer to the staging package during installation.
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}" NATIVE_INSTALLED_DIR)
vcpkg_replace_string("${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/inventory.txt"
    "${CURRENT_INSTALLED_DIR}" "${CURRENT_PACKAGES_DIR}")
vcpkg_replace_string("${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/inventory.txt"
    "${NATIVE_INSTALLED_DIR}" "${CURRENT_PACKAGES_DIR}")
vcpkg_qmake_build(BUILD_LOGNAME "install" TARGETS install)
vcpkg_restore_env_variables(VARS PATH)

vcpkg_python_test_import(MODULE "PyQt6.QtWebEngineCore")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
