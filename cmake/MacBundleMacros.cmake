# QGIS Mac Bundle Macros

# BundleUtilities has functions to bundle and fixup libraries into an
# application package, but it's all-or-nothing and is missing some features:
#
# - @loader_path
# - helper functions can't get install_name, just dependencies

# the following cmakecache vars must be set, redefine them
# with config-file substitutions in install-run scripts:
#
# CPACK_PACKAGE_VERSION_MAJOR, CPACK_PACKAGE_VERSION_MINOR
# CMAKE_INSTALL_PREFIX, CMAKE_VERBOSE_MAKEFILE, CMAKE_BUILD_TYPE
# CMAKE_OSX_ARCHITECTURES, OSX_HAVE_LOADERPATH
# QGIS_APP_NAME
# QGIS_MACAPP_PREFIX
# QGIS_*_SUBDIR, QGIS_*_SUBDIR_REV
# WITH_*

# this file must only be included after target installation is complete

# message only if verbose makefiles

CMAKE_POLICY (SET CMP0053 OLD)


FUNCTION (MYMESSAGE MSG)
    IF (@CMAKE_VERBOSE_MAKEFILE@)
        MESSAGE (STATUS "${MSG}")
    ENDIF (@CMAKE_VERBOSE_MAKEFILE@)
ENDFUNCTION (MYMESSAGE)

# get the install_name of a library or framework
# regex stuff taken from GetPrerequisites

FUNCTION (GET_INSTALL_NAME LIBFILE LIBNAME OUTVAR)
    IF (EXISTS "${LIBFILE}")
        EXECUTE_PROCESS (COMMAND otool -L "${LIBFILE}" OUTPUT_VARIABLE iname_out)
        # remove 1st line, it's just path to lib file
        STRING (REGEX REPLACE ".*:\n" "" iname "${iname_out}")
        IF (iname)
            # find libname
            STRING (REGEX MATCH "[^\n\t ]*${LIBNAME}[^\n]*" iname "${iname}")
            STRING (REGEX REPLACE " \\(compatibility version .*, current version .*\\)" "" iname "${iname}")
        ENDIF (iname)
        SET (${OUTVAR} ${iname} PARENT_SCOPE)
    ELSE ()
        SET (${OUTVAR} "" PARENT_SCOPE)
    ENDIF ()
ENDFUNCTION (GET_INSTALL_NAME)

# install_name_tool -change CHANGE CHANGETO CHANGEBIN

FUNCTION (INSTALLNAMETOOL_CHANGE CHANGE CHANGETO CHANGEBIN)
    IF (EXISTS "${CHANGEBIN}" AND CHANGE AND CHANGETO)
        # ensure CHANGEBIN is writable by user, e.g. Homebrew binaries are installed non-writable
        EXECUTE_PROCESS (COMMAND chmod u+w "${CHANGEBIN}")
        EXECUTE_PROCESS (COMMAND install_name_tool -change ${CHANGE} ${CHANGETO} "${CHANGEBIN}")
        # if that didn't work, try a symlink-resolved id
        # (some package systems, like Homebrew, heavily use symlinks; and, inter-package builds, like plugins,
        #  may point to the resolved location instead of the 'public' symlink installed to prefixes like /usr/local)
        get_filename_component(_chgreal ${CHANGE} REALPATH)
        EXECUTE_PROCESS (COMMAND install_name_tool -change ${_chgreal} ${CHANGETO} "${CHANGEBIN}")
    ENDIF ()
ENDFUNCTION (INSTALLNAMETOOL_CHANGE)

# copy a framework, only specified archs, current version, debug dep on CMAKE_BUILD_TYPE

