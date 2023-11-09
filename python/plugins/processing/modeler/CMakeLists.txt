file(GLOB PY_FILES *.py)
file(GLOB MODEL_FILES models/*.model)

PLUGIN_INSTALL(processing modeler ${PY_FILES})
PLUGIN_INSTALL(processing modeler/models ${MODEL_FILES})
