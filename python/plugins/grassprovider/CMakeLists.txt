file(GLOB PY_FILES *.py)
file(GLOB OTHER_FILES grass7.txt metadata.txt)
file(GLOB DESCR_FILES description/*.txt)

add_subdirectory(ext)
add_subdirectory(tests)

PLUGIN_INSTALL(grassprovider . ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(grassprovider ./description ${DESCR_FILES})
