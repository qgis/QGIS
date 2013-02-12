FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)
FILE(GLOB HELPER_FILES helper/*.py)

PLUGIN_INSTALL(sextante ./otb ${PY_FILES})
PLUGIN_INSTALL(sextante ./otb/description ${DESCR_FILES})
PLUGIN_INSTALL(sextante ./otb/helper ${HELPER_FILES})
