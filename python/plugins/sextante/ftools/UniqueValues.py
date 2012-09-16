import os.path

from PyQt4 import QtGui

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField

from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputNumber import OutputNumber

from sextante.ftools import ftools_utils

class UniqueValues(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    TOTAL_VALUES = "TOTAL_VALUES"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/unique.png")

    def defineCharacteristics(self):
        self.name = "List unique values"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(UniqueValues.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(UniqueValues.FIELD_NAME, "Targer field", UniqueValues.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addOutput(OutputHTML(UniqueValues.OUTPUT, "Unique values"))
        self.addOutput(OutputNumber(UniqueValues.TOTAL_VALUES, "Total unique values"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(UniqueValues.INPUT_LAYER))
        fieldName = self.getParameterValue(UniqueValues.FIELD_NAME)

        outputFile = self.getOutputValue(UniqueValues.OUTPUT)

        values = layer.uniqueValues(layer.fieldNameIndex(fieldName))
        self.createHTML(outputFile, values)
        self.setOutputValue(UniqueValues.TOTAL_VALUES, len(values))

    def createHTML(self, outputFile, algData):
        f = open(outputFile, "w")
        f.write("<p>Total unique values: " + str(len(algData)) + "</p>")
        f.write("<p>Unique values:</p>")
        f.write("<ul>")
        for s in algData:
            f.write("<li>" + unicode(s.toString()) + "</li>")
        f.write("</ul>")
        f.close()
