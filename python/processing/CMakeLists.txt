# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

set(QGIS_PYTHON_DIR ${Python_SITEARCH}/qgis)

set(PY_FILES
  __init__.py
  algfactory.py
)

file (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY}/processing)
install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/processing")

add_custom_target(pyprocessing ALL)
# stage to output to make available when QGIS is run from build directory
foreach(pyfile ${PY_FILES})
  add_custom_command(TARGET pyprocessing
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/processing"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    # DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/processing/${pyfile}")
endforeach(pyfile)
