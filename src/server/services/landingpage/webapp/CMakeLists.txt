find_program(YARN yarn yarnpkg REQUIRED)

set (YARN_INSTALL_OPT "--immutable" CACHE STRING "yarn install option [default: --immutable]")

file(GLOB_RECURSE WEBAPP_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

add_custom_target (server_landingpage ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dist/index.html)

# add_custom_command does not support OUTPUT_QUIET, so we need to make a cmake subcommand first:
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/yarn_commands.cmake
"execute_process(
  COMMAND ${YARN} install ${YARN_INSTALL_OPT}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  OUTPUT_QUIET
  ERROR_FILE ${CMAKE_CURRENT_BINARY_DIR}/yarn.log
  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/yarn.log
  RESULT_VARIABLE YARN_RESULT
)
if(NOT YARN_RESULT EQUAL 0)
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/yarn.log ERROR_CONTENTS)
  message(FATAL_ERROR \${ERROR_CONTENTS})
endif()

execute_process(
  COMMAND ${YARN} build --outDir ${CMAKE_CURRENT_BINARY_DIR}/dist
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  OUTPUT_QUIET
  ERROR_FILE ${CMAKE_CURRENT_BINARY_DIR}/yarn.log
  OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/yarn.log
  RESULT_VARIABLE YARN_RESULT
)
if(NOT YARN_RESULT EQUAL 0)
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/yarn.log ERROR_CONTENTS)
  message(FATAL_ERROR \${ERROR_CONTENTS})
endif()
")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dist/index.html
  DEPENDS ${WEBAPP_SRCS}
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/yarn_commands.cmake
)

install(
  DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/
  DESTINATION ${QGIS_DATA_DIR}/resources/server/api/ogc/static/landingpage/
)
