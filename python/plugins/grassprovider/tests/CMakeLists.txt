file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(grassprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingParametersTest ParametersTest.py)
endif()
