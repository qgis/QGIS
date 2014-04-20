FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.xml)
FILE(GLOB HELPER_FILES helper/*.py)
FiLE(GLOB HELP_FILES description/doc/*.html)

PLUGIN_INSTALL(processing ./algs/otb ${PY_FILES})
PLUGIN_INSTALL(processing ./algs/otb/description ${DESCR_FILES})
PLUGIN_INSTALL(processing ./algs/otb/helper ${HELPER_FILES})
PLUGIN_INSTALL(processing ./algs/otb/description/doc ${HELP_FILES})
