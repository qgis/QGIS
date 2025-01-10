########################################################
# Files

set(DM_SRCS
  qgsdamengdatabase.cpp
  qgsdamengconn.cpp
  qgsdamengconnpool.cpp
  qgsdamengdataitems.cpp
  qgsdamengprovider.cpp
  qgsdamengfeatureiterator.cpp
  qgsdamengprojectstorage.cpp
  qgsdamengtransaction.cpp
  qgsdamengtablemodel.cpp
  qgsdamengcolumntypethread.cpp
  qgsdamengexpressioncompiler.cpp
  qgsdamengproviderconnection.cpp
)

if (WITH_GUI)
  #set(DM_SRCS ${DM_SRCS}
  set(DM_GUI_SRCS
    qgsdamengprovidergui.cpp
    qgsdamengdataitemguiprovider.cpp
    qgsdamengsourceselect.cpp
    qgsdamengnewconnection.cpp
    qgsdamengprojectstoragedialog.cpp
  )

endif()

set(DM_HDRS
  qgsdamengexpressioncompiler.h
  qgsdamengproviderconnection.h
)

########################################################
# Build

include_directories(
  ${Qt5Sql_PRIVATE_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src/ui
)


add_library(provider_dameng MODULE ${DM_SRCS} ${DM_GUI_SRCS} ${DM_HDRS})

# require c++17
target_compile_features(provider_dameng PRIVATE cxx_std_17)

# We use private headers from core that need this
target_compile_definitions(provider_dameng PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_include_directories(provider_dameng SYSTEM PRIVATE ${DAMENG_INCLUDE_DIR})

target_link_libraries(provider_dameng
  qgis_core
  ${DAMENG_LIBRARY}
  ${QT_VERSION_BASE}::Sql
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (provider_dameng
    qgis_gui
  )
  add_dependencies(provider_dameng ui)
endif()

# static library
add_library (provider_dameng_a STATIC ${DM_SRCS} ${DM_HDRS})

# require c++17
target_compile_features(provider_dameng_a PRIVATE cxx_std_17)

# We use private headers from core that need this
target_compile_definitions(provider_dameng_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

set_target_properties(provider_dameng_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

target_include_directories(provider_dameng_a SYSTEM PUBLIC
  ${DAMENG_INCLUDE_DIR}
)

target_include_directories(provider_dameng_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/dameng
)

target_link_libraries (provider_dameng_a
  qgis_core
  ${DAMENG_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries (provider_dameng_a
    qgis_gui
  )
  add_dependencies(provider_dameng_a ui)
endif()

#################################################################
# Install

install (TARGETS provider_dameng
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
