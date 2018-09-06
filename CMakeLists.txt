#############################################################
# CMake settings
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.0)
SET(CMAKE_COLOR_MAKEFILE ON)
# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
# POLICIES
CMAKE_POLICY (SET CMP0048 NEW)
CMAKE_POLICY (SET CMP0053 NEW)

#############################################################
# Project and version
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(COMPLETE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
SET(RELEASE_NAME "Master")
PROJECT(qgis VERSION ${COMPLETE_VERSION})
IF (APPLE)
  SET(QGIS_APP_NAME "QGIS")
ELSE (APPLE)
  SET(QGIS_APP_NAME "qgis")
ENDIF (APPLE)

# Note the version no is Mmmpp for Major/minor/patch, 0-padded, thus '10100' for 1.1.0
MATH(EXPR QGIS_VERSION_INT "${CPACK_PACKAGE_VERSION_MAJOR}*10000+${CPACK_PACKAGE_VERSION_MINOR}*100+${CPACK_PACKAGE_VERSION_PATCH}")
MESSAGE(STATUS "QGIS version: ${COMPLETE_VERSION} ${RELEASE_NAME} (${QGIS_VERSION_INT})")


#############################################################
# Configure OpenCL if available
IF (APPLE)
  OPTION(USE_OPENCL "Use OpenCL" OFF)
ELSE (APPLE)
  OPTION(USE_OPENCL "Use OpenCL" ON)
ENDIF (APPLE)
IF (USE_OPENCL)
    FIND_PACKAGE(OpenCL)
    IF(${OpenCL_FOUND})
        SET(HAVE_OPENCL TRUE)
    ELSE(${OpenCL_FOUND})
        MESSAGE(STATUS "Couldn't find OpenCL: support DISABLED")
    ENDIF(${OpenCL_FOUND})
ENDIF(USE_OPENCL)

# Configure CCache if available
IF(NOT MSVC)
  option(USE_CCACHE "Use ccache" ON)
  if (USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
  endif(USE_CCACHE)
endif(NOT MSVC)

# in generated makefiles use relative paths so the project dir is moveable
# Note commented out since it cause problems but it would be nice to resolve these and enable
#
# issue is caused by INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}) near the end of this file generating incorrect path
#SET (CMAKE_USE_RELATIVE_PATHS ON)

SET (WITH_CORE TRUE CACHE BOOL "Determines whether QGIS core should be built.")
MARK_AS_ADVANCED(WITH_CORE)

IF(WITH_CORE)
  # Only GRASS 7 is currently supported but we keep dual version support in cmake for possible future switch to GRASS 8.
  # Try to configure and build GRASS plugin by default
  FOREACH (GRASS_SEARCH_VERSION 7)
    # Legacy note:
    # For GRASS 6 there were used cached variables without version suffix so that existing caches didn't have to be reconfigured.
    # Cached variables were: WITH_GRASS, WITH_GRASS7, GRASS_PREFIX, GRASS_PREFIX7, GRASS_INCLUDE_DIR, GRASS_INCLUDE_DIR7
    # Everywhere else each variable has version major appended.
    # Normal variables were: GRASS_FOUND6, GRASS_FOUND7, GRASS_MAJOR_VERSION6, GRASS_MAJOR_VERSION7, etc.
    # In addition there is also GRASS_FOUND, which is TRUE if at least one version of GRASS was found
    SET (GRASS_CACHE_VERSION ${GRASS_SEARCH_VERSION})
    SET (WITH_GRASS${GRASS_CACHE_VERSION} TRUE CACHE BOOL "Determines whether GRASS ${GRASS_SEARCH_VERSION} plugin should be built")
    IF (WITH_GRASS${GRASS_CACHE_VERSION})
      FIND_PACKAGE(GRASS ${GRASS_SEARCH_VERSION})
      SET (GRASS_PREFIX${GRASS_CACHE_VERSION} ${GRASS_PREFIX${GRASS_SEARCH_VERSION}} CACHE PATH "Path to GRASS ${GRASS_SEARCH_VERSION} base directory")
    ENDIF (WITH_GRASS${GRASS_CACHE_VERSION})
  ENDFOREACH (GRASS_SEARCH_VERSION)

  SET (WITH_GUI TRUE CACHE BOOL "Determines whether QGIS GUI library (and everything built on top of it) should be built")

  SET (WITH_DESKTOP TRUE CACHE BOOL "Determines whether QGIS desktop should be built")

  SET (WITH_3D FALSE CACHE BOOL "Determines whether QGIS 3D library should be built")

  SET (WITH_QUICK FALSE CACHE BOOL "Determines whether QGIS Quick library should be built")

  # server disabled default because it needs FastCGI (which is optional dependency)
  SET (WITH_SERVER FALSE CACHE BOOL "Determines whether QGIS server should be built")
  IF(WITH_SERVER)
    SET (SERVER_SKIP_ECW FALSE CACHE BOOL "Determines whether QGIS server should disable ECW (ECW in server apps requires a special license)")

    SET (WITH_SERVER_PLUGINS TRUE CACHE BOOL "Determines whether QGIS server support for python plugins should be built")
    IF(WITH_SERVER_PLUGINS)
      SET(HAVE_SERVER_PYTHON_PLUGINS TRUE)
    ENDIF(WITH_SERVER_PLUGINS)
  ENDIF(WITH_SERVER)

  # Custom widgets
  SET (WITH_CUSTOM_WIDGETS FALSE CACHE BOOL "Determines whether QGIS custom widgets for Qt Designer should be built")

  IF (NOT WITH_GUI)
    SET (HAVE_GUI FALSE)    # used in qgsconfig.h
    # force value of some options
    SET(WITH_DESKTOP FALSE)
    SET(WITH_CUSTOM_WIDGETS FALSE)
    SET(WITH_SERVER FALSE)  # for some reason server also uses GUI library
  ELSE ()
    SET (HAVE_GUI TRUE)     # used in qgsconfig.h
  ENDIF()

  # try to configure and build MDAL support
  SET (WITH_INTERNAL_MDAL TRUE CACHE BOOL "Determines whether MDAL support should be built")
  IF (NOT WITH_INTERNAL_MDAL)
    SET (MDAL_PREFIX "" CACHE PATH "Path to MDAL base directory")
  ENDIF (NOT WITH_INTERNAL_MDAL)

  # try to configure and build POSTGRESQL support
  SET (WITH_POSTGRESQL TRUE CACHE BOOL "Determines whether POSTGRESQL support should be built")
  IF (WITH_POSTGRESQL)
    SET (POSTGRESQL_PREFIX "" CACHE PATH "Path to POSTGRESQL base directory")
  ENDIF (WITH_POSTGRESQL)

  SET (WITH_QSPATIALITE FALSE CACHE BOOL "Determines whether QSPATIALITE sql driver should be built")

  SET (WITH_ORACLE FALSE CACHE BOOL "Determines whether Oracle support should be built")
  IF(WITH_ORACLE)
    SET(HAVE_ORACLE TRUE)
    SET(ORACLE_INCLUDEDIR "" CACHE STRING "Path to OCI headers")
    SET(ORACLE_LIBDIR "" CACHE STRING "Path to OCI libraries")
  ENDIF(WITH_ORACLE)


  # try to configure and build python bindings by default
  SET (WITH_BINDINGS TRUE CACHE BOOL "Determines whether python bindings should be built")
  IF (WITH_BINDINGS)
    # By default bindings will be installed only to QGIS directory
    # Someone might want to install it to python site-packages directory
    # as otherwise user has to use PYTHONPATH environment variable to add
    # QGIS bindings to package search path
    SET (BINDINGS_GLOBAL_INSTALL FALSE CACHE BOOL "Install bindings to global python directory? (might need root)")
    SET (WITH_STAGED_PLUGINS TRUE CACHE BOOL "Stage-install core Python plugins to run from build directory? (utilities and console are always staged)")
    SET (WITH_PY_COMPILE FALSE CACHE BOOL "Determines whether Python modules in staged or installed locations are byte-compiled")
    # concatenate QScintilla2 API files
    IF (WITH_GUI)
      SET (WITH_QSCIAPI TRUE CACHE BOOL "Whether to generate PyQGIS QScintilla2 API file. (For devs) run 'make qsci-pap-src' in between QGIS build and install to regenerate .pap file in source tree for console auto-completion.")
      # keep casual users from updating their source tree via WITH_QSCIAPI
      MARK_AS_ADVANCED (WITH_QSCIAPI)
    ENDIF (WITH_GUI)
    ADD_DEFINITIONS(-DWITH_BINDINGS)
  ENDIF (WITH_BINDINGS)

  #BUILD WITH QtMobility by default on android only. Other platform can force it
  IF (ANDROID)
      SET (DEFAULT_WITH_QTMOBILITY TRUE)
  ELSE (ANDROID)
      SET (DEFAULT_WITH_QTMOBILITY FALSE)
  ENDIF (ANDROID)
  SET (WITH_QTMOBILITY ${DEFAULT_WITH_QTMOBILITY} CACHE BOOL "Determines if QtMobility related code should be build (for example internal GPS)")

  SET (WITH_GEOREFERENCER TRUE CACHE BOOL "Determines whether GeoReferencer plugin should be built")

  SET (WITH_GLOBE FALSE CACHE BOOL "Determines whether Globe plugin should be built")
  IF (WITH_GLOBE)
    SET(QT_USE_QTOPENGL 1)
    FIND_PACKAGE(OSGEARTH REQUIRED)
    IF (OSGEARTHQT_LIBRARY)
      # following variable is used in qgsconfig.h
      SET(HAVE_OSGEARTHQT TRUE)
    ENDIF (OSGEARTHQT_LIBRARY)
  ENDIF (WITH_GLOBE)

  SET (WITH_THREAD_LOCAL TRUE CACHE BOOL "Determines whether std::thread_local should be used")
  MARK_AS_ADVANCED(WITH_THREAD_LOCAL)

  IF (MINGW OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    # MingW has broken support for thread_local, so force disabling it
    # see
    # https://sourceforge.net/p/mingw-w64/bugs/445/
    # https://sourceforge.net/p/mingw-w64/bugs/527/
    # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80816

    # also OpenBSD has no thread_local support, see https://issues.qgis.org/issues/17351

  ELSE (MINGW OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    IF (WITH_THREAD_LOCAL)
      SET (USE_THREAD_LOCAL TRUE)  # used in qgsconfig.h
    ENDIF (WITH_THREAD_LOCAL)
  ENDIF (MINGW OR CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")

  # Compile flag. Make it possible to turn it off.
  SET (PEDANTIC TRUE CACHE BOOL "Determines if we should compile in pedantic mode.")

  # whether coverage tests should be performed
  SET (ENABLE_COVERAGE FALSE CACHE BOOL "Perform coverage tests?")

  # whether coverage documentation should be generated
  SET (GENERATE_COVERAGE_DOCS FALSE CACHE BOOL "Generate coverage docs (requires lcov)?")

  # hide this variable because building of python bindings might fail
  # if set to other directory than expected
  MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH)

  IF (MSVC AND CMAKE_GENERATOR MATCHES "NMake")
    # following variable is also used in qgsconfig.h
    SET (USING_NMAKE TRUE)
  ENDIF (MSVC AND CMAKE_GENERATOR MATCHES "NMake")

  IF (CMAKE_GENERATOR MATCHES "Ninja")
    # following variable is also used in qgsconfig.h
    SET (USING_NINJA TRUE)
  ENDIF (CMAKE_GENERATOR MATCHES "Ninja")

  #############################################################
  # check if lexer and parser are not missing
  # http://www.mail-archive.com/cmake@cmake.org/msg02861.html

  INCLUDE(Flex)

  FIND_FLEX()

  IF (NOT FLEX_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Couldn't find Flex")
  ENDIF (NOT FLEX_EXECUTABLE)

  INCLUDE(Bison)

  FIND_BISON()

  IF (NOT BISON_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Couldn't find Bison")
  ENDIF (NOT BISON_EXECUTABLE)

  #############################################################
  # search for dependencies

  IF(NOT WIN32 AND NOT ANDROID)
    INCLUDE(CheckFunctionExists)
    CHECK_FUNCTION_EXISTS(openpty OPENPTY_IN_LIBC)
    IF(NOT OPENPTY_IN_LIBC)
      SET(CMAKE_REQUIRED_INCLUDES util.h)
      SET(CMAKE_REQUIRED_LIBRARIES util)
      CHECK_FUNCTION_EXISTS(openpty NEED_LIBUTIL)
      IF(NEED_LIBUTIL)
        SET(OPENPTY_LIBRARY util)
      ELSE(NEED_LIBUTIL)
        MESSAGE (SEND_ERROR "openpty not found!")
      ENDIF(NEED_LIBUTIL)
    ENDIF(NOT OPENPTY_IN_LIBC)
  ENDIF(NOT WIN32 AND NOT ANDROID)

  # required
  FIND_PACKAGE(Proj)
  FIND_PACKAGE(GEOS)
  FIND_PACKAGE(GDAL)
  FIND_PACKAGE(Expat REQUIRED)
  FIND_PACKAGE(Spatialindex REQUIRED)
  IF (WITH_GUI)
    FIND_PACKAGE(Qwt REQUIRED)
  ENDIF (WITH_GUI)
  FIND_PACKAGE(LibZip REQUIRED)

  FIND_PACKAGE(Sqlite3)
  IF (NOT SQLITE3_FOUND)
    MESSAGE (SEND_ERROR "sqlite3 dependency was not found!")
  ENDIF (NOT SQLITE3_FOUND)

  # optional
  IF (WITH_POSTGRESQL)
    FIND_PACKAGE(Postgres) # PostgreSQL provider
  ENDIF (WITH_POSTGRESQL)

  IF (NOT WITH_INTERNAL_MDAL)
    FIND_PACKAGE(MDAL REQUIRED) # MDAL provider
  ENDIF (NOT WITH_INTERNAL_MDAL)

  FIND_PACKAGE(SpatiaLite REQUIRED)

  IF (NOT PROJ_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)
    MESSAGE (SEND_ERROR "Some dependencies were not found! Proj: ${PROJ_FOUND}, Geos: ${GEOS_FOUND}, GDAL: ${GDAL_FOUND}")
  ENDIF (NOT PROJ_FOUND OR NOT GEOS_FOUND OR NOT GDAL_FOUND)

  IF (POSTGRES_FOUND)
    # following variable is used in qgsconfig.h
    SET (HAVE_POSTGRESQL TRUE)
  ENDIF (POSTGRES_FOUND)

  SET (WITH_QTWEBKIT TRUE CACHE BOOL "Enable QtWebkit support")
  IF (WITH_QTWEBKIT)
    ADD_DEFINITIONS(-DWITH_QTWEBKIT)
    MESSAGE(STATUS "Qt WebKit support enabled")
  ELSE (WITH_QTWEBKIT)
    MESSAGE(STATUS "Qt WebKit support DISABLED.")
  ENDIF(WITH_QTWEBKIT)
  #############################################################
  # search for Qt5
  IF (WITH_3D)
    # for 3D support we strictly require Qt >= 5.9
    # (Qt 3D was introduced in 5.7 but it is not stable enough in that branch)
    # Qt 5.8 is missing some classes, https://github.com/qgis/QGIS/pull/5203#discussion_r142319862
    SET(QT_MIN_VERSION 5.9.0)
  ELSE ()
    SET(QT_MIN_VERSION 5.4.0)
  ENDIF()
  FIND_PACKAGE(Qt5Core QUIET)
  FIND_PACKAGE(Qt5Gui REQUIRED)
  FIND_PACKAGE(Qt5Widgets REQUIRED)
  FIND_PACKAGE(Qt5Network REQUIRED)
  FIND_PACKAGE(Qt5Xml REQUIRED)
  FIND_PACKAGE(Qt5Svg REQUIRED)
  FIND_PACKAGE(Qt5Concurrent REQUIRED)
  FIND_PACKAGE(Qt5PrintSupport REQUIRED)
  FIND_PACKAGE(Qt5SerialPort REQUIRED)
  FIND_PACKAGE(Qt5Positioning)
  IF (WITH_QTWEBKIT)
    FIND_PACKAGE(Qt5WebKit REQUIRED)
    FIND_PACKAGE(Qt5WebKitWidgets REQUIRED)
  ENDIF(WITH_QTWEBKIT)
  FIND_PACKAGE(Qt5Test REQUIRED)
  FIND_PACKAGE(Qt5UiTools REQUIRED)
  FIND_PACKAGE(Qt5Sql REQUIRED)
  IF (WITH_3D)
    FIND_PACKAGE(Qt53DCore REQUIRED)
    FIND_PACKAGE(Qt53DRender REQUIRED)
    FIND_PACKAGE(Qt53DInput REQUIRED)
    FIND_PACKAGE(Qt53DLogic REQUIRED)
    FIND_PACKAGE(Qt53DExtras REQUIRED)
    SET(HAVE_3D TRUE)  # used in qgsconfig.h
  ENDIF (WITH_3D)
  IF (APPLE)
    FIND_PACKAGE(Qt5MacExtras REQUIRED)
  ENDIF (APPLE)
  INCLUDE("cmake/modules/ECMQt4To5Porting.cmake")
  MESSAGE(STATUS "Found Qt version: ${Qt5Core_VERSION_STRING}")
  IF (WITH_QUICK)
    FIND_PACKAGE(Qt5Qml REQUIRED)
    FIND_PACKAGE(Qt5Quick REQUIRED)
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Android")
        FIND_PACKAGE(Qt5AndroidExtras)
    ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Android")
        FIND_PACKAGE(QtQmlTools REQUIRED)
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Android")

    # following variable is used in qgsconfig.h
    SET (HAVE_QUICK TRUE)
  ENDIF (WITH_QUICK)

  IF(WITH_QTWEBKIT)
    SET(OPTIONAL_QTWEBKIT ${Qt5WebKitWidgets_LIBRARIES})
  ENDIF(WITH_QTWEBKIT)

  IF (WITH_QTMOBILITY)
    FIND_PACKAGE(QtMobility 1.1.0)
  ENDIF (WITH_QTMOBILITY)

  # search for QScintilla2 (C++ lib)
  IF (WITH_GUI)
    FIND_PACKAGE(QScintilla REQUIRED)
  ENDIF (WITH_GUI)

  # Password helper
  FIND_PACKAGE(QtKeychain REQUIRED)
  # Master password hash and authentication encryption
  FIND_PACKAGE(QCA REQUIRED)
  # Check for runtime dependency of qca-ossl plugin
  # REQUIRED if unit tests are to be run from build directory
  IF(NOT MSVC)
  include(QCAMacros)
  FIND_QCAOSSL_PLUGIN_CPP(ENABLE_TESTS)
  ENDIF(NOT MSVC)

  IF (APPLE)
    # Libtasn1 is for DER-encoded PKI ASN.1 parsing/extracting workarounds
    FIND_PACKAGE(Libtasn1 REQUIRED)
  ENDIF (APPLE)

  IF (SUPPRESS_QT_WARNINGS)
    # Newer versions of UseQt4.cmake include Qt with -isystem automatically
    # This can be used to force this behavior on older systems
    # Can be removed as soon as Travis-CI updates from precise
    INCLUDE_DIRECTORIES(SYSTEM ${QT_INCLUDE_DIR})
  ENDIF (SUPPRESS_QT_WARNINGS)

  # Disable automatic conversion from QString to ASCII 8-bit strings (char *)
  # (Keeps code compatible with Qt/Mac/64bit)
  ADD_DEFINITIONS(-DQT_NO_CAST_TO_ASCII)

  FIND_PROGRAM(QT_LRELEASE_EXECUTABLE
    NAMES lrelease
    PATHS ${QT_BINARY_DIR}
    NO_DEFAULT_PATH
  )
ENDIF(WITH_CORE)

# build our version of astyle
SET (WITH_ASTYLE FALSE CACHE BOOL "If you plan to contribute you should reindent with scripts/prepare-commit.sh (using 'our' astyle)")

# QML
SET(QML_IMPORT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" CACHE PATH "QML directory for QML autocomplete")

#############################################################
# testing
# whether unit tests should be build
SET (ENABLE_TESTS TRUE CACHE BOOL "Build unit tests?")
IF (ENABLE_TESTS)
SET( QT_USE_QTTEST  TRUE )
ADD_DEFINITIONS(-DENABLE_TESTS)
ENABLE_TESTING()
# Adds some testing specific build targets e.g. make Experimental
INCLUDE(Dart)
# Additional test configuration options e.g. max upload size of test report
CONFIGURE_FILE(
  "${CMAKE_SOURCE_DIR}/cmake_templates/CTestCustom.cmake.in"
  "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  IMMEDIATE @ONLY)
# For server side testing we have no X, we can use xfvb as a fake x
# sudo apt-get install xfvb
add_custom_target(check COMMAND xvfb-run --server-args=-screen\ 10\ 1024x768x24 ctest --output-on-failure)
ENDIF(ENABLE_TESTS)

IF (WITH_CORE)
# ModelTest
  SET(ENABLE_MODELTEST FALSE CACHE BOOL "Enable QT ModelTest (not for production)")
ENDIF (WITH_CORE)

#############################################################
# enable use of c++11 features where available
# full c++11 support in clang 3.3+: http://clang.llvm.org/cxx_status.html
# for Mac, this is probably Apple LLVM 4.2 (based on LLVM 3.2svn, in XCode 4.6+)
#   or definitely Apple LLVM 5.0 (based on LLVM 3.3svn, in Xcode 5+):
#   https://gist.github.com/yamaya/2924292

SET(CMAKE_CXX_STANDARD 11)

#############################################################
# enable warnings

IF (PEDANTIC)
  MESSAGE (STATUS "Pedantic compiler settings enabled")
  IF(MSVC)
    SET(_warnings "")
    IF (NOT USING_NMAKE AND NOT USING_NINJA)
      SET(_warnings "${_warnings} /W4" )
    ENDIF (NOT USING_NMAKE AND NOT USING_NINJA)

    # disable warnings
    SET(_warnings "${_warnings} /wd4100 ")  # unused formal parameters
    SET(_warnings "${_warnings} /wd4127 ")  # constant conditional expressions (used in Qt template classes)
    SET(_warnings "${_warnings} /wd4190 ")  # 'identifier' has C-linkage specified, but returns UDT 'identifier2' which is incompatible with C
    SET(_warnings "${_warnings} /wd4231 ")  # nonstandard extension used : 'identifier' before template explicit instantiation (used in Qt template classes)
    SET(_warnings "${_warnings} /wd4244 ")  # conversion from '...' to '...' possible loss of data
    SET(_warnings "${_warnings} /wd4251 ")  # needs to have dll-interface to be used by clients of class (occurs in Qt template classes)
    SET(_warnings "${_warnings} /wd4275 ")  # non dll-interface class '...' used as base for dll-interface class '...'
    SET(_warnings "${_warnings} /wd4456 ")  # declaration of '...' hides previous local declaration
    SET(_warnings "${_warnings} /wd4457 ")  # declaration of '...' hides a function parameter
    SET(_warnings "${_warnings} /wd4458 ")  # declaration of '...' hides class member
    SET(_warnings "${_warnings} /wd4505 ")  # unreferenced local function has been removed (QgsRasterDataProvider::extent)
    SET(_warnings "${_warnings} /wd4510 ")  # default constructor could not be generated (sqlite3_index_info, QMap)
    SET(_warnings "${_warnings} /wd4512 ")  # assignment operator could not be generated (sqlite3_index_info)
    SET(_warnings "${_warnings} /wd4610 ")  # user defined constructor required (sqlite3_index_info)
    SET(_warnings "${_warnings} /wd4706 ")  # assignment within conditional expression (pal)
    SET(_warnings "${_warnings} /wd4714 ")  # function '...' marked as __forceinline not inlined (QString::toLower/toUpper/trimmed)
    SET(_warnings "${_warnings} /wd4800 ")  # 'int' : forcing value to bool 'true' or 'false' (performance warning)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_warnings}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_warnings}")
  ELSE (MSVC)
    # add warnings via flags (not as definitions as on Mac -Wall can not be overridden per language )
    SET(_warnings "-Wall -Wextra -Wno-long-long -Wformat-security -Wno-strict-aliasing")
    SET(WERROR FALSE CACHE BOOL "Treat build warnings as errors.")
    IF (WERROR)
      SET(_warnings "${_warnings} -Werror")
    ENDIF(WERROR)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_warnings}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_warnings}")

    # Qt produces lots of warnings with strict aliasing (as of Qt 4.4.0 & GCC 4.3)
    # There are redundant declarations in Qt and GDAL
    # ADD_DEFINITIONS( -fstrict-aliasing -Wstrict-aliasing=1 -Wredundant-decls )

    IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -Wno-overloaded-virtual -Wimplicit-fallthrough")
    ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

    # add any extra CXXFLAGS flags set by user. can be -D CXX_EXTRA_FLAGS or environment variable
    # command line -D option overrides environment variable
    # e.g. useful for suppressing transient upstream warnings in dependencies, like Qt
    SET(CXX_EXTRA_FLAGS "" CACHE STRING "Additional appended CXXFLAGS")
    IF ("${CXX_EXTRA_FLAGS}" STREQUAL "" AND DEFINED $ENV{CXX_EXTRA_FLAGS})
      SET(CXX_EXTRA_FLAGS "$ENV{CXX_EXTRA_FLAGS}")
    ENDIF ("${CXX_EXTRA_FLAGS}" STREQUAL "" AND DEFINED $ENV{CXX_EXTRA_FLAGS})
    IF (NOT "${CXX_EXTRA_FLAGS}" STREQUAL "")
      MESSAGE (STATUS "Appending CXX_EXTRA_FLAGS")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_EXTRA_FLAGS}")
    ENDIF (NOT "${CXX_EXTRA_FLAGS}" STREQUAL "")
  ENDIF (MSVC)

ENDIF (PEDANTIC)

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Qunused-arguments")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Qunused-arguments")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Qunused-arguments")
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
  # spatialite crashes on ppc - see bugs.debian.org/603986
  ADD_DEFINITIONS( -fno-strict-aliasing )
ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")

IF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  MESSAGE (STATUS "Debug output enabled")
  SET(QGISDEBUG TRUE)
ELSE (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
  SET(QGISDEBUG FALSE)
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  # disable macros that offend std::numeric_limits<T>::min()/max()
  ADD_DEFINITIONS(-DNOMINMAX)
ENDIF(MSVC)

IF(ENABLE_COVERAGE)
  INCLUDE("cmake/modules/coverage/CodeCoverage.cmake")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage")
  SETUP_TARGET_FOR_COVERAGE(qgis_coverage ctest coverage)
ENDIF(ENABLE_COVERAGE)

#############################################################
# platform specific stuff
IF (WITH_CORE)
  IF (WIN32)
    SET (DEFAULT_LIB_SUBDIR     lib)
    SET (DEFAULT_LIBEXEC_SUBDIR .)
    SET (DEFAULT_DATA_SUBDIR    .)
    SET (DEFAULT_PLUGIN_SUBDIR  plugins)
    SET (DEFAULT_INCLUDE_SUBDIR include)
    SET (DEFAULT_QML_SUBDIR     qml)

    SET (DEFAULT_SERVER_MODULE_SUBDIR server)

    IF (MSVC)
      SET (DEFAULT_BIN_SUBDIR bin)
      SET (DEFAULT_CGIBIN_SUBDIR bin)
      # put all the build products into a single directory
      # under build (doesn't affect install target) to make for
      # easier debugging.

      # Turn on defines for non standard maths stuff
      ADD_DEFINITIONS(-D_USE_MATH_DEFINES)

      # Turn off deprecation warnings
      ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
      ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)

      IF (INSTALL_DEPS)
        INSTALL(DIRECTORY ${INSTALL_DEPS} DESTINATION .)
      ENDIF (INSTALL_DEPS)
    ELSE(MSVC)
      SET (DEFAULT_BIN_SUBDIR .)
      SET (DEFAULT_CGIBIN_SUBDIR .)
    ENDIF(MSVC)
  ELSE (WIN32)

    IF (APPLE)
      IF (POLICY CMP0042) # in CMake 3.0.0+
        SET (CMAKE_MACOSX_RPATH OFF) # otherwise ON by default
      ENDIF (POLICY CMP0042)
      IF (POLICY CMP0068) # in CMake 3.9.0+
        cmake_policy(SET CMP0068 NEW)
      ENDIF (POLICY CMP0068)
      # for Mac OS X, everything is put inside an application bundle
      # save the root install prefix for the app later
      SET (QGIS_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
      SET (QGIS_MACAPP_PREFIX ${CMAKE_INSTALL_PREFIX}/${QGIS_APP_NAME}.app/Contents)
      # common prefix for components, let cmake handle it
      SET (CMAKE_INSTALL_PREFIX ${QGIS_MACAPP_PREFIX}/MacOS)
      # 4 bundling levels, each includes previous
      #   0  nothing
      #   1  Qt frameworks
      #   2  non-system libraries, "standard"
      #   3  non-system frameworks, "standalone"
      SET (QGIS_MACAPP_BUNDLE 1 CACHE STRING "What to bundle into app package")
      SET (QGIS_MACAPP_BUNDLE_USER "" CACHE STRING "Path to user bundling script")
      SET (QGIS_MACAPP_INSTALL_DEV FALSE CACHE BOOL "Install developer frameworks")
      SET (QGIS_MACAPP_DEV_PREFIX "/Library/Frameworks" CACHE STRING "Path to install developer frameworks")
      # if we have @loader_path, >=OSX 10.5 (darwin 9+)
      IF (CMAKE_SYSTEM_VERSION VERSION_GREATER 9.0.0 OR CMAKE_SYSTEM_VERSION VERSION_EQUAL 9.0.0)
          SET (OSX_HAVE_LOADERPATH 1)
      ELSE ()
          SET (OSX_HAVE_LOADERPATH 0)
      ENDIF ()

      SET (DEFAULT_BIN_SUBDIR      bin)
      SET (QGIS_BIN_SUBDIR_REV     ..)
      SET (DEFAULT_CGIBIN_SUBDIR   fcgi-bin)
      SET (QGIS_CGIBIN_SUBDIR_REV  ..)
      SET (DEFAULT_LIB_SUBDIR      lib)
      SET (QGIS_LIB_SUBDIR_REV     ..)
      SET (QGIS_FW_SUBDIR          ../Frameworks)
      SET (QGIS_FW_SUBDIR_REV      ../MacOS)
      SET (DEFAULT_DATA_SUBDIR     ../Resources)
      SET (QGIS_DATA_SUBDIR_REV    ../MacOS)
      SET (DEFAULT_LIBEXEC_SUBDIR  lib/qgis)
      SET (QGIS_LIBEXEC_SUBDIR_REV ../..)
      SET (DEFAULT_PLUGIN_SUBDIR   ../PlugIns/qgis)
      SET (QGIS_PLUGIN_SUBDIR_REV  ../../MacOS)
      SET (DEFAULT_INCLUDE_SUBDIR  include/qgis)
      SET (DEFAULT_QML_SUBDIR     qml)

      # Set server moodules path to DEFAULT_LIBEXEC_SUBDIR+'/server'
      SET (DEFAULT_SERVER_MODULE_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR}/server)

      # path for framework references when running from build directory
      # changed later to reference in-app resources upon install
      SET (CMAKE_INSTALL_NAME_DIR ${CMAKE_BINARY_DIR}/output/lib)
      IF (WITH_GLOBE)
        SET (OSG_PLUGINS_PATH "" CACHE PATH "Path to OSG plugins for bundling")
      ENDIF (WITH_GLOBE)
      # recent cmakes force SDKs, recent SDKs don't have user symlinks
      # need to find non-system frameworks
      # cmake bug #0007250 - CMAKE_SHARED_LINKER_FLAGS ignored when creating
      # a framework, so these need to be manually handled with LINK_FLAGS options
      SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -F/Library/Frameworks")
      SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -F/Library/Frameworks")
      SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F/Library/Frameworks")

    ELSE (APPLE)
      # UNIX
      SET (DEFAULT_BIN_SUBDIR     bin)
      SET (DEFAULT_CGIBIN_SUBDIR  bin)
      SET (DEFAULT_LIB_SUBDIR     lib${LIB_SUFFIX})
      SET (DEFAULT_DATA_SUBDIR    share/qgis)
      SET (DEFAULT_LIBEXEC_SUBDIR lib${LIB_SUFFIX}/qgis)
      SET (DEFAULT_PLUGIN_SUBDIR  lib${LIB_SUFFIX}/qgis/plugins)
      SET (DEFAULT_INCLUDE_SUBDIR include/qgis)
      SET (DEFAULT_QML_SUBDIR     qml)

      SET (DEFAULT_SERVER_MODULE_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR}/server)
    ENDIF (APPLE)

  ENDIF (WIN32)

  IF (ANDROID)
    SET (DEFAULT_PLUGIN_SUBDIR  lib)
    SET (DEFAULT_DATA_SUBDIR    files/share)
    string(REPLACE "<CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME>" "" CMAKE_CXX_CREATE_SHARED_MODULE "${CMAKE_CXX_CREATE_SHARED_MODULE}")
  ENDIF (ANDROID)

  #assume we have escaped compiler directives
  #eventually we want to change this to new
  #since we don't need to jump through so many
  #hoops to escape compiler directives then
  IF(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
    IF(NOT "${CMAKE_VERSION}" VERSION_LESS "3.3")
      cmake_policy(SET CMP0063 NEW)
    ENDIF(NOT "${CMAKE_VERSION}" VERSION_LESS "3.3")
    IF(MSVC)
      cmake_policy(SET CMP0020 NEW)
    ENDIF(MSVC)
  ENDIF(COMMAND cmake_policy)

  if("${CMAKE_SYSTEM_NAME}"  MATCHES "Linux")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
  endif() #"${CMAKE_SYSTEM_NAME}"  MATCHES "Linux")

  SET(CMAKE_CXX_VISIBILITY_PRESET hidden)
  INCLUDE(GenerateExportHeader)
ENDIF (WITH_CORE)

#############################################################
# user-changeable settings which can be used to customize
# layout of QGIS installation
# (default values are platform-specific)

SET (QGIS_BIN_SUBDIR     ${DEFAULT_BIN_SUBDIR}     CACHE STRING "Subdirectory where executables will be installed")
SET (QGIS_CGIBIN_SUBDIR  ${DEFAULT_CGIBIN_SUBDIR}  CACHE STRING "Subdirectory where CGI executables will be installed")
SET (QGIS_LIB_SUBDIR     ${DEFAULT_LIB_SUBDIR}     CACHE STRING "Subdirectory where libraries will be installed")
SET (QGIS_LIBEXEC_SUBDIR ${DEFAULT_LIBEXEC_SUBDIR} CACHE STRING "Subdirectory where private executables will be installed")
SET (QGIS_DATA_SUBDIR    ${DEFAULT_DATA_SUBDIR}    CACHE STRING "Subdirectory where QGIS data will be installed")
SET (QGIS_PLUGIN_SUBDIR  ${DEFAULT_PLUGIN_SUBDIR}  CACHE STRING "Subdirectory where plugins will be installed")
SET (QGIS_INCLUDE_SUBDIR ${DEFAULT_INCLUDE_SUBDIR} CACHE STRING "Subdirectory where header files will be installed")
SET (QGIS_QML_SUBDIR     ${DEFAULT_QML_SUBDIR}     CACHE STRING "Subdirectory where qml files/libraries will be installed")

SET (QGIS_SERVER_MODULE_SUBDIR  ${DEFAULT_SERVER_MODULE_SUBDIR}  CACHE STRING "Subdirectory where server modules will be installed")

# mark *_SUBDIR variables as advanced as this is not something
# that an average user would use
MARK_AS_ADVANCED (QGIS_BIN_SUBDIR QGIS_CGIBIN_SUBDIR QGIS_LIB_SUBDIR QGIS_LIBEXEC_SUBDIR QGIS_DATA_SUBDIR QGIS_PLUGIN_SUBDIR QGIS_INCLUDE_SUBDIR)

# full paths for the installation
SET (QGIS_BIN_DIR     ${QGIS_BIN_SUBDIR})
SET (QGIS_CGIBIN_DIR  ${QGIS_CGIBIN_SUBDIR})
SET (QGIS_LIB_DIR     ${QGIS_LIB_SUBDIR})
SET (QGIS_LIBEXEC_DIR ${QGIS_LIBEXEC_SUBDIR})
SET (QGIS_DATA_DIR    ${QGIS_DATA_SUBDIR})
SET (QGIS_PLUGIN_DIR  ${QGIS_PLUGIN_SUBDIR})
SET (QGIS_INCLUDE_DIR ${QGIS_INCLUDE_SUBDIR})
SET (QGIS_QML_DIR     ${QGIS_QML_SUBDIR})

SET (QGIS_SERVER_MODULE_DIR ${QGIS_SERVER_MODULE_SUBDIR})

# set the default locations where the targets (executables, libraries) will land when compiled
# this is to allow running qgis from the source tree without having to actually do a "make install"
SET (QGIS_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_BIN_SUBDIR})
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR})

# write a marker with source directory path into the output's bin directory
# if run from the build directory QGIS will detect it and alter the paths
FILE(WRITE ${QGIS_OUTPUT_DIRECTORY}/${QGIS_BIN_SUBDIR}/qgisbuildpath.txt "${CMAKE_SOURCE_DIR}\n${QGIS_OUTPUT_DIRECTORY}")

# manual page - makes sense only on unix systems
IF (UNIX AND NOT APPLE)
  SET (DEFAULT_MANUAL_SUBDIR  man)
  SET (QGIS_MANUAL_SUBDIR  ${DEFAULT_MANUAL_SUBDIR}  CACHE STRING "Subdirectory where manual files will be installed")
  MARK_AS_ADVANCED (QGIS_MANUAL_SUBDIR)
  SET (QGIS_MANUAL_DIR  ${CMAKE_INSTALL_PREFIX}/${QGIS_MANUAL_SUBDIR})
ENDIF (UNIX AND NOT APPLE)

SET (DISABLE_DEPRECATED FALSE CACHE BOOL "If set to true, it will disable deprecated functionality to prepare for the next generation of QGIS")
IF (DISABLE_DEPRECATED)
  ADD_DEFINITIONS(-DQGIS_DISABLE_DEPRECATED)
ENDIF (DISABLE_DEPRECATED)


#############################################################
# Python build dependency

FIND_PACKAGE(PythonInterp 3 REQUIRED)

#############################################################
# Python bindings

IF (WITH_CORE AND WITH_BINDINGS)

  FIND_PACKAGE(PythonLibrary REQUIRED)

  # python support: check for interpreter, sip, pyqt5
  FIND_PACKAGE(PyQt5 REQUIRED)
  SET(PYQT_SIP_FLAGS ${PYQT5_SIP_FLAGS})
  SET(PYQT_SIP_DIR ${PYQT5_SIP_DIR})
  SEPARATE_ARGUMENTS(PYQT_SIP_FLAGS) # convert space separated values to a list

  FIND_PACKAGE(SIP REQUIRED)
  FIND_PACKAGE(Qsci REQUIRED)
  INCLUDE(PythonMacros)
  INCLUDE(PyQtMacros)
  INCLUDE(SIPMacros)

  SET(SIP_INCLUDES ${PYQT_SIP_DIR} ${CMAKE_SOURCE_DIR}/python)
  SET(SIP_CONCAT_PARTS 4)

  IF (NOT BINDINGS_GLOBAL_INSTALL)
    SET(PYTHON_SITE_PACKAGES_DIR ${QGIS_DATA_DIR}/python)
  ENDIF (NOT BINDINGS_GLOBAL_INSTALL)

  IF (WITH_CUSTOM_WIDGETS)
    SET(PYUIC_WIDGET_PLUGIN_DIRECTORY ${PYQT5_MOD_DIR}/uic/widget-plugins/)
  ENDIF (WITH_CUSTOM_WIDGETS)

ENDIF (WITH_CORE AND WITH_BINDINGS)

#############################################################
# create qgsconfig.h
# installed with app target
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake_templates/qgsconfig.h.in ${CMAKE_BINARY_DIR}/qgsconfig.h)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#############################################################
# create qgsversion.h
INCLUDE(CreateQgsVersion)
CREATE_QGSVERSION()

