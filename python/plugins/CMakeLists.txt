IF(WITH_PY_COMPILE)
  ADD_CUSTOM_TARGET(pycompile_staged ALL
    COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}/plugins"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Byte-compiling staged Python plugins..."
  )
ENDIF(WITH_PY_COMPILE)

MACRO (PLUGIN_INSTALL plugin subdir )
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})
  STRING(REPLACE "/" "_" subdir_sane "${subdir}")
  ADD_CUSTOM_TARGET(${plugin}_${subdir_sane}_stageinstall ALL DEPENDS ${ARGN})
  IF(WITH_STAGED_PLUGINS OR "${plugin}" STREQUAL "plugin_installer")
    IF(WITH_PY_COMPILE)
      ADD_DEPENDENCIES(pycompile_staged ${plugin}_${subdir_sane}_stageinstall)
    ENDIF(WITH_PY_COMPILE)
    FOREACH(file ${ARGN})
      ADD_CUSTOM_COMMAND(TARGET ${plugin}_${subdir_sane}_stageinstall
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
        COMMAND ${CMAKE_COMMAND} -E copy \"${file}\" ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        #COMMENT "copying ${file} to ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}"
      )
    ENDFOREACH(file)
  ENDIF()
ENDMACRO (PLUGIN_INSTALL)


ADD_SUBDIRECTORY(plugin_installer)
ADD_SUBDIRECTORY(mapserver_export)
ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(osm)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(sextante)
