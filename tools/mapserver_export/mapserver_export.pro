#################################################################
#
#         QMAKE Project File for mapserver export Gui
# 
#                      Tim Sutton 2006
#
#################################################################
TEMPLATE = app
TARGET = ms_export
CONFIG += debug_and_release
CONFIG += build_all 
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS +=  -lpython -L/usr/lib
mac:INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.3/include/python2.3/
linux-g++:INCLUDEPATH += /usr/include/python2.4/Python.h
QT += gui 
QT += qt3support


FORMS += qgsmapserverexportbase.ui

HEADERS += qgsmapserverexport.h
           
SOURCES += main.cpp \
           msexport_wrap.cxx \
           qgsmapserverexport.cpp
