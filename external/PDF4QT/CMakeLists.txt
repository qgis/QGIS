#    Copyright (C) 2023 Jakub Melka
#
#    This file is part of PDF4QT.
#
#    PDF4QT is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    with the written consent of the copyright owner, any later version.
#
#    PDF4QT is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)

set(PDF4QT_VERSION 1.3.6)
message(STATUS "Build type - CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

project(PDF4QT VERSION ${PDF4QT_VERSION} LANGUAGES CXX)

option(PDF4QT_INSTALL_DEPENDENCIES "Install dependencies" ON)
option(PDF4QT_INSTALL_QT_DEPENDENCIES "Install Qt dependencies" ON)

add_compile_definitions(PDF4QT_PROJECT_VERSION="${PDF4QT_VERSION}")
add_compile_definitions(QT_NO_EMIT)

if(WIN32 AND MSVC)
    option(PDF4QT_INSTALL_MSVC_REDISTRIBUTABLE "Install MSVC redistributable package" ON)
    option(PDF4QT_INSTALL_PREPARE_WIX_INSTALLER "Prepare Wix installer for Windows" ON)
endif()

option(PDF4QT_ENABLE_OPENGL "Enable OpenGL" ON)
option(PDF4QT_BUILD_ONLY_CORE_LIBRARY "Build only core library" OFF)

set(PDF4QT_QT_ROOT "" CACHE PATH "Qt root directory")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Installation directory" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

if(PDF4QT_BUILD_ONLY_CORE_LIBRARY)
    find_package(${QT_VERSION_BASE} REQUIRED COMPONENTS Core Gui Svg Xml)

    if(PDF4QT_ENABLE_OPENGL)
        add_compile_definitions(PDF4QT_ENABLE_OPENGL)
        find_package(Qt6 REQUIRED COMPONENTS OpenGL)
    endif()
else()
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg Xml PrintSupport TextToSpeech Test)

    if(PDF4QT_ENABLE_OPENGL)
        add_compile_definitions(PDF4QT_ENABLE_OPENGL)
        find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
    endif()
endif()

# qt_standard_project_setup()

