#######################################################################################################
#  macro to detect osg version and setup variables accordingly
#######################################################################################################
MACRO(DETECT_OSG_VERSION)

    OPTION(APPEND_OPENSCENEGRAPH_VERSION "Append the OSG version number to the osgPlugins directory" ON)
  
    # detect if osgversion can be found
    FIND_PROGRAM(OSG_VERSION_EXE NAMES osgversion)
    IF(OSG_VERSION_EXE AND NOT OPENSCENEGRAPH_MAJOR_VERSION AND NOT OPENSCENEGRAPH_MINOR_VERSION AND NOT OPENSCENEGRAPH_PATCH_VERSION)
        #MESSAGE("OSGVERSION IS AT ${OSG_VERSION_EXE}")
        # get parameters out of the osgversion
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} --major-number OUTPUT_VARIABLE OPENSCENEGRAPH_MAJOR_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} --minor-number OUTPUT_VARIABLE OPENSCENEGRAPH_MINOR_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} --patch-number OUTPUT_VARIABLE OPENSCENEGRAPH_PATCH_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} Matrix::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_MATRIX OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} Plane::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_PLANE OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} BoundingSphere::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_BOUNDINGSPHERE OUTPUT_STRIP_TRAILING_WHITESPACE)
        EXECUTE_PROCESS(COMMAND ${OSG_VERSION_EXE} BoundingBox::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_BOUNDINGBOX OUTPUT_STRIP_TRAILING_WHITESPACE)

        # setup version numbers if we have osgversion
        SET(OPENSCENEGRAPH_MAJOR_VERSION "${OPENSCENEGRAPH_MAJOR_VERSION}" CACHE STRING "OpenSceneGraph major version number")
        SET(OPENSCENEGRAPH_MINOR_VERSION "${OPENSCENEGRAPH_MINOR_VERSION}" CACHE STRING "OpenSceneGraph minor version number")
        SET(OPENSCENEGRAPH_PATCH_VERSION "${OPENSCENEGRAPH_PATCH_VERSION}" CACHE STRING "OpenSceneGraph patch version number")
        SET(OPENSCENEGRAPH_SOVERSION "${OPENSCENEGRAPH_SOVERSION}" CACHE STRING "OpenSceneGraph so version number")
    
        # just debug info
        #MESSAGE(STATUS "Detected OpenSceneGraph v${OPENSCENEGRAPH_VERSION}.")

        # setup float and double definitions
        IF(OSG_USE_FLOAT_MATRIX MATCHES "float")
            ADD_DEFINITIONS(-DOSG_USE_FLOAT_MATRIX)
        ENDIF(OSG_USE_FLOAT_MATRIX MATCHES "float")
        IF(OSG_USE_FLOAT_PLANE MATCHES "float")
            ADD_DEFINITIONS(-DOSG_USE_FLOAT_PLANE)
        ENDIF(OSG_USE_FLOAT_PLANE MATCHES "float")
        IF(OSG_USE_FLOAT_BOUNDINGSPHERE MATCHES "double")
            ADD_DEFINITIONS(-DOSG_USE_DOUBLE_BOUNDINGSPHERE)
        ENDIF(OSG_USE_FLOAT_BOUNDINGSPHERE MATCHES "double")
        IF(OSG_USE_FLOAT_BOUNDINGBOX MATCHES "double")
            ADD_DEFINITIONS(-DOSG_USE_DOUBLE_BOUNDINGBOX)
        ENDIF(OSG_USE_FLOAT_BOUNDINGBOX MATCHES "double")

    ENDIF(OSG_VERSION_EXE AND NOT OPENSCENEGRAPH_MAJOR_VERSION AND NOT OPENSCENEGRAPH_MINOR_VERSION AND NOT OPENSCENEGRAPH_PATCH_VERSION)
  
    #Initialize the version numbers to being empty.  If they were set by osgversion, they will be left alone
  SET(OPENSCENEGRAPH_MAJOR_VERSION "" CACHE STRING "OpenSceneGraph major version number")
    SET(OPENSCENEGRAPH_MINOR_VERSION "" CACHE STRING "OpenSceneGraph minor version number")
    SET(OPENSCENEGRAPH_PATCH_VERSION "" CACHE STRING "OpenSceneGraph patch version number")
    SET(OPENSCENEGRAPH_SOVERSION "" CACHE STRING "OpenSceneGraph so version number")
  
    if (OPENSCENEGRAPH_MAJOR_VERSION AND NOT OPENSCENEGRAPH_MINOR_VERSION STREQUAL "" AND NOT OPENSCENEGRAPH_PATCH_VERSION STREQUAL "")
    SET(OPENSCENEGRAPH_VERSION ${OPENSCENEGRAPH_MAJOR_VERSION}.${OPENSCENEGRAPH_MINOR_VERSION}.${OPENSCENEGRAPH_PATCH_VERSION})
  else (OPENSCENEGRAPH_MAJOR_VERSION AND NOT OPENSCENEGRAPH_MINOR_VERSION STREQUAL "" AND NOT OPENSCENEGRAPH_PATCH_VERSION STREQUAL "")
    #MESSAGE("osgversion was found at ${OSG_VERSION_EXE} but failed to run")
    SET(OPENSCENEGRAPH_VERSION)
  endif (OPENSCENEGRAPH_MAJOR_VERSION AND NOT OPENSCENEGRAPH_MINOR_VERSION STREQUAL "" AND NOT OPENSCENEGRAPH_PATCH_VERSION STREQUAL "")
  
  MARK_AS_ADVANCED(OPENSCENEGRAPH_VERSION)


    IF (APPEND_OPENSCENEGRAPH_VERSION AND OPENSCENEGRAPH_VERSION)
        SET(OSG_PLUGINS "osgPlugins-${OPENSCENEGRAPH_VERSION}"  CACHE STRING "" FORCE)
        MESSAGE(STATUS "Plugins will be installed under osgPlugins-${OPENSCENEGRAPH_VERSION} directory.")
  else (APPEND_OPENSCENEGRAPH_VERSION AND OPENSCENEGRAPH_VERSION)
    SET(OSG_PLUGINS  CACHE STRING "" FORCE)
    ENDIF(APPEND_OPENSCENEGRAPH_VERSION AND OPENSCENEGRAPH_VERSION)
  
  MARK_AS_ADVANCED(OSG_PLUGINS)
  
  #MESSAGE("OSG_PLUGINS=${OSG_PLUGINS}")

