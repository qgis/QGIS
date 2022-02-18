#############################################################
# CMake settings
cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_AUTORCC ON)
# set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
# POLICIES
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0094 NEW)

# don't relink it only the shared object changes
set(CMAKE_LINK_DEPENDS_NO_SHARED ON)

#############################################################
# Project and version
set(CPACK_PACKAGE_VERSION_MAJOR "3")
set(CPACK_PACKAGE_VERSION_MINOR "25")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(COMPLETE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(RELEASE_NAME "Master")
project(qgis VERSION ${COMPLETE_VERSION})

if (APPLE)
  set(QGIS_APP_NAME "QGIS")
else()
  set(QGIS_APP_NAME "qgis")
endif()

# Note the version no is Mmmpp for Major/minor/patch, 0-padded, thus '10100' for 1.1.0
math(EXPR QGIS_VERSION_INT "${CPACK_PACKAGE_VERSION_MAJOR}*10000+${CPACK_PACKAGE_VERSION_MINOR}*100+${CPACK_PACKAGE_VERSION_PATCH}")
message(STATUS "QGIS version: ${COMPLETE_VERSION} ${RELEASE_NAME} (${QGIS_VERSION_INT})")

set (ENABLE_LOCAL_BUILD_SHORTCUTS FALSE CACHE BOOL "Disables some build steps which are only relevant for releases to speed up compilation time for development")

#############################################################
if (APPLE)
  # QGIS custom dependencies package from qgis/QGIS-Mac-Packager
  # they can be downloaded from https://qgis.org/downloads/macos/qgis-deps
  # and extracted to /opt/QGIS/qgis-deps-<deps-version>/stage
  set (QGIS_MAC_DEPS_DIR "" CACHE PATH "Path to QGIS Mac custom dependencies directory")

  # Setup LIB_DIR and CMAKE_PREFIX_PATH to help CMake's
  # find_packages to look for these libraries instead of system libraries
  if ( QGIS_MAC_DEPS_DIR )
    set(ENV{LIB_DIR} ${QGIS_MAC_DEPS_DIR})
    list(APPEND CMAKE_PREFIX_PATH ${QGIS_MAC_DEPS_DIR})
  endif()
endif()

#############################################################
# Configure OpenCL if available
set(HAVE_OPENCL FALSE)
if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "BSD$")
  option(USE_OPENCL "Use OpenCL" OFF)
else()
  option(USE_OPENCL "Use OpenCL" ON)
endif()
if (USE_OPENCL)
    find_package(OpenCL)
    if(${OpenCL_FOUND})
        set(HAVE_OPENCL TRUE)
        # Fixup for standard FindOpenCL module not assigning proper framework headers directory
        if (APPLE AND "${OpenCL_INCLUDE_DIR}" MATCHES "OpenCL\\.framework/?$")
            set(OpenCL_INCLUDE_DIR "${OpenCL_INCLUDE_DIR}/Headers" CACHE PATH "" FORCE)
            set(OpenCL_INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
        endif()
        find_package(OpenCLhpp)
        if(NOT OPENCL_HPP_FOUND)
            # Use internal headers copied from OpenCL-CLHPP project
            set(OPENCL_HPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/opencl-clhpp/include CACHE PATH "" FORCE)
            message(STATUS "Couldn't find OpenCL C++ headers, using internal: ${OPENCL_HPP_INCLUDE_DIR}")
        endif()
        set(OpenCL_INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS} ${OPENCL_HPP_INCLUDE_DIR})
    else()
        message(STATUS "Couldn't find OpenCL: support DISABLED")
    endif()
endif()

