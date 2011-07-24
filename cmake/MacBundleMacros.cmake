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

# copy a framework, only specified archs, current version
# pass CMAKE_BUILD_TYPE to FWDEBUG
FUNCTION (COPY_FRAMEWORK FWPREFIX FWNAME FWARCHS FWDEBUG FWDEST)
	# reconstruct framework to avoid excessive copying, then deleting
	#   especially when debug variants are present
	# find current version
	# use python because pwd not working with WORKING_DIRECTORY param
	EXECUTE_PROCESS (
		COMMAND python -c "import os.path\nprint os.path.realpath(\"${FWPREFIX}/${FWNAME}.framework/Versions/Current\")"
		OUTPUT_VARIABLE FWDIRPHYS
	)
	STRING (STRIP "${FWDIRPHYS}" FWDIRPHYS)
	STRING (REGEX MATCH "[^/\n]+$" FWVER "${FWDIRPHYS}")
	EXECUTE_PROCESS (COMMAND mkdir -p "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}")
	EXECUTE_PROCESS (COMMAND ln -sfh ${FWVER} "${FWDEST}/${FWNAME}.framework/Versions/Current")
	EXECUTE_PROCESS (COMMAND ditto ${FWARCHS} "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWNAME}" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}/${FWNAME}")
	EXECUTE_PROCESS (COMMAND ln -sf Versions/Current/${FWNAME} "${FWDEST}/${FWNAME}.framework/${FWNAME}")
	IF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources")
		EXECUTE_PROCESS (COMMAND cp -Rfp "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}")
		EXECUTE_PROCESS (COMMAND ln -sfh Versions/Current/Resources "${FWDEST}/${FWNAME}.framework/Resources")
	ENDIF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources")
	# debug variants
	SET (FWD "${FWNAME}_debug")
	IF ("${FWDEBUG}" STREQUAL "Debug" AND EXISTS "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWD}")
		EXECUTE_PROCESS (COMMAND ditto ${FWARCHS} "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWD}" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}/${FWD}")
		EXECUTE_PROCESS (COMMAND ln -sf Versions/Current/${FWD} "${FWDEST}/${FWNAME}.framework/${FWD}")
		IF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/${FWD}.dSYM")
			EXECUTE_PROCESS (COMMAND ditto -X ${FWARCHS} "${FWPREFIX}/${FWNAME}.framework/${FWD}.dSYM" "${FWDEST}/${FWNAME}.framework")
		ENDIF ()
	ENDIF ()
ENDFUNCTION (COPY_FRAMEWORK)
