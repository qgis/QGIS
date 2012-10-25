
MACRO (PLUGIN_INSTALL plugin subdir )
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})
  STRING(REPLACE "/" "_" subdir_sane "${subdir}")
  ADD_CUSTOM_TARGET(${plugin}_${subdir_sane}_stageinstall ALL DEPENDS ${ARGN})
  FOREACH(file ${ARGN})
    ADD_CUSTOM_COMMAND(TARGET ${plugin}_${subdir_sane}_stageinstall
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
      COMMAND ${CMAKE_COMMAND} -E copy \"${file}\" ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      #COMMENT "copying ${file} to ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}"
    )
  ENDFOREACH(file)
ENDMACRO (PLUGIN_INSTALL)


ADD_SUBDIRECTORY(plugin_installer)
ADD_SUBDIRECTORY(mapserver_export)
ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(osm)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(sextante)
