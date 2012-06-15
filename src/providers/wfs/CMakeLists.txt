
########################################################
# Files

SET(WFS_SRCS
  qgswfsprovider.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsdata.cpp
  qgswfssourceselect.cpp
  qgswfsutils.cpp
)

SET (WFS_MOC_HDRS
     qgswfsdata.h
     qgswfscapabilities.h
     qgswfsdataitems.h
     qgswfsprovider.h
     qgswfssourceselect.h
)

#IF(NOT MSVC)
#  IF (PEDANTIC)
#    MESSAGE("providers/wfs : -Werror removed for qgswfsprovider.cpp for now - please get rid of any compiler warnings!")
#  ENDIF (PEDANTIC)
#  SET_SOURCE_FILES_PROPERTIES(qgswfsprovider.cpp PROPERTIES COMPILE_FLAGS -Wno-error )
#ENDIF (NOT MSVC)
########################################################
# Build

QT4_WRAP_CPP(WFS_MOC_SRCS ${WFS_MOC_HDRS})

INCLUDE_DIRECTORIES (
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GEOS_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}/geos
  ${EXPAT_INCLUDE_DIR}
)

ADD_LIBRARY (wfsprovider MODULE ${WFS_SRCS} ${WFS_MOC_SRCS})

TARGET_LINK_LIBRARIES (wfsprovider
  ${EXPAT_LIBRARY}
  qgis_core
  qgis_gui
)


########################################################
# Install
                
INSTALL(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
