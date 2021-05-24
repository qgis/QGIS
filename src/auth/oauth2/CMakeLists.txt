# OAuth 2 Authentication Method plugin

########################################################
# Packages

if(NOT QTKEYCHAIN_FOUND)
  find_package(QtKeychain REQUIRED)
endif()
option(WITH_INTERNAL_O2 "Download and locally include source of o2 library" ON)

if(WITH_INTERNAL_O2)
  set(O2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/o2/src)
  set(O2_INCLUDE_DIR "${O2_SOURCE_DIR}" CACHE INTERNAL "Path to o2 library headers" FORCE)
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
  core/qgso2.cpp
  core/qgsauthoauth2config.cpp
  core/qgsauthoauth2method.cpp
  ${CMAKE_SOURCE_DIR}/external/qjsonwrapper/Json.cpp
)

set(AUTH_OAUTH2_HDRS
  core/qgsauthoauth2config.h
  core/qgsauthoauth2method.h
  core/qgso2.h
  ${CMAKE_SOURCE_DIR}/external/qjsonwrapper/Json.h
)

if(WITH_INTERNAL_O2)
  set(AUTH_OAUTH2_HDRS ${AUTH_OAUTH2_HDRS} ${O2_HDRS})
  set(AUTH_OAUTH2_SRCS ${AUTH_OAUTH2_SRCS} ${O2_SRCS})
endif()

set(AUTH_OAUTH2_RCCS resources/oauth2_resources.qrc)

set(AUTH_OAUTH2_UIS_H "")

if (WITH_GUI)
  set(AUTH_OAUTH2_SRCS ${AUTH_OAUTH2_SRCS}
    gui/qgsauthoauth2edit.cpp
    gui/qgsauthoauth2methodgui.cpp
  )

  set(AUTH_OAUTH2_HDRS ${AUTH_OAUTH2_HDRS}
    gui/qgsauthoauth2edit.h
  )

  set(AUTH_OAUTH2_UIS gui/qgsauthoauth2edit.ui)

  QT5_WRAP_UI(AUTH_OAUTH2_UIS_H ${AUTH_OAUTH2_UIS})
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

  add_library(${_library_name} ${_library_type} ${AUTH_OAUTH2_SRCS} ${AUTH_OAUTH2_HDRS} ${AUTH_OAUTH2_UIS_H} ${AUTH_OAUTH2_RCCS})

  # require c++17
  target_compile_features(${_library_name} PRIVATE cxx_std_17)

  if(WITH_INTERNAL_O2)
    target_include_directories(${_library_name} SYSTEM PUBLIC ${O2_INCLUDE_DIR})
  endif()

  target_include_directories(${_library_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/external/qjsonwrapper
    ${CMAKE_CURRENT_SOURCE_DIR}/core
  )

  target_link_libraries(${_library_name} qgis_core)

  if (WITH_GUI)
    target_include_directories(${_library_name} PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/gui
      ${CMAKE_CURRENT_BINARY_DIR}
    )

    target_link_libraries(${_library_name} qgis_gui)
  endif()

  if(NOT WITH_INTERNAL_O2)
    if(NOT "${O2_LIBRARY}" STREQUAL "")
      # prefer dynamic linking
      target_link_libraries(${_library_name} ${O2_LIBRARY})
    else()
      target_link_libraries(${_library_name} ${O2_LIBRARY_STATIC})
    endif()
  endif()

  if(WIN32)
    add_definitions(-DO2_DLL_EXPORT)
  endif()

  install(TARGETS ${_library_name}
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endforeach()

