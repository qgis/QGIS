file(GLOB PY_FILES *.py)
file(GLOB DESCR_FILES description/*.txt)

add_subdirectory(ext)

PLUGIN_INSTALL(processing algs/saga ${PY_FILES})
PLUGIN_INSTALL(processing algs/saga/description/ ${DESCR_FILES})
