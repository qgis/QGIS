FILE(GLOB PY_FILES *.py)
FILE(GLOB MODEL_FILES models/*.model)

PLUGIN_INSTALL(sextante modeler ${PY_FILES})
PLUGIN_INSTALL(sextante modeler/models ${MODEL_FILES})
