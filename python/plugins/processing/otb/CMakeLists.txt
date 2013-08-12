FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)
FILE(GLOB HELPER_FILES helper/*.py)
FiLE(GLOB HELP_FILES description/doc/*.html)

PLUGIN_INSTALL(processing ./otb ${PY_FILES})
PLUGIN_INSTALL(processing ./otb/description ${DESCR_FILES})
PLUGIN_INSTALL(processing ./otb/helper ${HELPER_FILES})
PLUGIN_INSTALL(processing ./otb/description/doc ${HELP_FILES})
