FILE(GLOB PY_FILES *.py)
FILE(GLOB HELP_FILES help/*.rst)
FILE(GLOB SCRIPT_FILES scripts/*.*)

ADD_SUBDIRECTORY(mmqgisx)
ADD_SUBDIRECTORY(ftools)
ADD_SUBDIRECTORY(ui)

PLUGIN_INSTALL(processing algs/qgis ${PY_FILES})
PLUGIN_INSTALL(processing algs/qgis/help ${HELP_FILES})
PLUGIN_INSTALL(processing algs/qgis/scripts ${SCRIPT_FILES})
