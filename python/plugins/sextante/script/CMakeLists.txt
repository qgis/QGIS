FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPT_FILES scripts/*.py)

PLUGIN_INSTALL(sextante script ${PY_FILES})
PLUGIN_INSTALL(sextante script/scripts ${SCRIPT_FILES})
