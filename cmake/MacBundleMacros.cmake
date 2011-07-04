# Mac Bundle Macros

# BundleUtilities has functions to bundle and fixup libraries into an
# application package, but it's all-or-nothing and is missing some features:
# 
# - @loader_path
# - helper functions can't get install_name, just dependencies

# get the install_name of a library or framework
# regex stuff taken from GetPrerequisites

FUNCTION (GET_INSTALL_NAME LIBFILE LIBNAME OUTVAR)
    EXECUTE_PROCESS (COMMAND otool -L "${LIBFILE}" OUTPUT_VARIABLE iname_out)
    # remove 1st line, it's just path to lib file
    STRING (REGEX REPLACE ".*:\n" "" iname "${iname_out}")
    IF (iname)
        # find libname
        STRING (REGEX MATCH "[^\n\t ]*${LIBNAME}[^\n ]*" iname "${iname}")
    ENDIF (iname)
    SET (${OUTVAR} ${iname} PARENT_SCOPE)
ENDFUNCTION (GET_INSTALL_NAME)

# install_name_tool -change CHANGE CHANGETO CHANGEBIN

FUNCTION (INSTALLNAMETOOL_CHANGE CHANGE CHANGETO CHANGEBIN)
    EXECUTE_PROCESS (COMMAND install_name_tool -change ${CHANGE} ${CHANGETO} "${CHANGEBIN}")
ENDFUNCTION (INSTALLNAMETOOL_CHANGE)
