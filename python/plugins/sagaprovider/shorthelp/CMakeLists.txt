file(GLOB PY_FILES *.py)
file(GLOB YAML_FILES *.yaml)

PLUGIN_INSTALL(sagaprovider ./shorthelp/ ${PY_FILES})
PLUGIN_INSTALL(sagaprovider ./shorthelp/ ${YAML_FILES})
