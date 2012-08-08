

#############################################################
# sources

SET(QGIS_CORE_SRCS

  gps/qextserialport/qextserialport.cpp
  gps/qextserialport/qextserialenumerator.cpp

  gps/qgsgpsconnection.cpp
  gps/qgsgpsconnectionregistry.cpp
  gps/qgsnmeaconnection.cpp
  gps/qgsgpsdconnection.cpp
  gps/qgsgpsdetector.cpp
  gps/parse.c
  gps/sentence.c
  gps/info.c
  gps/time.c
  gps/gmath.c
  gps/tok.c
  gps/context.c

  symbology-ng/qgssymbolv2.cpp
  symbology-ng/qgssymbollayerv2.cpp
  symbology-ng/qgssymbollayerv2registry.cpp
  symbology-ng/qgssymbollayerv2utils.cpp
  symbology-ng/qgslinesymbollayerv2.cpp
  symbology-ng/qgsmarkersymbollayerv2.cpp
  symbology-ng/qgsfillsymbollayerv2.cpp
  symbology-ng/qgsrendererv2.cpp
  symbology-ng/qgsrendererv2registry.cpp
  symbology-ng/qgssinglesymbolrendererv2.cpp
  symbology-ng/qgscategorizedsymbolrendererv2.cpp
  symbology-ng/qgsgraduatedsymbolrendererv2.cpp
  symbology-ng/qgsrulebasedrendererv2.cpp
  symbology-ng/qgsvectorcolorrampv2.cpp
  symbology-ng/qgsstylev2.cpp
  symbology-ng/qgssymbologyv2conversion.cpp
  symbology-ng/qgssvgcache.cpp
  symbology-ng/qgsellipsesymbollayerv2.cpp
  symbology-ng/qgspointdisplacementrenderer.cpp
  symbology-ng/qgsvectorfieldsymbollayer.cpp

  qgis.cpp
  qgsapplication.cpp
  qgsattributeaction.cpp
  qgsbrowsermodel.cpp
  qgscentralpointpositionmanager.cpp
  qgsclipper.cpp
  qgscontexthelp.cpp
  qgscoordinatetransform.cpp
  qgscrscache.cpp
  qgsdatasourceuri.cpp
  qgsdataitem.cpp
  qgsdbfilterproxymodel.cpp
  qgsdiagram.cpp
  qgsdiagramrendererv2.cpp
  qgsdistancearea.cpp
  qgsexpression.cpp
  qgsfeature.cpp
  qgsfield.cpp
  qgsgeometry.cpp
  qgsgeometryvalidator.cpp
  qgshttptransaction.cpp
  qgslabel.cpp
  qgslabelattributes.cpp
  qgslabelsearchtree.cpp
  qgslogger.cpp
  qgsmaplayer.cpp
  qgsmaplayerregistry.cpp
  qgsmaprenderer.cpp
  qgsmaptopixel.cpp
  qgsmessageoutput.cpp
  qgsmimedatautils.cpp
  qgsmessagelog.cpp
  qgscredentials.cpp
  qgsoverlayobject.cpp
  qgsowsconnection.cpp
  qgspalgeometry.cpp
  qgspallabeling.cpp
  qgspalobjectpositionmanager.cpp
  qgspluginlayer.cpp
  qgspluginlayerregistry.cpp
  qgspoint.cpp
  qgsproject.cpp
  qgsprojectfiletransform.cpp
  qgsprojectversion.cpp
  qgsprojectproperty.cpp
  qgsprovidercountcalcevent.cpp
  qgsproviderextentcalcevent.cpp
  qgsprovidermetadata.cpp
  qgsproviderregistry.cpp
  qgspythonrunner.cpp
  qgsrasterprojector.cpp
  qgsrasterdataprovider.cpp
  qgsrendercontext.cpp
  qgsrectangle.cpp
  qgsrunprocess.cpp
  qgsscalecalculator.cpp
  qgssnapper.cpp
  qgscoordinatereferencesystem.cpp
  qgstolerance.cpp
  qgsvectordataprovider.cpp
  qgsvectorfilewriter.cpp
  qgsvectorlayer.cpp
  qgsvectorlayerimport.cpp
  qgsvectorlayerjoinbuffer.cpp
  qgsvectorlayerundocommand.cpp
  qgsvectoroverlay.cpp

  qgsnetworkaccessmanager.cpp

  composer/qgsaddremoveitemcommand.cpp
  composer/qgsaddremovemultiframecommand.cpp
  composer/qgscomposermultiframecommand.cpp
  composer/qgscomposerarrow.cpp
  composer/qgscomposerframe.cpp
  composer/qgscomposeritem.cpp
  composer/qgscomposeritemcommand.cpp
  composer/qgscomposeritemgroup.cpp
  composer/qgscomposerlabel.cpp
  composer/qgscomposerlegenditem.cpp
  composer/qgscomposerpicture.cpp
  composer/qgscomposermap.cpp
  composer/qgscomposertable.cpp
  composer/qgscomposerattributetable.cpp
  composer/qgscomposertexttable.cpp
  composer/qgscomposerscalebar.cpp
  composer/qgscomposershape.cpp
  composer/qgslegendmodel.cpp
  composer/qgscomposerlegend.cpp
  composer/qgspaperitem.cpp
  composer/qgsscalebarstyle.cpp
  composer/qgsdoubleboxscalebarstyle.cpp
  composer/qgsnumericscalebarstyle.cpp
  composer/qgssingleboxscalebarstyle.cpp
  composer/qgsticksscalebarstyle.cpp
  composer/qgscomposerhtml.cpp
  composer/qgscomposermultiframe.cpp
  composer/qgscomposition.cpp

  pal/costcalculator.cpp
  pal/feature.cpp
  pal/geomfunction.cpp
  pal/labelposition.cpp
  pal/layer.cpp
  pal/pal.cpp
  pal/palstat.cpp
  pal/pointset.cpp
  pal/priorityqueue.cpp
  pal/problem.cpp
  pal/util.cpp
  pal/linkedlist.hpp
  pal/hashtable.hpp
  pal/rtree.hpp

  raster/qgscliptominmaxenhancement.cpp
  raster/qgscolorrampshader.cpp
  raster/qgscontrastenhancement.cpp
  raster/qgscontrastenhancementfunction.cpp
  raster/qgsfreakoutshader.cpp
  raster/qgslinearminmaxenhancement.cpp
  raster/qgslinearminmaxenhancementwithclip.cpp
  raster/qgspseudocolorshader.cpp
  raster/qgsrasterinterface.cpp
  raster/qgsrasteriterator.cpp
  raster/qgsrasterlayer.cpp
  raster/qgsrastertransparency.cpp
  raster/qgsrasterpipe.cpp
  raster/qgsrastershader.cpp
  raster/qgsrastershaderfunction.cpp

  raster/qgsrasterdrawer.cpp
  raster/qgsrasterfilewriter.cpp
  raster/qgsrasterresamplefilter.cpp
  raster/qgsrasterrendererregistry.cpp
  raster/qgsrasterrenderer.cpp
  raster/qgsbilinearrasterresampler.cpp
  raster/qgscubicrasterresampler.cpp
  raster/qgspalettedrasterrenderer.cpp
  raster/qgsmultibandcolorrenderer.cpp
  raster/qgssinglebandcolordatarenderer.cpp
  raster/qgssinglebandgrayrenderer.cpp
  raster/qgssinglebandpseudocolorrenderer.cpp

  renderer/qgscontinuouscolorrenderer.cpp
  renderer/qgsgraduatedsymbolrenderer.cpp
  renderer/qgsrenderer.cpp
  renderer/qgssinglesymbolrenderer.cpp
  renderer/qgsuniquevaluerenderer.cpp

  symbology/qgsmarkercatalogue.cpp
  symbology/qgssymbol.cpp
  symbology/qgssymbologyutils.cpp

  qgsspatialindex.cpp

  qgspaintenginehack.cpp
  qgsscaleutils.cpp
)

