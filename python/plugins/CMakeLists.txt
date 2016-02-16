# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets
#
# When developing on a plugin, use the following make targets:
#   staged-[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean-staged-[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount 0)

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

ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(processing)
ADD_SUBDIRECTORY(MetaSearch)

PY_COMPILE(staged-plugins "${PYTHON_OUTPUT_DIRECTORY}/plugins")
