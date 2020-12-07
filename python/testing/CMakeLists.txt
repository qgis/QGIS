# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

set(QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)
set (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

set(PY_FILES
  __init__.py
  mocked.py
)

file (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing)
install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/testing")

add_custom_target(pytesting ALL)
# stage to output to make available when QGIS is run from build directory
foreach(pyfile ${PY_FILES})
  add_custom_command(TARGET pytesting
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing/${pyfile}")
endforeach(pyfile)
