if (ENABLE_TESTS)
  #############################################################
  # Compiler defines

  # This define is used for tests that need to locate the test
  # data under tests/testdata in the qgis source tree.
  # the TEST_DATA_DIR variable is set in the top level CMakeLists.txt
  add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

  add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

  # libraries
  # enable postgresql tests
  set (ENABLE_PGTEST FALSE CACHE BOOL "Enable PostgreSQL provider tests")
  if ( ENABLE_PGTEST )
    add_definitions(-DENABLE_PGTEST)
  endif()

  # because of htonl
  if (WIN32)
    set(PLATFORM_LIBRARIES wsock32)
  endif()

  # Since the tests are not actually installed, but rather
  # run directly from the build/src/tests dir we need to
  # ensure the qgis libs can be found.
  if (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  #  set (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
  endif()

  add_subdirectory(core)
  if (WITH_GUI)
    add_subdirectory(gui)
  endif()
  if (WITH_3D)
    add_subdirectory(3d)
  endif()
  if (WITH_ANALYSIS)
    add_subdirectory(analysis)
    add_subdirectory(geometry_checker)
  endif()
  if (WITH_AUTH)
    add_subdirectory(auth)
  endif()
  add_subdirectory(providers)
  if (WITH_DESKTOP)
    add_subdirectory(app)
  endif()
  add_subdirectory(native)
  if (WITH_BINDINGS)
    add_subdirectory(python)
  endif()
  if (WITH_QUICK)
    add_subdirectory(quickgui)
  endif()
  if (WITH_SERVER)
    add_subdirectory(server)
  endif()
endif()
