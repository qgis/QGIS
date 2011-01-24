SUBDIRS (copyright_label 
  delimited_text 
  interpolation 
  north_arrow 
  scale_bar
  oracle_raster
  raster_terrain_analysis
  quick_print
  coordinate_capture
  dxf2shp_converter
  ogr_converter
  diagram_overlay
  evis
  point_displacement_renderer
  spatialquery
  sqlanywhere
  roadgraph
  )

IF (WITH_SPATIALITE)
  SUBDIRS( offline_editing )
ENDIF (WITH_SPATIALITE)

IF (POSTGRES_FOUND)
  SUBDIRS (spit)
ENDIF (POSTGRES_FOUND)

IF (EXPAT_FOUND)
  SUBDIRS (gps_importer wfs)
ENDIF (EXPAT_FOUND)

IF (GSL_FOUND)
  SUBDIRS (georeferencer)
ENDIF (GSL_FOUND)

IF (GRASS_FOUND)
  SUBDIRS (grass)
ENDIF (GRASS_FOUND)

INSTALL(FILES qgisplugin.h qgsrendererplugin.h DESTINATION ${QGIS_INCLUDE_DIR})

