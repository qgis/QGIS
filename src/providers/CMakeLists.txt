
SUBDIRS (memory ogr wms delimitedtext osm_provider)

IF (POSTGRES_FOUND)
  SUBDIRS (postgres)
ENDIF (POSTGRES_FOUND)

IF (EXPAT_FOUND)
  SUBDIRS (gpx wfs)
ENDIF (EXPAT_FOUND)

IF (GRASS_FOUND)
  SUBDIRS (grass)
ENDIF (GRASS_FOUND)
