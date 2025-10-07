set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

vcpkg_from_pythonhosted(
    OUT_SOURCE_PATH SOURCE_PATH
    PACKAGE_NAME    pyqt5_sip
    VERSION         ${VERSION}
    SHA512          b7816215368a71c0ce0b1368cce9208c6e11c752a48aaa5d296308c82d46fa65adbba1b79ee49f80934f5069cd54243d5d0a34698a7a6de464e4e0175e622353
)

vcpkg_python_build_and_install_wheel(SOURCE_PATH "${SOURCE_PATH}")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

vcpkg_python_test_import(MODULE "PyQt5.sip")

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
