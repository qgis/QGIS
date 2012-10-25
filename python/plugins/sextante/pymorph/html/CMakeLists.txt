FILE(GLOB HTML_FILES *.html)

ADD_SUBDIRECTORY(morph)

PLUGIN_INSTALL(sextante ./pymorph/html ${HTML_FILES})