ENDMACRO(DETECT_OSG_VERSION)



#######################################################################################################
#  macro for linking libraries that come from Findxxxx commands, so there is a variable that contains the
#  full path of the library name. in order to differentiate release and debug, this macro get the
#  NAME of the variables, so the macro gets as arguments the target name and the following list of parameters
#  is intended as a list of variable names each one containing  the path of the libraries to link to
#  The existence of a variable name with _DEBUG appended is tested and, in case it' s value is used
#  for linking to when in debug mode
#  the content of this library for linking when in debugging
#######################################################################################################


MACRO(LINK_WITH_VARIABLES TRGTNAME)
    FOREACH(varname ${ARGN})
        IF(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${${varname}}" debug "${${varname}_DEBUG}")
        ELSE(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} "${${varname}}" )
        ENDIF(${varname}_DEBUG)
    ENDFOREACH(varname)
ENDMACRO(LINK_WITH_VARIABLES TRGTNAME)

MACRO(LINK_INTERNAL TRGTNAME)
    IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4)
        TARGET_LINK_LIBRARIES(${TRGTNAME} ${ARGN})
    ELSE(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4)
        FOREACH(LINKLIB ${ARGN})
            IF(MSVC AND OSG_MSVC_VERSIONED_DLL)
                #when using versioned names, the .dll name differ from .lib name, there is a problem with that:
                #CMake 2.4.7, at least seem to use PREFIX instead of IMPORT_PREFIX  for computing linkage info to use into projects,
                # so we full path name to specify linkage, this prevent automatic inferencing of dependencies, so we add explicit depemdencies
                #to library targets used
                TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${OUTPUT_LIBDIR}/${LINKLIB}${CMAKE_RELEASE_POSTFIX}.lib" debug "${OUTPUT_LIBDIR}/${LINKLIB}${CMAKE_DEBUG_POSTFIX}.lib")
                ADD_DEPENDENCIES(${TRGTNAME} ${LINKLIB})
            ELSE(MSVC AND OSG_MSVC_VERSIONED_DLL)
                TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${LINKLIB}${CMAKE_RELEASE_POSTFIX}" debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
            ENDIF(MSVC AND OSG_MSVC_VERSIONED_DLL)
        ENDFOREACH(LINKLIB)
    ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4)
