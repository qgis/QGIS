FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES copyright.txt)
FILE(GLOB ICON_FILES icons/*.png)

PLUGIN_INSTALL(processing ./algs/ftools ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(processing ./algs/ftools/icons ${ICON_FILES})
