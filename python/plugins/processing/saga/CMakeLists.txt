FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)

ADD_SUBDIRECTORY(ext)

PLUGIN_INSTALL(processing saga ${PY_FILES})
PLUGIN_INSTALL(processing saga/description ${DESCR_FILES})
