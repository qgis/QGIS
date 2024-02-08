file(GLOB PY_FILES *.py)
list(FILTER PY_FILES EXCLUDE REGEX ".*description_to_json.py")
file(GLOB OTHER_FILES grass.txt metadata.txt)

execute_process(
  COMMAND ${Python_EXECUTABLE} -m grassprovider.description_to_json ${CMAKE_CURRENT_SOURCE_DIR}/description ${CMAKE_CURRENT_SOURCE_DIR}/description/algorithms.json
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
  RESULT_VARIABLE result
  ERROR_VARIABLE error_output
)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Create grass provider algorithm descriptions failed with error: ${error_output}")
endif()
set(DESCR_FILES ${CMAKE_CURRENT_SOURCE_DIR}/description/algorithms.json)

add_subdirectory(ext)
add_subdirectory(tests)

PLUGIN_INSTALL(grassprovider . ${PY_FILES} ${OTHER_FILES})
PLUGIN_INSTALL(grassprovider ./description ${DESCR_FILES})
