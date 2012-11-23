FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPTS_FILES scripts/*.py)

PLUGIN_INSTALL(sextante script ${PY_FILES})
PLUGIN_INSTALL(sextante script/scripts ${SCRIPTS_FILES})
