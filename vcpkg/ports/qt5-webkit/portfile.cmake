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


# Adjust qt5-webkit qmake to vcpkg standards in tools/qt5/mkspecs/modules and include/qt5/*
if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    set(_qt5_headers_dir "${CURRENT_PACKAGES_DIR}/include/qt5")
    file(MAKE_DIRECTORY "${_qt5_headers_dir}")

    foreach(_qtwebkit_module QtWebKit QtWebKitWidgets)
        if(EXISTS "${CURRENT_PACKAGES_DIR}/include/${_qtwebkit_module}")
            file(COPY "${CURRENT_PACKAGES_DIR}/include/${_qtwebkit_module}" DESTINATION "${_qt5_headers_dir}")
        endif()
    endforeach()

    set(_qtwebkit_mkspecs_src "${CURRENT_PACKAGES_DIR}/mkspecs/modules")
    set(_qt5_mkspecs_dst "${CURRENT_PACKAGES_DIR}/tools/qt5/mkspecs/modules")
    file(MAKE_DIRECTORY "${_qt5_mkspecs_dst}")

    foreach(_qtwebkit_pri qt_lib_webkit.pri qt_lib_webkit_private.pri qt_lib_webkitwidgets.pri qt_lib_webkitwidgets_private.pri)
        if(EXISTS "${_qtwebkit_mkspecs_src}/${_qtwebkit_pri}")
            file(COPY "${_qtwebkit_mkspecs_src}/${_qtwebkit_pri}" DESTINATION "${_qt5_mkspecs_dst}")
            file(REMOVE "${_qtwebkit_mkspecs_src}/${_qtwebkit_pri}")
        endif()
    endforeach()

    # Replace absolute packaging paths with relative vcpkg paths
    file(WRITE "${_qt5_mkspecs_dst}/qt_lib_webkit.pri"
"QT.webkit.VERSION = 5.212.0\n"
"QT.webkit.name = QtWebKit\n"
"QT.webkit.module = QtWebKit\n"
"QT.webkit.libs = $$QT_MODULE_LIB_BASE\n"
"QT.webkit.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtWebKit\n"
"QT.webkit.frameworks =\n"
"QT.webkit.bins = $$QT_MODULE_BIN_BASE\n"
"QT.webkit.depends = core gui network\n"
"QT.webkit.uses =\n"
"QT.webkit.module_config = v2\n"
"QT.webkit.DEFINES = QT_WEBKIT_LIB\n"
"QT.webkit.enabled_features =\n"
"QT.webkit.disabled_features =\n"
"QT_CONFIG +=\n"
"QT_MODULES += webkit\n"
    )

    file(WRITE "${_qt5_mkspecs_dst}/qt_lib_webkitwidgets.pri"
"QT.webkitwidgets.VERSION = 5.212.0\n"
"QT.webkitwidgets.name = QtWebKitWidgets\n"
"QT.webkitwidgets.module = QtWebKitWidgets\n"
"QT.webkitwidgets.libs = $$QT_MODULE_LIB_BASE\n"
"QT.webkitwidgets.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtWebKitWidgets\n"
"QT.webkitwidgets.frameworks =\n"
"QT.webkitwidgets.bins = $$QT_MODULE_BIN_BASE\n"
"QT.webkitwidgets.depends = core gui network widgets webkit\n"
"QT.webkitwidgets.uses =\n"
"QT.webkitwidgets.module_config = v2\n"
"QT.webkitwidgets.DEFINES = QT_WEBKITWIDGETS_LIB\n"
"QT.webkitwidgets.enabled_features =\n"
"QT.webkitwidgets.disabled_features =\n"
"QT_CONFIG +=\n"
"QT_MODULES += webkitwidgets\n"
    )
endif()


# Handle copyright
vcpkg_install_copyright(FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/LICENSE.LGPLv21")
