# Mac Bundle Macros

# BundleUtilities has functions to bundle and fixup libraries into an
# application package, but it's all-or-nothing and is missing some features:
# 
# - @loader_path
# - helper functions can't get install_name, just dependencies

# get the install_name of a library or framework
# regex stuff taken from GetPrerequisites

FUNCTION (GET_INSTALL_NAME LIBFILE LIBNAME OUTVAR)
    EXECUTE_PROCESS (COMMAND otool -D "${LIBFILE}" OUTPUT_VARIABLE iname_out)
    STRING (REGEX REPLACE ".*:\n" "" iname "${iname_out}")
    IF (iname)
        # trim it
        STRING (REGEX MATCH "[^\n ].*[^\n ]" iname "${iname}")
        SET (${OUTVAR} ${iname} PARENT_SCOPE)
    ENDIF (iname)
ENDFUNCTION (GET_INSTALL_NAME)

# message only if verbose makefiles

FUNCTION (MYMESSAGE MSG)
    IF (@CMAKE_VERBOSE_MAKEFILE@)
        MESSAGE (STATUS "${MSG}")
    ENDIF (@CMAKE_VERBOSE_MAKEFILE@)
ENDFUNCTION (MYMESSAGE)

# install_name_tool -change CHANGE CHANGETO CHANGEBIN

FUNCTION (INSTALLNAMETOOL_CHANGE CHANGE CHANGETO CHANGEBIN)
    MYMESSAGE ("install_name_tool -change ${CHANGE} ${CHANGETO} \"${CHANGEBIN}\"")
    EXECUTE_PROCESS (COMMAND install_name_tool -change ${CHANGE} ${CHANGETO} "${CHANGEBIN}")
ENDFUNCTION (INSTALLNAMETOOL_CHANGE)
