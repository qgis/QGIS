FILE(GLOB PY_FILES *.py)
FILE(GLOB SCRIPT_FILES scripts/*.pl)

PLUGIN_INSTALL(processing algs/perl ${PY_FILES})
PLUGIN_INSTALL(processing algs/perl/scripts ${SCRIPT_FILES})