IF(WIN32)
  SET(QGIS_CORE_SRCS
    ${QGIS_CORE_SRCS}
    gps/qextserialport/win_qextserialport.cpp
  )
  ADD_DEFINITIONS(-D_TTY_WIN_)
ELSE(WIN32)
  SET(QGIS_CORE_SRCS
    ${QGIS_CORE_SRCS}
    gps/qextserialport/posix_qextserialport.cpp
  )
  ADD_DEFINITIONS(-D_TTY_POSIX_)
ENDIF(WIN32)

IF (QT_MOBILITY_LOCATION_FOUND)
  SET(QGIS_CORE_SRCS
    ${QGIS_CORE_SRCS}
    gps/qgsqtlocationconnection.cpp
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND)

IF (WITH_INTERNAL_SPATIALITE)
  IF (WIN32 OR APPLE OR ANDROID)
    INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
  ENDIF (WIN32 OR APPLE OR ANDROID)
  IF (WIN32)
    ADD_DEFINITIONS(-DDLL_EXPORT)
  ENDIF (WIN32)

#
# sqlite3 requires some extraflags to be defined
# in order to support:
# - huge database > 2 GB
# - RTree Spatial Index
#
  ADD_DEFINITIONS(-D_LARGE_FILE=1 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1)
  ADD_DEFINITIONS(-DSQLITE_ENABLE_RTREE=1)

  SET(QGIS_CORE_SRCS
    ${QGIS_CORE_SRCS}
    spatialite/sqlite3.c
    spatialite/spatialite.c
    )

  IF(MSVC)
    # dirty code
    SET_SOURCE_FILES_PROPERTIES(spatialite/sqlite3.c spatialite/spatialite.c PROPERTIES COMPILE_FLAGS /W0)
  ELSE(MSVC)
    SET_SOURCE_FILES_PROPERTIES(spatialite/sqlite3.c spatialite/spatialite.c PROPERTIES COMPILE_FLAGS -w)
  ENDIF(MSVC)

  INCLUDE_DIRECTORIES(BEFORE spatialite/headers/spatialite)
