####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = qgis

## INCLUDES were found outside kdevelop specific part

qgis_SOURCES = qgsrect.cpp qgsrasterlayer.cpp qgspoint.cpp qgsnewconnectionbase.ui qgsnewconnection.cpp qgsmaplayer.cpp qgsmapcanvas.cpp qgsdbsourceselectbase.ui qgsdbsourceselect.cpp qgsdatasource.cpp qgsdatabaselayer.cpp qgsdatabase.cpp qgscoordinatetransform.cpp qgisappbase.ui qgisapp.cpp main.cpp 
SOURCES = qgsrect.cpp qgsrasterlayer.cpp qgspoint.cpp qgsnewconnection.cpp qgsmaplayer.cpp qgsmapcanvas.cpp qgsdbsourceselect.cpp qgsdatasource.cpp qgsdatabaselayer.cpp qgsdatabase.cpp qgscoordinatetransform.cpp qgisapp.cpp main.cpp 
INTERFACES = qgsnewconnectionbase.ui qgsdbsourceselectbase.ui qgisappbase.ui 
TRANSLATIONS = 
KDE_OPTIONS = qtonly
qgis_LDADD   = -L$(PGSQL)/lib -lpq++ $(LIB_QT) $(LIBSOCKET)

SUBDIRS = docs 

EXTRA_DIST = filenew.xpm filesave.xpm fileopen.xpm main.cpp qgisapp.cpp qgisapp.h qgisappbase.ui qgscoordinatetransform.cpp qgscoordinatetransform.h qgsdatabase.cpp qgsdatabaselayer.cpp qgsdatabaselayer.h qgsdatasource.cpp qgsdatasource.h qgsdbsourceselect.cpp qgsdbsourceselectbase.ui qgsdbsourceselect.h qgslinesymbol.h qgsmapcanvas.cpp qgsmapcanvas.h qgsmaplayer.cpp qgsmaplayer.h qgsmarkersymbol.h qgsnewconnection.cpp qgsnewconnection.h qgsnewconnectionbase.ui qgspoint.cpp qgspoint.h qgspolygonsymbol.h qgsrasterlayer.cpp qgsrasterlayer.h qgsrect.cpp qgsrect.h qgsshapefilelayer.h qgssymbol.h qgstable.h 

####### kdevelop will overwrite this part!!! (end)############


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

qgis_METASOURCES = AUTO

# the library search path. 
qgis_LDFLAGS = $(all_libraries) 

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(qgis_METASOURCES)

