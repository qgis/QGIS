# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

SET(QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)
SET (NAME processing)

SET(PY_FILES
  __init__.py
  algfactory.py
)

FILE (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${NAME})
INSTALL(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/${NAME}")

ADD_CUSTOM_TARGET(py${NAME} ALL)
# stage to output to make available when QGIS is run from build directory
FOREACH(pyfile ${PY_FILES})
  ADD_CUSTOM_COMMAND(TARGET py${NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${NAME}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${NAME}/${pyfile}")
ENDFOREACH(pyfile)