ENDIF (WITH_INTERNAL_SPATIALITE)

ADD_FLEX_FILES(QGIS_CORE_SRCS qgsexpressionlexer.ll)

ADD_BISON_FILES(QGIS_CORE_SRCS qgsexpressionparser.yy)

SET(QGIS_CORE_MOC_HDRS
  qgsapplication.h
  qgsbrowsermodel.h
  qgscontexthelp.h
  qgscoordinatetransform.h
  qgsdataitem.h
  qgsdataprovider.h
  qgshttptransaction.h
  qgsmaplayer.h
  qgsmaplayerregistry.h
  qgsmaprenderer.h
  qgsmessageoutput.h
  qgsmessagelog.h
  qgscredentials.h
  qgspluginlayer.h
  qgsproject.h
  qgsrunprocess.h
  qgsvectorlayer.h
  qgsrasterdataprovider.h
  qgsnetworkaccessmanager.h
  qgsvectordataprovider.h
  qgsgeometryvalidator.h

  composer/qgsaddremoveitemcommand.h
  composer/qgscomposerlegend.h
  composer/qgscomposermap.h
  composer/qgscomposerpicture.h
  composer/qgscomposerscalebar.h
  composer/qgscomposeritem.h
  composer/qgscomposeritemgroup.h
  composer/qgscomposerlabel.h
  composer/qgscomposershape.h
  composer/qgscomposerattributetable.h
  composer/qgscomposerhtml.h
  composer/qgscomposermultiframe.h
  composer/qgscomposition.h

  composer/qgslegendmodel.h
  symbology/qgsmarkercatalogue.h
  raster/qgsrasterlayer.h

  gps/qgsgpsconnection.h
  gps/qgsgpsdetector.h
  gps/qgsnmeaconnection.h
  gps/qgsgpsdconnection.h
  gps/qextserialport/qextserialport.h
  gps/qextserialport/qextserialenumerator.h
)