# Configure CCache if available
if(NOT MSVC)
  option(USE_CCACHE "Use ccache" ON)
  if (USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
        message(STATUS "ccache found")
    endif(CCACHE_FOUND)
  endif(USE_CCACHE)
endif(NOT MSVC)

if (IOS)
    set (DEFAULT_FORCE_STATIC_LIBS TRUE)
else()
    set (DEFAULT_FORCE_STATIC_LIBS FALSE)
endif()

set (FORCE_STATIC_LIBS ${DEFAULT_FORCE_STATIC_LIBS} CACHE BOOL "Determines whether libraries should be static only")
mark_as_advanced(FORCE_STATIC_LIBS)
if (FORCE_STATIC_LIBS)
  set (LIBRARY_TYPE STATIC)
  # following variable is used in qgsconfig.h
  set (HAVE_STATIC_PROVIDERS TRUE)
else()
  set (LIBRARY_TYPE SHARED)
  unset(HAVE_STATIC_PROVIDERS)
endif()

# in generated makefiles use relative paths so the project dir is moveable
# Note commented out since it cause problems but it would be nice to resolve these and enable
#
# issue is caused by include_directories(${CMAKE_BINARY_DIR}) near the end of this file generating incorrect path
#set (CMAKE_USE_RELATIVE_PATHS ON)

set (WITH_CORE TRUE CACHE BOOL "Determines whether QGIS core should be built.")
mark_as_advanced(WITH_CORE)

if(WITH_CORE)
  # Try to configure and build GRASS plugin by default
  foreach (GRASS_SEARCH_VERSION 7 8)
    # Legacy note:
    # For GRASS 6 there were used cached variables without version suffix so that existing caches didn't have to be reconfigured.
    # Cached variables were: WITH_GRASS, WITH_GRASS7, GRASS_PREFIX, GRASS_PREFIX7, GRASS_INCLUDE_DIR, GRASS_INCLUDE_DIR7
    # Everywhere else each variable has version major appended.
    # Normal variables were: GRASS_FOUND6, GRASS_FOUND7, GRASS_MAJOR_VERSION6, GRASS_MAJOR_VERSION7, etc.
    # In addition there is also GRASS_FOUND, which is TRUE if at least one version of GRASS was found
    set (GRASS_CACHE_VERSION ${GRASS_SEARCH_VERSION})
    set (WITH_GRASS${GRASS_CACHE_VERSION} TRUE CACHE BOOL "Determines whether GRASS ${GRASS_SEARCH_VERSION} plugin should be built")
    if (WITH_GRASS${GRASS_CACHE_VERSION})
      find_package(GRASS ${GRASS_SEARCH_VERSION})
      set (GRASS_PREFIX${GRASS_CACHE_VERSION} ${GRASS_PREFIX${GRASS_SEARCH_VERSION}} CACHE PATH "Path to GRASS ${GRASS_SEARCH_VERSION} base directory")
    endif()
  endforeach (GRASS_SEARCH_VERSION)

  set (WITH_GUI TRUE CACHE BOOL "Determines whether QGIS GUI library (and everything built on top of it) should be built")

  set (WITH_OAUTH2_PLUGIN TRUE CACHE BOOL "Build OAuth2 authentication method plugin")
  if(WITH_OAUTH2_PLUGIN)
    set(HAVE_OAUTH2_PLUGIN TRUE)
  endif()

  set (WITH_AUTH TRUE CACHE BOOL "Determines whether QGIS authentication methods should be built")

  set (WITH_ANALYSIS TRUE CACHE BOOL "Determines whether QGIS analysis library should be built")

  set (WITH_DESKTOP TRUE CACHE BOOL "Determines whether QGIS desktop should be built")

  if(WITH_DESKTOP)
      if((WIN32 AND NOT MINGW) OR (UNIX AND NOT APPLE AND NOT ANDROID AND NOT IOS))
	set (CRASH_HANDLER_AVAILABLE TRUE)
      else()
	set (CRASH_HANDLER_AVAILABLE FALSE)
      endif()

      set (WITH_CRASH_HANDLER ${CRASH_HANDLER_AVAILABLE} CACHE BOOL "Determines whether the QGIS crash handler application should be built")
      if(WITH_CRASH_HANDLER AND NOT CRASH_HANDLER_AVAILABLE)
	message(FATAL_ERROR "Crash handler cannot be built on this environment. Set WITH_CRASH_HANDLER to false.")
      endif()
      if(WITH_CRASH_HANDLER)
	set (HAVE_CRASH_HANDLER TRUE)    # used in qgsconfig.h
      else()
	set (HAVE_CRASH_HANDLER FALSE)    # used in qgsconfig.h
      endif()
  endif()

  set (WITH_3D FALSE CACHE BOOL "Determines whether QGIS 3D library should be built")

  set (WITH_QUICK FALSE CACHE BOOL "Determines whether QGIS Quick library should be built")

  set (WITH_QGIS_PROCESS TRUE CACHE BOOL "Determines whether the standalone \"qgis_process\" tool should be built")

  set (NATIVE_CRSSYNC_BIN "" CACHE PATH "Path to a natively compiled synccrsdb binary. If set, crssync will not build but use provided bin instead.")
  mark_as_advanced (NATIVE_CRSSYNC_BIN)

  # try to configure and build python bindings by default
  set (WITH_BINDINGS TRUE CACHE BOOL "Determines whether python bindings should be built")
  if (WITH_BINDINGS)
    # By default bindings will be installed only to QGIS directory
    # Someone might want to install it to python site-packages directory
    # as otherwise user has to use PYTHONPATH environment variable to add
    # QGIS bindings to package search path
    set (BINDINGS_GLOBAL_INSTALL FALSE CACHE BOOL "Install bindings to global python directory? (might need root)")
    set (SIP_GLOBAL_INSTALL FALSE CACHE BOOL "Install sip source files to system sip directory? (might need root)")
    set (WITH_STAGED_PLUGINS TRUE CACHE BOOL "Stage-install core Python plugins to run from build directory? (utilities and console are always staged)")
    set (WITH_PY_COMPILE FALSE CACHE BOOL "Determines whether Python modules in staged or installed locations are byte-compiled")
    # concatenate QScintilla2 API files
    if (WITH_GUI)
      set (WITH_QSCIAPI TRUE CACHE BOOL "Whether to generate PyQGIS QScintilla2 API file. (For devs) run 'make qsci-pap-src' in between QGIS build and install to regenerate .pap file in source tree for console auto-completion.")
      # keep casual users from updating their source tree via WITH_QSCIAPI
      mark_as_advanced (WITH_QSCIAPI)
    endif()
  endif()

  # server disabled default because it needs FastCGI (which is optional dependency)
  set (WITH_SERVER FALSE CACHE BOOL "Determines whether QGIS server should be built")
  if(WITH_SERVER)
    set (WITH_SERVER_LANDINGPAGE_WEBAPP FALSE CACHE BOOL "Determines whether QGIS server landingpage webapp should be built (requires nodejs and yarn)")

    set (SERVER_SKIP_ECW FALSE CACHE BOOL "Determines whether QGIS server should disable ECW (ECW in server apps requires a special license)")

    set (WITH_SERVER_PLUGINS ${WITH_BINDINGS} CACHE BOOL "Determines whether QGIS server support for python plugins should be built")
    if(WITH_SERVER_PLUGINS AND NOT WITH_BINDINGS)
      message(FATAL_ERROR "Server plugins are not supported without python bindings. Enable WITH_BINDINGS or disable WITH_SERVER_PLUGINS")
    endif()
    if(WITH_SERVER_PLUGINS)
      set(HAVE_SERVER_PYTHON_PLUGINS TRUE)
    endif()
  endif()

  # Custom widgets
  set (WITH_CUSTOM_WIDGETS FALSE CACHE BOOL "Determines whether QGIS custom widgets for Qt Designer should be built")

  if (NOT WITH_GUI)
    set (HAVE_GUI FALSE)    # used in qgsconfig.h
    # force value of some options
    if(WITH_DESKTOP)
      message(FATAL_ERROR "Desktop cannot be built without gui. Enable WITH_GUI or disable WITH_DESKTOP.")
    endif()
    if(WITH_CUSTOM_WIDGETS)
      message(FATAL_ERROR "Custom widgets cannot be built without gui. Enable WITH_GUI or disable WITH_CUSTOM_WIDGETS.")
    endif()
  else()
    set (HAVE_GUI TRUE)     # used in qgsconfig.h
  endif()

  if ( WITH_DESKTOP AND NOT WITH_ANALYSIS )
    message(FATAL_ERROR "Desktop cannot be built without analysis")
  endif()

  if ( WITH_QGIS_PROCESS AND NOT WITH_ANALYSIS )
    message(FATAL_ERROR "Process tool cannot be built without analysis")
  endif()

  if ( WITH_DESKTOP )
    # The qgis_desktop target is meant to build a minimal but complete running QGIS during development
    # This should help to reduce compile time while still having a "complete enough" QGIS for most of the development
    add_custom_target(qgis_desktop
      DEPENDS qgis qgispython pycore pygui pyanalysis provider_postgres staged-plugins pyplugin-installer resources svg doc icons
    )
  endif()

  # try to configure and build MDAL support
  set (WITH_INTERNAL_MDAL TRUE CACHE BOOL "Determines whether MDAL should be built from internal copy")
  if (NOT WITH_INTERNAL_MDAL)
    set (MDAL_PREFIX "" CACHE PATH "Path to MDAL base directory")
  endif()

  # try to configure and build POLY2TRI support
  set (WITH_INTERNAL_POLY2TRI TRUE CACHE BOOL "Determines whether POLY2TRI should be built from internal copy")

  # try to configure and build POSTGRESQL support
  set (WITH_POSTGRESQL TRUE CACHE BOOL "Determines whether POSTGRESQL support should be built")
  if (WITH_POSTGRESQL)
    set (POSTGRESQL_PREFIX "" CACHE PATH "Path to POSTGRESQL base directory")
  endif()

  # try to configure and build POSTGRESQL support
  set (WITH_SPATIALITE TRUE CACHE BOOL "Determines whether Spatialite support should be built (required for spatialite, virtual, wfs providers)")
  if (WITH_SPATIALITE)
      set (WITH_QSPATIALITE FALSE CACHE BOOL "Determines whether QSPATIALITE sql driver should be built")
  endif()

  set (WITH_ORACLE FALSE CACHE BOOL "Determines whether Oracle support should be built")
  if(WITH_ORACLE)
    set(HAVE_ORACLE TRUE)
    set(ORACLE_INCLUDEDIR "" CACHE STRING "Path to OCI headers")
    set(ORACLE_LIBDIR "" CACHE STRING "Path to OCI libraries")
  endif()

  set (WITH_HANA FALSE CACHE BOOL "Determines whether SAP HANA Spatial support should be built")
  if(WITH_HANA)
    find_package(ODBC)
    if(ODBC_FOUND)
      set(HAVE_HANA TRUE)
      add_subdirectory(external/odbccpp)
      set_target_properties(odbccpp_static PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    else()
      message(STATUS "Couldn't find ODBC library")
    endif()
  endif(WITH_HANA)

  set (WITH_PDAL FALSE CACHE BOOL "Determines whether PDAL support should be built")

  set (WITH_EPT TRUE CACHE BOOL "Determines whether Entwine Point Cloud (EPT) support should be built")

  set (WITH_THREAD_LOCAL TRUE CACHE BOOL "Determines whether std::thread_local should be used")
  mark_as_advanced(WITH_THREAD_LOCAL)

  if (MINGW OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    # MingW has broken support for thread_local, so force disabling it
    # see
    # https://sourceforge.net/p/mingw-w64/bugs/445/
    # https://sourceforge.net/p/mingw-w64/bugs/527/
    # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80816

    # also OpenBSD has no thread_local support, see https://github.com/qgis/QGIS/issues/25248

  else()
    if (WITH_THREAD_LOCAL)
      set (USE_THREAD_LOCAL TRUE)  # used in qgsconfig.h
    endif()
  endif()

  # Compile flag. Make it possible to turn it off.
  set (PEDANTIC TRUE CACHE BOOL "Determines if we should compile in pedantic mode.")

  # whether coverage tests should be performed
  set (ENABLE_COVERAGE FALSE CACHE BOOL "Perform coverage tests?")

  # whether coverage documentation should be generated
  set (GENERATE_COVERAGE_DOCS FALSE CACHE BOOL "Generate coverage docs (requires lcov)?")

  # hide this variable because building of python bindings might fail
  # if set to other directory than expected
  mark_as_advanced(LIBRARY_OUTPUT_PATH)

  if (MSVC AND CMAKE_GENERATOR MATCHES "NMake")
    # following variable is also used in qgsconfig.h
    set (USING_NMAKE TRUE)
  endif()

  if (CMAKE_GENERATOR MATCHES "Ninja")
    # following variable is also used in qgsconfig.h
    set (USING_NINJA TRUE)
  endif()

  find_package(FLEX 2.6 REQUIRED)
  find_package(BISON REQUIRED)

  #############################################################
  # search for dependencies

  if(NOT WIN32 AND NOT ANDROID)
    include(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(openpty OPENPTY_IN_LIBC)
    if(NOT OPENPTY_IN_LIBC)
      set(CMAKE_REQUIRED_INCLUDES util.h)
      set(CMAKE_REQUIRED_LIBRARIES util)
      CHECK_FUNCTION_EXISTS(openpty NEED_LIBUTIL)
      if(NEED_LIBUTIL)
        set(OPENPTY_LIBRARY util)
      else()
        message (SEND_ERROR "openpty not found!")
      endif()
    endif()
  endif()

  # required
  find_package(Proj)
  find_package(GEOS)
  find_package(GDAL)
  find_package(Expat REQUIRED)
  find_package(Spatialindex REQUIRED)
  if (WITH_GUI)
    find_package(Qwt REQUIRED)
  endif()
  find_package(LibZip REQUIRED)

  find_package(Sqlite3)
  if (NOT SQLITE3_FOUND)
    message (SEND_ERROR "sqlite3 dependency was not found!")
  endif()

  find_package(Protobuf REQUIRED)   # for decoding of vector tiles in MVT format
  message(STATUS "Found Protobuf: ${Protobuf_LIBRARIES}")
  if (NOT Protobuf_PROTOC_EXECUTABLE)
    message (SEND_ERROR "Protobuf library's 'protoc' tool was not found!")
  endif()
  find_package(ZLIB REQUIRED)       # for decompression of vector tiles in MBTiles file
  message(STATUS "Found zlib: ${ZLIB_LIBRARIES}")

  find_package(EXIV2 REQUIRED)

  # optional
  if (WITH_POSTGRESQL)
    find_package(Postgres) # PostgreSQL provider
  endif()

  if (NOT WITH_INTERNAL_MDAL)
    find_package(MDAL REQUIRED) # MDAL provider
  endif()

  if (NOT WITH_INTERNAL_POLY2TRI)
    find_package(Poly2Tri REQUIRED)
  endif()

  if (WITH_SPATIALITE)
    find_package(SpatiaLite REQUIRED)
    set (HAVE_SPATIALITE TRUE)
  endif()

  if (NOT PROJ_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)
    message (SEND_ERROR "Some dependencies were not found! Proj: ${PROJ_FOUND}, Geos: ${GEOS_FOUND}, GDAL: ${GDAL_FOUND}")
  endif()

  if (POSTGRES_FOUND)
    # following variable is used in qgsconfig.h
    set (HAVE_POSTGRESQL TRUE)
  endif()

  set (WITH_QTWEBKIT TRUE CACHE BOOL "Enable QtWebkit support")
  if (WITH_QTWEBKIT)
    add_definitions(-DWITH_QTWEBKIT)
    message(STATUS "Qt WebKit support enabled")
  else()
    message(STATUS "Qt WebKit support DISABLED.")
  endif()

  if (WITH_EPT)  # EPT provider
    find_package(ZSTD REQUIRED) # for decompression of point clouds
    find_package(LazPerf) # for decompression of point clouds
    if (NOT LazPerf_FOUND)
      message(STATUS "Using embedded laz-perf")
    endif()
    set(HAVE_EPT TRUE)  # used in qgsconfig.h
  endif()

  if (WITH_PDAL)
    if (NOT WITH_EPT)
      message(FATAL_ERROR "PDAL provider cannot be built with EPT disabled")
    endif()
    find_package(PDAL REQUIRED) # PDAL provider
    set(HAVE_PDAL_QGIS TRUE)  # used in qgisconfig.h. note -- we can't use HAVE_PDAL here as the grass public headers redefine this!
  endif()

  #############################################################
  # search for Qt

  set (WITH_QT6 FALSE CACHE BOOL "Enable (broken, experimental) Qt6 support")
  if (WITH_QT6)
    set(QT_MIN_VERSION 6.0.0)
    set(QT_VERSION_BASE "Qt6")
  else()
    set(QT_MIN_VERSION 5.12.0)
    set(QT_VERSION_BASE "Qt5")
    set(HAS_KDE_QT5_PDF_TRANSFORM_FIX FALSE CACHE BOOL "Using KDE's Qt 5.15 fork with the PDF brush transform fix")
    set(HAS_KDE_QT5_SMALL_CAPS_FIX FALSE CACHE BOOL "Using KDE's Qt 5.15 fork with the QFont::SmallCaps fix")
    set(HAS_KDE_QT5_FONT_STRETCH_FIX FALSE CACHE BOOL "Using KDE's Qt 5.15 fork with the QFont stretch fix")
  endif()

  # Use Qt5SerialPort optionally for GPS
  set (WITH_QT5SERIALPORT TRUE CACHE BOOL "Determines whether Qt5SerialPort should be tried for GPS positioning")
  if (WITH_QT5SERIALPORT)
      find_package(${QT_VERSION_BASE} COMPONENTS SerialPort REQUIRED)
      # following variable is used in qgsconfig.h
      set (HAVE_QT5SERIALPORT TRUE)
  endif()

  find_package(${QT_VERSION_BASE} COMPONENTS Core Gui Widgets Network Xml Svg Concurrent Test UiTools Sql REQUIRED)
  if (WITH_QT6)
    find_package(${QT_VERSION_BASE} COMPONENTS Core5Compat REQUIRED)
  else()
    # TODO only available starting from Qt 6.2
    find_package(${QT_VERSION_BASE} COMPONENTS Positioning)
  endif()
  if (NOT IOS)
    find_package(${QT_VERSION_BASE} COMPONENTS PrintSupport REQUIRED)
  else()
    add_definitions(-DQT_NO_PRINTER)
  endif()
  if (WITH_QTWEBKIT AND NOT WITH_QT6)
    find_package(Qt5WebKit REQUIRED)
    find_package(Qt5WebKitWidgets REQUIRED)
  endif()
  if (WITH_3D)
    find_package(${QT_VERSION_BASE} COMPONENTS 3DCore 3DRender 3DInput 3DLogic 3DExtras REQUIRED)
    set(HAVE_3D TRUE)  # used in qgsconfig.h
  endif()
  if (APPLE)
    find_package(${QT_VERSION_BASE} COMPONENTS MacExtras REQUIRED)
  endif()

  # get the Qt plugins directory
  get_target_property(QMAKE_EXECUTABLE ${QT_VERSION_BASE}::qmake LOCATION)

  EXEC_PROGRAM(${QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_PLUGINS" RETURN_VALUE return_code OUTPUT_VARIABLE DEFAULT_QT_PLUGINS_DIR )
  set (QT_PLUGINS_DIR ${DEFAULT_QT_PLUGINS_DIR} CACHE STRING "Path to installation directory for Qt Plugins. Defaults to Qt native plugin directory")

  if (WITH_QT6)
    message(STATUS "Found Qt version: ${Qt6Core_VERSION}")
  else()
    message(STATUS "Found Qt version: ${Qt5Core_VERSION_STRING}")
  endif()
  if (WITH_QUICK)
    find_package(${QT_VERSION_BASE} COMPONENTS Qml Quick REQUIRED)
    if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
      find_package(${QT_VERSION_BASE} COMPONENTS AndroidExtras)
    endif()

    # following variable is used in qgsconfig.h
    set (HAVE_QUICK TRUE)
  endif()

  if(WITH_QTWEBKIT)
    set(OPTIONAL_QTWEBKIT ${Qt5WebKitWidgets_LIBRARIES})
  endif()

  # search for QScintilla2 (C++ lib)
  if (WITH_GUI)
    find_package(QScintilla REQUIRED)
  endif()

  # Password helper
  find_package(QtKeychain REQUIRED)
  # Master password hash and authentication encryption
  find_package(QCA REQUIRED)
  # Check for runtime dependency of qca-ossl plugin
  # REQUIRED if unit tests are to be run from build directory
  if(NOT MSVC)
    include(QCAMacros)
    FIND_QCAOSSL_PLUGIN_CPP(ENABLE_TESTS)
  endif()

  if (APPLE)
    # Libtasn1 is for DER-encoded PKI ASN.1 parsing/extracting workarounds
    find_package(Libtasn1 REQUIRED)
  endif()

  if (SUPPRESS_QT_WARNINGS)
    # Newer versions of UseQt4.cmake include Qt with -isystem automatically
    # This can be used to force this behavior on older systems
    # Can be removed as soon as Travis-CI updates from precise
    include_directories(SYSTEM ${QT_INCLUDE_DIR})
  endif()

  # Disable automatic conversion from QString to ASCII 8-bit strings (char *)
  # (Keeps code compatible with Qt/Mac/64bit)
  add_definitions(-DQT_NO_CAST_TO_ASCII)
endif()

set(CMAKE_AUTOMOC ON)

# build our version of astyle
set (WITH_ASTYLE FALSE CACHE BOOL "If you plan to contribute you should reindent with scripts/prepare_commit.sh (using 'our' astyle)")

# QML
set(QML_IMPORT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE PATH "QML directory for QML autocomplete")

#############################################################
# testing
# whether unit tests should be build
set (ENABLE_TESTS TRUE CACHE BOOL "Build unit tests?")
if (ENABLE_TESTS)
  set (PUSH_TO_CDASH FALSE CACHE BOOL "Determines whether test results should be pushed to CDASH site")
  set(QT_USE_QTTEST TRUE)
  enable_testing()
  # Adds some testing specific build targets e.g. make Experimental
  include(Dart)
  # Additional test configuration options e.g. max upload size of test report
  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake_templates/CTestCustom.cmake.in"
    "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    IMMEDIATE @ONLY)
  if (PUSH_TO_CDASH)
    configure_file(
      "${CMAKE_SOURCE_DIR}/cmake_templates/CTestConfig.cmake.in"
      "${CMAKE_BINARY_DIR}/CTestConfig.cmake"
      IMMEDIATE @ONLY)
  endif()
  # For server side testing we have no X, we can use xvfb as a fake x
  # sudo apt-get install xvfb
  add_custom_target(check COMMAND xvfb-run --server-args=-screen\ 0\ 1024x768x24 ctest --output-on-failure)
endif()

if (WITH_CORE)
# ModelTest
  set(ENABLE_MODELTEST FALSE CACHE BOOL "Enable QT ModelTest (not for production)")
endif()

#############################################################
# enable warnings

if (PEDANTIC)
  message (STATUS "Pedantic compiler settings enabled")
  if(MSVC)
    set(_warnings "")
    if (NOT USING_NMAKE AND NOT USING_NINJA)
      set(_warnings "${_warnings} /W4" )
    endif()

    # disable warnings
    set(_warnings "${_warnings} /wd4091 ")  # 'typedef': ignored on left of '' when no variable is declared (occurs in MS DbgHelp.h header)
    set(_warnings "${_warnings} /wd4100 ")  # unused formal parameters
    set(_warnings "${_warnings} /wd4127 ")  # constant conditional expressions (used in Qt template classes)
    set(_warnings "${_warnings} /wd4190 ")  # 'identifier' has C-linkage specified, but returns UDT 'identifier2' which is incompatible with C
    set(_warnings "${_warnings} /wd4231 ")  # nonstandard extension used : 'identifier' before template explicit instantiation (used in Qt template classes)
    set(_warnings "${_warnings} /wd4244 ")  # conversion from '...' to '...' possible loss of data
    set(_warnings "${_warnings} /wd4251 ")  # needs to have dll-interface to be used by clients of class (occurs in Qt template classes)
    set(_warnings "${_warnings} /wd4267 ")  # 'argument': conversion from 'size_t' to 'int', possible loss of data
    set(_warnings "${_warnings} /wd4275 ")  # non dll-interface class '...' used as base for dll-interface class '...'
    set(_warnings "${_warnings} /wd4290 ")  # c++ exception specification ignored except to indicate a function is not __declspec(nothrow) (occurs in sip generated bindings)
    set(_warnings "${_warnings} /wd4456 ")  # declaration of '...' hides previous local declaration
    set(_warnings "${_warnings} /wd4457 ")  # declaration of '...' hides a function parameter
    set(_warnings "${_warnings} /wd4458 ")  # declaration of '...' hides class member
    set(_warnings "${_warnings} /wd4505 ")  # unreferenced local function has been removed (QgsRasterDataProvider::extent)
    set(_warnings "${_warnings} /wd4510 ")  # default constructor could not be generated (sqlite3_index_info, QMap)
    set(_warnings "${_warnings} /wd4512 ")  # assignment operator could not be generated (sqlite3_index_info)
    set(_warnings "${_warnings} /wd4610 ")  # user defined constructor required (sqlite3_index_info)
    set(_warnings "${_warnings} /wd4706 ")  # assignment within conditional expression (pal)
    set(_warnings "${_warnings} /wd4714 ")  # function '...' marked as __forceinline not inlined (QString::toLower/toUpper/trimmed)
    set(_warnings "${_warnings} /wd4800 ")  # 'int' : forcing value to bool 'true' or 'false' (performance warning)
    set(_warnings "${_warnings} /wd4996 ")  # '...': was declared deprecated (unfortunately triggered when implementing deprecated interfaces even when it is deprecated too)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_warnings}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_warnings}")
  else()
    # add warnings via flags (not as definitions as on Mac -Wall can not be overridden per language )
    set(_warnings "-Wall -Wextra -Wno-long-long -Wformat-security -Wno-strict-aliasing")

    set(WERROR FALSE CACHE BOOL "Treat build warnings as errors.")
    if (WERROR)
      set(_warnings "${_warnings} -Werror")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_warnings}")

    # c++ only warnings

    set(_warnings "${_warnings} -Wnon-virtual-dtor")

    # unavoidable - we can't avoid these, as older, supported compilers do not support removing the redundant move
    set(_warnings "${_warnings} -Wno-redundant-move")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.9.999)
      # heaps of these thrown by Qt headers at the moment (sep 2019)
      set(_warnings "${_warnings} -Wno-deprecated-copy")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_warnings}")

    # Qt produces lots of warnings with strict aliasing (as of Qt 4.4.0 & GCC 4.3)
    # There are redundant declarations in Qt and GDAL
    # add_definitions( -fstrict-aliasing -Wstrict-aliasing=1 -Wredundant-decls )

    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wreturn-type-c-linkage -Woverloaded-virtual -Wimplicit-fallthrough")
    endif()

    # add any extra CXXFLAGS flags set by user. can be -D CXX_EXTRA_FLAGS or environment variable
    # command line -D option overrides environment variable
    # e.g. useful for suppressing transient upstream warnings in dependencies, like Qt
    set(CXX_EXTRA_FLAGS "" CACHE STRING "Additional appended CXXFLAGS")
    if ("${CXX_EXTRA_FLAGS}" STREQUAL "" AND DEFINED $ENV{CXX_EXTRA_FLAGS})
      set(CXX_EXTRA_FLAGS "$ENV{CXX_EXTRA_FLAGS}")
    endif()
    if (NOT "${CXX_EXTRA_FLAGS}" STREQUAL "")
      message (STATUS "Appending CXX_EXTRA_FLAGS")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_EXTRA_FLAGS}")
    endif()
  endif()

endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Qunused-arguments")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Qunused-arguments")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Qunused-arguments")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
  # spatialite crashes on ppc - see bugs.debian.org/603986
  add_definitions( -fno-strict-aliasing )
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  message (STATUS "Debug output enabled")
  set(QGISDEBUG TRUE)
else()
  set(QGISDEBUG FALSE)
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 /std:c++17")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /std:c++17")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  # disable macros that offend std::numeric_limits<T>::min()/max()
  add_definitions(-DNOMINMAX)
