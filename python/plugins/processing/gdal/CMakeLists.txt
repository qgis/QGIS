FILE(GLOB PY_FILES *.py)
FILE(GLOB ICON_FILES icons/*.png)

ADD_SUBDIRECTORY(scripts)
ADD_SUBDIRECTORY(pyogr)

PLUGIN_INSTALL(processing ./gdal ${PY_FILES})
PLUGIN_INSTALL(processing ./gdal/icons ${ICON_FILES})