IF (QT_MOBILITY_LOCATION_FOUND)
  SET(QGIS_CORE_MOC_HDRS
    ${QGIS_CORE_MOC_HDRS}
    gps/qgsqtlocationconnection.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND)

QT4_WRAP_CPP(QGIS_CORE_MOC_SRCS ${QGIS_CORE_MOC_HDRS})

# install headers
# install qgsconfig.h and plugin.h here so they can get into
# the OS X framework target

SET(QGIS_CORE_HDRS
  ${CMAKE_BINARY_DIR}/qgsconfig.h
  ../plugins/qgisplugin.h
  ../plugins/qgsrendererplugin.h
  qgis.h
  qgsapplication.h
  qgsattributeaction.h
  qgscentralpointpositionmanager.h
  qgsclipper.h
  qgscontexthelp.h
  qgscoordinatetransform.h
  qgsdatasourceuri.h
  qgsdataitem.h
  qgsdistancearea.h
  qgscsexception.h
  qgsexception.h
  qgsexpression.h
  qgsfeature.h
  qgsfield.h
  qgsgeometry.h
  qgshttptransaction.h
  qgslabel.h
  qgslabelattributes.h
  qgslogger.h
  qgsmaplayer.h
  qgsmaplayerregistry.h
  qgsmaprenderer.h
  qgsmaptopixel.h
  qgsmessageoutput.h
  qgsmimedatautils.h
  qgscredentials.h
  qgsoverlayobjectpositionmanager.h
  qgsowsconnection.h
  qgspallabeling.h
  qgspalobjectpositionmanager.h
  qgspluginlayer.h
  qgspluginlayerregistry.h
  qgspoint.h
  qgsproject.h
  qgsprojectfiletransform.h
  qgsprojectproperty.h
  qgsprojectversion.h
  qgsprovidercountcalcevent.h
  qgsproviderextentcalcevent.h
  qgsprovidermetadata.h
  qgsproviderregistry.h
  qgspythonrunner.h
  qgsrasterprojector.h
  qgsrasterdataprovider.h
  qgsrectangle.h
  qgsrendercontext.h
  qgsrunprocess.h
  qgsscalecalculator.h
  qgssnapper.h
  qgscoordinatereferencesystem.h
  qgsvectordataprovider.h
  qgsvectorfilewriter.h
  qgsvectorlayer.h
  qgsvectorlayerimport.h
  qgsvectoroverlay.h
  qgstolerance.h

  composer/qgslegendmodel.h
  composer/qgscomposerlegenditem.h

  raster/qgscliptominmaxenhancement.h
  raster/qgscolorrampshader.h
  raster/qgscolorrampshader.h
  raster/qgscontrastenhancement.h
  raster/qgscontrastenhancementfunction.h
  raster/qgsfreakoutshader.h
  raster/qgslinearminmaxenhancement.h
  raster/qgslinearminmaxenhancementwithclip.h
  raster/qgspseudocolorshader.h
  raster/qgsrasterpyramid.h
  raster/qgsrasterbandstats.h
  raster/qgsrasterhistogram.h
  raster/qgsrasterinterface.h
  raster/qgsrasterlayer.h
  raster/qgsrastertransparency.h
  raster/qgsrasterpipe.h
  raster/qgsrastershader.h
  raster/qgsrastershaderfunction.h
  raster/qgsrasterviewport.h

  renderer/qgscontinuouscolorrenderer.h
  renderer/qgsgraduatedsymbolrenderer.h
  renderer/qgsrenderer.h
  renderer/qgssinglesymbolrenderer.h
  renderer/qgsuniquevaluerenderer.h

  symbology/qgsmarkercatalogue.h
  symbology/qgssymbol.h
  symbology/qgssymbologyutils.h

  symbology-ng/qgscategorizedsymbolrendererv2.h
  symbology-ng/qgscolorrampv2data.h
  symbology-ng/qgsellipsesymbollayerv2.h
  symbology-ng/qgsfillsymbollayerv2.h
  symbology-ng/qgsgraduatedsymbolrendererv2.h
  symbology-ng/qgslinesymbollayerv2.h
  symbology-ng/qgsmarkersymbollayerv2.h
  symbology-ng/qgspointdisplacementrenderer.h
  symbology-ng/qgsrendererv2.h
  symbology-ng/qgsrendererv2registry.h
  symbology-ng/qgsrulebasedrendererv2.h
  symbology-ng/qgssinglesymbolrendererv2.h
  symbology-ng/qgsstylev2.h
  symbology-ng/qgssvgcache.h
  symbology-ng/qgssymbollayerv2.h
  symbology-ng/qgssymbollayerv2registry.h
  symbology-ng/qgssymbollayerv2utils.h
  symbology-ng/qgssymbologyv2conversion.h
  symbology-ng/qgssymbolv2.h
  symbology-ng/qgsvectorcolorrampv2.h
  symbology-ng/qgsvectorfieldsymbollayer.h

  qgsdiagramrendererv2.h

  qgsspatialindex.h

  qgspaintenginehack.h
  qgsscaleutils.h
)

