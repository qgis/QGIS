# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

set(QGIS_PYTHON_DIR ${Python_SITEARCH}/qgis)

set(PY_FILES
  __init__.py
  mocked.py
)

file (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing)
install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/testing")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/test_data_dir.py.in" "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing/test_data_dir.py")

add_custom_target(pytesting ALL)
# stage to output to make available when QGIS is run from build directory
foreach(pyfile ${PY_FILES})
  add_custom_command(TARGET pytesting
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing/${pyfile}")
endforeach(pyfile)

PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/testing/test_data_dir.py")