ENDMACRO(LINK_INTERNAL TRGTNAME)

MACRO(LINK_EXTERNAL TRGTNAME)
    FOREACH(LINKLIB ${ARGN})
        TARGET_LINK_LIBRARIES(${TRGTNAME} "${LINKLIB}" )
    ENDFOREACH(LINKLIB)
ENDMACRO(LINK_EXTERNAL TRGTNAME)


#######################################################################################################
#  macro for common setup of core libraries: it links OPENGL_LIBRARIES in undifferentiated mode
#######################################################################################################

MACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)
    LINK_EXTERNAL(${CORELIB_NAME} ${OPENGL_LIBRARIES})
    LINK_WITH_VARIABLES(${CORELIB_NAME} OPENTHREADS_LIBRARY)
    IF(OSGEARTH_SONAMES)
      SET_TARGET_PROPERTIES(${CORELIB_NAME} PROPERTIES VERSION ${OSGEARTH_VERSION} SOVERSION ${OSGEARTH_SOVERSION})
    ENDIF(OSGEARTH_SONAMES)
ENDMACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)


#######################################################################################################
#  macro for common setup of plugins, examples and applications it expect some variables to be set:
#  either within the local CMakeLists or higher in hierarchy
#  TARGET_NAME is the name of the folder and of the actually .exe or .so or .dll
#  TARGET_TARGETNAME  is the name of the target , this get buit out of a prefix, if present and TARGET_TARGETNAME
#  TARGET_SRC  are the sources of the target
#  TARGET_H are the eventual headers of the target
#  TARGET_LIBRARIES are the libraries to link to that are internal to the project and have d suffix for debug
#  TARGET_EXTERNAL_LIBRARIES are external libraries and are not differentiated with d suffix
#  TARGET_LABEL is the label IDE should show up for targets
##########################################################################################################

MACRO(SETUP_LINK_LIBRARIES)
    ######################################################################
    #
    # This set up the libraries to link to, it assumes there are two variable: one common for a group of examples or plagins
    # kept in the variable TARGET_COMMON_LIBRARIES and an example or plugin specific kept in TARGET_ADDED_LIBRARIES
    # they are combined in a single list checked for unicity
    # the suffix ${CMAKE_DEBUG_POSTFIX} is used for differentiating optimized and debug
    #
    # a second variable TARGET_EXTERNAL_LIBRARIES hold the list of  libraries not differentiated between debug and optimized
    ##################################################################################
    SET(TARGET_LIBRARIES ${TARGET_COMMON_LIBRARIES})

    FOREACH(LINKLIB ${TARGET_ADDED_LIBRARIES})
      SET(TO_INSERT TRUE)
      FOREACH (value ${TARGET_COMMON_LIBRARIES})
            IF (${value} STREQUAL ${LINKLIB})
                  SET(TO_INSERT FALSE)
            ENDIF (${value} STREQUAL ${LINKLIB})
        ENDFOREACH (value ${TARGET_COMMON_LIBRARIES})
      IF(TO_INSERT)
          LIST(APPEND TARGET_LIBRARIES ${LINKLIB})
      ENDIF(TO_INSERT)
    ENDFOREACH(LINKLIB)

