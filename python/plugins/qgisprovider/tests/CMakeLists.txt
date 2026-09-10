file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(qgisprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(QgisProviderProcessingToolsTest ToolsTest.py)
endif()