find_package(OpenSSL REQUIRED)
find_package(lcms2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(OpenJPEG CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTORCC_OPTIONS "--threshold;0;--compress;9")

if (MSVC)
    add_compile_options(/bigobj /W4 /wd5054 /wd4127 /wd4702)
endif()

if (MINGW)
    add_compile_options("-Wa,-mbig-obj")
endif()

if (UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX)
    set(LINUX_GCC ON)
endif()

if (UNIX AND NOT APPLE)
    set(PDF4QT_LINUX ON)
endif()

if (LINUX_GCC)
   # find_package(TBB REQUIRED)
endif()

option(PDF4QT_INSTALL_TO_USR "Install to usr directory" ON)

if(PDF4QT_LINUX)
    set(PDF4QT_INSTALL_LIB_DIR_IMPL lib)
    set(PDF4QT_INSTALL_BIN_DIR_IMPL bin)
else()
    set(PDF4QT_INSTALL_LIB_DIR_IMPL bin)
    set(PDF4QT_INSTALL_BIN_DIR_IMPL bin)
endif()

if(PDF4QT_INSTALL_TO_USR)
    set(PDF4QT_INSTALL_LIB_DIR usr/${PDF4QT_INSTALL_LIB_DIR_IMPL})
    set(PDF4QT_INSTALL_BIN_DIR usr/${PDF4QT_INSTALL_BIN_DIR_IMPL})
    set(PDF4QT_INSTALL_SHARE_DIR usr/share)
else()
    set(PDF4QT_INSTALL_LIB_DIR ${PDF4QT_INSTALL_LIB_DIR_IMPL})
    set(PDF4QT_INSTALL_BIN_DIR ${PDF4QT_INSTALL_BIN_DIR_IMPL})
    set(PDF4QT_INSTALL_SHARE_DIR share)
endif()

add_subdirectory(Pdf4QtLibCore)

if(NOT PDF4QT_BUILD_ONLY_CORE_LIBRARY)
    add_subdirectory(Pdf4QtLibWidgets)
    add_subdirectory(CodeGenerator)
    add_subdirectory(JBIG2_Viewer)
    add_subdirectory(PdfExampleGenerator)
    add_subdirectory(PdfTool)
    add_subdirectory(UnitTests)
    add_subdirectory(Pdf4QtViewer)
    add_subdirectory(Pdf4QtViewerPlugins)
    add_subdirectory(Pdf4QtViewerProfi)
    add_subdirectory(Pdf4QtViewerLite)
    add_subdirectory(Pdf4QtDocPageOrganizer)
    add_subdirectory(Pdf4QtDocDiff)
    add_subdirectory(WixInstaller)
endif()

message("CMAKE_PREFIX_PATH = " ${CMAKE_PREFIX_PATH})
message("CMAKE_TOOLCHAIN_FILE = " ${CMAKE_TOOLCHAIN_FILE})

if(NOT PDF4QT_BUILD_ONLY_CORE_LIBRARY)
    install(FILES Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtDocDiff.desktop Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtDocPageOrganizer.desktop Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtViewerLite.desktop Desktop/io.github.JakubMelka.Pdf4qt.desktop DESTINATION ${PDF4QT_INSTALL_SHARE_DIR}/applications)
    install(FILES Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtDocDiff.svg Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtDocPageOrganizer.svg Desktop/io.github.JakubMelka.Pdf4qt.Pdf4QtViewerLite.svg Desktop/io.github.JakubMelka.Pdf4qt.svg DESTINATION ${PDF4QT_INSTALL_SHARE_DIR}/icons/hicolor/scalable/apps)
    install(FILES Desktop/io.github.JakubMelka.Pdf4qt.appdata.xml DESTINATION ${PDF4QT_INSTALL_SHARE_DIR}/metainfo)
    install(FILES Desktop/128x128/io.github.JakubMelka.Pdf4qt.Pdf4QtDocDiff.png Desktop/128x128/io.github.JakubMelka.Pdf4qt.Pdf4QtDocPageOrganizer.png Desktop/128x128/io.github.JakubMelka.Pdf4qt.Pdf4QtViewerLite.png Desktop/128x128/io.github.JakubMelka.Pdf4qt.png DESTINATION ${PDF4QT_INSTALL_SHARE_DIR}/icons/hicolor/128x128/apps)

    configure_file(make-package.sh.in make-package.sh)
endif()

if(PDF4QT_INSTALL_DEPENDENCIES AND NOT PDF4QT_BUILD_ONLY_CORE_LIBRARY)
    install(DIRECTORY ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/
        RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/
        FILES_MATCHING
        PATTERN "*.dll"
        PATTERN "*.so"
        PATTERN "*.dylib"
    )

    if("${PDF4QT_QT_ROOT}" STREQUAL "")
        message(WARNING "Set variable PDF4QT_QT_ROOT to Qt6 root directory")
    endif()

    if(PDF4QT_INSTALL_QT_DEPENDENCIES)
        install(DIRECTORY ${PDF4QT_QT_ROOT}/bin/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/
            FILES_MATCHING
            REGEX "(Qt6Core|Qt6Gui|Qt6PrintSupport|Qt6Svg|Qt6TextToSpeech|Qt6Widgets|Qt6Xml|Qt6OpenGL|Qt6OpenGLWidgets|Qt6Multimedia|Qt6Network)\\..*"
            PATTERN "Debug" EXCLUDE
        )

        install(DIRECTORY ${PDF4QT_QT_ROOT}/plugins/platforms/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/platforms/
            FILES_MATCHING
            PATTERN "qwindows.dll"
            PATTERN "*.so"
            PATTERN "*.dylib"
        )

        install(DIRECTORY ${PDF4QT_QT_ROOT}/plugins/iconengines/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/iconengines/
            FILES_MATCHING
            REGEX "qsvgicon\\..*"
        )

        install(DIRECTORY ${PDF4QT_QT_ROOT}/plugins/imageformats/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/imageformats/
            FILES_MATCHING
            PATTERN "*.dll"
            PATTERN "*.so"
            PATTERN "*.dylib"
            REGEX "d\\..*" EXCLUDE
        )

        install(DIRECTORY ${PDF4QT_QT_ROOT}/plugins/styles/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/styles/
            FILES_MATCHING
            PATTERN "*.dll"
            PATTERN "*.so"
            PATTERN "*.dylib"
            REGEX "d\\..*" EXCLUDE
        )

        install(DIRECTORY ${PDF4QT_QT_ROOT}/plugins/texttospeech/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/texttospeech/
            FILES_MATCHING
            PATTERN "*.dll"
            PATTERN "*.so"
            PATTERN "*.dylib"
            REGEX "d\\..*" EXCLUDE
        )
    endif()

    if(WIN32 AND MSVC AND PDF4QT_INSTALL_MSVC_REDISTRIBUTABLE)
        get_filename_component(MSVC_REDISTRIBUTABLES_PATH $ENV{VCToolsRedistDir}/$ENV{VSCMD_ARG_TGT_ARCH}/Microsoft.VC${MSVC_TOOLSET_VERSION}.CRT/ ABSOLUTE)
        message(STATUS "MSVC Redistributable Package Path = ${MSVC_REDISTRIBUTABLES_PATH}")

        install(DIRECTORY ${MSVC_REDISTRIBUTABLES_PATH}/
            RUNTIME DESTINATION ${PDF4QT_INSTALL_LIB_DIR}/
            FILES_MATCHING
            PATTERN "*.dll"
        )
    endif()
endif()

#macro(dump_variables)
#    message(STATUS "dump_variables------------------------------------------{")
#    get_cmake_property(_variableNames VARIABLES)
#    foreach (_variableName ${_variableNames})
#        message(STATUS "${_variableName}=${${_variableName}}")
#    endforeach()
#    message(STATUS "dump_variables------------------------------------------}")
#endmacro()
#
#dump_variables()

