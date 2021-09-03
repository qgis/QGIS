file(GLOB PY_FILES *.py)
file(GLOB TEST_DATA_FILES testdata/points.* testdata/table.dbf)

PLUGIN_INSTALL(sagaprovider tests ${PY_FILES})
PLUGIN_INSTALL(sagaprovider tests/testdata ${TEST_DATA_FILES})

if(ENABLE_TESTS)
  # enable SAGA tests
  set (ENABLE_SAGA_TESTS FALSE CACHE BOOL "Enable SAGA tests")
  if ( ENABLE_SAGA_TESTS )
    add_definitions(-DENABLE_SAGA_TESTS)
  endif()

  include(UsePythonTest)
  if(ENABLE_SAGA_TESTS)
    ADD_PYTHON_TEST(ProcessingSagaAlgorithmsTest SagaAlgorithmsTest.py)
  endif()
endif()