endif()

# Prevent build when Qt api deprecated before this Qt version is used:
add_definitions(-DQT_DEPRECATED_WARNINGS)
# Unfortunately Qwt uses deprecated QString::null in headers, preventing this being raised above 5.8
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050800)

# For fast string concatenation
add_definitions(-DQT_USE_QSTRINGBUILDER)

set(WITH_GSL TRUE CACHE BOOL "Determines whether GSL library should be used")

if (WITH_ANALYSIS AND WITH_GSL)
  find_package(GSL REQUIRED)
  set(HAVE_GSL TRUE)
  set(HAVE_GEOREFERENCER TRUE)
endif()

if(ENABLE_COVERAGE)
  include("cmake/modules/coverage/CodeCoverage.cmake")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SETUP_TARGET_FOR_COVERAGE(qgis_coverage ctest coverage)
endif()

#############################################################
# platform specific stuff
if (WITH_CORE)
  if (WIN32)
    set (DEFAULT_LIB_SUBDIR     lib)
    set (DEFAULT_LIBEXEC_SUBDIR .)
    set (DEFAULT_DATA_SUBDIR    .)
    set (DEFAULT_PLUGIN_SUBDIR  plugins)
    set (DEFAULT_INCLUDE_SUBDIR include)
    set (DEFAULT_QML_SUBDIR     qml)

    set (DEFAULT_SERVER_MODULE_SUBDIR server)

    if (MSVC)
      set (DEFAULT_BIN_SUBDIR bin)
      set (DEFAULT_CGIBIN_SUBDIR bin)
      # put all the build products into a single directory
      # under build (doesn't affect install target) to make for
      # easier debugging.

      # Turn on defines for non standard maths stuff
      add_definitions(-D_USE_MATH_DEFINES)

      # Turn off deprecation warnings
      add_definitions(-D_CRT_SECURE_NO_WARNINGS)
      add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)

      if (INSTALL_DEPS)
        install(DIRECTORY ${INSTALL_DEPS} DESTINATION .)
      endif()
    else()
      set (DEFAULT_BIN_SUBDIR .)
      set (DEFAULT_CGIBIN_SUBDIR .)
    endif()
  else()

    if(APPLE)
        set(QGIS_MACAPP_FRAMEWORK TRUE CACHE BOOL "Build as a framework on OSX")
    endif()

    if (APPLE AND QGIS_MACAPP_FRAMEWORK)
      if (POLICY CMP0042) # in CMake 3.0.0+
        set (CMAKE_MACOSX_RPATH OFF) # otherwise ON by default
      endif()
      if (POLICY CMP0068) # in CMake 3.9.0+
        cmake_policy(SET CMP0068 NEW)
      endif()
      # for Mac OS X, everything is put inside an application bundle
      # save the root install prefix for the app later
      set (QGIS_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
      set (QGIS_MACAPP_PREFIX ${CMAKE_INSTALL_PREFIX}/${QGIS_APP_NAME}.app/Contents)
      # common prefix for components, let cmake handle it
      set (CMAKE_INSTALL_PREFIX ${QGIS_MACAPP_PREFIX}/MacOS)
      # 5 bundling levels, each includes previous
      #  -1  nothing
      #   0  fixup the library paths for all QGIS libraries with @loader_path
      #   1  Qt frameworks
      #   2  non-system libraries, "standard"
      #   3  non-system frameworks, "standalone"
      set (QGIS_MACAPP_BUNDLE 1 CACHE STRING "What to bundle into app package")
      set (QGIS_MACAPP_BUNDLE_USER "" CACHE STRING "Path to user bundling script")
      set (QGIS_MACAPP_INSTALL_DEV FALSE CACHE BOOL "Install developer frameworks")
      set (QGIS_MACAPP_DEV_PREFIX "/Library/Frameworks" CACHE STRING "Path to install developer frameworks")

      set (DEFAULT_BIN_SUBDIR      bin)
      set (QGIS_BIN_SUBDIR_REV     ..)
      set (DEFAULT_CGIBIN_SUBDIR   fcgi-bin)
      set (QGIS_CGIBIN_SUBDIR_REV  ..)
      set (DEFAULT_LIB_SUBDIR      lib)
      set (QGIS_LIB_SUBDIR_REV     ..)
      set (QGIS_FW_SUBDIR          ../Frameworks)
      set (QGIS_FW_SUBDIR_REV      ../MacOS)
      set (DEFAULT_DATA_SUBDIR     ../Resources)
      set (QGIS_DATA_SUBDIR_REV    ../MacOS)
      set (DEFAULT_LIBEXEC_SUBDIR  lib/qgis)
      set (QGIS_LIBEXEC_SUBDIR_REV ../..)
      set (DEFAULT_PLUGIN_SUBDIR   ../PlugIns/qgis)
      set (QGIS_PLUGIN_SUBDIR_REV  ../../MacOS)
      set (DEFAULT_INCLUDE_SUBDIR  include/qgis)
      set (DEFAULT_QML_SUBDIR     qml)

      # Set server moodules path to DEFAULT_LIBEXEC_SUBDIR+'/server'
      set (DEFAULT_SERVER_MODULE_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR}/server)

      # path for framework references when running from build directory
      # changed later to reference in-app resources upon install
      set (CMAKE_INSTALL_NAME_DIR ${CMAKE_BINARY_DIR}/output/lib)
      # recent cmakes force SDKs, recent SDKs don't have user symlinks
      # need to find non-system frameworks
      # cmake bug #0007250 - CMAKE_SHARED_LINKER_FLAGS ignored when creating
      # a framework, so these need to be manually handled with LINK_FLAGS options
      set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -F/Library/Frameworks")
      set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -F/Library/Frameworks")
      set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F/Library/Frameworks")

    else()
      # UNIX
      set (DEFAULT_BIN_SUBDIR     bin)
      set (DEFAULT_CGIBIN_SUBDIR  bin)
      set (DEFAULT_LIB_SUBDIR     lib${LIB_SUFFIX})
      set (DEFAULT_DATA_SUBDIR    share/qgis)
      set (DEFAULT_LIBEXEC_SUBDIR lib${LIB_SUFFIX}/qgis)
      set (DEFAULT_PLUGIN_SUBDIR  lib${LIB_SUFFIX}/qgis/plugins)
      set (DEFAULT_INCLUDE_SUBDIR include/qgis)
      set (DEFAULT_QML_SUBDIR     qml)

      set (DEFAULT_SERVER_MODULE_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR}/server)

      # QGIS_MACAPP_FRAMEWORK=FALSE
      if(APPLE)
        set (QGIS_MACAPP_BUNDLE -1)
        set (CMAKE_FRAMEWORK FALSE)
        set (QGIS_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
      endif()
    endif()

  endif()

  if (ANDROID)
    set (DEFAULT_PLUGIN_SUBDIR  lib)
    set (DEFAULT_DATA_SUBDIR    files/share)
    string(REPLACE "<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME>" "" CMAKE_CXX_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
  endif()

  #assume we have escaped compiler directives
  #eventually we want to change this to new
  #since we don't need to jump through so many
  #hoops to escape compiler directives then
  if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    if(NOT "${CMAKE_VERSION}" VERSION_LESS "3.3")
      cmake_policy(SET CMP0063 NEW)
    endif()
    if(MSVC)
      cmake_policy(SET CMP0020 NEW)
    endif()
  endif()

  if("${CMAKE_SYSTEM_NAME}"  MATCHES "Linux")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
  endif() #"${CMAKE_SYSTEM_NAME}"  MATCHES "Linux")

  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  include(GenerateExportHeader)

  set (ADD_CLAZY_CHECKS ${DEFAULT_MANUAL_SUBDIR} CACHE STRING "Add default set of clazy checks which should not raise any warnings")
  mark_as_advanced (ADD_CLAZY_CHECKS)
  if (ADD_CLAZY_CHECKS)
  set(CMAKE_CXX_BASE_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CLAZY_BASE_CHECKS "connect-3arg-lambda,lambda-unique-connection,empty-qstringliteral,fully-qualified-moc-types,lambda-in-connect,lowercase-qml-type-name,qcolor-from-literal,qfileinfo-exists,qmap-with-pointer-key,unused-non-trivial-variable,overridden-signal,qdeleteall,qstring-left,skipped-base-method,missing-qobject-macro,isempty-vs-count")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS} -Xclang -plugin-arg-clazy -Xclang ${CLAZY_BASE_CHECKS}")
  endif()
endif()

#############################################################
# user-changeable settings which can be used to customize
# layout of QGIS installation
# (default values are platform-specific)

set (QGIS_BIN_SUBDIR     ${DEFAULT_BIN_SUBDIR}     CACHE STRING "Subdirectory where executables will be installed")
set (QGIS_CGIBIN_SUBDIR  ${DEFAULT_CGIBIN_SUBDIR}  CACHE STRING "Subdirectory where CGI executables will be installed")
set (QGIS_LIB_SUBDIR     ${DEFAULT_LIB_SUBDIR}     CACHE STRING "Subdirectory where libraries will be installed")
set (QGIS_LIBEXEC_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR} CACHE STRING "Subdirectory where private executables will be installed")
set (QGIS_DATA_SUBDIR    ${DEFAULT_DATA_SUBDIR}    CACHE STRING "Subdirectory where QGIS data will be installed")
set (QGIS_PLUGIN_SUBDIR  ${DEFAULT_PLUGIN_SUBDIR}  CACHE STRING "Subdirectory where plugins will be installed")
set (QGIS_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING "Subdirectory where header files will be installed")
set (QGIS_QML_SUBDIR     ${DEFAULT_QML_SUBDIR}     CACHE STRING "Subdirectory where qml files/libraries will be installed")

