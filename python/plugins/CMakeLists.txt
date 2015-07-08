# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets
#
# When developing on a plugin, use the following make targets:
#   staged-[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean-staged-[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

FILE(WRITE ${CMAKE_BINARY_DIR}/stagecount 0)
SET(PYTHON_BINARY_TOPDIR ${CMAKE_CURRENT_BINARY_DIR})

MACRO (PLUGIN_INSTALL plugin subdir )
  # regular project build's install command and target
  INSTALL(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})

  SET(_cmake ${PYTHON_BINARY_TOPDIR}/${plugin}-stageinstall.cmake)

  IF(NOT TARGET ${plugin}-stageinstall)
    FILE(WRITE ${_cmake} "")
  ENDIF(NOT TARGET ${plugin}-stageinstall)

  SET(_out "FILE(MAKE_DIRECTORY \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")

  FOREACH(file ${ARGN})
    GET_FILENAME_COMPONENT(_file "${file}" ABSOLUTE)
    SET(_out "${_out}FILE(COPY \"${_file}\" DESTINATION \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")
  ENDFOREACH(file)

  FILE(APPEND ${_cmake} "${_out}")

  IF(NOT TARGET ${plugin}-stageinstall)
    # for staged plugin install (to run from build directory)
    ADD_CUSTOM_TARGET(${plugin}-stageinstall)

    ADD_CUSTOM_COMMAND(TARGET ${plugin}-stageinstall
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -P ${_cmake}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    ADD_DEPENDENCIES(staged-plugins ${plugin}-stageinstall)
  ENDIF(NOT TARGET ${plugin}-stageinstall)

  IF(TARGET staged-${plugin})
    ADD_DEPENDENCIES(staged-${plugin} ${plugin}-stageinstall)
  ELSE(TARGET staged-${plugin})
    ADD_CUSTOM_TARGET(staged-${plugin} DEPENDS ${plugin}-stageinstall)
    ADD_CUSTOM_TARGET(clean-staged-${plugin}
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}
    )
  ENDIF(TARGET staged-${plugin})

ENDMACRO (PLUGIN_INSTALL)

ADD_SUBDIRECTORY(fTools)
ADD_SUBDIRECTORY(GdalTools)
ADD_SUBDIRECTORY(db_manager)
ADD_SUBDIRECTORY(processing)
ADD_SUBDIRECTORY(MetaSearch)

PY_COMPILE(staged-plugins "${PYTHON_OUTPUT_DIRECTORY}/plugins")
