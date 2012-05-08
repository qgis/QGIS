from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector

class Explode(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputValue(self.OUTPUT)
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        writer = QgsVectorFileWriter( output, systemEncoding,
            fields, QGis.WKBLineString, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature( inFeat ):
            nElement += 1
            progress.setPercentage((nElement*100)/nFeat)
            inGeom = inFeat.geometry()
            atMap = inFeat.attributeMap()
            segments = self.extractAsSingleSegments( inGeom )
            outFeat.setAttributeMap( atMap )
            for segment in segments:
                outFeat.setGeometry(segment)
                writer.addFeature(outFeat)
        del writer


    def extractAsSingleSegments( self, geom ):
        segments = []
        if geom.isMultipart():
            multi = geom.asMultiPolyline()
            for polyline in multi:
                segments.extend( self.getPolylineAsSingleSegments(polyline))
        else:
            segments.extend( self.getPolylineAsSingleSegments(geom.asPolyline()))
        return segments

    def getPolylineAsSingleSegments(self, polyline):
        segments = []
        for i in range(len(polyline)-1):
            ptA = polyline[i]
            ptB = polyline[i+1]
            segment = QgsGeometry.fromPolyline([ptA, ptB])
            segments.append(segment)
        return segments

    def defineCharacteristics(self):
        self.name = "Explode lines"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT, "Input layer",ParameterVector.VECTOR_TYPE_LINE))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

