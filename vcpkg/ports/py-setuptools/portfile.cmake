vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME        setuptools
    VERSION         ${VERSION}
    SHA512          d0a34f16dfa6bb9a6df39076cd43528cf854d343f6f801c448ea0ebab2a259aec3d03571e2a26709df6082ed2fcb6c43b86448be556fd559b6af41831b4f38e0
    PATCHES 
      fix-prefix.patch
)

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
#execute_process(COMMAND ${PYTHON3} "${SOURCE_PATH}/setup.py" install "--prefix=${CURRENT_INSTALLED_DIR}/tools/python3" "--root=${CURRENT_PACKAGES_DIR}"
#  COMMAND_ERROR_IS_FATAL ANY
#  WORKING_DIRECTORY  "${CURRENT_PACKAGES_DIR}/tools/python3"
#)
