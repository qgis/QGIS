from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.core.SextanteLog import SextanteLog

class SimplifyGeometries(GeoAlgorithm):

    TOLERANCE = "TOLERANCE"
    USE_SELECTION = "USE_SELECTION"
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/simplify.png")

    def processAlgorithm(self, progress):
        self.processedFeatures = 0
        self.progress = progress
        tolerance =self.getParameterValue(SimplifyGeometries.TOLERANCE)
        useSelection = self.getParameterValue(SimplifyGeometries.USE_SELECTION)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(SimplifyGeometries.INPUT))
        self.generalize(vlayer, useSelection, tolerance)


    def defineCharacteristics(self):
        self.name = "Simplify geometries"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(SimplifyGeometries.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterNumber(SimplifyGeometries.TOLERANCE, "Tolerance", 0, 10000000, 1))
        self.addParameter(ParameterBoolean(SimplifyGeometries.USE_SELECTION, "Use only selected features"))
        self.addOutput(OutputVector(SimplifyGeometries.OUTPUT, "Simplified layer"))


    def geomVertexCount(self, geometry):
        geomType = geometry.type()
        if geomType == 1: # line
            points = geometry.asPolyline()
            return len( points )
        elif geomType == 2: # polygon
            polylines = geometry.asPolygon()
            points = []
            for l in polylines:
                points.extend( l )
            return len( points )
        else:
            return None

    def generalize( self, inputLayer, useSelection, tolerance):
        self.inputLayer = inputLayer
        self.useSelection = useSelection
        self.tolerance = tolerance
        self.pointsBefore = 0
        self.pointsAfter = 0
        vProvider = self.inputLayer.dataProvider()
        allAttrs = vProvider.attributeIndexes()
        vProvider.select( allAttrs )
        shapeFields = vProvider.fields()
        crs = vProvider.crs()
        wkbType = self.inputLayer.wkbType()
        if not crs.isValid():
            crs = None
        shapeFileWriter = self.getOutputFromName(SimplifyGeometries.OUTPUT).getVectorWriter(shapeFields, wkbType, crs )
        featureId = 0
        if self.useSelection:
            selection = self.inputLayer.selectedFeatures()
            self.maxRange = len( selection )
            for f in selection:
              featGeometry = QgsGeometry( f.geometry() )
              attrMap = f.attributeMap()
              self.pointsBefore += geomVertexCount( featGeometry )
              newGeometry = featGeometry.simplify( self.tolerance )
              self.pointsAfter += geomVertexCount( newGeometry )
              feature = QgsFeature()
              feature.setGeometry( newGeometry )
              feature.setAttributeMap( attrMap )
              shapeFileWriter.addFeature( feature )
              featureId += 1
              self.emit( SIGNAL( "featureProcessed()" ) )
        else:
            self.maxRange =  vProvider.featureCount()
            f = QgsFeature()
            while vProvider.nextFeature( f ):
              featGeometry = QgsGeometry( f.geometry() )
              attrMap = f.attributeMap()
              self.pointsBefore += self.geomVertexCount( featGeometry )
              newGeometry = featGeometry.simplify( self.tolerance )
              self.pointsAfter += self.geomVertexCount( newGeometry )
              feature = QgsFeature()
              feature.setGeometry( newGeometry )
              feature.setAttributeMap( attrMap )
              shapeFileWriter.addFeature( feature )
              featureId += 1
              self.progress.setPercentage(self.processedFeatures/self.maxRange * 100)

        if shapeFileWriter != None:
            del shapeFileWriter

        SextanteLog.addToLog(SextanteLog.LOG_INFO, "Simplify: Input geometries have been simplified from"
                             + str(self.pointsBefore) + " to "  + str(self.pointsAfter) + " points.")

