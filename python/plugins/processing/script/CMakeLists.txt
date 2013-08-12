FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPTS_FILES scripts/*.py)

PLUGIN_INSTALL(processing script ${PY_FILES})
PLUGIN_INSTALL(processing script/scripts ${SCRIPTS_FILES})
