

MACRO(ADD_TRANSLATION_FILES _sources )

    FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

      SET(_out ${QGIS_OUTPUT_DIRECTORY}/i18n/${_basename}.qm)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_out}
         COMMAND ${QT_LRELEASE_EXECUTABLE}
         ARGS -verbose ${_in} -qm ${_out}
         DEPENDS ${_in}
      )

      SET(${_sources} ${${_sources}} ${_out} )
   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_TRANSLATION_FILES)

# make sure the output directory exists
file(MAKE_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/i18n)

FILE (GLOB TS_FILES *.ts)

ADD_TRANSLATION_FILES (QM_FILES ${TS_FILES})

# creating a custom target is needed to make the files build
# "ALL" means that it will be run by default
ADD_CUSTOM_TARGET (translations ALL
                   DEPENDS ${QM_FILES})

# first compile sources, then compile translations
ADD_DEPENDENCIES (translations qgis)

INSTALL (FILES ${QM_FILES}
         DESTINATION ${QGIS_DATA_DIR}/i18n)
