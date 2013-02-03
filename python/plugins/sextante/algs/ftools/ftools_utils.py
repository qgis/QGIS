# -*- coding: utf-8 -*-

"""
***************************************************************************
    ftools_utils.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Carson Farmer, Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from sextante.core.QGisLayers import QGisLayers

__author__ = 'Carson Farmer, Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Carson Farmer, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# Utility functions
# -------------------------------------------------
#
# convertFieldNameType( QgsField.name() )
# combineVectorFields( QgsVectorLayer, QgsVectorLayer )
# checkCRSCompatibility( QgsCoordinateReferenceSystem, QgsCoordinateReferenceSystem )
# writeVectorLayerToShape(QgsVectorLayer, QString *file path, QString *encoding style )
# getVectorTypeAsString( QgsVectorLayer )
# measurePerimeter( QgsGeometry )
# extractPoints( QgsGeometry )
# testForUniqueness( QList *QgsField, QList *QgsField )
# createUniqueFieldName( QgsField.name() )
# checkFieldNameLength( QgsFieldMap )
# getLayerNames( QGis.vectorType() )
# getFieldNames( QgsVectorLayer )
# getVectorLayerByName( QgsVectorLayer.name() )
# getFieldList( QgsVectorLayer )
# createIndex( QgsFeatureList )
# addShapeToCanvas( QString *file path )
# getUniqueValues( QgsVectorDataProvider, int *field id )
# saveDialog( QWidget *parent )
# getFieldType( QgsVectorLayer, QgsField.name() )
# getUniqueValuesCount( QgsVectorLayer, int fieldIndex )
#
# -------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

import locale

# For use with memory provider/layer, converts full field type to simple string
def convertFieldNameType( inName ):
    if inName == "Integer":
        return "int"
    elif inName == "Real":
        return "double"
    else:
        return "string"

# From two input field maps, create single field map
def combineVectorFields( layerA, layerB ):
    fieldsA = layerA.dataProvider().fields()
    fieldsB = layerB.dataProvider().fields()
    fieldsB = testForUniqueness( fieldsA, fieldsB )
    fieldsA.extend( fieldsB )
    return fieldsA

# Check if two input CRSs are identical
def checkCRSCompatibility( crsA, crsB ):
    if crsA == crsB:
        return True
    else:
        return False

# Convenience function to write vector layer to shapefile
def writeVectorLayerToShape( vlayer, outputPath, encoding ):
    mCodec = QTextCodec.codecForName( QString(encoding).toLocal8Bit().data() )
    if not mCodec:
        return False
    #Here we should check that the output path is valid
    QgsVectorFileWriter.writeAsVectorFormat( vlayer, outputPath, encoding, vlayer.dataProvider().crs(), "ESRI Shapefile", False )
    return True

# For use with memory provider/layer, converts QGis vector type definition to simple string
def getVectorTypeAsString( vlayer ):
    if vlayer.geometryType() == QGis.Polygon:
        return "Polygon"
    elif vlayer.geometryType() == QGis.Line:
        return "LineString"
    elif vlayer.geometryType() == QGis.Point:
        return "Point"
    else:
        return False

# Compute area and perimeter of input polygon geometry
def getAreaAndPerimeter( geom ):
    measure = QgsDistanceArea()
    area = measure.measure( geom )
    perim = measurePerimeter( geom, measure )
    return ( area, perim )

# Compute perimeter of input polygon geometry
def measurePerimeter( geom ):
    measure = QgsDistanceArea()
    value = 0.00
    polygon = geom.asPolygon()
    for line in polygon:
        value += measure.measureLine( line )
    return value

# Generate list of QgsPoints from input geometry ( can be point, line, or polygon )
def extractPoints( geom ):
    multi_geom = QgsGeometry()
    temp_geom = []
    if geom.type() == 0: # it's a point
        if geom.isMultipart():
            temp_geom = geom.asMultiPoint()
        else:
            temp_geom.append(geom.asPoint())
    if geom.type() == 1: # it's a line
        if geom.isMultipart():
            multi_geom = geom.asMultiPolyline() #multi_geog is a multiline
            for i in multi_geom: #i is a line
                temp_geom.extend( i )
        else:
            temp_geom = geom.asPolyline()
    elif geom.type() == 2: # it's a polygon
        if geom.isMultipart():
            multi_geom = geom.asMultiPolygon() #multi_geom is a multipolygon
            for i in multi_geom: #i is a polygon
                for j in i: #j is a line
                    temp_geom.extend( j )
        else:
            multi_geom = geom.asPolygon() #multi_geom is a polygon
            for i in multi_geom: #i is a line
                temp_geom.extend( i )
    return temp_geom

# Check if two input field maps are unique, and resolve name issues if they aren't
def testForUniqueness( fieldList1, fieldList2 ):
    changed = True
    while changed:
        changed = False
        for i in fieldList1:
            for j in fieldList2:
                if j.name() == i.name():
                    j = createUniqueFieldName( j )
                    changed = True
    return fieldList2

# Create a unique field name based on input field name
def createUniqueFieldName( field ):
    check = field.name().right( 2 )
    shortName = field.name().left( 8 )
    if check.startsWith("_"):
        ( val, test ) = check.right( 1 ).toInt()
        if test:
            if val < 2:
                val = 2
            else:
                val = val + 1
            field.setName( shortName.left( len( shortName )-1 ) + unicode( val ) )
        else:
            field.setName( shortName + "_2" )
    else:
        field.setName( shortName + "_2" )
    return field

# Return list of field names with more than 10 characters length
def checkFieldNameLength( fieldList ):
    longNames = QStringList()
    for field in fieldList:
        if field.name().size() > 10:
            longNames << unicode( field.name() )
    return longNames

# Return list of names of all layers in QgsMapLayerRegistry
def getLayerNames( vTypes ):
    layermap = QgsMapLayerRegistry.instance().mapLayers()
    layerlist = []
    if vTypes == "all":
        for name, layer in layermap.iteritems():
            layerlist.append( unicode( layer.name() ) )
    else:
        for name, layer in layermap.iteritems():
            if layer.type() == QgsMapLayer.VectorLayer:
                if layer.geometryType() in vTypes:
                    layerlist.append( unicode( layer.name() ) )
            elif layer.type() == QgsMapLayer.RasterLayer:
                if "Raster" in vTypes:
                    layerlist.append( unicode( layer.name() ) )
    return sorted( layerlist, cmp=locale.strcoll )

# Return list of names of all fields from input QgsVectorLayer
def getFieldNames( vlayer ):
    fieldmap = getFieldList( vlayer )
    fieldlist = []
    for field in fieldmap:
        if not field.name() in fieldlist:
            fieldlist.append( unicode( field.name() ) )
    return sorted( fieldlist, cmp=locale.strcoll )

# Return QgsVectorLayer from a layer name ( as string )
def getVectorLayerByName( myName ):
    layermap = QgsMapLayerRegistry.instance().mapLayers()
    for name, layer in layermap.iteritems():
        if layer.type() == QgsMapLayer.VectorLayer and layer.name() == myName:
            if layer.isValid():
                return layer
            else:
                return None

# Return QgsRasterLayer from a layer name ( as string )
def getRasterLayerByName( myName ):
    layermap = QgsMapLayerRegistry.instance().mapLayers()
    for name, layer in layermap.iteritems():
        if layer.type() == QgsMapLayer.RasterLayer and layer.name() == myName:
            if layer.isValid():
                return layer
            else:
                return None

# Return QgsMapLayer from a layer name ( as string )
def getMapLayerByName( myName ):
    layermap = QgsMapLayerRegistry.instance().mapLayers()
    for name, layer in layermap.iteritems():
        if layer.name() == myName:
            if layer.isValid():
                return layer
            else:
                return None

# Return the field list of a vector layer
def getFieldList( vlayer ):
    return vlayer.dataProvider().fields()

# Convinience function to create a spatial index for input QgsVectorDataProvider
def createIndex( features ):
    index = QgsSpatialIndex()
    for feat in features:
        index.insertFeature( feat )
    return index

# Convinience function to add a vector layer to canvas based on input shapefile path ( as string )
def addShapeToCanvas( shapefile_path ):
    file_info = QFileInfo( shapefile_path )
    if file_info.exists():
        layer_name = file_info.completeBaseName()
    else:
        return False
    vlayer_new = QgsVectorLayer( shapefile_path, layer_name, "ogr" )
    print layer_name
    if vlayer_new.isValid():
        QgsMapLayerRegistry.instance().addMapLayers( [vlayer_new] )
        return True
    else:
        return False

# Return all unique values in field based on field index
def getUniqueValues( provider, index ):
    return provider.uniqueValues( index )

# Generate a save file dialog with a dropdown box for choosing encoding style
def saveDialog( parent, filtering="Shapefiles (*.shp *.SHP)"):
    settings = QSettings()
    dirName = settings.value( "/UI/lastShapefileDir" ).toString()
    encode = settings.value( "/UI/encoding" ).toString()
    fileDialog = QgsEncodingFileDialog( parent, "Save output shapefile", dirName, QString(filtering), encode )
    fileDialog.setDefaultSuffix( QString( "shp" ) )
    fileDialog.setFileMode( QFileDialog.AnyFile )
    fileDialog.setAcceptMode( QFileDialog.AcceptSave )
    fileDialog.setConfirmOverwrite( True )
    if not fileDialog.exec_() == QDialog.Accepted:
            return None, None
    files = fileDialog.selectedFiles()
    settings.setValue("/UI/lastShapefileDir", QVariant( QFileInfo( unicode( files.first() ) ).absolutePath() ) )
    return ( unicode( files.first() ), unicode( fileDialog.encoding() ) )

# Generate a save file dialog with a dropdown box for choosing encoding style
def openDialog( parent, filtering="Shapefiles (*.shp *.SHP)" ):
    settings = QSettings()
    dirName = settings.value( "/UI/lastShapefileDir" ).toString()
    encode = settings.value( "/UI/encoding" ).toString()
    fileDialog = QgsEncodingFileDialog( parent, "Save output shapefile", dirName, QString(filtering), encode )
    fileDialog.setFileMode( QFileDialog.AnyFile )
    fileDialog.setAcceptMode( QFileDialog.AcceptOpen )
    if not fileDialog.exec_() == QDialog.Accepted:
            return None, None
    files = fileDialog.selectedFiles()
    settings.setValue("/UI/lastShapefileDir", QVariant( QFileInfo( unicode( files.first() ) ).absolutePath() ) )
    return ( unicode( files.first() ), unicode( fileDialog.encoding() ) )

# Generate a select directory dialog with a dropdown box for choosing encoding style
def dirDialog( parent ):
    settings = QSettings()
    dirName = settings.value( "/UI/lastShapefileDir" ).toString()
    encode = settings.value( "/UI/encoding" ).toString()
    fileDialog = QgsEncodingFileDialog( parent, "Save output shapefile", dirName, encode )
    fileDialog.setFileMode( QFileDialog.DirectoryOnly )
    fileDialog.setAcceptMode( QFileDialog.AcceptSave )
    fileDialog.setConfirmOverwrite( False )
    if not fileDialog.exec_() == QDialog.Accepted:
            return None, None
    folders = fileDialog.selectedFiles()
    settings.setValue("/UI/lastShapefileDir", QVariant( QFileInfo( unicode( folders.first() ) ).absolutePath() ) )
    return ( unicode( folders.first() ), unicode( fileDialog.encoding() ) )

# Return field type from it's name
def getFieldType(vlayer, fieldName):
    for field in vlayer.dataProvider().fields():
        if field.name() == fieldName:
            return field.typeName()

# return the number of unique values in field
def getUniqueValuesCount( layer, fieldIndex ):
    count = 0
    values = []
    features = QGisLayers.features(layer)
    for feat in features:
        if feat.attributeMap()[ fieldIndex ].toString() not in values:
            values.append( feat.attributeMap()[ fieldIndex ].toString() )
            count += 1
    return count

def getGeomType(gT):
  if gT == 3 or gT == 6:
    gTypeListPoly = [ QGis.WKBPolygon, QGis.WKBMultiPolygon ]
    return gTypeListPoly
  elif gT == 2 or gT == 5:
    gTypeListLine = [ QGis.WKBLineString, QGis.WKBMultiLineString ]
    return gTypeListLine
  elif gT == 1 or gT == 4:
    gTypeListPoint = [ QGis.WKBPoint, QGis.WKBMultiPoint ]
    return gTypeListPoint

def getShapesByGeometryType( baseDir, inShapes, geomType ):
  outShapes = QStringList()
  for fileName in inShapes:
    layerPath = QFileInfo( baseDir + "/" + fileName ).absoluteFilePath()
    vLayer = QgsVectorLayer( layerPath, QFileInfo( layerPath ).baseName(), "ogr" )
    if not vLayer.isValid():
      continue
    layerGeometry = vLayer.geometryType()
    if layerGeometry == QGis.Polygon and geomType == 0:
      outShapes << fileName
    elif layerGeometry == QGis.Line and geomType == 1:
      outShapes << fileName
    elif layerGeometry == QGis.Point and geomType == 2:
      outShapes << fileName

  if outShapes.count() == 0:
    return None

  return outShapes
