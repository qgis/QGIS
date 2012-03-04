from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector

class Centroids(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/centroids.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(Centroids.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(Centroids.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        writer = QgsVectorFileWriter( output, systemEncoding, fields, QGis.WKBPoint, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature( inFeat ):
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
          inGeom = inFeat.geometry()
          atMap = inFeat.attributeMap()
          outGeom = QgsGeometry(inGeom.centroid())
          if outGeom is None:
            raise GeoAlgorithmExecutionException("Error calculating centroid")
          outFeat.setAttributeMap( atMap )
          outFeat.setGeometry( outGeom )
          writer.addFeature( outFeat )
        del writer

    def defineCharacteristics(self):
        self.name = "Centroids"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(Centroids.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addOutput(OutputVector(Centroids.OUTPUT, "Output layer"))
    #=========================================================
