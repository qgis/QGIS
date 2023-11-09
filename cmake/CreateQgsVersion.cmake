# Creates version files
#  qgsversion.h that defines QGSVERSION
#  qgsversion.inc for doxygen

MACRO(CREATE_QGSVERSION)
  IF (${ENABLE_LOCAL_BUILD_SHORTCUTS})
    FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.h.out "#define QGSVERSION \"dev\"\n")
    FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.inc "PROJECT_NUMBER = \"${COMPLETE_VERSION}-${RELEASE_NAME} (dev)\"\n")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/qgsversion.h.out ${CMAKE_BINARY_DIR}/qgsversion.h)
  ELSE (${ENABLE_LOCAL_BUILD_SHORTCUTS})
    IF (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)
      FIND_PROGRAM(GITCOMMAND git PATHS c:/cygwin/bin)
      IF(GITCOMMAND)
        IF(WIN32 AND NOT CMAKE_CROSS_COMPILING)
          IF(USING_NINJA)
           SET(ARG %a)
          ELSE(USING_NINJA)
           SET(ARG %%a)
          ENDIF(USING_NINJA)
          ADD_CUSTOM_COMMAND(
            OUTPUT ${CMAKE_BINARY_DIR}/qgsversion.h ${CMAKE_BINARY_DIR}/qgsversion.inc
            COMMAND for /f \"usebackq tokens=1\" ${ARG} in "(`\"${GITCOMMAND}\" log -n1 --oneline`)" do echo \#define QGSVERSION \"${ARG}\" >${CMAKE_BINARY_DIR}/qgsversion.h.temp
            COMMAND for /f \"usebackq tokens=1\" ${ARG} in "(`\"${GITCOMMAND}\" log -n1 --oneline`)" do echo PROJECT_NUMBER = \"${COMPLETE_VERSION}-${RELEASE_NAME} \(${ARG}\)\" >${CMAKE_BINARY_DIR}/qgsversion.inc
            COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_BINARY_DIR}/qgsversion.h.temp -DDST=${CMAKE_BINARY_DIR}/qgsversion.h -P ${CMAKE_SOURCE_DIR}/cmake/CopyIfChanged.cmake
            MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/.git/index
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          )
        ELSE(WIN32 AND NOT CMAKE_CROSS_COMPILING)
          STRING(REPLACE "'" "%x27" _rn ${RELEASE_NAME})
          ADD_CUSTOM_COMMAND(
            OUTPUT ${CMAKE_BINARY_DIR}/qgsversion.h ${CMAKE_BINARY_DIR}/qgsversion.inc
            COMMAND ${GITCOMMAND} log -n1 --pretty=\#define\\ QGSVERSION\\ \\"%h\\" >${CMAKE_BINARY_DIR}/qgsversion.h.temp
            COMMAND ${GITCOMMAND} log -n1 --pretty='PROJECT_NUMBER = \"${COMPLETE_VERSION}-${_rn} \(%h\)\"' >${CMAKE_BINARY_DIR}/qgsversion.inc
            COMMAND ${GITCOMMAND} config remote.$$\(${GITCOMMAND} config branch.$$\(${GITCOMMAND} name-rev --name-only HEAD\).remote\).url | sed -e 's/^/\#define QGS_GIT_REMOTE_URL \"/' -e 's/$$/\"/' >>${CMAKE_BINARY_DIR}/qgsversion.h.temp
            COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_BINARY_DIR}/qgsversion.h.temp -DDST=${CMAKE_BINARY_DIR}/qgsversion.h -P ${CMAKE_SOURCE_DIR}/cmake/CopyIfChanged.cmake
            MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/.git/index
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          )
        ENDIF(WIN32 AND NOT CMAKE_CROSS_COMPILING)
      ELSE(GITCOMMAND)
        MESSAGE(STATUS "git marker, but no git found - version will be unknown")
        IF(NOT SHA)
          SET(SHA "unknown")
        ENDIF(NOT SHA)
        FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.h.out "#define QGSVERSION \"${SHA}\"\n")
        FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.inc "PROJECT_NUMBER = \"${COMPLETE_VERSION}-${RELEASE_NAME} (${SHA})\"\n")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/qgsversion.h.out ${CMAKE_BINARY_DIR}/qgsversion.h)
      ENDIF(GITCOMMAND)
    ELSE (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)
      IF(NOT SHA)
        SET(SHA "exported")
      ENDIF(NOT SHA)
      FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.h.out "#define QGSVERSION \"${SHA}\"\n")
      FILE(WRITE ${CMAKE_BINARY_DIR}/qgsversion.inc "PROJECT_NUMBER = \"${COMPLETE_VERSION}-${RELEASE_NAME} (${SHA})\"\n")
      execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/qgsversion.h.out ${CMAKE_BINARY_DIR}/qgsversion.h)
    ENDIF (EXISTS ${CMAKE_SOURCE_DIR}/.git/index)
  ENDIF (${ENABLE_LOCAL_BUILD_SHORTCUTS})

  ADD_CUSTOM_TARGET(version ALL DEPENDS ${CMAKE_BINARY_DIR}/qgsversion.h)
ENDMACRO(CREATE_QGSVERSION)
