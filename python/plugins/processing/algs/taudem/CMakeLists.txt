FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)

PLUGIN_INSTALL(processing algs/taudem ${PY_FILES})
PLUGIN_INSTALL(processing algs/taudem/description ${DESCR_FILES})
