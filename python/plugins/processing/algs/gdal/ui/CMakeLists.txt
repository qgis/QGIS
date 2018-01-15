FILE(GLOB PY_FILES *.py)
FILE(GLOB UI_FILES *.ui)

PLUGIN_INSTALL(processing ./algs/gdal/ui ${PY_FILES} ${UI_FILES})
