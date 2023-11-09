#[[ Find Qwt
 ~~~~~~~~
 Copyright (c) 2010, Tim Sutton <tim at linfiniti.com>
 Redistribution and use is allowed according to the terms of the BSD license.
 For details see the accompanying COPYING-CMAKE-SCRIPTS file.

 Once run this will define:

 QWT_FOUND       = system has QWT lib
 QWT_LIBRARY     = full path to the QWT library
 QWT_INCLUDE_DIR = where to find headers
]]

set( QWT_LIBRARY_NAMES
  qwt-${QT_VERSION_BASE_LOWER}
  qwt6-${QT_VERSION_BASE_LOWER}
  qwt
  qwt6
)
set( QWT_LIB_PATHS
  /usr/lib
  /usr/lib64
  /usr/local/lib
  /usr/local/lib/qwt.framework
  /usr/local/lib/${QT_VERSION_BASE_LOWER}
  "$ENV{LIB_DIR}"
  "$ENV{LIB}"
)
set( QWT_INCLUDE_PATHS
  /usr/include
  /usr/include/${QT_VERSION_BASE_LOWER}
  /usr/local/include
  /usr/local/include/${QT_VERSION_BASE_LOWER}
  "$ENV{LIB_DIR}/include"
  "$ENV{INCLUDE}"
)

find_package(PkgConfig QUIET)
if( PkgConfig_FOUND )
  pkg_search_module(PC_QWT QUIET ${QT_VERSION_BASE}Qwt6)
endif()

if( PC_QWT_FOUND )
  list( PREPEND QWT_LIB_PATHS "${PC_QWT_LIBDIR}" )
  list( PREPEND QWT_INCLUDE_PATHS "${PC_QWT_INCLUDEDIR}" )
endif()

find_library( QWT_LIBRARY_RELEASE
  NAMES ${QWT_LIBRARY_NAMES}
  PATHS ${QWT_LIB_PATHS}
)

if( MSVC )
  find_library( QWT_LIBRARY_DEBUG
    NAMES qwtd qwt-${QT_VERSION_BASE_LOWER}d qwt6d qwt6-${QT_VERSION_BASE_LOWER}d
    PATHS "$ENV{LIB_DIR}" "${PC_QWT_LIBDIR}"
  )
endif()

include(SelectLibraryConfigurations)
select_library_configurations(QWT)

set( _qwt_fw )
if( QWT_LIBRARY_RELEASE MATCHES "/qwt.*\\.framework" )
  string( REGEX REPLACE "^(.*/qwt.*\\.framework).*$" "\\1" _qwt_fw "${QWT_LIBRARY}" )
  set ( QWT_LIBRARY_RELEASE "${QWT_LIBRARY_RELEASE}/qwt" )
  list( PREPEND QWT_INCLUDE_PATHS "${_qwt_fw}/Headers" )
endif()

find_path( QWT_INCLUDE_DIR
  NAMES qwt.h
  PATHS ${QWT_INCLUDE_PATHS}
  PATH_SUFFIXES ${QWT_LIBRARY_NAMES} ${QT_VERSION_BASE_LOWER}/qwt
)

# version
if( PC_QWT_FOUND )
  set( QWT_VERSION_STRING ${PC_QWT_VERSION} )
else()
  set ( _VERSION_FILE ${QWT_INCLUDE_DIR}/qwt_global.h )
  if ( EXISTS ${_VERSION_FILE} )
    file ( STRINGS ${_VERSION_FILE} _VERSION_LINE REGEX "define[ ]+QWT_VERSION_STR" )
    if ( _VERSION_LINE )
      string ( REGEX REPLACE ".*define[ ]+QWT_VERSION_STR[ ]+\"([^\"]*)\".*" "\\1" QWT_VERSION_STRING "${_VERSION_LINE}" )
    endif ()
  endif ()
  unset ( _VERSION_FILE )
endif()

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Qwt
  FOUND_VAR Qwt_FOUND
  REQUIRED_VARS
    QWT_LIBRARY
    QWT_INCLUDE_DIR
  VERSION_VAR
    QWT_VERSION_STRING
)

if( Qwt_FOUND )
  set ( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
  if ( NOT TARGET Qwt::Qwt )
    add_library( Qwt::Qwt UNKNOWN IMPORTED )
    set_target_properties( Qwt::Qwt PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIRS}"
    )
    if( WIN32 )
      set_target_properties( Qwt::Qwt PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS QWT_DLL
      )
    endif()
    if( QWT_LIBRARY_RELEASE )
      set_property( TARGET Qwt::Qwt APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE
      )
      set_target_properties( Qwt::Qwt PROPERTIES
        IMPORTED_LOCATION_RELEASE "${QWT_LIBRARY_RELEASE}"
      )
    endif()
    if( QWT_LIBRARY_DEBUG )
      set_property( TARGET Qwt::Qwt APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG
      )
      set_target_properties( Qwt::Qwt PROPERTIES
        IMPORTED_LOCATION_DEBUG "${QWT_LIBRARY_DEBUG}"
      )
    endif()
  endif ()
else()
  if( QWT_FIND_REQUIRED )
    message(FATAL_ERROR "Could not find Qwt")
  endif()
endif()

mark_as_advanced (
  QWT_LIBRARY
  QWT_INCLUDE_DIR
)
