# Copyright (C) 2003 Gary Sherman <sherman at mrcc.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

##
## ----------------------------------------------------------------------
##        Initial setup 
## ----------------------------------------------------------------------
##
if HAVE_QTMAC
MAC = mac
endif


if !HAVE_QTMAC
PREFIX=-DPREFIX=\"$(prefix)\"
PLUGINPATH=-DPLUGINPATH=\"$(pkglibdir)\"
PKGDATAPATH=-DPKGDATAPATH=\"$(pkgdatadir)\"
endif

%.moc.cpp: %.h
	$(MOC) -o $@ $<

##
## ----------------------------------------------------------------------
##      Define Targets
## ----------------------------------------------------------------------
##
lib_LTLIBRARIES = libqgis_core.la 


##
## ----------------------------------------------------------------------
##         libqgis_core stuff starts here
## ----------------------------------------------------------------------
##

libqgis_coreHEADERS =				\
		qgis.h						\
		qgsacetatelines.h			\
		qgsacetateobject.h			\
		qgsacetaterectangle.h		\
		qgsapplication.h			\
		qgsbookmarkitem.h			\
		qgsclipper.h				\
		qgscolortable.h				\
		qgscontexthelp.h			\
		qgscsexception.h			\
		qgscustomsymbol.h			\
		qgsdatamanager.h			\
		qgsdataprovider.h			\
		qgsdatasource.h				\
		qgsdatasourceuri.h			\
		qgsdistancearea.h			\
		qgsexception.h				\
		qgsfeatureattribute.h		\
		qgsfield.h					\
		qgsfile.h					\
		qgsgeometry.h				\
		qgsgeometryvertexindex.h	\
		qgslabelattributes.h		\
		qgsline.h					\
		qgslinesymbol.h				\
		qgsmaptopixel.h				\
		qgsmarkercatalogue.h		\
		qgsmarkersymbol.h			\
		qgsnumericsortlistviewitem.h\
		qgspluginitem.h				\
		qgspluginregistry.h			\
		qgspoint.h					\
		qgspolygonsymbol.h			\
		qgsprojectproperty.h		\
		qgsprovidercountcalcevent.h	\
		qgsproviderextentcalcevent.h\
		qgsprovidermetadata.h		\
		qgsproviderregistry.h		\
		qgsrangerenderitem.h		\
		qgsrasterdataprovider.h		\
		qgsrect.h					\
		qgsrenderer.h				\
		qgsrenderitem.h				\
		qgsscalecalculator.h		\
		qgssearchstring.h			\
        qgssearchtreenode.h			\
		qgssinglesymrenderer.h		\
		qgssvgcache.h				\
		qgssymbol.h					\
		qgssymbologyutils.h			\
		qgsvectordataprovider.h		

## files generated from MOC                             
libqgis_core_la_MOC =							\
		qgscontexthelp.moc.cpp			\
		qgsdataprovider.moc.cpp				\
		qgsrasterdataprovider.moc.cpp		

libqgis_core_la_SOURCES =\
		qgsacetatelines.cpp					\
		qgsacetateobject.cpp				\
		qgsacetaterectangle.cpp				\
		qgsapplication.cpp	\
		qgsbookmarkitem.cpp	\
		qgsclipper.cpp						\
		qgscolortable.cpp					\
		qgscontexthelp.cpp		\
		qgscustomsymbol.cpp		\
		qgsdatamanager.cpp				\
		qgsdatasource.cpp		\
		qgsdistancearea.cpp			\
		qgsexception.cpp				\
		qgsfeatureattribute.cpp				\
		qgsfield.cpp						\
		qgsfile.cpp			\
		qgsgeometry.cpp						\
		qgsgeometryvertexindex.cpp			\
		qgslabelattributes.cpp		\
		qgsline.cpp			\
		qgslinesymbol.cpp		\
		qgsmaptopixel.cpp					\
		qgsmarkercatalogue.cpp				\
		qgsmarkersymbol.cpp					\
		qgsnumericsortlistviewitem.cpp			\
		qgspluginitem.cpp		\
		qgspluginregistry.cpp		\
		qgspoint.cpp						\
		qgspolygonsymbol.cpp		\
		qgsprojectproperty.cpp				\
		qgsprovidercountcalcevent.cpp		\
		qgsproviderextentcalcevent.cpp		\
		qgsprovidermetadata.cpp		\
		qgsproviderregistry.cpp				\
		qgsrangerenderitem.cpp				\
		qgsrasterdataprovider.cpp	\
		qgsrect.cpp							\
		qgsrenderer.cpp						\
		qgsrenderitem.cpp					\
		qgsscalecalculator.cpp				\
		qgssearchstring.cpp             \
		qgssearchstringlexer.ll		\
		qgssearchstringparser.yy	\
		qgssearchtreenode.cpp		\
		qgssvgcache.cpp			\
		qgssymbol.cpp						\
		qgssymbologyutils.cpp		\
		qgsvectordataprovider.cpp			
		

AM_YFLAGS       = -d
qgis_YACC       = qgssearchstringparser.h


nodist_libqgis_core_la_SOURCES = $(libqgis_core_la_MOC)  

BUILT_SOURCES = $(libqgis_core_la_MOC) $(qgis_YACC)  $(postgresMOC)


#libqgis_core_la_LIBADD = ../raster/libqgis_raster.la ../legend/libqgis_legend.la ../composer/libqgis_composer.la ../widgets/projectionselector/libqgsprojectionselector.la $(QT_LDADD) $(GEOS_LDADD) $(GDAL_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_core_la_LIBADD = $(QT_LDADD) $(GEOS_LDADD) $(PYTHON_LIB) -lsqlite3
libqgis_core_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_core_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -I../ui/ -I../widgets/projectionselector/ -I../legend/ -I../raster/ -I../composer/ -I../gui/

##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##
pkginclude_HEADERS = $(qgisHEADERS) 

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST=


noinst_HEADERS = qgsdatasourceuri.h
