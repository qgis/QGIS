file(GLOB PY_FILES *.py)
file(GLOB DESCR_FILES description/*.txt)
file(GLOB OTHER_FILES metadata.txt)

add_subdirectory(ext)
add_subdirectory(shorthelp)
add_subdirectory(tests)

PLUGIN_INSTALL(sagaprovider . ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(sagaprovider ./description/ ${DESCR_FILES})
