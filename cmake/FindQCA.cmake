# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file COPYING-CMAKE-SCRIPTS or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindQCA
---------

CMake module to search for QCA library

IMPORTED targets
^^^^^^^^^^^^^^^^
This module defines the following :prop_tgt:`IMPORTED` target:
``Qca::qca``

The macro sets the following variables:
``QCA_FOUND``
  if the library found

``QCA_LIBRARIES``
  full path to the library

``QCA_INCLUDE_DIRS``
  where to find the library headers

``QCA_VERSION_STRING``
  version string of QCA

#]=======================================================================]

if(BUILD_WITH_QT6)
  set(QCA_INCLUDE_SUFFIXES
            QtCrypto
            qt6/QtCrypto
            Qca-qt6/QtCrypto
            qt6/Qca-qt6/QtCrypto)
  set(QCA_NAMES ${QCA_NAMES} qca-qt6 qca)
else()
  set(QCA_INCLUDE_SUFFIXES
            QtCrypto
            qt5/QtCrypto
            Qca-qt5/QtCrypto
            qt5/Qca-qt5/QtCrypto)
  set(QCA_NAMES ${QCA_NAMES} qca-qt5 qca)
endif()
find_path(QCA_INCLUDE_DIR qca.h
          PATHS
            ${QCA_ROOT}/include/
          PATH_SUFFIXES
            ${QCA_INCLUDE_SUFFIXES}
          DOC "Path to QCA include directory")


if(NOT QCA_LIBRARY)
  find_library(QCA_LIBRARY_RELEASE NAMES ${QCA_NAMES})
  find_library(QCA_LIBRARY_DEBUG NAMES ${QCA_NAMES})
  include(SelectLibraryConfigurations)
  select_library_configurations(QCA)
  mark_as_advanced(QCA_LIBRARY_RELEASE QCA_LIBRARY_DEBUG)
endif()

unset(QCA_INCLUDE_SUFFIXES)
unset(QCA_NAMES)

if(QCA_INCLUDE_DIR)
  # qca_version.h header only available with 2.1.0+
  set(_qca_version_h "${QCA_INCLUDE_DIR}/qca_version.h")
  file(STRINGS "${_qca_version_h}" _qca_version_str REGEX "^.*QCA_VERSION_STR +\"[^\"]+\".*$")
  string(REGEX REPLACE "^.*QCA_VERSION_STR +\"([^\"]+)\".*$" "\\1" QCA_VERSION_STRING "${_qca_version_str}")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QCA
                                  REQUIRED_VARS QCA_LIBRARY QCA_INCLUDE_DIR
                                  VERSION_VAR QCA_VERSION_STRING)
mark_as_advanced(QCA_INCLUDE_DIR QCA_LIBRARY)

if(QCA_FOUND)
  set(QCA_LIBRARIES ${QCA_LIBRARY})
  set(QCA_INCLUDE_DIRS ${QCA_INCLUDE_DIR})
  if(NOT TARGET Qca::qca)
    add_library(Qca::qca UNKNOWN IMPORTED)
    set_target_properties(Qca::qca PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES ${QCA_INCLUDE_DIR}
                          IMPORTED_LINK_INTERFACE_LANGUAGES "C")
    if(EXISTS "${QCA_LIBRARY}")
      set_target_properties(Qca::qca PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${QCA_LIBRARY}")
    endif()
    if(EXISTS "${QCA_LIBRARY_RELEASE}")
      set_property(TARGET Qca::qca APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(Qca::qca PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
        IMPORTED_LOCATION_RELEASE "${QCA_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${QCA_LIBRARY_DEBUG}")
      set_property(TARGET Qca::qca APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(Qca::qca PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
        IMPORTED_LOCATION_DEBUG "${QCA_LIBRARY_DEBUG}")
    endif()
  endif()
endif()

