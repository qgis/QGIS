

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

ADD_SUBDIRECTORY(../../../src/astyle qgisstyle)

ENABLE_TESTING()

ADD_TEST(qgis_indentation ${CMAKE_SOURCE_DIR}/../../../scripts/verify-indentation.sh)
ADD_TEST(qgis_spelling ${CMAKE_SOURCE_DIR}/../../../scripts/spell_check/spell_test.sh)
