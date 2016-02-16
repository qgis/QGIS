FILE(GLOB PY_FILES *.py)
FILE(GLOB SINGLE_DESCR_FILES description/single/*.txt)
FILE(GLOB MULTI_DESCR_FILES description/multi/*.txt)

PLUGIN_INSTALL(processing algs/taudem ${PY_FILES})
PLUGIN_INSTALL(processing algs/taudem/description/single ${SINGLE_DESCR_FILES})
PLUGIN_INSTALL(processing algs/taudem/description/multi ${MULTI_DESCR_FILES})
