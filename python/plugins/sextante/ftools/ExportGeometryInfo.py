from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

class ExportGeometryInfo(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/export_geometry.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(ExportGeometryInfo.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(ExportGeometryInfo.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        ( fields, index1, index2 ) = self.checkGeometryFields(vlayer)
        writer = QgsVectorFileWriter( output, systemEncoding,fields, vprovider.geometryType(), vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int(nElement/nFeat * 100))
          nElement += 1
          inGeom = inFeat.geometry()
          ( attr1, attr2 ) = self.simpleMeasure( inGeom )
          outFeat.setGeometry( inGeom )
          atMap = inFeat.attributeMap()
          outFeat.setAttributeMap( atMap )
          outFeat.addAttribute( index1, QVariant( attr1 ) )
          outFeat.addAttribute( index2, QVariant( attr2 ) )
          writer.addFeature( outFeat )
        del writer


    def simpleMeasure( self, inGeom ):
        if inGeom.wkbType() in (QGis.WKBPoint, QGis.WKBPoint25D):
          pt = QgsPoint()
          pt = inGeom.asPoint()
          attr1 = pt.x()
          attr2 = pt.y()
        elif inGeom.wkbType() in (QGis.WKBMultiPoint, QGis.WKBMultiPoint25D):
          pt = inGeom.asMultiPoint()
          attr1 = pt[ 0 ].x()
          attr2 = pt[ 0 ].y()
        else:
          measure = QgsDistanceArea()
          attr1 = measure.measure(inGeom)
          if inGeom.type() == QGis.Polygon:
            attr2 = self.perimMeasure( inGeom, measure )
          else:
            attr2 = attr1
        return ( attr1, attr2 )

    def perimMeasure( self, inGeom, measure ):
        value = 0.00
        if inGeom.isMultipart():
          poly = inGeom.asMultiPolygon()
          for k in poly:
            for j in k:
              value = value + measure.measureLine( j )
        else:
          poly = inGeom.asPolygon()
          for k in poly:
            value = value + measure.measureLine( k )
        return value

    def checkForField( self, L, e ):
        e = QString( e ).toLower()
        fieldRange = range( 0,len( L ) )
        for item in fieldRange:
          if L[ item ].toLower() == e:
            return True, item
        return False, len( L )

    def checkGeometryFields( self, vlayer ):
        vprovider = vlayer.dataProvider()
        nameList = []
        fieldList = vprovider.fields()
        geomType = vlayer.geometryType()
        for i in fieldList.keys():
          nameList.append( fieldList[ i ].name().toLower() )
        if geomType == QGis.Polygon:
          plp = "Poly"
          ( found, index1 ) = self.checkForField( nameList, "AREA" )
          if not found:
            field = QgsField( "AREA", QVariant.Double, "double", 21, 6, "Polygon area" )
            index1 = len( fieldList.keys() )
            fieldList[ index1 ] = field
          ( found, index2 ) = self.checkForField( nameList, "PERIMETER" )

          if not found:
            field = QgsField( "PERIMETER", QVariant.Double, "double", 21, 6, "Polygon perimeter" )
            index2 = len( fieldList.keys() )
            fieldList[ index2 ] = field
        elif geomType == QGis.Line:
          plp = "Line"
          (found, index1) = self.checkForField(nameList, "LENGTH")
          if not found:
            field = QgsField("LENGTH", QVariant.Double, "double", 21, 6, "Line length" )
            index1 = len(fieldList.keys())
            fieldList[index1] = field
          index2 = index1
        else:
          plp = "Point"
          (found, index1) = self.checkForField(nameList, "XCOORD")
          if not found:
            field = QgsField("XCOORD", QVariant.Double, "double", 21, 6, "Point x coordinate" )
            index1 = len(fieldList.keys())
            fieldList[index1] = field
          (found, index2) = self.checkForField(nameList, "YCOORD")
          if not found:
            field = QgsField("YCOORD", QVariant.Double, "double", 21, 6, "Point y coordinate" )
            index2 = len(fieldList.keys())
            fieldList[index2] = field
        return (fieldList, index1, index2)


    def defineCharacteristics(self):
        self.name = "Export/Add geometry columns"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(ExportGeometryInfo.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(ExportGeometryInfo.OUTPUT, "Output layer"))
    #=========================================================
