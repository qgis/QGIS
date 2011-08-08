# This module defines

# OSGEARTH_LIBRARY
# OSGEARTH_FOUND, if false, do not try to link to osg
# OSGEARTH_INCLUDE_DIRS, where to find the headers
# OSGEARTH_INCLUDE_DIR, where to find the source headers
# OSGEARTH_GEN_INCLUDE_DIR, where to find the generated headers

# to use this module, set variables to point to the osg build
# directory, and source directory, respectively
# OSGEARTHDIR or OSGEARTH_SOURCE_DIR: osg source directory, typically OpenSceneGraph
# OSGEARTH_DIR or OSGEARTH_BUILD_DIR: osg build directory, place in which you've
#    built osg via cmake 

# Header files are presumed to be included like
# #include <osgEarth/Common>
# #include <osgEarth/TileSource>

###### headers ######

MACRO( FIND_OSGEARTH_INCLUDE THIS_OSGEARTH_INCLUDE_DIR THIS_OSGEARTH_INCLUDE_FILE )

FIND_PATH( ${THIS_OSGEARTH_INCLUDE_DIR} ${THIS_OSGEARTH_INCLUDE_FILE}
    PATHS
        ${OSGEARTH_DIR}
        $ENV{OSGEARTH_SOURCE_DIR}
        $ENV{OSGEARTHDIR}
        $ENV{OSGEARTH_DIR}
        $ENV{OSGEO4W_ROOT}
        /usr/local/
        /usr/
        /sw/ # Fink
        /opt/local/ # DarwinPorts
        /opt/csw/ # Blastwave
        /opt/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSGEARTH_ROOT]/
        ~/Library/Frameworks
        /Library/Frameworks
    PATH_SUFFIXES
        /include/
)

ENDMACRO( FIND_OSGEARTH_INCLUDE THIS_OSGEARTH_INCLUDE_DIR THIS_OSGEARTH_INCLUDE_FILE )

FIND_OSGEARTH_INCLUDE( OSGEARTH_GEN_INCLUDE_DIR   osgEarth/Common )
FIND_OSGEARTH_INCLUDE( OSGEARTH_INCLUDE_DIR       osgEarth/TileSource )

###### libraries ######

MACRO( FIND_OSGEARTH_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME}
    PATHS
        ${OSGEARTH_DIR}
        $ENV{OSGEARTH_BUILD_DIR}
        $ENV{OSGEARTH_DIR}
        $ENV{OSGEARTHDIR}
        $ENV{OSGEARTH_ROOT}
        $ENV{OSGEO4W_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSGEARTH_ROOT]/lib
        /usr/freeware
    PATH_SUFFIXES
        /lib/
        /lib64/
        /build/lib/
        /build/lib64/
        /Build/lib/
        /Build/lib64/
     )

ENDMACRO(FIND_OSGEARTH_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSGEARTH_LIBRARY( OSGEARTH_LIBRARY osgEarth )
FIND_OSGEARTH_LIBRARY( OSGEARTH_LIBRARY_DEBUG osgEarthd )

FIND_OSGEARTH_LIBRARY( OSGEARTHUTIL_LIBRARY osgEarthUtil )
FIND_OSGEARTH_LIBRARY( OSGEARTHUTIL_LIBRARY_DEBUG osgEarthUtild )

FIND_OSGEARTH_LIBRARY( OSGEARTHFEATURES_LIBRARY osgEarthFeatures )
FIND_OSGEARTH_LIBRARY( OSGEARTHFEATURES_LIBRARY_DEBUG osgEarthFeaturesd )

FIND_OSGEARTH_LIBRARY( OSGEARTHSYMBOLOGY_LIBRARY osgEarthSymbology )
FIND_OSGEARTH_LIBRARY( OSGEARTHSYMBOLOGY_LIBRARY_DEBUG osgEarthSymbologyd )


SET( OSGEARTH_FOUND "NO" )
IF( OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR )
    SET( OSGEARTH_FOUND "YES" )
    SET( OSGEARTH_INCLUDE_DIRS ${OSGEARTH_INCLUDE_DIR} ${OSGEARTH_GEN_INCLUDE_DIR} )
    INCLUDE(CheckCXXSourceCompiles)
    SET(SAFE_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
    SET(SAFE_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    SET(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OSGEARTH_INCLUDE_DIR})
    SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${OSGEARTHUTIL_LIBRARY})
    CHECK_CXX_SOURCE_COMPILES("
#include <osgEarthUtil/Controls>
using namespace osgEarth::Util::Controls;
int main(int argc, char **argv)
{
  Container *c;
  c->setChildSpacing(0.0);
}
" HAVE_OSGEARTH_CHILD_SPACING)
    SET(CMAKE_REQUIRED_INCLUDES ${SAFE_CMAKE_REQUIRED_INCLUDES})
    SET(CMAKE_REQUIRED_LIBRARIES ${SAFE_CMAKE_REQUIRED_LIBRARIES})
    GET_FILENAME_COMPONENT( OSGEARTH_LIBRARIES_DIR ${OSGEARTH_LIBRARY} PATH )
ENDIF( OSGEARTH_LIBRARY AND OSGEARTH_INCLUDE_DIR )