#    FOREACH(LINKLIB ${TARGET_LIBRARIES})
#            TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} optimized ${LINKLIB} debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
#    ENDFOREACH(LINKLIB)
    LINK_INTERNAL(${TARGET_TARGETNAME} ${TARGET_LIBRARIES})

    FOREACH(LINKLIB ${TARGET_EXTERNAL_LIBRARIES})
            TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} ${LINKLIB})
    ENDFOREACH(LINKLIB)
        IF(TARGET_LIBRARIES_VARS)
            LINK_WITH_VARIABLES(${TARGET_TARGETNAME} ${TARGET_LIBRARIES_VARS})
        ENDIF(TARGET_LIBRARIES_VARS)
ENDMACRO(SETUP_LINK_LIBRARIES)

############################################################################################
# this is the common set of command for all the plugins


MACRO(SETUP_PLUGIN PLUGIN_NAME)

    SET(TARGET_NAME ${PLUGIN_NAME} )

    #MESSAGE("in -->SETUP_PLUGIN<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")

    SOURCE_GROUP( "Header Files" FILES ${TARGET_H} )

    ## we have set up the target label and targetname by taking into account global prfix (osgdb_)

    IF(NOT TARGET_TARGETNAME)
            SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    ENDIF(NOT TARGET_TARGETNAME)
    IF(NOT TARGET_LABEL)
            SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    ENDIF(NOT TARGET_LABEL)

# here we use the command to generate the library

    IF   (DYNAMIC_OSGEARTH)
        ADD_LIBRARY(${TARGET_TARGETNAME} MODULE ${TARGET_SRC} ${TARGET_H})
    ELSE (DYNAMIC_OSGEARTH)
        ADD_LIBRARY(${TARGET_TARGETNAME} STATIC ${TARGET_SRC} ${TARGET_H})
    ENDIF(DYNAMIC_OSGEARTH)

    #not sure if needed, but for plugins only msvc need the d suffix
    IF(NOT MSVC)
      IF(NOT UNIX)
           SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_POSTFIX "")
      ENDIF(NOT UNIX)
    ENDIF(NOT MSVC)
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")

    SETUP_LINK_LIBRARIES()

#the installation path are differentiated for win32 that install in bib versus other architecture that install in lib${LIB_POSTFIX}/${VPB_PLUGINS}
    IF(WIN32)
        INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib/${OSG_PLUGINS} LIBRARY DESTINATION bin/${OSG_PLUGINS} )

    #Install to the OSG_DIR as well
    IF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)
      INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION ${OSG_DIR}/bin/${OSG_PLUGINS} LIBRARY DESTINATION ${OSG_DIR}/bin/${OSG_PLUGINS} )
    ENDIF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)

    ELSE(WIN32)
        INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} LIBRARY DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} )

    #Install to the OSG_DIR as well
    IF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)
      INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION ${OSG_DIR}/bin LIBRARY DESTINATION lib${LIB_POSTFIX}/bin)
    ENDIF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)

    ENDIF(WIN32)

#finally, set up the solution folder -gw
    SET_PROPERTY(TARGET ${TARGET_TARGETNAME} PROPERTY FOLDER "Plugins")

ENDMACRO(SETUP_PLUGIN)


#################################################################################################################
# this is the macro for example and application setup
###########################################################

