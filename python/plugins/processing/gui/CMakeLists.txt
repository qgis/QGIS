FILE(GLOB PY_FILES *.py)
FILE(GLOB OTHER_FILES help/*.*)

PLUGIN_INSTALL(processing gui ${PY_FILES})
PLUGIN_INSTALL(processing gui/help ${OTHER_FILES})
