if (ENABLE_TESTS)

  if (WITH_CORE)
    add_subdirectory(src)
    add_subdirectory(bench)
  endif()

  add_subdirectory(code_layout)
endif()


