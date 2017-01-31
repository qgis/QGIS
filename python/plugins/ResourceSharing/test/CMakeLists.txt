FILE(GLOB PY_FILES *.py)
FILE(GLOB TEST_DATA_FILES testdata/points.* testdata/table.dbf)

PLUGIN_INSTALL(ResourceSharing test ${PY_FILES})
PLUGIN_INSTALL(ResourceSharing test/testdata ${TEST_DATA_FILES})

IF(ENABLE_TESTS)

    FOREACH(dir
        gui
        data
        data/collections
        data/collections/test_collection
        data/collections/test_collection/preview
        data/collections/test_collection/svg
        ￼data/collections/test_collection/processing
        data/collections/test_collection/image
        data/collections/test_collection/symbol
        data/collections/test_collection/style
        repository_handler
        resource_handler
    )
      FILE(GLOB _items ${dir}/*)
      SET(_files)
      FOREACH(_item ${_items})
        IF(NOT IS_DIRECTORY ${_item})
          LIST(APPEND _files ${_item})
        ENDIF(NOT IS_DIRECTORY ${_item})
      ENDFOREACH(_item)
      PLUGIN_INSTALL(ResourceSharing test/${dir} ${_files})
    ENDFOREACH(dir)


  INCLUDE(UsePythonTest)
  ADD_PYTHON_TEST(ResourceSharingInitTest test_init.py)
  ADD_PYTHON_TEST(ResourceSharingCollectionManagerTest test_collections_manager.py)
  ADD_PYTHON_TEST(ResourceSharingSymbolXmlExtractorTest test_symbol_xml_extractor.py)
ENDIF(ENABLE_TESTS)
