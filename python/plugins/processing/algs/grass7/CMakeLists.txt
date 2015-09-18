FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES grass7.txt)
FILE(GLOB DESCR_FILES description/*.txt)

PLUGIN_INSTALL(processing algs/grass7 ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(processing algs/grass7/description ${DESCR_FILES})