FUNCTION (COPY_FRAMEWORK FWPREFIX FWNAME FWDEST)
    # reconstruct framework to avoid excessive copying, then deleting
    #   especially when debug variants are present
    # find current version
    # use python because pwd not working with WORKING_DIRECTORY param
    EXECUTE_PROCESS (
        COMMAND python -c "import os.path\nprint os.path.realpath(\"${FWPREFIX}/${FWNAME}.framework/Versions/Current\")"
        OUTPUT_VARIABLE FWDIRPHYS
    )
    STRING (STRIP "${FWDIRPHYS}" FWDIRPHYS)
    IF (IS_DIRECTORY "${FWDIRPHYS}")
        STRING (REGEX MATCH "[^/\n]+$" FWVER "${FWDIRPHYS}")
        EXECUTE_PROCESS (COMMAND mkdir -p "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}")
        EXECUTE_PROCESS (COMMAND ln -sfh ${FWVER} "${FWDEST}/${FWNAME}.framework/Versions/Current")
        EXECUTE_PROCESS (COMMAND ditto ${QARCHS} "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWNAME}" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}/${FWNAME}")
        EXECUTE_PROCESS (COMMAND ln -sf Versions/Current/${FWNAME} "${FWDEST}/${FWNAME}.framework/${FWNAME}")
        IF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources")
            EXECUTE_PROCESS (COMMAND cp -Rfp "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}")
            EXECUTE_PROCESS (COMMAND ln -sfh Versions/Current/Resources "${FWDEST}/${FWNAME}.framework/Resources")
        ENDIF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/Resources")
        # ensure writable by user, e.g. Homebrew frameworks are installed non-writable
        EXECUTE_PROCESS (COMMAND chmod -R u+w "${FWDEST}/${FWNAME}.framework")
        EXECUTE_PROCESS (COMMAND install_name_tool -id "${ATEXECUTABLE}/${QGIS_FW_SUBDIR}/${FWNAME}" "${FWDEST}/${FWNAME}.framework/${FWNAME}")
        # debug variants
        SET (FWD "${FWNAME}_debug")
        IF ("${FWDEBUG}" STREQUAL "Debug" AND EXISTS "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWD}")
            EXECUTE_PROCESS (COMMAND ditto ${QARCHS} "${FWPREFIX}/${FWNAME}.framework/Versions/${FWVER}/${FWD}" "${FWDEST}/${FWNAME}.framework/Versions/${FWVER}/${FWD}")
            EXECUTE_PROCESS (COMMAND ln -sf Versions/Current/${FWD} "${FWDEST}/${FWNAME}.framework/${FWD}")
            IF (IS_DIRECTORY "${FWPREFIX}/${FWNAME}.framework/${FWD}.dSYM")
                EXECUTE_PROCESS (COMMAND ditto -X ${QARCHS} "${FWPREFIX}/${FWNAME}.framework/${FWD}.dSYM" "${FWDEST}/${FWNAME}.framework")
            ENDIF ()
        ENDIF ()
    ENDIF ()
ENDFUNCTION (COPY_FRAMEWORK)

# update a library path in all QGIS binary files
#  if dylib, change LIBFROM to LIBTO as is
#  else assumes it's a framework, change LIBFROM to LIBTO.framework/LIBTO

FUNCTION (UPDATEQGISPATHS LIBFROM LIBTO)
    IF (LIBFROM)
        STRING (REGEX MATCH "\\.dylib$" ISLIB "${LIBTO}")
        IF (ISLIB)
            SET (LIBPOST "${LIBTO}")
            SET (LIBMID "${QGIS_LIB_SUBDIR}")
        ElSE ()
            SET (LIBPOST "${LIBTO}.framework/${LIBTO}")
            SET (LIBMID "${QGIS_FW_SUBDIR}")
        ENDIF ()
        SET (LIB_CHG_TO "${ATEXECUTABLE}/${LIBMID}/${LIBPOST}")
        # app - always @executable_path
        INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QAPPDIR}/${QGIS_APP_NAME}")
        # qgis helper apps - don't link anything else than Qt/Qgis
        FOREACH (QA ${QGAPPLIST})
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QBINDIR}/${QA}.app/Contents/MacOS/${QA}")
        ENDFOREACH (QA)
        # qgis-mapserver
        IF (${WITH_SERVER})
            IF (${OSX_HAVE_LOADERPATH})
                SET (LIB_CHG_TO "${ATEXECUTABLE}/${QGIS_CGIBIN_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
            ENDIF ()
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QCGIDIR}/qgis_mapserv.fcgi")
        ENDIF ()
        # libs
        IF (${OSX_HAVE_LOADERPATH})
            # bundled frameworks can use short relative path
            IF (ISLIB)
                SET (LIB_CHG_TO "${ATLOADER}/../../../${QGIS_FW_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
            ElSE ()
                SET (LIB_CHG_TO "${ATLOADER}/../../../${LIBPOST}")
            ENDIF ()
        ENDIF ()
        FOREACH (QL ${QGFWLIST})
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QFWDIR}/${QL}.framework/${QL}")
        ENDFOREACH (QL)
        # non-framework qgis libs
        IF (${OSX_HAVE_LOADERPATH})
            SET (LIB_CHG_TO "${ATLOADER}/${QGIS_LIB_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
        ENDIF ()
        FOREACH (QL ${QGLIBLIST})
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QLIBDIR}/${QL}")
        ENDFOREACH (QL)
        # crssync
        IF (${OSX_HAVE_LOADERPATH})
            SET (LIB_CHG_TO "${ATEXECUTABLE}/${QGIS_LIBEXEC_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
        ENDIF ()
        INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QLIBXDIR}/crssync")
        # GRASS libexec stuff
        FOREACH (QG ${QGRASSEXECLIST})
           IF (EXISTS "${QLIBXDIR}/grass/${QG}")
              IF (${OSX_HAVE_LOADERPATH})
                  SET (LIB_CHG_TO "${ATLOADER}/../../${QGIS_LIBEXEC_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
              ENDIF ()
              INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QLIBXDIR}/grass/${QG}")
           ENDIF ()
        ENDFOREACH (QG)
        # plugins
        IF (${OSX_HAVE_LOADERPATH})
            SET (LIB_CHG_TO "${ATLOADER}/${QGIS_PLUGIN_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
        ENDIF ()
        FOREACH (QP ${QGPLUGLIST})
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QP}")
        ENDFOREACH (QP)
        # qgis python
        IF (${OSX_HAVE_LOADERPATH})
            SET (LIB_CHG_TO "${ATLOADER}/../../${QGIS_DATA_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
        ENDIF ()
        FOREACH (PG ${QGPYLIST})
            INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${PG}")
        ENDFOREACH (PG)
        # bin - nothing yet
        #IF (${OSX_HAVE_LOADERPATH})
        #    SET (LIB_CHG_TO "${ATLOADER}/${QGIS_BIN_SUBDIR_REV}/${LIBMID}/${LIBPOST}")
        #ENDIF ()
        #FOREACH (PB ...)
        #    INSTALLNAMETOOL_CHANGE ("${LIBFROM}" "${LIB_CHG_TO}" "${QBINDIR}/${PB}")
        #ENDFOREACH (PB)
    ENDIF (LIBFROM)
ENDFUNCTION (UPDATEQGISPATHS)


# Find directory path for a known Python module (or package) directory or file name
# see: PYTHON_MODULE_PATHS in 0vars.cmake.in
FUNCTION (PYTHONMODULEDIR MOD_NAME OUTVAR)
    FOREACH (MOD_PATH ${PYTHON_MODULE_PATHS})
        IF (EXISTS "${MOD_PATH}/${MOD_NAME}")
            SET (${OUTVAR} "${MOD_PATH}" PARENT_SCOPE)
            RETURN()
        ENDIF()
    ENDFOREACH (MOD_PATH)
    SET (${OUTVAR} "" PARENT_SCOPE)
ENDFUNCTION (PYTHONMODULEDIR)


SET (ATEXECUTABLE "@executable_path")
SET (ATLOADER "@loader_path")
SET (Q_FWVER ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})

