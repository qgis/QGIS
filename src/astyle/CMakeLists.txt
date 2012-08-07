SET(ASTYLE_SRCS astyle_main.cpp ASBeautifier.cpp ASFormatter.cpp ASEnhancer.cpp ASResource.cpp)
ADD_EXECUTABLE(qgisstyle ${ASTYLE_SRCS})
SET_TARGET_PROPERTIES(qgisstyle PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts)
