
########################################################
# Files

SET (HELP_SRCS
     main.cpp
     qgshelpserver.cpp
     qgshelpviewer.cpp
     ../core/qgsapplication.cpp
     ../core/qgslogger.cpp
)

SET (HELP_UIS qgshelpviewerbase.ui)

SET (HELP_MOC_HDRS
     qgshelpserver.h
     qgshelpviewer.h
)

#SET (HELP_RCCS  qgsgps_plugin.qrc)


########################################################
# Build

QT4_WRAP_UI (HELP_UIS_H  ${HELP_UIS})

QT4_WRAP_CPP (HELP_MOC_SRCS  ${HELP_MOC_HDRS})

#QT4_ADD_RESOURCES(HELP_RCC_SRCS ${HELP_RCCS})

ADD_EXECUTABLE (qgis_help MACOSX_BUNDLE ${HELP_SRCS} ${HELP_MOC_SRCS} ${HELP_UIS_H})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/../core
     ${CMAKE_CURRENT_BINARY_DIR}
     ${SQLITE3_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(qgis_help
  ${QT_LIBRARIES}
  ${SQLITE3_LIBRARY}
  qgis_core
)


########################################################
# Install

INSTALL(TARGETS qgis_help RUNTIME DESTINATION ${QGIS_BIN_DIR})
