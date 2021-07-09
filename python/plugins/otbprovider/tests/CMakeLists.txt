file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(otbprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingOtbAlgorithmsTest OtbAlgorithmsTest.py)
endif()
