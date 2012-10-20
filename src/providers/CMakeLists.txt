# override default path where built files are put to allow running qgis without installing
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})

ADD_SUBDIRECTORY(memory)
ADD_SUBDIRECTORY(ogr)
ADD_SUBDIRECTORY(wms)
ADD_SUBDIRECTORY(delimitedtext)
#ADD_SUBDIRECTORY(osm)           # TODO: enable when migrated to new api
#ADD_SUBDIRECTORY(sqlanywhere)   # TODO: enable when migrated to new api
ADD_SUBDIRECTORY(gdal)
#ADD_SUBDIRECTORY(mssql)         # TODO: enable when migrated to new api
ADD_SUBDIRECTORY(ows)
ADD_SUBDIRECTORY(wcs)

IF (POSTGRES_FOUND)
  ADD_SUBDIRECTORY(postgres)
ENDIF (POSTGRES_FOUND)

IF (SPATIALITE_FOUND OR WITH_INTERNAL_SPATIALITE)
  ADD_SUBDIRECTORY(spatialite)
ENDIF (SPATIALITE_FOUND OR WITH_INTERNAL_SPATIALITE)

IF (EXPAT_FOUND)
#  ADD_SUBDIRECTORY(gpx)         # TODO: enable when migrated to new api
#  ADD_SUBDIRECTORY(wfs)         # TODO: enable when migrated to new api
ENDIF (EXPAT_FOUND)

IF (GRASS_FOUND)
  ADD_SUBDIRECTORY(grass)
ENDIF (GRASS_FOUND)
