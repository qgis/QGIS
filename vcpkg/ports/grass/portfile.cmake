vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO OSGeo/grass
    REF b4640a2e9c7151698e0aa2250648835a2f7b7b5c
    SHA512 c55291751eb44b2c197ccd2899d757c80873d1b693c6d92685a4e417c1b11082ab3509a57a07c16efd92c4a3e48627a0aedaf240911339a291b04a6cf961d556
    HEAD_REF main
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        postgres            WITH_POSTGRES
        sqlite              WITH_SQLITE
        zstd                WITH_ZSTD
        geos                WITH_GEOS
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DWITH_GUI=OFF
        -DWITH_DOCS=OFF
        -DWITH_LAPACKE=OFF # https://github.com/microsoft/vcpkg/issues/23333
        -DWITH_OPENMP=OFF
        -DWITH_PDAL=OFF
        -DWITH_FHS=ON
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH "lib/cmake/GRASS")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/GPL.txt")
