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
endif()

########################################################
# Source files
set(AUTH_OAUTH2_SRCS
  qgso2.cpp
  qgsauthoauth2config.cpp
  qgsauthoauth2method.cpp
  qjsonwrapper/Json.cpp
)

set(AUTH_OAUTH2_HDRS
  qgsauthoauth2method.h
)

if(WITH_INTERNAL_O2)
  set(OAUTH2_HDRS ${OAUTH2_HDRS} ${O2_HDRS})
  set(OAUTH2_SRCS ${OAUTH2_SRCS} ${O2_SRCS})
endif()

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

set(_library_suffix_MODULE "")
set(_library_suffix_STATIC "_static")

foreach(_library_type MODULE STATIC)
  set(_library_name "oauth2authmethod${_library_suffix_${_library_type}}")

  add_library(${_library_name} ${_library_type} ${AUTH_OAUTH2_SRCS} ${AUTH_OAUTH2_HDRS})

  # require c++17
  target_compile_features(${_library_name} PRIVATE cxx_std_17)

  if(WITH_INTERNAL_O2)
    target_include_directories(${_library_name} SYSTEM PUBLIC ${O2_INCLUDE_DIR})
  endif()

  target_include_directories(${_library_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(${_library_name} qgis_core)

  target_compile_definitions(${_library_name} PRIVATE "-DQT_NO_FOREACH")

  if(WIN32)
    add_definitions(-DO2_DLL_EXPORT)
  endif()

  install(TARGETS ${_library_name}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endforeach()

