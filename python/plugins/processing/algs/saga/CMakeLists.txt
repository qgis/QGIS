FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)

ADD_SUBDIRECTORY(ext)

PLUGIN_INSTALL(processing algs/saga ${PY_FILES})
PLUGIN_INSTALL(processing algs/saga/description/ ${DESCR_FILES})
