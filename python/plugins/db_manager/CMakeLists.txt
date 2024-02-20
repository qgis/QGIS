add_subdirectory(db_plugins)
add_subdirectory(icons)

file(GLOB OTHER_FILES LICENSE README TODO)
file(GLOB PY_FILES *.py)

file(GLOB UI_FILES ui/*.ui)

PLUGIN_INSTALL(db_manager . ${OTHER_FILES} ${PY_FILES} ${PYRC_FILES} metadata.txt)
PLUGIN_INSTALL(db_manager ui ${UI_FILES})
