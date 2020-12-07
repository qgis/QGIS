file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES grass7.txt)
file(GLOB DESCR_FILES description/*.txt)

add_subdirectory(ext)

PLUGIN_INSTALL(processing algs/grass7 ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(processing algs/grass7/description ${DESCR_FILES})