# install destinations
SET (QAPPDIRC "$ENV{DESTDIR}${QGIS_MACAPP_PREFIX}")
SET (QAPPDIR "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
SET (QFWDIR "${QAPPDIR}/${QGIS_FW_SUBDIR}")
SET (QBINDIR "${QAPPDIR}/${QGIS_BIN_SUBDIR}")
SET (QCGIDIR "${QAPPDIR}/${QGIS_CGIBIN_SUBDIR}")
SET (QLIBDIR "${QAPPDIR}/${QGIS_LIB_SUBDIR}")
SET (QLIBXDIR "${QAPPDIR}/${QGIS_LIBEXEC_SUBDIR}")
SET (QDATADIR "${QAPPDIR}/${QGIS_DATA_SUBDIR}")
SET (QPLUGDIR "${QAPPDIR}/${QGIS_PLUGIN_SUBDIR}")
SET (QGISPYDIR "${QAPPDIR}/${QGIS_DATA_SUBDIR}/python")

# build arches
SET (QARCHS "")
FOREACH (QARCH ${CMAKE_OSX_ARCHITECTURES})
    SET (QARCHS ${QARCHS} "--arch" "${QARCH}")
ENDFOREACH (QARCH)

# common file lists
FILE (GLOB QGFWLIST RELATIVE "${QFWDIR}" "${QFWDIR}/qgis*.framework")
# for some reason, REPLACE is stripping list seps
STRING(REPLACE ".framework" ";" QGFWLIST ${QGFWLIST})
# don't collect any library symlinks, limit to versioned libs
SET (Q_LIBVER ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})
FILE (GLOB QGLIBLIST  RELATIVE "${QLIBDIR}" "${QLIBDIR}/libqgis*.dylib")
FILE (GLOB QGPLUGLIST "${QPLUGDIR}/*.so")
FILE (GLOB QGPYLIST "${QGISPYDIR}/qgis/*.so")
FILE (GLOB QGAPPLIST RELATIVE "${QBINDIR}" "${QBINDIR}/q*.app")
FILE (GLOB QGRASSEXECLIST RELATIVE "${QLIBXDIR}/grass" "${QLIBXDIR}/grass/*/*")
STRING(REPLACE ".app" ";" QGAPPLIST ${QGAPPLIST})
