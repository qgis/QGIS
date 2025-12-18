vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO qtwebkit/qtwebkit
    REF qtwebkit-5.212.0-alpha4
    SHA512 5b7c11b8d07d03aed634cbffd85eec48e865ad65b5b00a1a2e6426692471a95f7005295a9ae5cf2791fb30d097fe2f97162f754d4eb368cf5d3f37e145daf2c7
    PATCHES
    bison.patch
    skip_private_header_check.patch
    libxml_min_version.patch
    icu_targets.patch
    osgeo4w.patch
    makevalues_gperf.patch
    arm64-osx.patch
    fix-macos-build.patch
    noframeworks.patch
)

file(REMOVE ${SOURCE_PATH}/Source/cmake/FindICU.cmake)

vcpkg_find_acquire_program(PERL)
vcpkg_find_acquire_program(PYTHON3)
vcpkg_find_acquire_program(RUBY)
vcpkg_find_acquire_program(BISON)
vcpkg_find_acquire_program(GPERF)

# Configure and build
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
      "-DPERL_EXECUTABLE=${PERL}"
      "-DPYTHON_EXECUTABLE=${PYTHON3}"
      "-DRuby_EXECUTABLE=${RUBY}"
      "-DBISON_EXECUTABLE=${BISON}"
      "-DGPERF_EXECUTABLE=${GPERF}"
      "-DENABLE_XSLT=OFF"
      "-DUSE_GSTREAMER=FALSE"
      "-DUSE_LIBHYPHEN=FALSE"
      "-DUSE_WOFF2=FALSE"
      "-DPORT=Qt"
      "-DENABLE_QT_WEBCHANNEL=FALSE"
      "-DENABLE_API_TESTS=FALSE"
)

vcpkg_cmake_install()

# Handle copyright
vcpkg_install_copyright(FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/LICENSE.LGPLv21")
