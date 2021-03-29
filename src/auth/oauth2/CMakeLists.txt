# OAuth 2 Authentication Method plugin

########################################################
# Packages
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

if(NOT QTKEYCHAIN_FOUND)
  find_package(QtKeychain REQUIRED)
endif()
option(WITH_INTERNAL_O2 "Download and locally include source of o2 library" ON)

if(WITH_INTERNAL_O2)
  set(O2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/o2/src)
  set(O2_INCLUDE_DIR "${O2_SOURCE_DIR}"  CACHE INTERNAL "Path to o2 library headers" FORCE)
  set(O2_LIBRARY "" CACHE INTERNAL "Path to o2 built shared library" FORCE)
  set(O2_LIBRARY_STATIC "" CACHE INTERNAL "Path to o2 built static library" FORCE)
  set(O2_FOUND TRUE CACHE INTERNAL "Whether O2 has been found" FORCE)
else()
  find_package(O2 REQUIRED)
endif()

########################################################
# Source files

set(OAUTH2_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qgsauthoauth2edit.cpp
  qjsonwrapper/Json.cpp
)
if(WITH_INTERNAL_O2)
  set(O2_SRCS
    ${O2_SOURCE_DIR}/o0baseauth.cpp
    ${O2_SOURCE_DIR}/o0keychainstore.cpp
    ${O2_SOURCE_DIR}/o0settingsstore.cpp
    ${O2_SOURCE_DIR}/o2.cpp
    ${O2_SOURCE_DIR}/o2reply.cpp
    ${O2_SOURCE_DIR}/o2replyserver.cpp
    ${O2_SOURCE_DIR}/o2requestor.cpp
    ${O2_SOURCE_DIR}/o2simplecrypt.cpp
  )
  set(OAUTH2_SRCS ${OAUTH2_SRCS} ${O2_SRCS})
endif()

set(OAUTH2_HDRS
  qgsauthoauth2config.h
  qgsauthoauth2edit.h
  qgsauthoauth2method.h
  qgso2.h
  qjsonwrapper/Json.h
)
if(WITH_INTERNAL_O2)
  set(O2_HDRS
    ${O2_INCLUDE_DIR}/o0abstractstore.h
    ${O2_INCLUDE_DIR}/o0baseauth.h
    ${O2_INCLUDE_DIR}/o0export.h
    ${O2_INCLUDE_DIR}/o0globals.h
    ${O2_INCLUDE_DIR}/o0keychainstore.h
    ${O2_INCLUDE_DIR}/o0requestparameter.h
    ${O2_INCLUDE_DIR}/o0settingsstore.h
    ${O2_INCLUDE_DIR}/o0simplecrypt.h
    ${O2_INCLUDE_DIR}/o2.h
    ${O2_INCLUDE_DIR}/o2reply.h
    ${O2_INCLUDE_DIR}/o2replyserver.h
    ${O2_INCLUDE_DIR}/o2requestor.h
  )
  set(OAUTH2_HDRS ${OAUTH2_HDRS} ${O2_HDRS})
endif()

set(OAUTH2_UIS qgsauthoauth2edit.ui)

set(OAUTH2_RCCS oauth2_resources.qrc)

# Common includes
include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
include_directories (SYSTEM
  ${O2_INCLUDE_DIR}
)
include_directories (
  ${CMAKE_SOURCE_DIR}/src/auth/oauth2
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/ui/auth
  ${CMAKE_BINARY_DIR}/src/auth/oauth2
)

############################################################
# Generate files
if(WITH_INTERNAL_O2 AND CMAKE_GENERATOR MATCHES "Ninja")
  # handle `cmake --help-policy CMP0058`
  #  "Ninja requires custom command byproducts to be explicit,"
  #   which the downloaded o2 source files are not. Add phony command target...
  add_custom_command(
    OUTPUT ${O2_HDRS}
    COMMAND
  )
endif()
QT5_WRAP_UI(OAUTH2_UIS_H ${OAUTH2_UIS})

############################################################
# Set up plugin targets

add_library (oauth2authmethod MODULE
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_RCCS}
  ${OAUTH2_UIS_H}
)

# For unit testing
add_library (oauth2authmethod_static STATIC
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_RCCS}
  ${OAUTH2_UIS_H}
)

# require c++17
target_compile_features(oauth2authmethod PRIVATE cxx_std_17)
target_compile_features(oauth2authmethod_static PRIVATE cxx_std_17)

# Common linked libs
set(OAUTH2_TARGET_LIBS ${QTKEYCHAIN_LIBRARY})

if(NOT WITH_INTERNAL_O2)
  if(NOT "${O2_LIBRARY}" STREQUAL "")
    # prefer dynamic linking
    set(OAUTH2_TARGET_LIBS ${OAUTH2_TARGET_LIBS} ${O2_LIBRARY})
  else()
    set(OAUTH2_TARGET_LIBS ${OAUTH2_TARGET_LIBS} ${O2_LIBRARY_STATIC})
  endif()
  #message(STATUS "OAUTH2_TARGET_LIBS: ${OAUTH2_TARGET_LIBS}")
endif()

set(OAUTH2_TARGET_LIBS
  qgis_core
  qgis_gui
  ${OAUTH2_TARGET_LIBS}
)

target_link_libraries (oauth2authmethod
  ${OAUTH2_TARGET_LIBS}
)
target_link_libraries (oauth2authmethod_static
  ${OAUTH2_TARGET_LIBS}
)

if(WIN32)
  add_definitions(-DO2_DLL_EXPORT)
endif()

########################################################
# Install

# don't install static variant, as it's only for unit testing from build directory
install(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
