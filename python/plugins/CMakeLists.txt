# See ../CMakeLists.txt for info on staged-plugins* and clean-staged-plugins targets
#
# When developing on a plugin, use the following make targets:
#   staged-[plugin_dir_name] - stage specific plugin, regenerating any changed resources
#   clean-staged-[plugin_dir_name] - removes the plugin directory and its contents
#
# NOTE: regular project 'make install' is unaffected

file(WRITE ${CMAKE_BINARY_DIR}/stagecount 0)

macro (PLUGIN_INSTALL plugin subdir )
  file(READ ${CMAKE_BINARY_DIR}/stagecount n)
  math(EXPR n "${n}+1")
  file(WRITE ${CMAKE_BINARY_DIR}/stagecount ${n})

  # regular project build's install command and target
  install(FILES ${ARGN} DESTINATION ${QGIS_DATA_DIR}/python/plugins/${plugin}/${subdir})

  set(_cmake ${CMAKE_BINARY_DIR}/python/plugins/${plugin}/stageinstall.cmake)

  if(NOT TARGET zzz-${plugin}-stageinstall)
    add_custom_target(zzz-${plugin}-stageinstall)
    add_custom_target(zzz-${plugin}-depend)
    add_dependencies(zzz-${plugin}-stageinstall zzz-${plugin}-depend)

    add_custom_command(TARGET zzz-${plugin}-stageinstall
      COMMAND ${CMAKE_COMMAND} -P ${_cmake}
    )

    add_custom_target(clean-staged-${plugin}
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}
    )

    add_dependencies(staged-plugins zzz-${plugin}-stageinstall)
    add_custom_target(staged-${plugin} DEPENDS zzz-${plugin}-stageinstall)
    file(WRITE ${_cmake} "")
  endif()

  add_custom_target(zzz-${plugin}-${n}-depend DEPENDS ${ARGN})
  add_dependencies(zzz-${plugin}-depend zzz-${plugin}-${n}-depend)

  file(APPEND ${_cmake} "file(MAKE_DIRECTORY \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")

  foreach(file ${ARGN})
    get_filename_component(_file "${file}" ABSOLUTE)
    file(APPEND ${_cmake} "file(COPY \"${_file}\" DESTINATION \"${PYTHON_OUTPUT_DIRECTORY}/plugins/${plugin}/${subdir}\")
")
  endforeach(file)
endmacro (PLUGIN_INSTALL)

if (WITH_GUI)
	add_subdirectory(db_manager)
endif()
add_subdirectory(processing)
add_subdirectory(grassprovider)
add_subdirectory(otbprovider)
add_subdirectory(MetaSearch)

PY_COMPILE(staged-plugins "${PYTHON_OUTPUT_DIRECTORY}/plugins")
