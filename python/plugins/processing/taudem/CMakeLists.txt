FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)

PLUGIN_INSTALL(processing taudem ${PY_FILES})
PLUGIN_INSTALL(processing taudem/description ${DESCR_FILES})
