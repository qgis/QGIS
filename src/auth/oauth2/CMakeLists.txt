# OAuth 2 Authentication Method plugin

########################################################
# Packages
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

IF(NOT QTKEYCHAIN_FOUND)
  FIND_PACKAGE(QtKeychain REQUIRED)
ENDIF()
OPTION(WITH_INTERNAL_O2 "Download and locally include source of o2 library" ON)

IF(WITH_INTERNAL_O2)
  SET(O2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/o2/src)
  SET(O2_INCLUDE_DIR "${O2_SOURCE_DIR}"  CACHE INTERNAL "Path to o2 library headers" FORCE)
  SET(O2_LIBRARY "" CACHE INTERNAL "Path to o2 built shared library" FORCE)
  SET(O2_LIBRARY_STATIC "" CACHE INTERNAL "Path to o2 built static library" FORCE)
  SET(O2_FOUND TRUE CACHE INTERNAL "Whether O2 has been found" FORCE)
ELSE()
  FIND_PACKAGE(O2 REQUIRED)
ENDIF()

########################################################
# Source files

SET(OAUTH2_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qgsauthoauth2edit.cpp
  qjsonwrapper/Json.cpp
)
IF(WITH_INTERNAL_O2)
  SET(O2_SRCS
    ${O2_SOURCE_DIR}/o0baseauth.cpp
    ${O2_SOURCE_DIR}/o0keychainstore.cpp
    ${O2_SOURCE_DIR}/o0settingsstore.cpp
    ${O2_SOURCE_DIR}/o2.cpp
    ${O2_SOURCE_DIR}/o2reply.cpp
    ${O2_SOURCE_DIR}/o2replyserver.cpp
    ${O2_SOURCE_DIR}/o2requestor.cpp
    ${O2_SOURCE_DIR}/o2simplecrypt.cpp
  )
  SET(OAUTH2_SRCS ${OAUTH2_SRCS} ${O2_SRCS})
ENDIF()

SET(OAUTH2_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
  qjsonwrapper/Json.h
)
IF(WITH_INTERNAL_O2)
  SET(O2_HDRS
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
  SET(OAUTH2_HDRS ${OAUTH2_HDRS} ${O2_HDRS})
ENDIF()

SET(OAUTH2_MOC_HDRS
  qgso2.h
  qgsauthoauth2config.h
  qgsauthoauth2method.h
  qgsauthoauth2edit.h
)
IF(WITH_INTERNAL_O2)
  SET(O2_MOC_HDRS
    ${O2_INCLUDE_DIR}/o0abstractstore.h
    ${O2_INCLUDE_DIR}/o0baseauth.h
    ${O2_INCLUDE_DIR}/o0keychainstore.h
    ${O2_INCLUDE_DIR}/o0settingsstore.h
    ${O2_INCLUDE_DIR}/o2.h
    ${O2_INCLUDE_DIR}/o2reply.h
    ${O2_INCLUDE_DIR}/o2replyserver.h
    ${O2_INCLUDE_DIR}/o2requestor.h
  )
  SET(OAUTH2_MOC_HDRS ${OAUTH2_MOC_HDRS} ${O2_MOC_HDRS})
ENDIF()

SET(OAUTH2_UIS qgsauthoauth2edit.ui)

SET(OAUTH2_RCCS oauth2_resources.qrc)

# Common includes
INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${O2_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/auth/oauth2
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/ui/auth
  ${CMAKE_BINARY_DIR}/src/auth/oauth2
)

############################################################
# Generate files
IF(WITH_INTERNAL_O2 AND CMAKE_GENERATOR MATCHES "Ninja")
  # handle `cmake --help-policy CMP0058`
  #  "Ninja requires custom command byproducts to be explicit,"
  #   which the downloaded o2 source files are not. Add phony command target...
  ADD_CUSTOM_COMMAND(
    OUTPUT ${O2_HDRS}
    COMMAND
  )
ENDIF()
QT5_WRAP_UI(OAUTH2_UIS_H ${OAUTH2_UIS})

QT5_WRAP_CPP(OAUTH2_MOC_SRCS ${OAUTH2_MOC_HDRS})

QT5_ADD_RESOURCES(OAUTH2_RCC_SRCS ${OAUTH2_RCCS})
############################################################
# Set up plugin targets

ADD_LIBRARY (oauth2authmethod MODULE
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_MOC_SRCS}
  ${OAUTH2_RCC_SRCS}
  ${OAUTH2_UIS_H}
)

# For unit testing
ADD_LIBRARY (oauth2authmethod_static STATIC
  ${OAUTH2_SRCS}
  ${OAUTH2_HDRS}
  ${OAUTH2_MOC_SRCS}
  ${OAUTH2_RCC_SRCS}
  ${OAUTH2_UIS_H}
)

# Common linked libs
SET(OAUTH2_TARGET_LIBS ${QTKEYCHAIN_LIBRARY})

IF(NOT WITH_INTERNAL_O2)
  IF(NOT "${O2_LIBRARY}" STREQUAL "")
    # prefer dynamic linking
    SET(OAUTH2_TARGET_LIBS ${OAUTH2_TARGET_LIBS} ${O2_LIBRARY})
  ELSE()
    SET(OAUTH2_TARGET_LIBS ${OAUTH2_TARGET_LIBS} ${O2_LIBRARY_STATIC})
  ENDIF()
  #message(STATUS "OAUTH2_TARGET_LIBS: ${OAUTH2_TARGET_LIBS}")
ENDIF()

SET(OAUTH2_TARGET_LIBS
  qgis_core
  qgis_gui
  ${OAUTH2_TARGET_LIBS}
)

TARGET_LINK_LIBRARIES (oauth2authmethod
  ${OAUTH2_TARGET_LIBS}
)
TARGET_LINK_LIBRARIES (oauth2authmethod_static
  ${OAUTH2_TARGET_LIBS}
)

if(WIN32)
  add_definitions(-DO2_DLL_EXPORT)
ENDIF()

########################################################
# Install

# don't install static variant, as it's only for unit testing from build directory
INSTALL(TARGETS oauth2authmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
)
