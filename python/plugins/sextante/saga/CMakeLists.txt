FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)

ADD_SUBDIRECTORY(ext)

PLUGIN_INSTALL(sextante saga ${PY_FILES})
PLUGIN_INSTALL(sextante saga/description ${DESCR_FILES})
