FILE(GLOB PY_FILES *.py)
FILE(GLOB HELPER_FILES helper/*.py)
FILE(GLOB DESCR_FILES description/5.0.0/*.xml)
FiLE(GLOB HELP_FILES description/5.0.0/doc/*.html)

PLUGIN_INSTALL(processing ./algs/otb ${PY_FILES})
PLUGIN_INSTALL(processing ./algs/otb/helper ${HELPER_FILES})
PLUGIN_INSTALL(processing ./algs/otb/description/5.0.0 ${DESCR_FILES})
PLUGIN_INSTALL(processing ./algs/otb/description/5.0.0/doc ${HELP_FILES})