set (QGIS_SERVER_MODULE_SUBDIR  ${DEFAULT_SERVER_MODULE_SUBDIR}  CACHE STRING "Subdirectory where server modules will be installed")

# mark *_SUBDIR variables as advanced as this is not something
# that an average user would use
mark_as_advanced (QGIS_BIN_SUBDIR QGIS_CGIBIN_SUBDIR QGIS_LIB_SUBDIR QGIS_LIBEXEC_SUBDIR QGIS_DATA_SUBDIR QGIS_PLUGIN_SUBDIR QGIS_INCLUDE_SUBDIR)

# full paths for the installation
set (QGIS_BIN_DIR     ${QGIS_BIN_SUBDIR})
set (QGIS_CGIBIN_DIR  ${QGIS_CGIBIN_SUBDIR})
set (QGIS_LIB_DIR     ${QGIS_LIB_SUBDIR})
set (QGIS_LIBEXEC_DIR ${QGIS_LIBEXEC_SUBDIR})
set (QGIS_DATA_DIR    ${QGIS_DATA_SUBDIR})
set (QGIS_PLUGIN_DIR  ${QGIS_PLUGIN_SUBDIR})
set (QGIS_INCLUDE_DIR ${QGIS_INCLUDE_SUBDIR})
set (QGIS_QML_DIR     ${QGIS_QML_SUBDIR})

