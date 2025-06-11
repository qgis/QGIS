vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO qgis/o2
    REF "v${VERSION}"
    SHA512 e0116a8f72e8c98ef4237e36598a8f246319a30bb5bfab629e3220e7b819b1e9028c2e6caa4c92f17c4b0548c94c242120c5b4750abce2441a2db5d88b2e22f5
    HEAD_REF master
)

vcpkg_cmake_configure(
    DISABLE_PARALLEL_CONFIGURE
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -Do2_WITH_QT6=ON
        -Do2_WITH_KEYCHAIN=ON
)
vcpkg_cmake_install()
vcpkg_fixup_pkgconfig()

vcpkg_copy_pdbs()

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
