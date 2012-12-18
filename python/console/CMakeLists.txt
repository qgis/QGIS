ADD_SUBDIRECTORY(console_help)

SET(PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)
SET(QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)

# PyQGIS console files to copy to staging or install
SET(PY_CONSOLE_FILES
  console.py
  console_sci.py
  console_help.py
  console_settings.py
  console_output.py
)

FILE(GLOB UI_FILES *.ui)
PYQT4_WRAP_UI(PYUI_FILES ${UI_FILES}) # returns absolute paths
ADD_CUSTOM_TARGET(pyconsole ALL DEPENDS ${PYUI_FILES})

# stage to output to make available when QGIS is run from build directory
FOREACH(pyfile ${PY_CONSOLE_FILES} ${PYUI_FILES})
  ADD_CUSTOM_COMMAND(TARGET pyconsole
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
ENDFOREACH(pyfile)

INSTALL(FILES ${PY_CONSOLE_FILES} ${PYUI_FILES} DESTINATION "${QGIS_PYTHON_DIR}")

