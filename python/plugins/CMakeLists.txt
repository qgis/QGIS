# Python plugins can be staged to PYTHON_OUTPUT_DIRECTORY so plugins
# will function when app is run from build directory

# When staging all plugins, use the following make targets:
#   staged-plugins - stage plugins (usually after repo pull/build and project make)
#   staged-plugins-pyc - stage and byte-compile all Python scripts
#   clean-staged-plugins - removes the plugins directory and all contents
#
# When developing on a plugin, use the following make targets:
#   staged-[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean-staged-[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

ADD_CUSTOM_TARGET(staged-plugins)

ADD_CUSTOM_TARGET(staged-plugins-pyc DEPENDS staged-plugins
  COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}/plugins"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Byte-compiling staged Python plugins..."
)

# plugins can also be staged with CMake option at build time
IF(WITH_STAGED_PLUGINS)
  IF(WITH_PY_COMPILE)
    ADD_CUSTOM_TARGET(staged-plugins-on-build ALL DEPENDS staged-plugins-pyc)
  ELSE(WITH_PY_COMPILE)
    ADD_CUSTOM_TARGET(staged-plugins-on-build ALL DEPENDS staged-plugins)
  ENDIF(WITH_PY_COMPILE)
ENDIF(WITH_STAGED_PLUGINS)

ADD_CUSTOM_TARGET(clean-staged-plugins
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins
)

FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount 0)

MACRO (PLUGIN_INSTALL plugin subdir )
  IF(MSVC)
    FILE(READ ${CMAKE_BINARY_DIR}/stagecount n)
    MATH(EXPR n "${n}+1")
    FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount ${n})
  ELSE(MSVC)
    STRING(REPLACE "/" "_" n "${subdir}")
  ENDIF(MSVC)

  # regular project build's install command and target
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})
  IF(WITH_STAGED_PLUGINS)
    ADD_CUSTOM_TARGET(${plugin}-${n} DEPENDS ${ARGN})
  ELSE(WITH_STAGED_PLUGINS)
    ADD_CUSTOM_TARGET(${plugin}-${n} ALL DEPENDS ${ARGN})
  ENDIF(WITH_STAGED_PLUGINS)

  # for staged plugin install (to run from build directory)
  ADD_CUSTOM_TARGET(${plugin}-${n}-stageinstall DEPENDS ${ARGN})
  ADD_CUSTOM_COMMAND(TARGET ${plugin}-${n}-stageinstall
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  FOREACH(file ${ARGN})
    ADD_CUSTOM_COMMAND(TARGET ${plugin}-${n}-stageinstall
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${file}\" ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      #COMMENT "copying ${file} to ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}"
    )
  ENDFOREACH(file)
  ADD_DEPENDENCIES(staged-plugins ${plugin}-${n}-stageinstall)

  IF(TARGET staged-${plugin})
    ADD_DEPENDENCIES(staged-${plugin} ${plugin}-${n}-stageinstall)
  ELSE(TARGET staged-${plugin})
    ADD_CUSTOM_TARGET(staged-${plugin} DEPENDS ${plugin}-${n}-stageinstall)
    ADD_CUSTOM_TARGET(clean-staged-${plugin}
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}
    )
  ENDIF(TARGET staged-${plugin})

ENDMACRO (PLUGIN_INSTALL)


ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(processing)
