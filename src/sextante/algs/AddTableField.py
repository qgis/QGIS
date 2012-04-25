from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterVector import ParameterVector
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.QGisLayers import QGisLayers
import os
from PyQt4 import QtGui


class AddTableField(GeoAlgorithm):

    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FIELD_TYPE = "FIELD_TYPE"
    TYPE_NAMES = ["Integer", "Float", "String"]
    TYPES = [QVariant.Int, QVariant.Double, QVariant.String]

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def defineCharacteristics(self):
        self.name = "Add field to attributes table"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Field name"))
        self.addParameter(ParameterSelection(self.FIELD_TYPE, "Field type", self.TYPE_NAMES))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer", True))

    def processAlgorithm(self, progress):
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        layer = QGisLayers.getObjectFromUri(inputFilename)
        caps = layer.dataProvider().capabilities()
        if caps & QgsVectorDataProvider.AddAttributes:
            fieldName = self.getParameterValue(self.FIELD_NAME)
            fieldType = self.TYPES[self.getParameterValue(self.FIELD_TYPE)]
            layer.dataProvider().addAttributes([QgsField(fieldName, fieldType)])


