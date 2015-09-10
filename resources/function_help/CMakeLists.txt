FILE(GLOB HELP_FILES text/*)
FILE(GLOB JSON_HELP_FILES json/*)

SET(HELP_OUTPUT "${CMAKE_SOURCE_DIR}/src/core/qgsexpression_texts.cpp")

FILE(WRITE ${HELP_OUTPUT} "#include \"qgsexpression.h\"\n#include <QCoreApplication>\n\nQHash<QString, QString> QgsExpression::gFunctionHelpTexts;\n\nvoid QgsExpression::initFunctionHelp()\n{\n  if( !gFunctionHelpTexts.isEmpty() )\n    return;\n")

LIST(SORT HELP_FILES)

FOREACH(HELP_FILE ${HELP_FILES})
  STRING(REGEX REPLACE "^.*/([^/]+)$" "\\1" FN ${HELP_FILE})
  FILE(READ ${HELP_FILE} HELP)
  STRING(REPLACE "\\" "&#92;" HELP "${HELP}")
  STRING(REPLACE "\"" "\\\"" HELP "${HELP}")
  STRING(REPLACE "\n" "\\n\"\n\"" HELP "${HELP}")
  FILE(APPEND ${HELP_OUTPUT} "\n  gFunctionHelpTexts.insert( \"${FN}\", QCoreApplication::translate( \"function_help\", \"${HELP}\") );")
ENDFOREACH(HELP_FILE ${HELP_FILES})

FOREACH(HELP_FILE ${JSON_HELP_FILES})
  STRING(REGEX REPLACE "^.*/([^/]+)$" "\\1" FN ${HELP_FILE})
  STRING(REPLACE "\$" "\\$" HELP_FILE "${HELP_FILE}")
  EXEC_PROGRAM(${PYTHON_EXECUTABLE} ARGS "${CMAKE_CURRENT_SOURCE_DIR}/../process_function_template.py" "${HELP_FILE}" OUTPUT_VARIABLE HELP RETURN_VALUE RET )
  IF(NOT "${RET}" STREQUAL "0")
    MESSAGE(FATAL_ERROR "parsing function help ${HELP_FILE} failed: ${HELP}")
  ENDIF()
  STRING(REPLACE "\\" "&#92;" HELP "${HELP}")
  STRING(REPLACE "\"" "\\\"" HELP "${HELP}")
  STRING(REPLACE "\n" "\\n\"\n\"" HELP "${HELP}")
  FILE(APPEND ${HELP_OUTPUT} "\n  gFunctionHelpTexts.insert( \"${FN}\", QCoreApplication::translate( \"function_help\", \"${HELP}\") );")
ENDFOREACH(HELP_FILE ${HELP_FILES})

FILE(APPEND ${HELP_OUTPUT} "\n}\n")
