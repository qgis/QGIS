import os.path
import codecs

from PyQt4 import QtGui

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField

from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputNumber import OutputNumber

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
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.FIELD_NAME, "Targer field", self.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addOutput(OutputHTML(self.OUTPUT, "Unique values"))
        self.addOutput(OutputNumber(self.TOTAL_VALUES, "Total unique values"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)

        outputFile = self.getOutputValue(self.OUTPUT)

        values = layer.uniqueValues(layer.fieldNameIndex(fieldName))
        self.createHTML(outputFile, values)
        self.setOutputValue(self.TOTAL_VALUES, len(values))

    def createHTML(self, outputFile, algData):
        f = codecs.open(outputFile, "w", encoding="utf-8")
        f.write('<html><head>')
        f.write('<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head><body>')
        f.write("<p>Total unique values: " + str(len(algData)) + "</p>")
        f.write("<p>Unique values:</p>")
        f.write("<ul>")
        for s in algData:
            f.write("<li>" + unicode(s.toString()) + "</li>")
        f.write("</ul></body></html>")
        f.close()
