
SET (DOC_FILES
favicon.ico
index.html
qgisdoc.css
)

#SUBDIRS (images plugins install_guide)

INSTALL(FILES ${DOC_FILES}
        DESTINATION ${QGIS_DATA_DIR}/doc)

INSTALL(FILES images/qgis_new_80pct.png
        DESTINATION ${QGIS_DATA_DIR}/doc/images)
