# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets

set (QGIS_PYTHON_DIR ${QGIS_DATA_DIR}/python)

macro(EXT_PYLIB lib enable_by_default)
  string(TOUPPER ${lib} ulib)
  set (WITH_INTERNAL_${ulib} ${enable_by_default} CACHE BOOL "Determines whether python ${lib} should be included")
  if(WITH_INTERNAL_${ulib})
    install(DIRECTORY ${lib} DESTINATION "${QGIS_PYTHON_DIR}")

    add_custom_target(py${lib})

    file(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${lib}/*)
    add_custom_command(TARGET py${lib}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${lib} "${PYTHON_OUTPUT_DIRECTORY}/${lib}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${files}
    )
    PY_COMPILE(py${lib} "${PYTHON_OUTPUT_DIRECTORY}/${lib}")

    add_dependencies(staged-plugins py${lib})
    add_custom_command(TARGET clean-staged-plugins
      COMMAND ${CMAKE_COMMAND} -E remove_directory "${PYTHON_OUTPUT_DIRECTORY}/${lib}"
    )

  endif()
endmacro(EXT_PYLIB lib)
