FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)
FILE(GLOB HELP_FILES help/*.*)

PLUGIN_INSTALL(sextante taudem ${PY_FILES})
PLUGIN_INSTALL(sextante taudem/description ${DESCR_FILES})
PLUGIN_INSTALL(sextante taudem/help ${HELP_FILES})
