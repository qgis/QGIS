# for installing headers

include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
)

set(QGIS_TEST_HDRS
  qgstest.h
)

install(FILES ${QGIS_TEST_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
