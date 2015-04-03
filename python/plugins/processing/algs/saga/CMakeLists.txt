FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR212_FILES description/2.1.2/*.txt)
FILE(GLOB DESCR213_FILES description/2.1.2/*.txt)
FILE(GLOB HELP_FILES help/*.rst)

ADD_SUBDIRECTORY(ext)

PLUGIN_INSTALL(processing algs/saga ${PY_FILES})
PLUGIN_INSTALL(processing algs/saga/description/2.1.2 ${DESCR212_FILES})
PLUGIN_INSTALL(processing algs/saga/description/2.1.3 ${DESCR213_FILES})
PLUGIN_INSTALL(processing algs/saga/help ${HELP_FILES})
