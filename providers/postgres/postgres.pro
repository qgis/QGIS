######################################################################
Qmake project file for building the postgres provider.
This file is needed for building on Windows
######################################################################
# postgres.pro,v 1.1 2004/06/17 04:27:07 gsherman Exp 

TEMPLATE = lib
INCLUDEPATH += . \
                C:\Development\qgis_win32\src \
                C:\Development\postgresql-7.4.3\src\interfaces\libpq \
                C:\Development\postgresql-7.4.3\src\include
LIBS += C:\Development\postgresql-7.4.3\src\interfaces\libpq\Release\libpq.lib

CONFIG += qt dll thread
DLLDESTDIR= ..\..\src\lib\qgis
# Input
HEADERS += qgspostgresprovider.h
SOURCES += qgspostgresprovider.cpp \
           ..\..\src\qgsfeature.cpp \
           ..\..\src\qgsfield.cpp \
           ..\..\src\qgsrect.cpp \
           ..\..\src\qgsfeatureattribute.cpp \
           ..\..\src\qgspoint.cpp
