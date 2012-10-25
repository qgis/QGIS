FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)
FILE(GLOB HELP_FILES help/*.html)

PLUGIN_INSTALL(sextante saga ${PY_FILES})
PLUGIN_INSTALL(sextante saga/description ${DESCR_FILES})
PLUGIN_INSTALL(sextante saga/help ${HELP_FILES})
