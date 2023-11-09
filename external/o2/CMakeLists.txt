cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0048) # in CMake >= 3.0.0
    cmake_policy(SET CMP0048 OLD) # keep project() from clearing VERSION variables
endif(POLICY CMP0048)

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

set(VER_MAJOR "1")
set(VER_MINOR "0")
set(VER_PATCH "1")
set(API_VER_PATCH "0")
set(CPACK_PACKAGE_VERSION_MAJOR ${VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VER_PATCH})
set(PROJECT_VERSION ${VER_MAJOR}.${VER_MINOR}.${VER_PATCH})
set(API_VERSION ${VER_MAJOR}.${VER_MINOR}.${API_VER_PATCH})

project(o2)

option(o2_WITH_QT5 "Use Qt5" ON)

set(o2_LIB_SUFFIX "" CACHE STRING "Suffix for install 'lib' directory, e.g. 64 for lib64")

option(o2_SHOW_TRACE "Show debugging messages" OFF)
if(NOT o2_SHOW_TRACE)
  add_definitions(-DQT_NO_DEBUG_OUTPUT=1)
endif()

option(o2_WITH_TWITTER "Authenticate with Twitter" OFF)
option(o2_WITH_DROPBOX "Authenticate with Dropbox" OFF)
option(o2_WITH_GOOGLE "Authenticate with Google" OFF)
option(o2_WITH_FACEBOOK "Authenticate with Facebook" OFF)
option(o2_WITH_SKYDRIVE "Authenticate with SkyDrive" OFF)
option(o2_WITH_FLICKR "Authenticate with Flickr" OFF)
option(o2_WITH_HUBIC "Authenticate with Hubic" OFF)
option(o2_WITH_SPOTIFY "Authenticate with Spotify" OFF)
option(o2_WITH_SURVEYMONKEY "Authenticate with SurveyMonkey" OFF)
option(o2_WITH_KEYCHAIN "keychain store" ON)

option(o2_WITH_OAUTH1 "Include OAuth1 authentication" OFF)
if(o2_WITH_TWITTER OR o2_WITH_DROPBOX OR o2_WITH_FLICKR)
  set(o2_WITH_OAUTH1 ON)
endif()

option(o2_BUILD_EXAMPLES "Build examples" OFF)

if(WIN32)
  add_definitions(-DO2_DLL_EXPORT)
endif()

add_subdirectory(src)

if(o2_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(o2_BUILD_EXAMPLES)
