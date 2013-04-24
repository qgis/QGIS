SET(OWS_SRCS 
  qgsowsprovider.cpp 
  qgsowsdataitems.cpp 
)
SET(OWS_MOC_HDRS  
  qgsowsprovider.h 
  qgsowsdataitems.h
)

INCLUDE_DIRECTORIES (
  ../../core
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

QT4_WRAP_CPP(OWS_MOC_SRCS ${OWS_MOC_HDRS})
ADD_LIBRARY (owsprovider MODULE ${OWS_SRCS} ${OWS_MOC_SRCS})

TARGET_LINK_LIBRARIES (owsprovider
  qgis_core
  qgis_gui
)

INSTALL(TARGETS owsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
