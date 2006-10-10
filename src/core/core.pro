#################################################################
#
#         QMAKE Project File for Quantum GIS 
# 
#                   Tim Sutton 2006
#
# NOTE: Do not place any hard coded external paths in this file
#       all libs and includes should be specified in settings.pro
#       in the top level qgis directory.
# 
#################################################################

include(../../settings.pro)
TEMPLATE=lib
TARGET=qgis_core
system(echo $$QGSSVNVERSION > qgssvnversion.h)
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
DESTDIR=$${QGISLIBDIR}
message("Building libs into $${DESTDIR}")

#AM_YFLAGS       = -d
#qgis_YACC       = qgssearchstringparser.h
HEADERS =				\
		qgis.h					\
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
		qgsfeature.h				\
		qgsfeatureattribute.h			\
		qgsfield.h				\
		qgsgeometry.h				\
		qgsgeometryvertexindex.h		\
		qgshttptransaction.h			\
		qgslabelattributes.h			\
		qgsline.h				\
		qgslinesymbol.h				\
		qgslogger.h				\
		qgsmaptopixel.h				\
		qgsmarkercatalogue.h			\
		qgsmarkersymbol.h			\
		qgsnumericsortlistviewitem.h		\
		qgspluginitem.h				\
		qgspluginregistry.h			\
		qgspoint.h				\
		qgspolygonsymbol.h			\
		qgsprojectproperty.h			\
		qgsprovidercountcalcevent.h		\
		qgsproviderextentcalcevent.h		\
		qgsprovidermetadata.h			\
		qgsproviderregistry.h			\
		qgsrangerenderitem.h			\
		qgsrasterdataprovider.h			\
		qgsrect.h				\
		qgsrenderer.h				\
		qgsrenderitem.h				\
		qgsscalecalculator.h			\
		qgssearchstring.h			\
                qgssearchtreenode.h			\
		qgssinglesymrenderer.h			\
		qgssymbol.h				\
		qgssymbologyutils.h			\
		qgsvectordataprovider.h		


SOURCES =\
		qgis.cpp				\
		qgsapplication.cpp			\
		qgsbookmarkitem.cpp			\
		qgsclipper.cpp				\
		qgscolortable.cpp			\
		qgscontexthelp.cpp			\
		qgscustomsymbol.cpp			\
		qgsdatamanager.cpp			\
		qgsdatasource.cpp			\
		qgsdistancearea.cpp			\
		qgsexception.cpp			\
		qgsfeature.cpp				\
		qgsfeatureattribute.cpp			\
		qgsfield.cpp				\
		qgsgeometry.cpp				\
		qgsgeometryvertexindex.cpp		\
		qgshttptransaction.cpp			\
		qgslabelattributes.cpp			\
		qgsline.cpp				\
		qgslinesymbol.cpp			\
		qgslogger.cpp				\
		qgsmaptopixel.cpp			\
		qgsmarkercatalogue.cpp			\
		qgsmarkersymbol.cpp			\
		qgsnumericsortlistviewitem.cpp		\
		qgspluginitem.cpp			\
		qgspluginregistry.cpp			\
		qgspoint.cpp				\
		qgspolygonsymbol.cpp			\
		qgsprojectproperty.cpp			\
		qgsprovidercountcalcevent.cpp		\
		qgsproviderextentcalcevent.cpp		\
		qgsprovidermetadata.cpp			\
		qgsproviderregistry.cpp			\
		qgsrangerenderitem.cpp			\
		qgsrasterdataprovider.cpp		\
		qgsrect.cpp				\
		qgsrenderer.cpp				\
		qgsrenderitem.cpp			\
		qgsscalecalculator.cpp			\
		qgssearchstring.cpp   		        \
		qgssearchstringlexer.ll			\
		qgssearchstringparser.yy		\
		qgssearchtreenode.cpp			\
		qgssymbol.cpp				\
		qgssymbologyutils.cpp			\
		qgsvectordataprovider.cpp			