####################################################
# Added by Jef to prevent python core and gui libs linking to other qgisCore and qgisGui libs
# that may be in the same install prefix
IF (WITH_CORE)
  LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/src/core ${CMAKE_BINARY_DIR}/src/gui)
ENDIF (WITH_CORE)

####################################################
# clang-tidy
SET (WITH_CLANG_TIDY FALSE CACHE BOOL "Use Clang tidy")
MARK_AS_ADVANCED(WITH_CLANG_TIDY)
IF (WITH_CORE)
  IF(WITH_CLANG_TIDY)
    FIND_PROGRAM(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      DOC "Path to clang-tidy executable"
      )
    IF(NOT CLANG_TIDY_EXE)
      MESSAGE(STATUS "clang-tidy not found.")
    ELSE(NOT CLANG_TIDY_EXE)
      MESSAGE(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      SET(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.*,-cppcoreguidelines*,-readability-implicit-bool-cast,-llvm-include-order,-cert-err58-cpp,-modernize-pass-by-value,-google-readability-braces-around-statements,-modernize-use-auto,-modernize-loop-convert,-readability-else-after-return,-readability-braces-around-statements,-google-runtime-references,-readability-named-parameter,-google-default-arguments,-google-readability-todo,-readability-inconsistent-declaration-parameter-name,-cert-flp30-c,-google-readability-casting,-clang-analyzer-security.FloatLoopCounter,-google-runtime-int,-modernize-use-using,-google-explicit-constructor,-google-build-using-namespace,-cert-err34-c,-clang-analyzer-core.CallAndMessage,-google-readability-function-size,-modernize-make-shared,-modernize-use-nullptr,-clang-analyzer-cplusplus.NewDeleteLeaks,-clang-analyzer-core.NonNullParamChecker,performance-unnecessary-copy-initialization,-readability-simplify-boolean-expr,-modernize-raw-string-literal,-performance-unnecessary-copy-initialization")
    ENDIF(NOT CLANG_TIDY_EXE)
  ENDIF(WITH_CLANG_TIDY)
ENDIF (WITH_CORE)

#############################################################
# process subdirs

#create a variable to specify where our test data is
#so that unit tests can use TEST_DATA_DIR to locate
#the test data. See CMakeLists in test dirs for more info
#TEST_DATA_DIR is also used by QgsRenderChecker currently in core
SET (TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/testdata")

ADD_SUBDIRECTORY(doc)

IF (WITH_CORE)
  ADD_SUBDIRECTORY(src)
  ADD_SUBDIRECTORY(images)
  ADD_SUBDIRECTORY(resources)
  ADD_SUBDIRECTORY(i18n)

  IF (WITH_BINDINGS)
    ADD_SUBDIRECTORY(python)
  ENDIF (WITH_BINDINGS)

  IF (APPLE)
    # must be last for install, so install_name_tool can do its work
    ADD_SUBDIRECTORY(mac)

    # allow QGIS to be run directly from build directory and to run unit tests
    EXECUTE_PROCESS(COMMAND /bin/mkdir -p "${QGIS_OUTPUT_DIRECTORY}/lib")
    EXECUTE_PROCESS(
      COMMAND /bin/ln -fs ../../Plugins/qgis/qgisgrass6.framework lib/
      WORKING_DIRECTORY "${QGIS_OUTPUT_DIRECTORY}"
    )
    EXECUTE_PROCESS(
      COMMAND /bin/ln -fs ../../Plugins/qgis/qgisgrass7.framework lib/
      WORKING_DIRECTORY "${QGIS_OUTPUT_DIRECTORY}"
    )
  ENDIF (APPLE)

  # manual page - makes sense only on unix systems
  IF (UNIX AND NOT APPLE)
    INSTALL (FILES qgis.1 DESTINATION ${QGIS_MANUAL_DIR}/man1)
  ENDIF (UNIX AND NOT APPLE)

  INSTALL(FILES cmake/FindQGIS.cmake DESTINATION ${QGIS_DATA_DIR})
ENDIF (WITH_CORE)

IF (WITH_ASTYLE)
  ADD_SUBDIRECTORY(external/astyle)
ENDIF(WITH_ASTYLE)

IF (ENABLE_TESTS)
  ADD_SUBDIRECTORY(tests)
  SET (CTEST_BINARY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output/bin" )
  MESSAGE (STATUS "Ctest Binary Directory set to: ${CTEST_BINARY_DIRECTORY}")
ENDIF (ENABLE_TESTS)

#############################################################
# Post-install commands
IF (WITH_CORE)
  ADD_SUBDIRECTORY(postinstall)
ENDIF (WITH_CORE)

#############################################################
# Uninstall stuff see: http://www.vtk.org/Wiki/CMake_FAQ
IF (WITH_CORE)
  CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_templates/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
ENDIF (WITH_CORE)

#############################################################
# Enable packaging
IF (WITH_CORE)
  # Do not warn about runtime libs when building using VS Express
  IF(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
  ENDIF()
  INCLUDE(InstallRequiredSystemLibraries)

  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "QGIS")
  SET(CPACK_PACKAGE_VENDOR "Open Source Geospatial Foundation")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "QGIS ${COMPLETE_VERSION}")
  IF(WIN32 AND NOT UNIX)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backslashes.
    SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/win_build\\\\sidebar.bmp")
    SET(CPACK_NSIS_INSTALLED_ICON_NAME "\\\\qgis.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} QGIS")
    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\qgis.org")
    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\qgis.org")
    SET(CPACK_NSIS_CONTACT "info@qgis.org")
    SET(CPACK_NSIS_MODIFY_PATH ON)

  #  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "  !include \\\"${CMAKE_SOURCE_DIR}\\\\win_build\\\\extra.nsh\\\"")
  ELSE(WIN32 AND NOT UNIX)
    #SET(CPACK_STRIP_FILES "QGIS")
    #SET(CPACK_SOURCE_STRIP_FILES "")
  ENDIF(WIN32 AND NOT UNIX)
  SET(CPACK_PACKAGE_EXECUTABLES "qgis" "QGIS")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
  INCLUDE(CPack)
ENDIF (WITH_CORE)

IF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  ADD_SUBDIRECTORY(linux)
ENDIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
