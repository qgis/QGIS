# Python plugins can be staged to PYTHON_OUTPUT_DIRECTORY so plugins
# will function when app is run from build directory

# When staging all plugins, use the following make targets:
#   staged_plugins - stage plugins (usually after repo pull/build and project make)
#   staged_plugins_pyc - stage and byte-compile all Python scripts
#   clean_staged_plugins - removes the plugins directory and all contents
#
# When developing on a plugin, use the following make targets:
#   staged_[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean_staged_[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

ADD_CUSTOM_TARGET(staged_plugins)

ADD_CUSTOM_TARGET(staged_plugins_pyc DEPENDS staged_plugins
  COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}/plugins"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Byte-compiling staged Python plugins..."
)

# plugins can also be staged with CMake option at build time
IF(WITH_STAGED_PLUGINS)
  IF(WITH_PY_COMPILE)
    ADD_CUSTOM_TARGET(staged_plugins_on_build ALL DEPENDS staged_plugins_pyc)
  ELSE(WITH_PY_COMPILE)
    ADD_CUSTOM_TARGET(staged_plugins_on_build ALL DEPENDS staged_plugins)
  ENDIF(WITH_PY_COMPILE)
ENDIF(WITH_STAGED_PLUGINS)

ADD_CUSTOM_TARGET(clean_staged_plugins
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins
)

MACRO (PLUGIN_INSTALL plugin subdir )

  # regular project build's install command and target
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})
  STRING(REPLACE "/" "_" subdir_sane "${subdir}")
  IF(WITH_STAGED_PLUGINS)
    ADD_CUSTOM_TARGET(${plugin}_${subdir_sane} DEPENDS ${ARGN})
  ELSE(WITH_STAGED_PLUGINS)
    ADD_CUSTOM_TARGET(${plugin}_${subdir_sane} ALL DEPENDS ${ARGN})
  ENDIF(WITH_STAGED_PLUGINS)

  # for staged plugin install (to run from build directory)
  ADD_CUSTOM_TARGET(${plugin}_${subdir_sane}_stageinstall DEPENDS ${ARGN})
  ADD_CUSTOM_COMMAND(TARGET ${plugin}_${subdir_sane}_stageinstall
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  FOREACH(file ${ARGN})
    ADD_CUSTOM_COMMAND(TARGET ${plugin}_${subdir_sane}_stageinstall
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${file}\" ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      #COMMENT "copying ${file} to ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}"
    )
  ENDFOREACH(file)
  ADD_DEPENDENCIES(staged_plugins ${plugin}_${subdir_sane}_stageinstall)

  IF(TARGET staged_${plugin})
    ADD_DEPENDENCIES(staged_${plugin} ${plugin}_${subdir_sane}_stageinstall)
  ELSE(TARGET staged_${plugin})
    ADD_CUSTOM_TARGET(staged_${plugin} DEPENDS ${plugin}_${subdir_sane}_stageinstall)
    ADD_CUSTOM_TARGET(clean_staged_${plugin}
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}
    )
  ENDIF(TARGET staged_${plugin})

ENDMACRO (PLUGIN_INSTALL)


ADD_SUBDIRECTORY(plugin_installer)
ADD_SUBDIRECTORY(mapserver_export)
ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(sextante)
