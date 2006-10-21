#################################################################
#
#         QMAKE Project File for openModeller Gui
# 
#                      Tim Sutton 2005
#
#################################################################
message(******************** settings.pro ***********************)

#################################################################
#
#    all platforms - change these settings as you need them
#
#################################################################

unix:WORKDIR=$$system(pwd)
#a hack to get the current working dir under windows
win32:WORKDIR=$$system(cd)
message(Building in $${WORKDIR})
QGIS_APP_NAME=qgis
QGIS_LOCALPLUGIN=true
QGIS_WEBSERVICESPLUGIN=false #disabled until renato gets it fully implemented
QGIS_DUMMYPLUGIN=false
#wether to try to build the  witty web toolkit fast cgi module
QGIS_WITTY=false
#whether to build unit tests
QGIS_TESTS=false
#whether to build console only applications
QGIS_CONSOLE=false
#whether to build the qt designer plugin
QGIS_DESIGNER=false
#whether to show experimental features to the user (set to true to hide 
#experimental features when making a release
QGIS_ALLOW_EXPERIMENTAL=true
#whether to use to qgis mapping component
#at the moment it only builds on linux :-(
QGIS_USE_QGIS=true
#linux-g++:QGIS_USE_QGIS=true
#below is needed for winows - cant seemt to get ver_maj just by setting VERSION
unix:VER_MAJ = 1
unix:VERSION = 1.0.0

#################################################################
#         Should not need to change below this point!!!!        #
#################################################################


#################################################################
##
## General compilation settings and defines
##
#################################################################
CONFIG += warn_off 
#create both debug and relase makefiles
CONFIG += debug_and_release
#build both release and debug targets when make is run
CONFIG += build_all 
LANGUAGE  = C++
CONFIG += exceptions
# Require that there are no undefined symbols in any libs!
QMAKE_LFLAGS_SHLIB *= --no-undefined
#clear all qt modules - each pro should specify exactly which qt modules it wants
QT =

QGSSVNVERSION=version0.8pre2
DEFINES += HAVE_POSTGRESQL=0
#################################################################
##
## Destination dir
##
#################################################################

# Where binary exes and libs should be placed when built
CONFIG(debug, debug|release){
  message(DEBUG?       : yes)
  # for ifdefs in code that should run only 
  # when debug support is enabled
  QGIS_DEBUGMODE=true 
  QGIS_APP_NAME=$${QGIS_APP_NAME}-debug
  win32:CONFIG+=console
}else{
  message(DEBUG?       : no )
  QGIS_APP_NAME=$${QGIS_APP_NAME}-release
}
DESTDIR=$${WORKDIR}/$${QGIS_APP_NAME}

#################################################################
#
# INSTALL PATHS
#
#################################################################

linux-g++:QGISBINDIR=$${DESTDIR}/bin
win32:QGISBINDIR=$${DESTDIR}
macx:QGISBINDIR=$${DESTDIR}/$${QGIS_APP_NAME}.app/Contents/MacOS/

linux-g++:QGISLIBDIR=$${DESTDIR}/lib
macx:QGISLIBDIR=$${QGISBINDIR}
win32:QGISLIBDIR=$${DESTDIR}

QGISPLUGINDIR=$${QGISBINDIR}/plugins
macx:QGISPLUGINDIR=$${DESTDIR}/$${QGIS_APP_NAME}.app/Contents/plugins

message(WORKDIR      : $${WORKDIR})
message(DESTDIR      : $${DESTDIR})
message(QGISBINDIR    : $${QGISBINDIR})
message(QGISLIBDIR    : $${QGISLIBDIR})
message(QGISPLUGINDIR : $${QGISPLUGINDIR})

#################################################################
##
## Libraries to link to (used on a case by case basis as needed)
##
#################################################################

QGISCORELIBADD=-lqgis_core
CONFIG(debug, debug|release){
  QGISCORELIBADD=$$member(QGISCORELIBADD, 0)-debug
}

