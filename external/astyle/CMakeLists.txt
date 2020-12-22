set(ASTYLE_SRCS
  astyle_main.cpp
  ASBeautifier.cpp
  ASEnhancer.cpp
  ASFormatter.cpp
  ASLocalizer.cpp
  ASResource.cpp
)

# No warnings for astyle build
if(NOT MSVC)
  set_source_files_properties(${ASTYLE_SRCS} PROPERTIES COMPILE_FLAGS -w)
endif()
add_executable(qgisstyle ${ASTYLE_SRCS})
set_target_properties(qgisstyle PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts)
