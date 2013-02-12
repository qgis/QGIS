FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES copyright.txt)
FILE(GLOB ICON_FILES icons/*.png)

PLUGIN_INSTALL(sextante ./algs/mmqgisx ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(sextante ./algs/mmqgisx/icons ${ICON_FILES})
