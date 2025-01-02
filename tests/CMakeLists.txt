if (ENABLE_TESTS)

  if (WITH_CORE)
    add_subdirectory(src)
    add_subdirectory(bench)
  endif()

  add_subdirectory(code_layout)

  configure_file(
    "${CMAKE_SOURCE_DIR}/tests/env.sh.in"
    "${CMAKE_BINARY_DIR}/tests/env.sh"
    IMMEDIATE @ONLY)
endif()


