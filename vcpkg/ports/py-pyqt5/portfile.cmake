set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_BUILD_TYPE release)

vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    PyQt5
    VERSION         ${VERSION}
    SHA512          e62debe112210b68993377264448199b66a43fc5db13c583a25e210759f5cd945bd1056c1c19fb86bcf005c6904358dfb89e005d27167a349f1d55987a63013e
)

# https://www.riverbankcomputing.com/static/Docs/PyQt5/installation.html
#   sipbuild repo: /project.py
set(SIPBUILD_ARGS
  "--confirm-license"
  "--qmake" "${CURRENT_INSTALLED_DIR}/tools/qt5/bin/qmake${VCPKG_HOST_EXECUTABLE_SUFFIX}"
  "--api-dir" "${CURRENT_PACKAGES_DIR}/share/qt5/qsci/api/python"
  "--verbose"
  "--qt-shared"
  "--no-make"
  "--disable" "QtDesigner"
  "--pep484-pyi"
  "--build-dir" "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel"
  "--target-dir" "${PYTHON3_SITEPACKAGES}"
)

vcpkg_backup_env_variables(VARS PATH)

vcpkg_add_to_path(PREPEND "${CURRENT_HOST_INSTALLED_DIR}/tools/python3/Scripts/")

message(STATUS "Running sipbuild...")
vcpkg_execute_required_process(
    COMMAND "${PYTHON3}" "-m" "sipbuild.tools.build" ${SIPBUILD_ARGS}
    WORKING_DIRECTORY "${SOURCE_PATH}"
    LOGNAME "sipbuild-${TARGET_TRIPLET}"
)
message(STATUS "Running sipbuild...finished.")

# inventory.txt is consumed by the distinfo tool which is run during make and should be run against the package directory
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}" NATIVE_INSTALLED_DIR)
vcpkg_replace_string("${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/inventory.txt"
            "${CURRENT_INSTALLED_DIR}"
            "${CURRENT_PACKAGES_DIR}")
            vcpkg_replace_string("${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/inventory.txt"
            "${NATIVE_INSTALLED_DIR}"
            "${CURRENT_PACKAGES_DIR}")

vcpkg_qmake_build(BUILD_LOGNAME "install" TARGETS "install")

vcpkg_restore_env_variables(VARS PATH)

vcpkg_python_test_import(MODULE "PyQt5.QtCore")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")