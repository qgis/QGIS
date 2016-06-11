FILE(GLOB PY_FILES *.py)
FILE(GLOB TEST_DATA_FILES data/*)

PLUGIN_INSTALL(processing tests ${PY_FILES})
PLUGIN_INSTALL(processing tests/data ${TEST_DATA_FILES})

IF(ENABLE_TESTS)
  INCLUDE(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingParametersTest ParametersTest.py)
  ADD_PYTHON_TEST(ProcessingQgisAlgorithmsTest QgisAlgorithmsTest.py)
  ADD_PYTHON_TEST(ProcessingGdalAlgorithmsTest GdalAlgorithmsTest.py)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsImageryTest Grass7AlgorithmsImageryTest.py)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsRasterTest Grass7AlgorithmsRasterTest.py)
ENDIF(ENABLE_TESTS)
