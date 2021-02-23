
########################################################
# Files

set (DTEXT_SRCS
  qgsdelimitedtextfeatureiterator.cpp
  qgsdelimitedtextprovider.cpp
  qgsdelimitedtextfile.cpp
)

if (WITH_GUI)
  set(DTEXT_SRCS ${DTEXT_SRCS}
    qgsdelimitedtextprovidergui.cpp
    qgsdelimitedtextsourceselect.cpp
  )
endif()

########################################################
# Build

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library(delimitedtextprovider MODULE ${DTEXT_SRCS})

# require c++17
target_compile_features(delimitedtextprovider PRIVATE cxx_std_17)

target_link_libraries(delimitedtextprovider
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (delimitedtextprovider
    qgis_gui
  )
  add_dependencies(delimitedtextprovider ui)
endif()

target_compile_definitions(delimitedtextprovider PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    delimitedtextprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install (TARGETS delimitedtextprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
