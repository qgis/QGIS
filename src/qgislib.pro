TEMPLATE = lib
TARGET=libqgis
INCLUDEPATH += . $(GDAL)\include
LIBS += $(GDAL)\lib\gdal_i.lib
CONFIG += qt thread
DLLDESTDIR= ..\..\src\lib\qgis
# Input
SOURCES +=qgsmapcanvas.cpp \
	moc_qgsmapcanvas.cpp \
	qgssymbol.cpp \
	qgsmaplayer.cpp \
	qgsrasterlayer.cpp \
	qgsvectorlayer.cpp \
	moc_qgsmaplayer.cpp \
	moc_qgsrasterlayer.cpp \
	moc_qgsvectorlayer.cpp \
	qgsfield.cpp \
	qgsfeature.cpp \
	qgsfeatureattribute.cpp \
	qgsrect.cpp \
	qgscoordinatetransform.cpp \
	qgsscalecalculator.cpp \
	qgsacetaterectangle.cpp \
	qgsacetateobject.cpp \
	qgsmaplayerregistry.cpp \
	moc_qgsmaplayerregistry.cpp \
	qgspoint.cpp \
	qgslegend.cpp \
	moc_qgslegend.cpp \
	qgslegenditem.cpp
