########################################################
# Files

set(VRP_SRCS
    qgsvirtualrasterprovider.cpp
)
set(VRP_HDRS
    qgsvirtualrasterprovider.h
)

########################################################
# Build

add_library (provider_virtualraster MODULE ${VRP_SRCS} ${VRP_HDRS})

# require c++17
target_compile_features(provider_virtualraster PRIVATE cxx_std_17)

target_link_libraries (provider_virtualraster
  qgis_core
  qgis_analysis
)

target_compile_definitions(provider_virtualraster PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    provider_virtualraster PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install(TARGETS provider_virtualraster
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
