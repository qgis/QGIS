FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES grass.txt)
FILE(GLOB DESCR_FILES description/*.txt)

ADD_SUBDIRECTORY(ext)

PLUGIN_INSTALL(sextante grass ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(sextante grass/description ${DESCR_FILES})
