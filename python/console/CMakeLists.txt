set (QGIS_CONSOLE_DIR ${QGIS_DATA_DIR}/python/console)

set(PY_CONSOLE_FILES
  console.py
  console_sci.py
  console_settings.py
  console_output.py
  console_editor.py
  console_compile_apis.py
  __init__.py
)

file(GLOB UI_FILES *.ui)
PYQT_WRAP_UI(PYUI_FILES ${UI_FILES})
add_custom_target(pyconsole ALL DEPENDS ${PYUI_FILES})

foreach(pyfile ${PY_CONSOLE_FILES} ${PYUI_FILES})
  add_custom_command(TARGET pyconsole
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/console
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} ${PYTHON_OUTPUT_DIRECTORY}/console
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
endforeach(pyfile)
PY_COMPILE(pyconsole "${PYTHON_OUTPUT_DIRECTORY}/console")

install(FILES ${PY_CONSOLE_FILES} ${PYUI_FILES} DESTINATION "${QGIS_CONSOLE_DIR}")

