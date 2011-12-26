# Mac Bundle Macros

# BundleUtilities has functions to bundle and fixup libraries into an
# application package, but it's all-or-nothing and is missing some features:
# 
# - @loader_path
# - helper functions can't get install_name, just dependencies

# get the install_name of a library or framework
# regex stuff taken from GetPrerequisites

FUNCTION (GET_INSTALL_NAME LIBFILE LIBNAME OUTVAR)
    IF (EXISTS "${LIBFILE}")
        EXECUTE_PROCESS (COMMAND otool -L "${LIBFILE}" OUTPUT_VARIABLE iname_out)
        # remove 1st line, it's just path to lib file
        STRING (REGEX REPLACE ".*:\n" "" iname "${iname_out}")
        IF (iname)
            # find libname
            STRING (REGEX MATCH "[^\n\t ]*${LIBNAME}[^\n ]*" iname "${iname}")
        ENDIF (iname)
        SET (${OUTVAR} ${iname} PARENT_SCOPE)
    ELSE ()
        SET (${OUTVAR} "" PARENT_SCOPE)
    ENDIF ()
ENDFUNCTION (GET_INSTALL_NAME)

# message only if verbose makefiles

FUNCTION (MYMESSAGE MSG)
    IF (@CMAKE_VERBOSE_MAKEFILE@)
        MESSAGE (STATUS "${MSG}")
    ENDIF (@CMAKE_VERBOSE_MAKEFILE@)
ENDFUNCTION (MYMESSAGE)

# install_name_tool -change CHANGE CHANGETO CHANGEBIN

FUNCTION (INSTALLNAMETOOL_CHANGE CHANGE CHANGETO CHANGEBIN)
    IF (EXISTS "${CHANGEBIN}" AND CHANGE AND CHANGETO)
        MYMESSAGE ("install_name_tool -change ${CHANGE} ${CHANGETO} \"${CHANGEBIN}\"")
        EXECUTE_PROCESS (COMMAND install_name_tool -change ${CHANGE} ${CHANGETO} "${CHANGEBIN}")
    ENDIF ()
ENDFUNCTION (INSTALLNAMETOOL_CHANGE)
