## 
## Try to find gnu scientific library GSL  
## (see http://www.gnu.org/software/gsl/)
## Once run this will define: 
## 
## GSL_FOUND       = system has GSL lib
##
## GSL_LIBRARIES   = full path to the libraries
##    on Unix/Linux with additional linker flags from "gsl-config --libs"
## 
## CMAKE_GSL_CXX_FLAGS  = Unix compiler flags for GSL, essentially "`gsl-config --cxxflags`"
##
## GSL_INCLUDE_DIR      = where to find headers 
##
## GSL_LINK_DIRECTORIES = link directories, useful for rpath on Unix
## GSL_EXE_LINKER_FLAGS = rpath on Unix
##
## Felix Woelk 07/2004
## minor corrections Jan Woetzel
##
## www.mip.informatik.uni-kiel.de
## --------------------------------
##


IF(WIN32)

  SET(GSL_MINGW_PREFIX "c:/msys/local" )
  SET(GSL_MSVC_PREFIX "$ENV{LIB_DIR}")

  FIND_PATH(GSL_INCLUDE_DIR gsl/gsl_blas.h 
    ${GSL_MINGW_PREFIX}/include 
    ${GSL_MSVC_PREFIX}/include
    $ENV{INCLUDE}
    )

  FIND_LIBRARY(GSL_LIB gsl PATHS 
    ${GSL_MINGW_PREFIX}/lib 
    ${GSL_MSVC_PREFIX}/lib
    $ENV{LIB}
    )

  FIND_LIBRARY(GSLCBLAS_LIB gslcblas cblas PATHS 
    ${GSL_MINGW_PREFIX}/lib 
    ${GSL_MSVC_PREFIX}/lib
    $ENV{LIB}
  )

  SET (GSL_LIBRARIES ${GSL_LIB} ${GSLCBLAS_LIB})
ELSE(WIN32)
  IF(UNIX) 

    # try to use framework on mac
    # want clean framework path, not unix compatibility path
    IF (APPLE)
      IF (CMAKE_FIND_FRAMEWORK MATCHES "FIRST"
          OR CMAKE_FRAMEWORK_PATH MATCHES "ONLY"
          OR NOT CMAKE_FIND_FRAMEWORK)
        SET (CMAKE_FIND_FRAMEWORK_save ${CMAKE_FIND_FRAMEWORK} CACHE STRING "" FORCE)
        SET (CMAKE_FIND_FRAMEWORK "ONLY" CACHE STRING "" FORCE)
        FIND_LIBRARY(GSL_LIBRARIES GSL)
        IF (GSL_LIBRARIES)
          # they're all the same in a framework
          SET (GSL_PREFIX ${GSL_LIBRARIES})
          SET (GSL_INCLUDE_DIR ${GSL_LIBRARIES}/Headers CACHE PATH "Path to a file.")
          SET (GSL_CONFIG ${GSL_LIBRARIES}/Programs/gsl-config CACHE FILEPATH "Path to a program.")
        ENDIF (GSL_LIBRARIES)
        SET (CMAKE_FIND_FRAMEWORK ${CMAKE_FIND_FRAMEWORK_save} CACHE STRING "" FORCE)
      ENDIF ()
    ENDIF (APPLE)
    
    IF (NOT GSL_INCLUDE_DIR AND NOT GSL_LIBRARIES AND NOT GSL_CONFIG)
      # didn't find OS X framework, or was set by user
      SET(GSL_CONFIG_PREFER_PATH "$ENV{GSL_HOME}/bin" CACHE STRING "preferred path to GSL (gsl-config)")
      FIND_PROGRAM(GSL_CONFIG gsl-config
          ${GSL_CONFIG_PREFER_PATH}
          /usr/local/bin/
          /usr/bin/
          )
      # MESSAGE("DBG GSL_CONFIG ${GSL_CONFIG}")

      IF (GSL_CONFIG) 
        # set CXXFLAGS to be fed into CXX_FLAGS by the user:
        SET(GSL_CXX_FLAGS "`${GSL_CONFIG} --cflags`")
      
        # set INCLUDE_DIRS to prefix+include
        EXEC_PROGRAM(${GSL_CONFIG}
            ARGS --prefix
            OUTPUT_VARIABLE GSL_PREFIX)
        SET(GSL_INCLUDE_DIR ${GSL_PREFIX}/include CACHE STRING INTERNAL)

        # set link libraries and link flags
        EXEC_PROGRAM(${GSL_CONFIG}
            ARGS --libs
            OUTPUT_VARIABLE GSL_LIBRARIES)
      
        ## extract link dirs for rpath  
        EXEC_PROGRAM(${GSL_CONFIG}
            ARGS ${LIBS_ARG}
            OUTPUT_VARIABLE GSL_CONFIG_LIBS )

        ## split off the link dirs (for rpath)
        ## use regular expression to match wildcard equivalent "-L*<endchar>"
        ## with <endchar> is a space or a semicolon
        STRING(REGEX MATCHALL "[-][L]([^ ;])+" 
            GSL_LINK_DIRECTORIES_WITH_PREFIX 
            "${GSL_CONFIG_LIBS}" )
        #      MESSAGE("DBG  GSL_LINK_DIRECTORIES_WITH_PREFIX=${GSL_LINK_DIRECTORIES_WITH_PREFIX}")

        ## remove prefix -L because we need the pure directory for LINK_DIRECTORIES
      
        IF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
          STRING(REGEX REPLACE "[-][L]" "" GSL_LINK_DIRECTORIES ${GSL_LINK_DIRECTORIES_WITH_PREFIX} )
        ENDIF (GSL_LINK_DIRECTORIES_WITH_PREFIX)
        SET(GSL_EXE_LINKER_FLAGS "-Wl,-rpath,${GSL_LINK_DIRECTORIES}" CACHE STRING INTERNAL)
        #      MESSAGE("DBG  GSL_LINK_DIRECTORIES=${GSL_LINK_DIRECTORIES}")
        #      MESSAGE("DBG  GSL_EXE_LINKER_FLAGS=${GSL_EXE_LINKER_FLAGS}")

        #      ADD_DEFINITIONS("-DHAVE_GSL")
        #      SET(GSL_DEFINITIONS "-DHAVE_GSL")
        MARK_AS_ADVANCED(
            GSL_CXX_FLAGS
            GSL_INCLUDE_DIR
            GSL_LIBRARIES
            GSL_LINK_DIRECTORIES
            GSL_DEFINITIONS
        )
      
      ELSE(GSL_CONFIG)
        MESSAGE("FindGSL.cmake: gsl-config not found. Please set it manually. GSL_CONFIG=${GSL_CONFIG}")
      ENDIF(GSL_CONFIG)
    ENDIF (NOT GSL_INCLUDE_DIR AND NOT GSL_LIBRARIES AND NOT GSL_CONFIG)
  ENDIF(UNIX)
ENDIF(WIN32)


IF(GSL_LIBRARIES)
  IF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)

    SET(GSL_FOUND 1)
    
    MESSAGE(STATUS "Found GSL: ${GSL_LIBRARIES}")

  ENDIF(GSL_INCLUDE_DIR OR GSL_CXX_FLAGS)
ENDIF(GSL_LIBRARIES)

