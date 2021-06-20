file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(grassprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsImageryTest Grass7AlgorithmsImageryTest.py)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsRasterTestPt1 Grass7AlgorithmsRasterTestPt1.py)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsRasterTestPt2 Grass7AlgorithmsRasterTestPt2.py)
  ADD_PYTHON_TEST(ProcessingGrass7AlgorithmsVectorTest Grass7AlgorithmsVectorTest.py)
endif()
