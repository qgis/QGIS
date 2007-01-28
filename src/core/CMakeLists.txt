

#############################################################
# sources

SET(QGIS_CORE_SRCS
qgis.cpp
qgsapplication.cpp
qgsbookmarkitem.cpp
qgsclipper.cpp
qgscolortable.cpp
qgscontexthelp.cpp
qgscustomsymbol.cpp
qgsdatamanager.cpp
qgsdatasource.cpp
qgsdistancearea.cpp
qgsexception.cpp
qgsfeatureattribute.cpp
qgsfeature.cpp
qgsfield.cpp
qgsgeometry.cpp
qgsgeometryvertexindex.cpp
qgshttptransaction.cpp
qgslabelattributes.cpp
qgsline.cpp
qgslinesymbol.cpp
qgslogger.cpp
qgsmaptopixel.cpp
qgsmarkercatalogue.cpp
qgsmarkersymbol.cpp
qgsnumericsortlistviewitem.cpp
qgspluginitem.cpp
qgspluginregistry.cpp
qgspoint.cpp
qgspolygonsymbol.cpp
qgsprojectproperty.cpp
qgsprovidercountcalcevent.cpp
qgsproviderextentcalcevent.cpp
qgsprovidermetadata.cpp
qgsproviderregistry.cpp
qgsrangerenderitem.cpp
qgsrasterdataprovider.cpp
qgsrect.cpp
qgsrenderer.cpp
qgsrenderitem.cpp
qgsscalecalculator.cpp
qgssearchstring.cpp
qgssearchtreenode.cpp
qgssymbol.cpp
qgssymbologyutils.cpp
qgsvectordataprovider.cpp

../raster/qgsrasterlayer.cpp

)

ADD_FLEX_FILES(QGIS_CORE_SRCS qgssearchstringlexer.ll)

ADD_BISON_FILES(QGIS_CORE_SRCS qgssearchstringparser.yy)

SET(QGIS_CORE_MOC_HDRS
qgscontexthelp.h
qgsdataprovider.h
qgshttptransaction.h
qgsrasterdataprovider.h

../raster/qgsrasterlayer.h
)

QT4_WRAP_CPP(QGIS_CORE_MOC_SRCS ${QGIS_CORE_MOC_HDRS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ../raster ../legend ../gui
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${QT_QTSVG_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCORE_EXPORT \"-DCORE_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

#############################################################
# qgis_core library

ADD_LIBRARY(qgis_core SHARED ${QGIS_CORE_SRCS} ${QGIS_CORE_MOC_SRCS} ${INDEX_SRC})

# make sure to create qgssvnversion.h before compiling
ADD_DEPENDENCIES(qgis_core svnversionsounds ui)

# because of htonl
IF (WIN32)
  SET(PLATFORM_LIBRARIES wsock32)
ENDIF (WIN32)

TARGET_LINK_LIBRARIES(qgis_core
  ${QT_LIBRARIES} ${QT_QTSVG_LIBRARY}
  ${PROJ_LIBRARY}
  ${SQLITE3_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${PLATFORM_LIBRARIES}
)

INSTALL(TARGETS qgis_core
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION lib)
