FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPT_FILES scripts/*.rsx)

PLUGIN_INSTALL(processing r ${PY_FILES})
PLUGIN_INSTALL(processing r/scripts ${SCRIPT_FILES})
