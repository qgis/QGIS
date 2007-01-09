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
		qgsapplication.h			\
		qgsattributeaction.h		\
		qgsclipper.h				\
		qgscontexthelp.h			\
		qgscoordinatetransform.h			\
		qgscsexception.h			\
		qgsdataprovider.h			\
		qgsdatasourceuri.h			\
		qgsdistancearea.h			\
		qgsexception.h				\
		qgsfeature.h				\
		qgsfeatureattribute.h		\
		qgsfield.h					\
		qgsgeometry.h				\
		qgsgeometryvertexindex.h	\
		qgshttptransaction.h		\
		qgslabel.h					\
		qgslabelattributes.h		\
		qgsline.h					\
		qgslogger.h				\
		qgsmaplayer.h				\
		qgsmaplayerregistry.h		\
		qgsmaprender.h			\
		qgsmaptopixel.h				\
		qgsmessageoutput.h		\
		qgspoint.h					\
		qgsproject.h				\
		qgsprojectproperty.h		\
		qgsprovidercountcalcevent.h	\
		qgsproviderextentcalcevent.h\
		qgsprovidermetadata.h		\
		qgsproviderregistry.h		\
		qgsrasterdataprovider.h		\
		qgsrect.h					\
		qgsrunprocess.h			\
		qgsscalecalculator.h		\
		qgssearchstring.h			\
    qgssearchtreenode.h			\
		qgsspatialrefsys.h		\
		qgsvectorfilewriter.h		\
		qgsvectorlayer.h			\
		qgsvectordataprovider.h		\
    \
		raster/qgscolortable.h		\
		raster/qgsrasterbandstats.h \
		raster/qgsrasterpyramid.h \
    raster/qgsrasterlayer.h \
	  raster/qgsrasterviewport.h \
		\
		renderer/qgscontinuouscolorrenderer.h \
		renderer/qgsgraduatedsymbolrenderer.h \
		renderer/qgsrangerenderitem.h	\
		renderer/qgsrenderer.h \
		renderer/qgsrenderitem.h \
		renderer/qgssinglesymbolrenderer.h \
		renderer/qgsuniquevaluerenderer.h \
		\
		symbology/qgssymbol.h \
		symbology/qgssymbologyutils.h \
		symbology/qgslinesymbol.h \
		symbology/qgsmarkercatalogue.h \
		symbology/qgsmarkersymbol.h \
		symbology/qgspolygonsymbol.h
	
						

## files generated from MOC                             
libqgis_core_la_MOC =							\
		qgscontexthelp.moc.cpp			\
		qgscoordinatetransform.moc.cpp		\
		qgsdataprovider.moc.cpp				\
		qgshttptransaction.moc.cpp		\
		qgsmaplayer.moc.cpp				\
		qgsmaplayerregistry.moc.cpp		\
		qgsmaprender.moc.cpp			\
		qgsmessageoutput.moc.cpp		\
		qgsproject.moc.cpp		\
		qgsrasterdataprovider.moc.cpp		\
		qgsrunprocess.moc.cpp			\
		qgsvectorlayer.moc.cpp \
		\
		raster/qgsrasterlayer.moc.cpp

libqgis_core_la_SOURCES =\
		qgis.cpp								\
		qgsapplication.cpp	\
		qgsattributeaction.cpp	\
		qgsclipper.cpp						\
		qgscontexthelp.cpp		\
		qgscoordinatetransform.cpp		\
		qgsdistancearea.cpp			\
		qgsexception.cpp				\
		qgsfeature.cpp					\
		qgsfeatureattribute.cpp				\
		qgsfield.cpp						\
		qgsgeometry.cpp						\
		qgsgeometryvertexindex.cpp			\
		qgshttptransaction.cpp		\
		qgslabel.cpp				\
		qgslabelattributes.cpp		\
		qgsline.cpp			\
		qgslogger.cpp			\
		qgsmaplayer.cpp				\
		qgsmaplayerregistry.cpp		\
		qgsmaprender.cpp			\
		qgsmaptopixel.cpp					\
		qgsmessageoutput.cpp			\
		qgspoint.cpp						\
		qgsproject.cpp			\
		qgsprojectproperty.cpp			\
		qgsprovidercountcalcevent.cpp		\
		qgsproviderextentcalcevent.cpp		\
		qgsprovidermetadata.cpp		\
		qgsproviderregistry.cpp				\
		qgsrasterdataprovider.cpp	\
		qgsrect.cpp							\
		qgsrunprocess.cpp				\
		qgsscalecalculator.cpp				\
		qgssearchstring.cpp             \
		qgssearchstringlexer.ll		\
		qgssearchstringparser.yy	\
		qgssearchtreenode.cpp		\
		qgsspatialrefsys.cpp		\
		qgsvectorfilewriter.cpp		\
		qgsvectorlayer.cpp		\
		qgsvectordataprovider.cpp \
		\
		raster/qgscolortable.cpp		\
		raster/qgsrasterlayer.cpp \
		\
		renderer/qgscontinuouscolorrenderer.cpp \
		renderer/qgsgraduatedsymbolrenderer.cpp \
		renderer/qgssinglesymbolrenderer.cpp \
		renderer/qgsrangerenderitem.cpp \
		renderer/qgsrenderer.cpp \
		renderer/qgsrenderitem.cpp \
		renderer/qgsuniquevaluerenderer.cpp \
		\
		symbology/qgssymbol.cpp \
		symbology/qgssymbologyutils.cpp \
		symbology/qgslinesymbol.cpp \
		symbology/qgsmarkercatalogue.cpp \
		symbology/qgsmarkersymbol.cpp \
		symbology/qgspolygonsymbol.cpp
		

AM_YFLAGS       = -d
libqgis_core_la_YACC = qgssearchstringparser.h


nodist_libqgis_core_la_SOURCES = $(libqgis_core_la_MOC)  

BUILT_SOURCES = $(libqgis_core_la_YACC) $(libqgis_core_la_MOC)  $(postgresMOC)


libqgis_core_la_LIBADD = $(QT_LDADD) $(GDAL_LDADD) $(GEOS_LDADD) $(PYTHON_LIB) $(GDAL_LDADD) -lsqlite3
libqgis_core_la_LDFLAGS = -version-info $(INTERFACE_VERSION)

libqgis_core_la_CXXFLAGS = $(PREFIX) $(PLUGINPATH) $(PKGDATAPATH) $(CXXFLAGS) $(EXTRA_CXXFLAGS) $(QT_CXXFLAGS) $(PG_INC) $(DEBUG_QGIS) $(GEOS_CFLAGS) $(GDAL_CFLAGS) $(PYTHON_INCLUDE_DIR) $(HAVE_PYTHON) -Iraster -Irenderer -Isymbology

# Link headers to include directory
all-local:
	rm -rf include/
	mkdir include
	includes='$(pkginclude_HEADERS)' ; \
	for file in $$includes ; do \
	  base=`basename $$file` ; \
		ln -s ../$$file include/$$base ; \
	done

clean-local:
	rm -rf include/


##
## ----------------------------------------------------------------------
##         additional files that need to be installed go here
## ----------------------------------------------------------------------
##
pkginclude_HEADERS = $(libqgis_coreHEADERS) 

pkginclude_dir = $(includedir)/qgis


CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST=


noinst_HEADERS = qgsdatasourceuri.h
