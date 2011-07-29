SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIRECTORY}/pyspatialite)
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIRECTORY}/pyspatialite)


INCLUDE_DIRECTORIES(
  ../../src/core/spatialite/headers
  ../../src/core/spatialite/headers/spatialite

  ${PYTHON_INCLUDE_PATH}
  ${GEOS_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
)

SET(PYSPATIALITE_SRC
	src/cache.c
	src/connection.c
	src/cursor.c
	src/microprotocols.c
	src/module.c
	src/prepare_protocol.c
	src/row.c
	src/statement.c
	src/util.c
)

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${PYSPATIALITE_SRC} PROPERTIES COMPILE_FLAGS /W0)
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${PYSPATIALITE_SRC} PROPERTIES COMPILE_FLAGS -w)
ENDIF(MSVC)

ADD_DEFINITIONS(-DMODULE_NAME=\\\"spatialite.dbapi2\\\")

IF (CYGWIN OR APPLE)
  ADD_LIBRARY(pyspatialite MODULE ${PYSPATIALITE_SRC})
ELSE (CYGWIN OR APPLE)
  ADD_LIBRARY(pyspatialite SHARED ${PYSPATIALITE_SRC})
ENDIF (CYGWIN OR APPLE)

IF (NOT APPLE)
  TARGET_LINK_LIBRARIES(pyspatialite ${PYTHON_LIBRARY})
ENDIF (NOT APPLE)

TARGET_LINK_LIBRARIES(pyspatialite qgis_core)

IF (APPLE)
  SET_TARGET_PROPERTIES(pyspatialite PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
ENDIF (APPLE)

SET_TARGET_PROPERTIES(pyspatialite PROPERTIES PREFIX "" OUTPUT_NAME _spatialite)
    
IF (WIN32)
  SET_TARGET_PROPERTIES(pyspatialite PROPERTIES SUFFIX ".pyd")
ENDIF (WIN32)

FOREACH(file
  lib/__init__.py
  lib/dbapi2.py
  lib/dump.py
)
  ADD_CUSTOM_COMMAND(TARGET pyspatialite
	  POST_BUILD
	  COMMAND ${CMAKE_COMMAND} -E copy ${file} ${PYTHON_OUTPUT_DIRECTORY}/pyspatialite/${file}
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
ENDFOREACH(file)

INSTALL(TARGETS pyspatialite DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/pyspatialite")
INSTALL(FILES
	lib/__init__.py
	lib/dbapi2.py
	lib/dump.py
	DESTINATION "${PYTHON_SITE_PACKAGES_DIR}/pyspatialite")