MACRO(SETUP_EXE IS_COMMANDLINE_APP)
    #MESSAGE("in -->SETUP_EXE<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")
    IF(NOT TARGET_TARGETNAME)
            SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    ENDIF(NOT TARGET_TARGETNAME)
    IF(NOT TARGET_LABEL)
            SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    ENDIF(NOT TARGET_LABEL)

    IF(${IS_COMMANDLINE_APP})

        ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC} ${TARGET_H})

    ELSE(${IS_COMMANDLINE_APP})

        IF(APPLE)
            # SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VIRTUALPLANETBUILDER_MAJOR_VERSION}.${VIRTUALPLANETBUILDER_MINOR_VERSION}.${VIRTUALPLANETBUILDER_PATCH_VERSION}")
            # Short Version is the "marketing version". It is the version
            # the user sees in an information panel.
            SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${OSGEARTH_MAJOR_VERSION}.${OSGEARTH_MINOR_VERSION}.${OSGEARTH_PATCH_VERSION}")
            # Bundle version is the version the OS looks at.
            SET(MACOSX_BUNDLE_BUNDLE_VERSION "${OSGEARTH_MAJOR_VERSION}.${OSGEARTH_MINOR_VERSION}.${OSGEARTH__PATCH_VERSION}")
            SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.osgearth.${TARGET_TARGETNAME}" )
            SET(MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_NAME}" )
            # SET(MACOSX_BUNDLE_ICON_FILE "myicon.icns")
            # SET(MACOSX_BUNDLE_COPYRIGHT "")
            # SET(MACOSX_BUNDLE_INFO_STRING "Info string, localized?")
        ENDIF(APPLE)

        IF(WIN32)
            IF (REQUIRE_WINMAIN_FLAG)
                SET(PLATFORM_SPECIFIC_CONTROL WIN32)
            ENDIF(REQUIRE_WINMAIN_FLAG)
        ENDIF(WIN32)

        IF(APPLE)
            IF(VPB_BUILD_APPLICATION_BUNDLES)
                SET(PLATFORM_SPECIFIC_CONTROL MACOSX_BUNDLE)
            ENDIF(VPB_BUILD_APPLICATION_BUNDLES)
        ENDIF(APPLE)

        ADD_EXECUTABLE(${TARGET_TARGETNAME} ${PLATFORM_SPECIFIC_CONTROL} ${TARGET_SRC} ${TARGET_H})

    ENDIF(${IS_COMMANDLINE_APP})

    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_OUTPUT_NAME "${TARGET_NAME}${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES RELEASE_OUTPUT_NAME "${TARGET_NAME}${CMAKE_RELEASE_POSTFIX}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES RELWITHDEBINFO_OUTPUT_NAME "${TARGET_NAME}${CMAKE_RELWITHDEBINFO_POSTFIX}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES MINSIZEREL_OUTPUT_NAME "${TARGET_NAME}${CMAKE_MINSIZEREL_POSTFIX}")

    SETUP_LINK_LIBRARIES()

ENDMACRO(SETUP_EXE)

# Takes optional second argument (is_commandline_app?) in ARGV1
MACRO(SETUP_APPLICATION APPLICATION_NAME)

        SET(TARGET_NAME ${APPLICATION_NAME} )

        IF(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP ${ARGV1})
        ELSE(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP 0)
        ENDIF(${ARGC} GREATER 1)

        SETUP_EXE(${IS_COMMANDLINE_APP})

    INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin  )
  #Install to the OSG_DIR as well
  IF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)
    INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION ${OSG_DIR}/bin)
  ENDIF(OSGEARTH_INSTALL_TO_OSG_DIR AND OSG_DIR)

  SET_PROPERTY(TARGET ${TARGET_TARGETNAME} PROPERTY FOLDER "Samples")

ENDMACRO(SETUP_APPLICATION)

MACRO(SETUP_COMMANDLINE_APPLICATION APPLICATION_NAME)

    SETUP_APPLICATION(${APPLICATION_NAME} 1)

ENDMACRO(SETUP_COMMANDLINE_APPLICATION)

# Takes optional second argument (is_commandline_app?) in ARGV1
MACRO(SETUP_EXAMPLE EXAMPLE_NAME)

        SET(TARGET_NAME ${EXAMPLE_NAME} )

        IF(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP ${ARGV1})
        ELSE(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP 0)
        ENDIF(${ARGC} GREATER 1)

        SETUP_EXE(${IS_COMMANDLINE_APP})

    INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION share/OpenSceneGraph/bin  )

ENDMACRO(SETUP_EXAMPLE)


MACRO(SETUP_COMMANDLINE_EXAMPLE EXAMPLE_NAME)

    SETUP_EXAMPLE(${EXAMPLE_NAME} 1)

ENDMACRO(SETUP_COMMANDLINE_EXAMPLE)
