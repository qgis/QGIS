FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPT_FILES scripts/*.rsx)

PLUGIN_INSTALL(processing algs/r ${PY_FILES})
PLUGIN_INSTALL(processing algs/r/scripts ${SCRIPT_FILES})
