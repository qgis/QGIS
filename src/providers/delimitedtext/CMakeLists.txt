
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

add_library(provider_delimitedtext MODULE ${DTEXT_SRCS})

# require c++17
target_compile_features(provider_delimitedtext PRIVATE cxx_std_17)

target_link_libraries(provider_delimitedtext
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (provider_delimitedtext
    qgis_gui
  )
  add_dependencies(provider_delimitedtext ui)
endif()

target_compile_definitions(provider_delimitedtext PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    provider_delimitedtext PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install (TARGETS provider_delimitedtext
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
