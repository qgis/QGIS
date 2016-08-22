FILE(GLOB PY_FILES *.py)
FILE(GLOB SINGLE_DESCR_FILES description/*.txt)

PLUGIN_INSTALL(processing algs/taudem ${PY_FILES})
PLUGIN_INSTALL(processing algs/taudem/description/ ${SINGLE_DESCR_FILES})
