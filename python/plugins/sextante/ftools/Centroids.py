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

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))

        outFileName = self.getOutputValue(self.OUTPUT_LAYER)

        settings = QSettings()
        encoding = settings.value( "/UI/encoding", "System" ).toString()

        writer = self.getOutputFromName(self.OUTPUT_LAYER).getVectorWriter(layer.pendingFields(),
                     QGis.WKBPoint, layer.dataProvider().crs())

        layer.select(layer.pendingAllAttributesList())

        inFeat = QgsFeature()
        outFeat = QgsFeature()

        total = 100.0 / float(layer.featureCount())
        current = 0

        while layer.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            attrMap = inFeat.attributeMap()

            outGeom = QgsGeometry(inGeom.centroid())
            if outGeom is None:
                raise GeoAlgorithmExecutionException("Error calculating centroid")

            outFeat.setGeometry(outGeom)
            outFeat.setAttributeMap(attrMap)
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(int(current * total))

        del writer
