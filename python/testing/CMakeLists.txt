# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

SET(QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

SET(PY_FILES
  __init__.py
  mocked.py
)

FILE (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing)
INSTALL(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/testing")

ADD_CUSTOM_TARGET(pytesting ALL)
# stage to output to make available when QGIS is run from build directory
FOREACH(pyfile ${PY_FILES})
  ADD_CUSTOM_COMMAND(TARGET pytesting
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing/${pyfile}")
ENDFOREACH(pyfile)
