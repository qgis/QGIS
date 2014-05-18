SET (QGIS_PYTHON_DIR ${QGIS_DATA_DIR}/python)
SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

SET (WITH_PYSPATIALITE FALSE CACHE BOOL "Determines whether PYSPATIALITE should be built")
IF (WITH_PYSPATIALITE)
  ADD_SUBDIRECTORY(pyspatialite)
ENDIF (WITH_PYSPATIALITE)

MACRO(EXT_PYLIB lib)
  STRING(TOUPPER ${lib} ulib)
  SET (WITH_INTERNAL_${ulib} TRUE CACHE BOOL "Determines whether python ${lib} should be included")
  IF(WITH_INTERNAL_${ulib})
    INSTALL(DIRECTORY ${lib} DESTINATION "${QGIS_PYTHON_DIR}")

    IF(WITH_STAGED_PLUGINS)
      ADD_CUSTOM_TARGET(py${lib} ALL)

      FILE(GLOB_RECURSE files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${lib}/*)
      ADD_CUSTOM_COMMAND(TARGET py${lib}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${lib} "${PYTHON_OUTPUT_DIRECTORY}/${lib}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${files}
      )

      IF(WITH_PY_COMPILE)
        ADD_CUSTOM_TARGET(pycompile-py${lib} ALL
          COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}/${lib}"
          WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
          COMMENT "Byte-compiling staged extension ${lib}..."
          DEPENDS py${lib}
        )
      ENDIF(WITH_PY_COMPILE)
    ENDIF(WITH_STAGED_PLUGINS)
  ENDIF(WITH_INTERNAL_${ulib})
ENDMACRO(EXT_PYLIB lib)

FOREACH(pkg httplib2 jinja2 markupsafe owslib pygments dateutil pytz)
  EXT_PYLIB(${pkg})
ENDFOREACH(pkg)

SET (WITH_INTERNAL_SIX TRUE CACHE BOOL "Determines whether python six should be included")
IF(WITH_INTERNAL_SIX)
  INSTALL(FILES six.py DESTINATION "${QGIS_PYTHON_DIR}")

  IF(WITH_STAGED_PLUGINS)
    ADD_CUSTOM_TARGET(pysix ALL)

    ADD_CUSTOM_COMMAND(TARGET pysix
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy six.py "${PYTHON_OUTPUT_DIRECTORY}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS six.py
    )

    IF(WITH_PY_COMPILE)
      ADD_CUSTOM_TARGET(pycompile-pysix ALL
        COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}/${lib}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Byte-compiling staged extension ${lib}..."
        DEPENDS pysix
      )
    ENDIF(WITH_PY_COMPILE)
  ENDIF(WITH_STAGED_PLUGINS)
ENDIF(WITH_INTERNAL_SIX)

