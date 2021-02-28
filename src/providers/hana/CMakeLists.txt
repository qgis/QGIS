########################################################
# Files

set (HANA_SRCS
  qgshanacolumntypethread.cpp
  qgshanaconnection.cpp
  qgshanaconnectionpool.cpp
  qgshanaconnectionstringbuilder.cpp
  qgshanadriver.cpp
  qgshanaexpressioncompiler.cpp
  qgshanafeatureiterator.cpp
  qgshanaprimarykeys.cpp
  qgshanaprovider.cpp
  qgshanaproviderconnection.cpp
  qgshanaresultset.cpp
  qgshanasettings.cpp
  qgshanatablemodel.cpp
  qgshanacrsutils.cpp
  qgshanautils.cpp
)

if (WITH_GUI)
  set(HANA_SRCS ${HANA_SRCS}
    qgshanadataitems.cpp
    qgshanadataitemguiprovider.cpp
    qgshananewconnection.cpp
    qgshanaprovidergui.cpp
    qgshanasourceselect.cpp
  )
endif()

set(HANA_HDRS
  qgshanacolumntypethread.h
  qgshanaconnection.h
  qgshanaconnectionpool.h
  qgshanaconnectionstringbuilder.h
  qgshanadriver.h
  qgshanaexception.h
  qgshanaexpressioncompiler.h
  qgshanafeatureiterator.h
  qgshanaprimarykeys.h
  qgshanaprovider.h
  qgshanaproviderconnection.h
  qgshanaresultset.h
  qgshanasettings.h
  qgshanatablemodel.h
  qgshanacrsutils.h
  qgshanautils.h
)

if (WITH_GUI)
  set(HANA_HDRS ${HANA_HDRS}
    qgshanadataitems.h
    qgshanadataitemguiprovider.h
    qgshananewconnection.h
    qgshanasourceselect.h
  )
endif()

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/external/odbccpp/src

  ${CMAKE_BINARY_DIR}/src/ui
)

include_directories (SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${ODBC_INCLUDE_DIRS}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

add_library(hanaprovider MODULE ${HANA_SRCS} ${HANA_HDRS})
add_library(hanaprovider_a STATIC ${HANA_SRCS} ${HANA_HDRS})

# require c++17
target_compile_features(hanaprovider PRIVATE cxx_std_17)
target_compile_features(hanaprovider_a PRIVATE cxx_std_17)

target_compile_definitions(hanaprovider PUBLIC ODBC_STATIC)
target_compile_definitions(hanaprovider_a PUBLIC ODBC_STATIC)

target_link_libraries(hanaprovider
  qgis_core
  odbccpp_static
  ${ODBC_LIBRARIES}
)

target_link_libraries(hanaprovider_a
  qgis_core
  odbccpp_static
  ${ODBC_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (hanaprovider_a
    qgis_gui
  )

  target_link_libraries (hanaprovider
    qgis_gui
  )
endif()

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    hanaprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install (TARGETS hanaprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
