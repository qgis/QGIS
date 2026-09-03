file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(qgisprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingQgisPythonAlgorithmsTest QgisAlgorithmsTest1.py)
  ADD_PYTHON_TEST(ProcessingToolsTest ToolsTest.py)
endif()