set (QGIS_SERVER_MODULE_DIR ${QGIS_SERVER_MODULE_SUBDIR})

# set the default locations where the targets (executables, libraries) will land when compiled
# this is to allow running qgis from the source tree without having to actually do a "make install"
set (QGIS_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_BIN_SUBDIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR})

# write a marker with source directory path into the output's bin directory
# if run from the build directory QGIS will detect it and alter the paths
file(WRITE ${QGIS_OUTPUT_DIRECTORY}/${QGIS_BIN_SUBDIR}/qgisbuildpath.txt "${CMAKE_SOURCE_DIR}\n${QGIS_OUTPUT_DIRECTORY}")

# manual page - makes sense only on unix systems
if (UNIX AND NOT APPLE)
  set (DEFAULT_MANUAL_SUBDIR  man)
  set (QGIS_MANUAL_SUBDIR  ${DEFAULT_MANUAL_SUBDIR}  CACHE STRING "Subdirectory where manual files will be installed")
  mark_as_advanced (QGIS_MANUAL_SUBDIR)
  set (QGIS_MANUAL_DIR  ${CMAKE_INSTALL_PREFIX}/${QGIS_MANUAL_SUBDIR})
endif()

set (DISABLE_DEPRECATED FALSE CACHE BOOL "If set to true, it will disable deprecated functionality to prepare for the next generation of QGIS")
if (DISABLE_DEPRECATED)
  add_definitions(-DQGIS_DISABLE_DEPRECATED)
