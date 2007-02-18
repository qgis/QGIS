
########################################################
# Files

SET (MSEXPORT_SRCS
     qgsmapserverexport.cpp
     ms_main.cpp
     msexport_wrap.cxx
)

SET (MSEXPORT_MOC_HDRS qgsmapserverexport.h)

SET (MSEXPORT_UIS qgsmapserverexportbase.ui)

# Disable compiler warnings for lex generated sources
SET_SOURCE_FILES_PROPERTIES(
                ${CMAKE_CURRENT_SOURCE_DIR}/msexport_wrap.cxx 
                PROPERTIES COMPILE_FLAGS -w)

########################################################
# Build

QT4_WRAP_UI (MSEXPORT_UIS_H ${MSEXPORT_UIS})

QT4_WRAP_CPP (MSEXPORT_MOC_SRCS ${MSEXPORT_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PYTHON_INCLUDE_PATH}
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/gui
)

ADD_EXECUTABLE (msexport MACOSX_BUNDLE ${MSEXPORT_SRCS} ${MSEXPORT_MOC_SRCS} ${MSEXPORT_UIS_H})

TARGET_LINK_LIBRARIES (msexport
   ${QT_LIBRARIES}
   ${PYTHON_LIBRARIES}
   qgis_core
   qgis_gui
)


########################################################
# Install

INSTALL(TARGETS msexport RUNTIME DESTINATION ${QGIS_BIN_DIR})

FILE (GLOB PYTHON_SRCS *.py)
INSTALL(FILES ${PYTHON_SRCS} DESTINATION ${QGIS_DATA_DIR}/python)
