
SUBDIRS (copyright_label delimited_text grid_maker navigation north_arrow scale_bar wfs)

IF (POSTGRES_FOUND)
  SUBDIRS (geoprocessing spit)
ENDIF (POSTGRES_FOUND)

IF (EXPAT_FOUND)
  SUBDIRS (gps_importer)
ENDIF (EXPAT_FOUND)

IF (GSL_FOUND)
  SUBDIRS (georeferencer)
ENDIF (GSL_FOUND)

IF (GRASS_FOUND)
  SUBDIRS (grass)
ENDIF (GRASS_FOUND)