IF (QT_MOBILITY_LOCATION_FOUND)
  SET(QGIS_CORE_HDRS
    ${QGIS_CORE_HDRS}
    gps/qgsqtlocationconnection.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  composer
  pal
  raster
  renderer
  symbology
  symbology-ng
  gps/qextserialport
  ${SPATIALINDEX_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

IF (NOT WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (NOT WITH_INTERNAL_SPATIALITE)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCORE_EXPORT \"-DCORE_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

#for PAL classes
IF (WIN32)
    ADD_DEFINITIONS("-D_HAVE_WINDOWS_H_")
ELSE (WIN32)
    ADD_DEFINITIONS("-D_HAVE_PTHREAD_")
ENDIF (WIN32)

#############################################################
# qgis_core library

ADD_LIBRARY(qgis_core SHARED ${QGIS_CORE_SRCS} ${QGIS_CORE_MOC_SRCS} ${QGIS_CORE_HDRS} ${QGIS_CORE_MOC_HDRS})

IF(APPLE)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    PUBLIC_HEADER "${QGIS_CORE_HDRS};${QGIS_CORE_MOC_HDRS}"
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis_core
  )
ENDIF(APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

# make sure to create qgsversion.h before compiling
ADD_DEPENDENCIES(qgis_core version)

# because of htonl
IF (WIN32)
  FIND_LIBRARY(SETUPAPI_LIBRARY NAMES setupapi PATHS $ENV{LIB})
  TARGET_LINK_LIBRARIES(qgis_core wsock32 ${SETUPAPI_LIBRARY})
ENDIF (WIN32)

IF(APPLE)
  TARGET_LINK_LIBRARIES(qgis_core "-framework CoreFoundation -framework IOKit")
ENDIF(APPLE)

IF (QT_MOBILITY_LOCATION_FOUND)
    TARGET_LINK_LIBRARIES(qgis_core ${QT_MOBILITY_LOCATION_LIBRARY})
ENDIF (QT_MOBILITY_LOCATION_FOUND)

TARGET_LINK_LIBRARIES(qgis_core
  ${QT_QTMAIN_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}

  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${SPATIALINDEX_LIBRARY}
)

IF (WITH_INTERNAL_SPATIALITE)
  IF (WIN32 OR APPLE)
    TARGET_LINK_LIBRARIES(qgis_core ${ICONV_LIBRARY})
  ENDIF (WIN32 OR APPLE)
  IF (ANDROID)
    TARGET_LINK_LIBRARIES(qgis_core ${ICONV_LIBRARY} ${CHARSET_LIBRARY})
  ENDIF (ANDROID)
  IF (UNIX AND NOT ANDROID)
    TARGET_LINK_LIBRARIES(qgis_core pthread ${CMAKE_DL_LIBS})
  ENDIF (UNIX AND NOT ANDROID)
ELSE (WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(qgis_core ${SQLITE3_LIBRARY})
ENDIF (WITH_INTERNAL_SPATIALITE)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
ENDIF (APPLE)

# install

INSTALL(TARGETS qgis_core
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_core FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/qgis_core\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
