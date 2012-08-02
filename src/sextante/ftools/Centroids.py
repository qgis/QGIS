import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from qgis.core import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterVector import ParameterVector
from sextante.outputs.OutputVector import OutputVector

class Centroids(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    OUTPUT_LAYER = "OUTPUT_LAYER"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/centroids.png")

    def defineCharacteristics(self):
        self.name = "Polygon centroids"
        self.group = "Geometry tools"

        self.addParameter(ParameterVector(Centroids.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))

        self.addOutput(OutputVector(Centroids.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(Centroids.INPUT_LAYER))

        outFileName = self.getOutputValue(Centroids.OUTPUT_LAYER)

        provider = layer.dataProvider()

        settings = QSettings()
        encoding = settings.value( "/UI/encoding", "System" ).toString()

        writer = QgsVectorFileWriter(outFileName, encoding, provider.fields(),
                                     QGis.WKBPoint, provider.crs())

        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)

        inFeat = QgsFeature()
        outFeat = QgsFeature()
        total = provider.featureCount()
        current = 0
        while provider.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            attrMap = inFeat.attributeMap()

            outGeom = QgsGeometry(inGeom.centroid())
            if outGeom is None:
                raise GeoAlgorithmExecutionException("Error calculating centroid")

            outFeat.setGeometry(outGeom)
            outFeat.setAttributeMap(attrMap)
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(int(current / total * 100))

        del writer
