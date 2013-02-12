FILE(GLOB PY_FILES *.py)
FILE(GLOB DESCR_FILES description/*.txt)
FILE(GLOB HELP_FILES help/*.*)
FILE(GLOB HELP_IMAGES help/img/*.*)

PLUGIN_INSTALL(sextante taudem ${PY_FILES})
PLUGIN_INSTALL(sextante taudem/description ${DESCR_FILES})
PLUGIN_INSTALL(sextante taudem/help ${HELP_FILES})
PLUGIN_INSTALL(sextante taudem/help/img ${HELP_IMAGES})
