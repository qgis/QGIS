add_subdirectory(db_plugins)
add_subdirectory(icons)

file(GLOB OTHER_FILES LICENSE README TODO)
file(GLOB PY_FILES *.py)

file(GLOB UI_FILES ui/*.ui)
PYQT_WRAP_UI(PYUI_FILES ${UI_FILES})
PYQT_ADD_RESOURCES(PYRC_FILES resources.qrc)

PLUGIN_INSTALL(db_manager . ${OTHER_FILES} ${PY_FILES} ${PYRC_FILES} metadata.txt)
PLUGIN_INSTALL(db_manager ui ${PYUI_FILES} ui/__init__.py)
