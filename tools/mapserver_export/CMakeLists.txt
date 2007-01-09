
########################################################
# Files

SET (MSEXPORT_SRCS
     qgsmapserverexport.cpp
     ms_main.cpp
     msexport_wrap.cxx
)

SET (MSEXPORT_MOC_HDRS qgsmapserverexport.h)

SET (MSEXPORT_UIS qgsmapserverexportbase.ui)


########################################################
# Build

QT4_WRAP_UI (MSEXPORT_UIS_H ${MSEXPORT_UIS})

QT4_WRAP_CPP (MSEXPORT_MOC_SRCS ${MSEXPORT_MOC_HDRS} ${MSEXPORT_UIS_H})

INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PYTHON_INCLUDE_PATH}
)

ADD_EXECUTABLE (msexport ${MSEXPORT_SRCS} ${MSEXPORT_MOC_SRCS})

TARGET_LINK_LIBRARIES (msexport
   ${QT_LIBRARIES}
   ${PYTHON_LIBRARIES}
   qgis_core
)


########################################################
# Install

INSTALL(TARGETS msexport RUNTIME DESTINATION ${QGIS_BIN_DIR})

FILE (GLOB PYTHON_SRCS *.py)
INSTALL(FILES ${PYTHON_SRCS} DESTINATION ${QGIS_DATA_DIR}/python)
