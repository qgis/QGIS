file(GLOB PY_FILES *.py)
file(GLOB UI_FILES *.ui)

PLUGIN_INSTALL(processing ./algs/gdal/ui ${PY_FILES} ${UI_FILES})
