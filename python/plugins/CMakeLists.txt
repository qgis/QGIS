# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets
#
# When developing on a plugin, use the following make targets:
#   staged-[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean-staged-[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount 0)

IF(ENABLE_QT5)
  # The target port-plugins acts as a container for all port-plugin-XXX targets
  # Each port-plugin-XXX target will run scripts/2to3 on a plugin and migrate
  # Python 3 and Qt 5 code.
  SET (PORT_PLUGINS FALSE CACHE BOOL "Determines whether python plugins in staged or installed locations should be migrated to Python 3 and Qt 5")
  IF(PORT_PLUGINS)
    ADD_CUSTOM_TARGET(port-plugins ALL)
  ELSE(PORT_PLUGINS)
    ADD_CUSTOM_TARGET(port-plugins)
  ENDIF(PORT_PLUGINS)
ENDIF(ENABLE_QT5)

MACRO (PLUGIN_INSTALL plugin subdir )
  FILE(READ ${CMAKE_BINARY_DIR}/stagecount n)
  MATH(EXPR n "${n}+1")
  FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount ${n})

  # regular project build's install command and target
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})

  SET(_cmake ${CMAKE_BINARY_DIR}/python/plugins/${plugin}/stageinstall.cmake)

  IF(NOT TARGET zzz-${plugin}-stageinstall)
    ADD_CUSTOM_TARGET(zzz-${plugin}-stageinstall)
    ADD_CUSTOM_TARGET(zzz-${plugin}-depend)
    ADD_DEPENDENCIES(zzz-${plugin}-stageinstall zzz-${plugin}-depend)

    ADD_CUSTOM_COMMAND(TARGET zzz-${plugin}-stageinstall
      COMMAND ${CMAKE_COMMAND} -P ${_cmake}
    )

    ADD_CUSTOM_TARGET(clean-staged-${plugin}
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}
    )

    ADD_DEPENDENCIES(staged-plugins zzz-${plugin}-stageinstall)
    ADD_CUSTOM_TARGET(staged-${plugin} DEPENDS zzz-${plugin}-stageinstall)
    FILE(WRITE ${_cmake} "")

    IF(ENABLE_QT5)
      ADD_CUSTOM_TARGET(port-plugin-${plugin})
      PY_2TO3(port-plugin-${plugin} ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin})
      ADD_DEPENDENCIES(port-plugins port-plugin-${plugin})
      ADD_DEPENDENCIES(port-plugin-${plugin} staged-${plugin})
    ENDIF(ENABLE_QT5)
  ENDIF(NOT TARGET zzz-${plugin}-stageinstall)

  ADD_CUSTOM_TARGET(zzz-${plugin}-${n}-depend DEPENDS ${ARGN})
  ADD_DEPENDENCIES(zzz-${plugin}-depend zzz-${plugin}-${n}-depend)

  FILE(APPEND ${_cmake} "FILE(MAKE_DIRECTORY \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")

  FOREACH(file ${ARGN})
    GET_FILENAME_COMPONENT(_file "${file}" ABSOLUTE)
    FILE(APPEND ${_cmake} "FILE(COPY \"${_file}\" DESTINATION \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")
  ENDFOREACH(file)
ENDMACRO (PLUGIN_INSTALL)

ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(processing)
ADD_SUBDIRECTORY(MetaSearch)

PY_COMPILE(staged-plugins "${PYTHON_OUTPUT_DIRECTORY}/plugins")
