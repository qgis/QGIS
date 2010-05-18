# -*- coding: utf-8 -*-
# Utility functions
# -------------------------------------------------
#
# combineVectorAttributes( QgsAttributeMap, QgsAttributeMap )
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
# createIndex( QgsVectorDataProvider )
# addShapeToCanvas( QString *file path )
# getUniqueValues( QgsVectorDataProvider, int *field id )
# saveDialog( QWidget *parent )
# getFieldType( QgsVectorLayer, QgsField.name() )
# getUniqueValuesCount( QgsVectorLayer, int fieldIndex, bool useSelection ):
#
# -------------------------------------------------

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

# From two input attribute maps, create single attribute map
def combineVectorAttributes( atMapA, atMapB ):
    attribA = atMapA.values()
    lengthA = len(attribA)
    attribB = atMapB.values()
    lengthB = len(attribB)
    attribA.extend( attribB )
    return dict( zip( range( 0, lengthB + lengthA ), attribA ) )

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
    fieldsA = layerA.dataProvider().fields().values()
    fieldsB = layerB.dataProvider().fields().values()
    fieldsB = testForUniqueness( fieldsA, fieldsB )
    seq = range( 0, len( fieldsA ) + len( fieldsB ) )
    fieldsA.extend( fieldsB )
    fieldsA = dict( zip ( seq, fieldsA ) )
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
    QgsVectorFileWriter.writeAsShapefile( vlayer, outputPath, encoding, vlayer.dataProvider().crs(), False )
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
    for num, field in fieldList.iteritems():
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
    return layerlist

# Return list of names of all fields from input QgsVectorLayer
def getFieldNames( vlayer ):
    fieldmap = getFieldList( vlayer )
    fieldlist = []
    for name, field in fieldmap.iteritems():
        if not field.name() in fieldlist:
            fieldlist.append( unicode( field.name() ) )
    return fieldlist

# Return QgsVectorLayer from a layer name ( as string )
def getVectorLayerByName( myName ):
    layermap = QgsMapLayerRegistry.instance().mapLayers()
    for name, layer in layermap.iteritems():
        if layer.type() == QgsMapLayer.VectorLayer and layer.name() == myName:
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
    vprovider = vlayer.dataProvider()
    feat = QgsFeature()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    myFields = vprovider.fields()
    return myFields

# Convinience function to create a spatial index for input QgsVectorDataProvider
def createIndex( provider ):
    feat = QgsFeature()
    index = QgsSpatialIndex()
    provider.rewind()
    while provider.nextFeature( feat ):
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
        QgsMapLayerRegistry.instance().addMapLayer( vlayer_new )
        return True
    else:
        return False

# Return all unique values in field based on field index
def getUniqueValues( provider, index ):
    allAttrs = provider.attributeIndexes()
    provider.select( allAttrs )
    f = QgsFeature()
    values = []
    check = []
    while provider.nextFeature( f ):
        if not f.attributeMap()[ index ].toString() in check:
            values.append( f.attributeMap()[ index ] )
            check.append( f.attributeMap()[ index ].toString() )
    return values

# Generate a save file dialog with a dropdown box for choosing encoding style
def saveDialog( parent, filtering="Shapefiles (*.shp)"):
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
def openDialog( parent, filtering="Shapefiles (*.shp)"):
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
    settings.setValue("/UI/lastShapefileDir", QVariant( QFileInfo( unicode( folders.first() ) ) ) )
    return ( unicode( folders.first() ), unicode( fileDialog.encoding() ) )

# Return field type from it's name
def getFieldType(vlayer, fieldName):
    fields = vlayer.dataProvider().fields()
    for name, field in fields.iteritems():
        if field.name() == fieldName:
            return field.typeName()

# return the number of unique values in field
def getUniqueValuesCount( vlayer, fieldIndex, useSelection ):
    vprovider = vlayer.dataProvider()
    allAttrs = vprovider.attributeIndexes()
    vprovider.select( allAttrs )
    count = 0
    values = []
    if useSelection:
        selection = vlayer.selectedFeatures()
        for f in selection:
            if f.attributeMap()[ fieldIndex ].toString() not in values:
                values.append( f.attributeMap()[ fieldIndex ].toString() )
                count += 1
    else:
        feat = QgsFeature()
        while vprovider.nextFeature( feat ):
            if feat.attributeMap()[ fieldIndex ].toString() not in values:
                values.append( feat.attributeMap()[ fieldIndex ].toString() )
                count += 1
    return count
