file(GLOB PY_FILES *.py)

PLUGIN_INSTALL(processing tests ${PY_FILES})

if(ENABLE_TESTS)
  include(UsePythonTest)
  ADD_PYTHON_TEST(ProcessingGeneralTest ProcessingGeneralTest.py)
  ADD_PYTHON_TEST(ProcessingGuiTest GuiTest.py)
  ADD_PYTHON_TEST(ProcessingProjectProviderGuiTest ProjectProvider.py)
  ADD_PYTHON_TEST(ProcessingScriptUtilsTest ScriptUtilsTest.py)
endif()
