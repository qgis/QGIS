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
#need to figure out how to automate making qgssvnversion file
#  line below doesnt work
#system(echo $$QGSSVNVERSION >> qgssvnversion.h)
#suffix debug to target if applicable
CONFIG(debug, debug|release){
  TARGET = $$member(TARGET, 0)-debug
}
LIBS += $${GDALLIBADD}
LIBS += $${SQLITELIBADD}
LIBS += $${GEOSLIBADD}
LIBS += $${PROJLIBADD}
DESTDIR=$${QGISLIBDIR}
QT += network qt3support xml svg core gui
QT = $$unique(QT)
message("Building libs into $${DESTDIR}")
win32{
  #generate lex and yacc sources
  message("Generating lex and yacc sources")
  system(genlexyacc)
} 

HEADERS =       qgis.h					\
		qgsapplication.h			\	
		qgsbookmarkitem.h			\
		qgsclipper.h				\
		qgscolortable.h				\
		qgscontexthelp.h			\
		qgscoordinatetransform.h		\
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
		qgssearchstringparser.h			\
		qgsspatialrefsys.h			\
		qgssymbol.h				\
		qgssymbologyutils.h			\
		qgsvectordataprovider.h			\
                qgssearchtreenode.h

HEADERS = $$unique(HEADERS)

SOURCES =\
		qgis.cpp				\
		qgsapplication.cpp			\
		qgsbookmarkitem.cpp			\
		qgsclipper.cpp				\
		qgscolortable.cpp			\
		qgscontexthelp.cpp			\
		qgscoordinatetransform.cpp		\
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
		qgssearchstringlexer.cc                 \
		qgssearchstringparser.cc                \
		qgssearchtreenode.cpp			\
		qgsspatialrefsys.cpp			\
		qgssymbol.cpp				\
		qgssymbologyutils.cpp			\
		qgsvectordataprovider.cpp			



