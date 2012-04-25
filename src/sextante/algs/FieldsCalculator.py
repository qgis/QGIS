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
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException


class FieldsCalculator(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    FORMULA = "FORMULA"
    OUTPUT_LAYER ="OUTPUT_LAYER"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def defineCharacteristics(self):
        self.name = "Field calculator"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterString(self.FIELD_NAME, "Result field name"))
        self.addParameter(ParameterString(self.FORMULA, "Formula"))
        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer", True))

    def processAlgorithm(self, progress):
        inputFilename = self.getParameterValue(self.INPUT_LAYER)
        formula = self.getParameterValue(self.FORMULA)
        layer = QGisLayers.getObjectFromUri(inputFilename)
        provider = layer.dataProvider()
        caps = provider.capabilities()
        if caps & QgsVectorDataProvider.AddAttributes:
            fieldName = self.getParameterValue(self.FIELD_NAME)
            layer.dataProvider().addAttributes([QgsField(fieldName, QVariant.Double)])
            feat = QgsFeature()
            allAttrs = provider.attributeIndexes()
            provider.select(allAttrs)
            fields = provider.fields()
            while provider.nextFeature(feat):
                attrs = feat.attributeMap()
                expression = formula
                for (k,attr) in attrs.iteritems():
                    expression = expression.replace(str(fields[k].name()), str(attr.toString()))
                try:
                    result = eval(expression)
                except Exception:
                    raise GeoAlgorithmExecutionException("Problem evaluation formula: Wrong field values or formula")
                attrs[len(attrs) - 1] = QVariant(result)
                provider.changeAttributeValues({feat.id() : attrs})


    def checkParameterValuesBeforeExecuting(self):
        ##TODO check that formula is correct and fields exist
        pass