endif()

# whether to install required system libs in the output package
set(QGIS_INSTALL_SYS_LIBS TRUE CACHE BOOL "If set to TRUE install all required system libs in the output package")

#############################################################
# Python

set(MIN_PYTHON_VERSION "3.7")
set(Python_FIND_FRAMEWORK "LAST")


if (WITH_BINDINGS)
  find_package(Python ${MIN_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)
else()
  find_package(Python ${MIN_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter)
endif()

message("-- Found Python executable: ${Python_EXECUTABLE} (version ${Python_VERSION})")
message("-- Python library: ${Python_LIBRARIES}")
message("-- Python site-packages: ${Python_SITEARCH}")

if (WITH_CORE AND WITH_BINDINGS AND NOT WITH_QT6)
  set(PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)
  set (QGIS_PYTHON_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIRECTORY}/qgis)

  # python support: check for interpreter, sip, pyqt5
  find_package(SIP REQUIRED)
  find_package(PyQt5 REQUIRED)
  set(PYQT_SIP_FLAGS ${PYQT5_SIP_FLAGS})
  set(PYQT_SIP_DIR ${PYQT5_SIP_DIR})
  separate_arguments(PYQT_SIP_FLAGS) # convert space separated values to a list

  find_package(Qsci REQUIRED)
  include(PythonMacros)
  include(PyQtMacros)
  include(SIPMacros)

  set(SIP_INCLUDES ${PYQT_SIP_DIR} ${CMAKE_SOURCE_DIR}/python)
  set(SIP_CONCAT_PARTS 16)

  if (NOT BINDINGS_GLOBAL_INSTALL)
    set(Python_SITEARCH ${QGIS_DATA_DIR}/python)
  endif()

  if (WITH_CUSTOM_WIDGETS)
    set(PYUIC_WIDGET_PLUGIN_DIRECTORY ${PYQT5_MOD_DIR}/uic/widget-plugins/)
  endif()
endif()


#############################################################
# create qgsconfig.h
# installed with app target
configure_file(${CMAKE_SOURCE_DIR}/cmake_templates/qgsconfig.h.in ${CMAKE_BINARY_DIR}/qgsconfig.h)
include_directories(${CMAKE_BINARY_DIR})

#############################################################
# create qgsversion.h
include(CreateQgsVersion)
CREATE_QGSVERSION()

####################################################
# Added by Jef to prevent python core and gui libs linking to other qgisCore and qgisGui libs
# that may be in the same install prefix
if (WITH_CORE)
  link_directories(${CMAKE_BINARY_DIR}/src/core ${CMAKE_BINARY_DIR}/src/gui)
endif()

####################################################
# clang-tidy
set (WITH_CLANG_TIDY FALSE CACHE BOOL "Use Clang tidy")
mark_as_advanced(WITH_CLANG_TIDY)
if (WITH_CORE)
  if(WITH_CLANG_TIDY)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      DOC "Path to clang-tidy executable"
      )
    if(NOT CLANG_TIDY_EXE)
      message(STATUS "clang-tidy not found.")
    else()
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-cppcoreguidelines*,-readability-implicit-bool-cast,-llvm-include-order,-cert-err58-cpp,-modernize-pass-by-value,-google-readability-braces-around-statements,-modernize-use-auto,-modernize-loop-convert,-readability-else-after-return,-readability-braces-around-statements,-google-runtime-references,-readability-named-parameter,-google-default-arguments,-google-readability-todo,-readability-inconsistent-declaration-parameter-name,-cert-flp30-c,-google-readability-casting,-clang-analyzer-security.FloatLoopCounter,-google-runtime-int,-modernize-use-using,-google-explicit-constructor,-google-build-using-namespace,-cert-err34-c,-clang-analyzer-core.CallAndMessage,-google-readability-function-size,-modernize-make-shared,-modernize-use-nullptr,-clang-analyzer-cplusplus.NewDeleteLeaks,-clang-analyzer-core.NonNullParamChecker,performance-unnecessary-copy-initialization,-readability-simplify-boolean-expr,-modernize-raw-string-literal,-performance-unnecessary-copy-initialization")
    endif()
  endif()