QGISPROJECTIONSELECTORLIBADD=-lqgis_projectionselector
CONFIG(debug, debug|release){
  QGISPROJECTIONSELECTORLIBADD=$$member(QGISPROJECTIONSELECTORLIBADD, 0)-debug
}

#not currently used since I had to incorporate composer into gui lib due to 
#cyclical dependency issues
QGISCOMPOSERLIBADD=-lqgis_composer
CONFIG(debug, debug|release){
  QGISCOMPOSERLIBADD=$$member(QGISCOMPOSERLIBADD, 0)-debug
}

QGISGUILIBADD=-lqgis_gui
CONFIG(debug, debug|release){
  QGISGUILIBADD=$$member(QGISGUILIBADD, 0)-debug
}

win32:GDALLIBADD=-lgdal
unix:GDALLIBADD=-lgdal
macx:GDALLIBADD=-framework gdal

SQLITELIBADD=-lsqlite3
PROJLIBADD=-lproj
GEOSLIBADD=-lgeos

win32:LIBS += -lWs2_32

#################################################################
#
# Lib search paths (globally set for all pro files)
#
#################################################################

#win32:LIBS+=-LC:\MinGW\lib
win32:LIBS+=-LC:\msys\local\lib
win32:LIBS+=-L$${DESTDIR}
linux-g++:LIBS+=-L$${DESTDIR}/lib
linux-g++:LIBS+=-L/usr/lib/ccache/lib
macx:LIBS+=-L$${QGISLIBDIR}
macx:LIBS+=-F/Library/Frameworks/
macx:LIBS+=-L/usr/local/lib


#################################################################
#
# Include paths (globally set for all pro files)
#
#################################################################

INCLUDEPATH +=$${WORKDIR}/src \
              $${WORKDIR}/src/core \
              $${WORKDIR}/src/gui \
              $${WORKDIR}/src/legend \
              $${WORKDIR}/src/composer \
              $${WORKDIR}/src/widgets/projectionselector \
              $${WORKDIR}/src/plugins \
              $${WORKDIR}/src/providers \
              $${WORKDIR}/src/raster \
              $${WORKDIR}/src/ui 

#################################################################
#
#   windows platform (MinGW) 
#
#################################################################

win32{
  message(Installing for windows!)
  #add any win specific rules here 
  INCLUDEPATH += c:/msys/local/include
}


#################################################################
#
# MacOSX platform specific directives
#
#################################################################

macx{
  #fixme should not need the next line
  #INCLUDEPATH += /Users/timsutton/dev/cpp/om/src
  FRAMEWORKSDIR=$${DESTDIR}/$${QGIS_APP_NAME}.app/Contents/Frameworks
  message (Checking if $${FRAMEWORKSDIR}/gdal.framework/gdal exists)
  exists( $${FRAMEWORKSDIR}/gdal.framework/gdal )
  {
    message(Gdal framework already in the bundle...skipping copy)
  }else{
    system(mkdir -p $${FRAMEWORKSDIR})
    system(cp -RP /Library/Frameworks/gdal.framework $${FRAMEWORKSDIR}/)
    message(Gdal framework copied into the bundle)
  }
  system(cp mac/Info.plist $${DESTDIR}/bin/$${QGIS_APP_NAME}.app/Contents)
}


#################################################################
#
# Where intermediate build files should go
#
#################################################################

OBJDIR                =  $${WORKDIR}/obj
MOC_DIR               =  $${OBJDIR}/moc
UI_DIR                =  $${OBJDIR}/ui
macx:OBJECTS_DIR       =  $${OBJDIR}/o/mac
linux-g++:OBJECTS_DIR =  $${OBJDIR}/o/linux
win32:OBJECTS_DIR     =  $${OBJDIR}/o/win32
#These next two are not currently needed for this simple project
#RCC_DIR               =  $${OBJDIR}/rcc
#RC_FILE               =  $${APPNAME}.rc
