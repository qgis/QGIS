file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(grassprovider tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingGrassAlgorithmsImageryTest grass_algorithms_imagery_test.py)
  ADD_PYTHON_TEST(ProcessingGrassAlgorithmsRasterTestPt1 grass_algorithms_raster_test_pt1.py)
  ADD_PYTHON_TEST(ProcessingGrassAlgorithmsRasterTestPt2 grass_algorithms_raster_test_pt2.py)
  ADD_PYTHON_TEST(ProcessingGrassAlgorithmsVectorTest grass_algorithms_vector_test.py)
  ADD_PYTHON_TEST(ProcessingGrassAlgsPostgreRasterProvider grass_algorithm_postgreraster_test.py)
endif()