endif()

#############################################################
# process subdirs

#create a variable to specify where our test data is
#so that unit tests can use TEST_DATA_DIR to locate
#the test data. See CMakeLists in test dirs for more info
#TEST_DATA_DIR is also used by QgsRenderChecker currently in core
set (TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/testdata")

add_subdirectory(doc)

if (WITH_CORE)
  add_subdirectory(src)
  add_subdirectory(images)
  add_subdirectory(resources)
  add_subdirectory(i18n)

  if (WITH_BINDINGS)
    add_subdirectory(python)
  endif()

  if (APPLE)
    # must be last for install, so install_name_tool can do its work
    add_subdirectory(mac)

    # allow QGIS to be run directly from build directory and to run unit tests
    execute_process(COMMAND /bin/mkdir -p "${QGIS_OUTPUT_DIRECTORY}/lib")
    execute_process(
      COMMAND /bin/ln -fs ../../Plugins/qgis/qgisgrass6.framework lib/
      WORKING_DIRECTORY "${QGIS_OUTPUT_DIRECTORY}"
    )
    execute_process(
      COMMAND /bin/ln -fs ../../Plugins/qgis/qgisgrass7.framework lib/
      WORKING_DIRECTORY "${QGIS_OUTPUT_DIRECTORY}"
    )
  endif()

  # manual page - makes sense only on unix systems
  if (UNIX AND NOT APPLE)
    install (FILES qgis.1 DESTINATION ${QGIS_MANUAL_DIR}/man1)
  endif()

  install(FILES cmake/FindQGIS.cmake DESTINATION ${QGIS_DATA_DIR})
endif()

if (WITH_ASTYLE)
  add_subdirectory(external/astyle)
endif()

if (ENABLE_TESTS)
  add_subdirectory(tests)
  set (CTEST_BINARY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin" )
  message (STATUS "Ctest Binary Directory set to: ${CTEST_BINARY_DIRECTORY}")
endif()

#############################################################
# Post-install commands
if (WITH_CORE)
  add_subdirectory(postinstall)
endif()

#############################################################
# Uninstall stuff see: http://www.vtk.org/Wiki/CMake_FAQ
if (WITH_CORE)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_templates/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

#############################################################
# Enable packaging
if (WITH_CORE)
  # Do not warn about runtime libs when building using VS Express
  if(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  endif()

  if(QGIS_INSTALL_SYS_LIBS)
    include(InstallRequiredSystemLibraries)
  endif()

  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QGIS")
  set(CPACK_PACKAGE_VENDOR "Open Source Geospatial Foundation")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "QGIS ${COMPLETE_VERSION}")
  if(WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backslashes.
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/win_build\\\\sidebar.bmp")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\qgis.exe")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} QGIS")
    set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\qgis.org")
    set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\qgis.org")
    set(CPACK_NSIS_CONTACT "info@qgis.org")
    set(CPACK_NSIS_MODIFY_PATH ON)

  #  set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  !include \\\"${CMAKE_SOURCE_DIR}\\\\win_build\\\\extra.nsh\\\"")
  else()
    #set(CPACK_STRIP_FILES "QGIS")
    #set(CPACK_SOURCE_STRIP_FILES "")
  endif()
  set(CPACK_PACKAGE_EXECUTABLES "qgis" "QGIS")
  set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
  include(CPack)
endif()

if (UNIX AND NOT APPLE)
  add_subdirectory(linux)
endif()
