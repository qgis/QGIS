IF (ENABLE_TESTS)
  #############################################################
  # Compiler defines

  # This define is used for tests that need to locate the test
  # data under tests/testdata in the qgis source tree.
  # the TEST_DATA_DIR variable is set in the top level CMakeLists.txt
  ADD_DEFINITIONS(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

  ADD_DEFINITIONS(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

  # libraries
  # enable postgresql tests
  SET (ENABLE_PGTEST FALSE CACHE BOOL "Enable PostgreSQL provider tests")
  IF ( ENABLE_PGTEST )
    ADD_DEFINITIONS(-DENABLE_PGTEST)
  ENDIF ()

  # because of htonl
  IF (WIN32)
    SET(PLATFORM_LIBRARIES wsock32)
  ENDIF (WIN32)

  # Since the tests are not actually installed, but rather
  # run directly from the build/src/tests dir we need to
  # ensure the qgis libs can be found.
  IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  #  SET (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
  ENDIF (APPLE)

  ADD_SUBDIRECTORY(core)
  IF (WITH_GUI)
    ADD_SUBDIRECTORY(gui)
  ENDIF (WITH_GUI)
  IF (WITH_3D)
    ADD_SUBDIRECTORY(3d)
  ENDIF (WITH_3D)
  ADD_SUBDIRECTORY(analysis)
  ADD_SUBDIRECTORY(auth)
  ADD_SUBDIRECTORY(providers)
  IF (WITH_DESKTOP)
    ADD_SUBDIRECTORY(app)
  ENDIF (WITH_DESKTOP)
  ADD_SUBDIRECTORY(native)
  IF (WITH_BINDINGS)
    ADD_SUBDIRECTORY(python)
  ENDIF (WITH_BINDINGS)
  ADD_SUBDIRECTORY(geometry_checker)
  IF (WITH_QUICK)
    ADD_SUBDIRECTORY(quickgui)
  ENDIF (WITH_QUICK)
  IF (WITH_SERVER)
    ADD_SUBDIRECTORY(server)
  ENDIF (WITH_SERVER)
ENDIF (ENABLE_TESTS)
